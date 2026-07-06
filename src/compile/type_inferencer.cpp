#include "compile/type_inferencer.h"

#include <algorithm>
#include <fstream>
#include <optional>
#include <ranges>

#include "compile/cfg.h"
#include "compile/shape_type.h"
#include "compile/ssa.h"
#include "compile/syntax_tree.h"
#include "compile/unified_type_analyzer.h"
#include "fakelua.h"
#include "util/common.h"
#include "util/file_util.h"

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────
// 主入口（SSA 单轨；legacy 字段由 SSA 快照桥接填充）
// ─────────────────────────────────────────────────────────────────────────
InferResult TypeInferencer::InferTypes(const ParseResult &pr, const CompileConfig &cfg) {
    InferResult ir;

    // ── Phase 1: SSA/CFG/Shape 分析 ─────────────────────────────────
    RunSSAAnalysis(pr, ir);
    RunSSASpecialization(pr, ir);

    // ── Phase 2: 兼容桥 —— 从 SSA 快照派生 legacy 字段 ─────────────
    PopulateMainEvalTypesFromSSA(pr.chunk, ir);
    PopulateGlobalConstVarsFromSSA(pr.chunk, ir);
    // PopulateLegacyReturnTypes 已删除：CGen 直接读 spec_ssa_return_types / spec_ssa_snapshots
    PopulateMathParamPositionsFromSSA(ir);
    PopulateTableSpecInfosFromSSA(pr.chunk, ir);
    // ── Phase 3: Flow-sensitive local 类型修复（覆盖 UTA 的 AST-walk
    //    单遍局限）。删除 ⇔ UTA 的流敏感 worklist 实现完成（规范 §6）。
    PopulateLocalFlowSensitiveTypes(pr.chunk, ir);

    // ── Debug dump（legacy 形态以便 diff）────────────────────────
    if (cfg.debug_mode) {
        const auto dumpfile = GenerateTmpFilename("fakelua_infer_", ".txt");
        if (std::ofstream ofs(dumpfile); ofs.is_open()) {
            ofs << "=== main_eval_types (from SSA main_ssa_types) ===\n";
            for (auto &[node, ty] : ir.main_eval_types) {
                ofs << "  node=" << node << " type=" << InferredTypeToString(ty) << "\n";
            }
            ofs.close();
            LOG_INFO("Type inference results generated: {}", dumpfile);
        }
    }

    return ir;
}

// 从 SSA main_ssa_types 桥接填充 legacy main_eval_types（InferredType 形态）
void TypeInferencer::PopulateMainEvalTypesFromSSA(const SyntaxTreeInterfacePtr &/*chunk*/, InferResult &ir) {
    ir.main_eval_types.clear();
    ir.global_const_vars.clear();
    for (const auto &[node, ssa] : ir.main_ssa_types) {
        ir.main_eval_types[const_cast<SyntaxTreeInterface *>(node)] = ssa.type;
    }
}

// 填充文件级数值常量集合（global_const_vars）——从 SSA 快照查找赋值
void TypeInferencer::PopulateGlobalConstVarsFromSSA(const SyntaxTreeInterfacePtr &chunk, InferResult &ir) {
    if (!chunk || chunk->Type() != SyntaxTreeType::Block) return;
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    for (const auto &stmt : block->Stmts()) {
        if (stmt->Type() != SyntaxTreeType::LocalVar) continue;
        const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
        const auto namelist = local_var->Namelist();
        if (!namelist || namelist->Type() != SyntaxTreeType::NameList) continue;
        const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
        auto explist = local_var->Explist()
            ? std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist()) : nullptr;
        const auto &names = namelist_ptr->Names();
        const auto &exps = explist ? explist->Exps() : std::vector<SyntaxTreeInterfacePtr>{};
        for (size_t i = 0; i < names.size(); ++i) {
            InferredType type = T_DYNAMIC;
            if (i < exps.size()) {
                auto it = ir.main_eval_types.find(exps[i].get());
                if (it != ir.main_eval_types.end()) type = it->second;
            }
            ir.global_const_vars[names[i]] = type;
        }
    }
}

// PopulateLegacyReturnTypes 已删除 — CGen 通过 spec_ssa_return_types / spec_ssa_snapshots
// 直接桥接，不再需要 legacy 副本。

// 填充 legacy math_param_positions —— 从 specializable_params 按 is_math 选出参数下标
void TypeInferencer::PopulateMathParamPositionsFromSSA(InferResult &ir) {
    ir.math_param_positions.clear();
    for (auto &[func, params] : ir.specializable_params) {
        std::vector<int> pos;
        for (const auto &sp : params) {
            if (sp.is_math) pos.push_back(sp.param_index);
        }
        if (!pos.empty()) ir.math_param_positions[func] = std::move(pos);
    }
}

// 填充 legacy table_spec_infos —— 从 shape_registry 派生字段列表
void TypeInferencer::PopulateTableSpecInfosFromSSA(const SyntaxTreeInterfacePtr &/*chunk*/, InferResult &ir) {
    ir.table_spec_infos.clear();
    if (!ir.shape_registry) return;
    for (const auto &[node, shape_id] : ir.ctor_target_shapes) {
        if (shape_id < 0) continue;
        const ShapeType &shape = ir.shape_registry->Get(shape_id);
        TableSpecInfo info;
        info.can_specialize = !shape.is_open;
        for (const auto &fd : shape.fields) {
            TableFieldInfo fi;
            fi.key = fd.name;
            fi.type = fd.type;
            fi.optional = fd.optional;
            fi.c_field_name = fd.c_field_name;
            if (fd.is_int_key) {
                fi.key_kind = TableKeyKind::kInt;
                fi.int_value = std::atoll(fd.name.c_str());
            }
            info.fields.push_back(std::move(fi));
        }
        ir.table_spec_infos[const_cast<SyntaxTreeInterface *>(node)] = std::move(info);
    }
}

// 空占位（避免重构时 dangling reference）
void TypeInferencer::AnnotateSimpleConstants(const SyntaxTreeInterfacePtr &/*node*/, InferResult &/*ir*/) {
    // all simple constant population happens in UTA.WalkSyntaxTree via main_ssa_types
}

// ── Flow-sensitive local 类型修复 ──────────────────────────────────────────
//
// 扫描每个 block（顶层 + 每个函数 block），找出所有 local 声明：
//   local x = e        → 初始类型 T_init = main_eval_types[e]
//   local x, y = e1, e2 → 每个 name 独立
//   for x = a, b, c do  → for-loop 游标 x 视作 local
// 如果同一块后续（非嵌套 shadowable 作用域内）对同名变量执行赋值，
// 且被赋值类型与当前类型不同（或类型是 dynamic/字符串/table），则退化为 T_DYNAMIC。
//
// 这是 legacy walker 的简化替代，仅处理 "同一变量跨赋值被不同类型覆盖" 的退化；
// 完整流敏感分析由 §6 Worklist 算法接管后此 pass 可删除。

namespace {

using CurTypeMap = std::unordered_map<std::string, InferredType>;

// type-safe meet: 不同类型 → T_DYNAMIC
inline InferredType MeetFlow(InferredType a, InferredType b) {
    if (a == b) return a;
    using fakelua::T_DYNAMIC; using fakelua::T_NIL; using fakelua::T_BOOL;
    using fakelua::T_INT; using fakelua::T_FLOAT; using fakelua::T_STRING;
    using fakelua::T_RECORD; using fakelua::T_RECORD_OPEN;
    // scalar + scalar: promote / degrade
    if ((a == T_INT && b == T_FLOAT) || (a == T_FLOAT && b == T_INT)) return T_FLOAT;
    // nil / anything → keep other (no change)
    if (a == T_NIL) return b;
    if (b == T_NIL) return a;
    // record + scalar / record + record mismatch → dynamic
    return T_DYNAMIC;
}

// 给定一个 SyntaxTreeExp 节点，返回它的 InferredType（可能是 literal）。
// 对 Binop/Unop/PrefixExp(Var) 递归解析操作数，当两侧均为已知数值时推导结果类型。
// cur_type 提供变量名 → 当前推断类型的查找，以支持代数表达式类型传播。
//
// 重要：对于 Binop/Unop/PrefixExp(Var) 语法模式，递归推导优先级高于 map 查找，
// 因为 SSA 阶段由于缺少完整的 worklist 推导不会传播变量类型信息，导致 map 中
// 存的是 T_DYNAMIC；而 PopulateLocalFlowSensitiveTypes 已经在 cur_type 中建立了
// 正确的变量类型上下文。
InferredType TypeOfScalar(const SyntaxTreeInterfacePtr &exp,
                          fakelua::EvalTypeSnapshot &map,
                          const CurTypeMap &cur_type) {
    if (!exp) return {T_DYNAMIC};
    // 对语法模式做先验递归解析（不依赖 map 缓存）
    if (exp->Type() == fakelua::SyntaxTreeType::Exp) {
        auto e = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(exp);
        auto k = e->GetExpKind();
        if (k == fakelua::ExpKind::kBinop && e->Left() && e->Right()) {
            auto lt = TypeOfScalar(e->Left(), map, cur_type);
            auto rt = TypeOfScalar(e->Right(), map, cur_type);
            if (fakelua::IsNumericInferredType(lt) && fakelua::IsNumericInferredType(rt)) {
                return MeetFlow(lt, rt);
            }
        }
        if (k == fakelua::ExpKind::kUnop && e->Right()) {
            auto inner = TypeOfScalar(e->Right(), map, cur_type);
            if (fakelua::IsNumericInferredType(inner)) return inner;
        }
        if (k == fakelua::ExpKind::kPrefixExp) {
            auto pe = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(e->Right());
            if (pe && pe->GetPrefixKind() == fakelua::PrefixExpKind::kVar) {
                auto v = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>(pe->GetValue());
                if (v && v->GetVarKind() == fakelua::VarKind::kSimple) {
                    auto vit = cur_type.find(v->GetName());
                    if (vit != cur_type.end() && vit->second != fakelua::T_UNKNOWN) {
                        return vit->second;
                    }
                }
            }
        }
    }
    // map 缓存：存放字面量等编译期已知的类型
    auto it = map.find(exp.get());
    if (it != map.end()) return it->second;
    // 兜底字面量识别
    if (exp->Type() == fakelua::SyntaxTreeType::Exp) {
        auto e = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(exp);
        auto k = e->GetExpKind();
        if (k == fakelua::ExpKind::kNumber) {
            const auto &v = e->ExpValue();
            if (v.find('.') == std::string::npos && v.find('e') == std::string::npos && v.find('E') == std::string::npos)
                return fakelua::T_INT;
            return fakelua::T_FLOAT;
        }
        if (k == fakelua::ExpKind::kString || k == fakelua::ExpKind::kNil) return {T_DYNAMIC};
    }
    return {T_DYNAMIC};
}

// 处理单个 block 的 locals
void ProcessBlockLocals(const fakelua::SyntaxTreeInterfacePtr &node,
                        fakelua::EvalTypeSnapshot &map) {
    if (!node) return;
    auto ntype = node->Type();
    if (ntype == fakelua::SyntaxTreeType::FuncBody) {
        auto fb = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>(node);
        ProcessBlockLocals(fb->Block(), map);
        return;
    }
    if (ntype == fakelua::SyntaxTreeType::LocalFunction) {
        auto lf = std::dynamic_pointer_cast<fakelua::SyntaxTreeLocalFunction>(node);
        ProcessBlockLocals(lf->Funcbody(), map);
        return;
    }
    if (ntype != fakelua::SyntaxTreeType::Block) return;

    auto blk = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(node);

    // block_local_types: 变量名 → 当前推断类型
    std::unordered_map<std::string, InferredType> cur_type;
    // 变量名 → 其声明 local 语句指针（用于后续清理 main_eval_types）
    std::unordered_map<std::string, fakelua::SyntaxTreeInterfacePtr> decl_node_by_name;

    for (const auto &stmt : blk->Stmts()) {
        if (!stmt) continue;
        auto st = stmt->Type();

        if (st == fakelua::SyntaxTreeType::LocalVar) {
            auto lv = std::dynamic_pointer_cast<fakelua::SyntaxTreeLocalVar>(stmt);
            auto nl = lv->Namelist();
            if (!nl || nl->Type() != fakelua::SyntaxTreeType::NameList) continue;
            auto nlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(nl);
            auto el = lv->Explist() ? std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(lv->Explist()) : nullptr;
            const auto &names = nlist->Names();
            const auto &exps = el ? el->Exps() : std::vector<fakelua::SyntaxTreeInterfacePtr>{};
            for (size_t i = 0; i < names.size(); ++i) {
                InferredType t = T_DYNAMIC;
                if (i < exps.size()) {
                    t = TypeOfScalar(exps[i], map, cur_type);
                }
                // 退化：如果变量已存在，meet 两者的类型
                auto it = cur_type.find(names[i]);
                if (it != cur_type.end()) {
                    t = MeetFlow(it->second, t);
                }
                cur_type[names[i]] = t;
                if (i < exps.size()) decl_node_by_name[names[i]] = exps[i];
                // 更新 map 中初始表达式对应的类型（SSA 阶段可能因缺少 worklist
                // 推导而错误填为 T_DYNAMIC；我们用 cur_type 上下文重新解析后写回）
                if (i < exps.size() && exps[i]) {
                    if (t == T_INT || t == T_FLOAT) {
                        map[exps[i].get()] = t;
                    }
                }
            }
        } else if (st == fakelua::SyntaxTreeType::Function || st == fakelua::SyntaxTreeType::LocalFunction) {
            // 递归到函数体
            auto fb = (st == fakelua::SyntaxTreeType::Function)
                ? std::dynamic_pointer_cast<fakelua::SyntaxTreeFunction>(stmt)->Funcbody()
                : std::dynamic_pointer_cast<fakelua::SyntaxTreeLocalFunction>(stmt)->Funcbody();
            if (fb) ProcessBlockLocals(fb, map);
            continue;
        } else if (st == fakelua::SyntaxTreeType::Assign) {
            auto as = std::dynamic_pointer_cast<fakelua::SyntaxTreeAssign>(stmt);
            auto vl = std::dynamic_pointer_cast<fakelua::SyntaxTreeVarlist>(as->Varlist());
            auto el = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(as->Explist());
            if (!vl || !el) continue;
            const auto &vars = vl->Vars();
            const auto &exps = el->Exps();
            for (size_t i = 0; i < vars.size() && i < exps.size(); ++i) {
                if (vars[i]->Type() != fakelua::SyntaxTreeType::Var) continue;
                auto v = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>(vars[i]);
                if (v->GetVarKind() != fakelua::VarKind::kSimple) continue;
                const auto &vn = v->GetName();
                auto rt = TypeOfScalar(exps[i], map, cur_type);
                auto it = cur_type.find(vn);
                if (it == cur_type.end()) {
                    cur_type[vn] = rt;
                } else {
                    auto m = MeetFlow(it->second, rt);
                    if (m != it->second) {
                        it->second = m;
                        auto dit = decl_node_by_name.find(vn);
                        if (dit != decl_node_by_name.end()) map[dit->second.get()] = T_DYNAMIC;
                    }
                }
                // 更新 map 中 RHS 表达式对应的类型，便于后续 CGen 类型查询
                if (rt == T_INT || rt == T_FLOAT) {
                    map[exps[i].get()] = rt;
                } else if (cur_type[vn] == T_DYNAMIC) {
                    map[exps[i].get()] = T_DYNAMIC;
                }
            }
        } else if (st == fakelua::SyntaxTreeType::FunctionCall) {
            // 函数调用可能修改任意变量 → 对 cur_type 当前保留的每个变量执行 meet-with-dynamic
            // 简化：不清接管 — 调用后的退化由 JIT 的运行时检查兜底（entry dispatcher 已有断言）
            (void)0;
        } else if (st == fakelua::SyntaxTreeType::ForLoop) {
            auto fl = std::dynamic_pointer_cast<fakelua::SyntaxTreeForLoop>(stmt);
            const auto &cname = fl->Name();
            // Save outer state of cursor variable; shadow it in inner block
            // so that inner mutations don't bleed back to the outer scope
            // unless the outer variable itself is assigned inside the loop.
            // (For-loop cursors are new locals scoped to the body.)
            std::optional<InferredType> saved_outer = std::nullopt;
            auto it_cur = cur_type.find(cname);
            if (it_cur != cur_type.end()) { saved_outer = it_cur->second; cur_type.erase(cname); }
            std::unordered_map<std::string, fakelua::SyntaxTreeInterfacePtr> saved_decls;
            auto dit = decl_node_by_name.find(cname);
            if (dit != decl_node_by_name.end()) { saved_decls[cname] = dit->second; decl_node_by_name.erase(cname); }
            ProcessBlockLocals(fl->Block(), map);
            // Detect whether the for-loop body reassigns the cursor to a non-numeric
            // value (case2).  cur_type mutations inside recursive ProcessBlockLocals
            // do not propagate back, so we walk the body manually to look for
            // direct `cname = <non-numeric>` Assign statements.
            InferredType cursor_type = T_INT;
            if (fl->Block()) {
                auto body_blk = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(fl->Block());
                if (body_blk) {
                    for (const auto &body_stmt : body_blk->Stmts()) {
                        if (!body_stmt || body_stmt->Type() != fakelua::SyntaxTreeType::Assign) continue;
                        auto ba = std::dynamic_pointer_cast<fakelua::SyntaxTreeAssign>(body_stmt);
                        auto bvl = ba->Varlist() ? std::dynamic_pointer_cast<fakelua::SyntaxTreeVarlist>(ba->Varlist()) : nullptr;
                        auto bel = ba->Explist() ? std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(ba->Explist()) : nullptr;
                        if (!bvl || !bel) continue;
                        auto &bvars = bvl->Vars();
                        auto &bexps = bel->Exps();
                        for (size_t k = 0; k < bvars.size() && k < bexps.size(); ++k) {
                            if (bvars[k]->Type() != fakelua::SyntaxTreeType::Var) continue;
                            auto bv = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>(bvars[k]);
                            if (!bv || bv->GetVarKind() != fakelua::VarKind::kSimple) continue;
                            if (bv->GetName() != cname) continue;
                            auto rt = TypeOfScalar(bexps[k], map, cur_type);
                            if (!IsNumericInferredType(rt)) {
                                cursor_type = T_DYNAMIC;
                                break;
                            }
                        }
                        if (cursor_type == T_DYNAMIC) break;
                    }
                }
            }
            map[stmt.get()] = cursor_type;
            // Restore outer state (unchanged: cursor is fresh in inner block)
            if (saved_outer) cur_type[cname] = *saved_outer;
            else cur_type.erase(cname);
            for (auto &kv : saved_decls) decl_node_by_name[kv.first] = kv.second;
            continue;
        } else if (st == fakelua::SyntaxTreeType::ForIn) {
            auto fi = std::dynamic_pointer_cast<fakelua::SyntaxTreeForIn>(stmt);
            ProcessBlockLocals(fi->Block(), map);
        } else if (st == fakelua::SyntaxTreeType::If) {
            auto ifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeIf>(stmt);
            ProcessBlockLocals(ifs->Block(), map);
            if (auto elseifs = ifs->ElseIfs()) {
                auto el = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>(elseifs);
                for (size_t i = 0; i < el->ElseifSize(); ++i)
                    ProcessBlockLocals(el->ElseifBlock(i), map);
            }
            if (auto eb = ifs->ElseBlock()) ProcessBlockLocals(eb, map);
        } else if (st == fakelua::SyntaxTreeType::While) {
            auto ws = std::dynamic_pointer_cast<fakelua::SyntaxTreeWhile>(stmt);
            ProcessBlockLocals(ws->Block(), map);
        } else if (st == fakelua::SyntaxTreeType::Repeat) {
            auto rs = std::dynamic_pointer_cast<fakelua::SyntaxTreeRepeat>(stmt);
            ProcessBlockLocals(rs->Block(), map);
        }
    }
}
}  // namespace

void TypeInferencer::PopulateLocalFlowSensitiveTypes(const SyntaxTreeInterfacePtr &chunk, InferResult &ir) {
    ProcessBlockLocals(chunk, ir.main_eval_types);
}

// ─────────────────────────────────────────────────────────────────────────
// SSA 管线入口
// ─────────────────────────────────────────────────────────────────────────
void TypeInferencer::RunSSAAnalysis(const ParseResult &pr, InferResult &ir) {
    if (!ir.shape_registry) {
        ir.shape_registry = std::make_shared<ShapeRegistry>();
    }
    if (!ir.shape_registry) return;  // 防止未被初始化

    UnifiedTypeAnalyzer uta(ir.shape_registry.get());
    CFGBuilder cfg_builder;
    SSABuilder ssa_builder;

    const auto function_infos = CollectFunctionSpecInfos(pr);

    for (const auto &func_info : function_infos) {
        CFGFunction cfg = cfg_builder.Build(func_info.block, func_info.params, func_info.name, /*is_vararg=*/false);
        SSAFunction ssa = ssa_builder.Build(cfg);

        uta.Analyze(func_info.name, func_info.block, cfg, ssa, ir, /*bitmask=*/-1);
        uta.ComputeCtorTargetShapes(func_info.block, ssa, ir);

        auto spec_params = uta.FindSpecializableParams(func_info.block, cfg, ssa, ir);
        if (!spec_params.empty()) {
            ir.specializable_params[func_info.name] = std::move(spec_params);
        }
    }

    // 顶层 chunk
    {
        const std::string init_name = kInitFunctionName;
        std::vector<std::string> empty_params;
        CFGFunction cfg = cfg_builder.Build(pr.chunk, empty_params, init_name, /*is_vararg=*/false);
        SSAFunction ssa = ssa_builder.Build(cfg);
        uta.Analyze(init_name, pr.chunk, cfg, ssa, ir, /*bitmask=*/-1);
    }
}

// ─────────────────────────────────────────────────────────────────────────
// 特化版本的不动点迭代
// ─────────────────────────────────────────────────────────────────────────
void TypeInferencer::RunSSASpecialization(const ParseResult &pr, InferResult &ir) {
    if (!ir.shape_registry) return;

    UnifiedTypeAnalyzer uta(ir.shape_registry.get());
    CFGBuilder cfg_builder;
    SSABuilder ssa_builder;

    const auto function_infos = CollectFunctionSpecInfos(pr);

    for (const auto &func_info : function_infos) {
        auto it = ir.specializable_params.find(func_info.name);
        if (it == ir.specializable_params.end() || it->second.empty()) continue;

        const auto &spec_params = it->second;
        int num_math = 0;
        for (const auto &sp : spec_params) {
            if (sp.is_math) num_math++;
        }
        if (num_math == 0) continue;

        int num_specs = 1 << num_math;

        CFGFunction cfg = cfg_builder.Build(func_info.block, func_info.params, func_info.name, /*is_vararg=*/false);
        SSAFunction ssa = ssa_builder.Build(cfg);

        auto &rets = ir.spec_ssa_return_types[func_info.name];
        rets.assign(num_specs, SSATypeInfo{T_INT, -1});

        for (int pass = 0; pass < 10; ++pass) {
            bool changed = false;

            for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
                UnifiedTypeAnalyzer::ParamAssumption assumptions;
                for (const auto &sp : spec_params) {
                    if (!sp.is_math) continue;
                    int param_ver = ssa.param_versions[sp.param_index];
                    if (param_ver >= 0) {
                        int idx = &sp - &spec_params[0];  // 相对位置
                        bool is_float = (bitmask & (1 << idx)) != 0;
                        assumptions[param_ver] = SSATypeInfo{is_float ? T_FLOAT : T_INT, -1};
                    }
                }

                uta.Analyze(func_info.name, func_info.block, cfg, ssa, ir, bitmask, assumptions);
            }

            if (!changed) break;
        }

        // 推导每个特化版本的返回类型（基于快照中 return 表达式的类型；
        // 当快照全为 T_DYNAMIC 时，回退到 param_assumptions 直接代入计算）
        auto &snaps = ir.spec_ssa_snapshots[func_info.name];
        for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
            if ((size_t)bitmask >= snaps.size()) break;
            InferredType merged_ret = inferReturnTypeFromSnaps(func_info.block, snaps[(size_t)bitmask]);
            if (!IsNumericInferredType(merged_ret)) {
                // 回退：直接用 param_assumptions 推导
                merged_ret = inferRetTypeFromAssumptions(func_info.block, func_info.params,
                                                          spec_params, bitmask);
            }
            rets[(size_t)bitmask].type = merged_ret;
        }
    }
}

// 把参数类型代入 return 表达式做类型推导：沿 AST 递归，变量引用替换为假设类型
static InferredType inferRetTypeFromAssumptionsImpl(
    const SyntaxTreeInterfacePtr &exp,
    const std::unordered_map<std::string, InferredType> &var_types);

static InferredType inferRetTypeFromAssumptionsImpl(const SyntaxTreeInterfacePtr &exp,
                                                     const std::unordered_map<std::string, InferredType> &var_types) {
    if (!exp) return {T_DYNAMIC};
    if (exp->Type() == SyntaxTreeType::PrefixExp) {
        auto *pe = static_cast<SyntaxTreePrefixexp *>(exp.get());
        if (pe->GetPrefixKind() == PrefixExpKind::kExp && pe->GetValue()) {
            return inferRetTypeFromAssumptionsImpl(pe->GetValue(), var_types);
        }
        if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
            return inferRetTypeFromAssumptionsImpl(pe->GetValue(), var_types);
        }
        return {T_DYNAMIC};
    }
    if (exp->Type() == SyntaxTreeType::Var) {
        auto *v = static_cast<SyntaxTreeVar *>(exp.get());
        if (v->GetVarKind() == VarKind::kSimple) {
            auto it = var_types.find(v->GetName());
            if (it != var_types.end()) return it->second;
        }
        return {T_DYNAMIC};
    }
    if (exp->Type() == SyntaxTreeType::PrefixExp) {
        auto *pe = static_cast<SyntaxTreePrefixexp *>(exp.get());
        if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
            return inferRetTypeFromAssumptionsImpl(pe->GetValue(), var_types);
        }
        return {T_DYNAMIC};
    }
    if (exp->Type() != SyntaxTreeType::Exp) return {T_DYNAMIC};
    auto *e = static_cast<SyntaxTreeExp *>(exp.get());
    auto k = e->GetExpKind();
    if (k == ExpKind::kNumber) {
        const auto &v = e->ExpValue();
        if (v.find('.') == std::string::npos && v.find('e') == std::string::npos && v.find('E') == std::string::npos)
            return T_INT;
        return T_FLOAT;
    }
    if (k == ExpKind::kString || k == ExpKind::kNil) return {T_DYNAMIC};
    if (k == ExpKind::kPrefixExp) {
        if (e->Right()) return inferRetTypeFromAssumptionsImpl(e->Right(), var_types);
        return {T_DYNAMIC};
    }
    if (k == ExpKind::kBinop && e->Left() && e->Right()) {
        auto lt = inferRetTypeFromAssumptionsImpl(e->Left(), var_types);
        auto rt = inferRetTypeFromAssumptionsImpl(e->Right(), var_types);
        auto *bop = e->Op() ? static_cast<SyntaxTreeBinop *>(e->Op().get()) : nullptr;
        // 算术 / 位运算 / 比较：两侧均为数值 → 返回数值类型（比较 → T_INT）
        bool is_arith = bop && (bop->GetOpKind() == BinOpKind::kPlus || bop->GetOpKind() == BinOpKind::kMinus ||
                                bop->GetOpKind() == BinOpKind::kStar || bop->GetOpKind() == BinOpKind::kSlash ||
                                bop->GetOpKind() == BinOpKind::kDoubleSlash || bop->GetOpKind() == BinOpKind::kPow ||
                                bop->GetOpKind() == BinOpKind::kMod || bop->GetOpKind() == BinOpKind::kBitAnd ||
                                bop->GetOpKind() == BinOpKind::kBitOr || bop->GetOpKind() == BinOpKind::kXor ||
                                bop->GetOpKind() == BinOpKind::kLeftShift || bop->GetOpKind() == BinOpKind::kRightShift ||
                                bop->GetOpKind() == BinOpKind::kLess || bop->GetOpKind() == BinOpKind::kLessEqual ||
                                bop->GetOpKind() == BinOpKind::kMore || bop->GetOpKind() == BinOpKind::kMoreEqual ||
                                bop->GetOpKind() == BinOpKind::kEqual || bop->GetOpKind() == BinOpKind::kNotEqual);
        if (is_arith && IsNumericInferredType(lt) && IsNumericInferredType(rt))
            return (lt == T_FLOAT || rt == T_FLOAT || bop->GetOpKind() == BinOpKind::kSlash || bop->GetOpKind() == BinOpKind::kPow) ? T_FLOAT : T_INT;
        // 三元 (cond) and val1 or val2：如果两个分支都数值化，结果数值化
        bool is_and_or = bop && (bop->GetOpKind() == BinOpKind::kAnd || bop->GetOpKind() == BinOpKind::kOr);
        if (is_and_or && IsNumericInferredType(lt) && IsNumericInferredType(rt))
            return (lt == T_FLOAT || rt == T_FLOAT) ? T_FLOAT : T_INT;
        return {T_DYNAMIC};
    }
    if (k == ExpKind::kUnop && e->Right()) {
        auto *unop = e->Op() ? static_cast<SyntaxTreeUnop *>(e->Op().get()) : nullptr;
        if (unop && (unop->GetOpKind() == UnOpKind::kNumberSign || unop->GetOpKind() == UnOpKind::kBitNot)) {
            // #s 和 ~x 的结果永远是 int（即使 operand 类型未知）
            return T_INT;
        }
        auto in = inferRetTypeFromAssumptionsImpl(e->Right(), var_types);
        if (IsNumericInferredType(in)) return in;
        return {T_DYNAMIC};
    }
    return {T_DYNAMIC};
}

// 顶层 helper：给定函数参数表达式 / 假设类型，计算返回类型
InferredType TypeInferencer::inferRetTypeFromAssumptions(
    const SyntaxTreeInterfacePtr &func_block,
    const std::vector<std::string> &func_params,
    const std::vector<SpecParam> &spec_params,
    int bitmask) {

    // 建立参数的假设类型映射
    std::unordered_map<std::string, InferredType> var_types;
    for (const auto &sp : spec_params) {
        if (!sp.is_math) continue;
        int idx = &sp - &spec_params[0];
        bool is_float = (bitmask & (1 << idx)) != 0;
        if (sp.param_index >= 0 && sp.param_index < (int)func_params.size()) {
            var_types[func_params[sp.param_index]] = is_float ? T_FLOAT : T_INT;
        }
    }
    // 收集所有 return 表达式的推导结果，同时做 per-bitmark 简单的 forward propagation
    InferredType result = T_UNKNOWN;

    std::function<void(const SyntaxTreeInterfacePtr &)> walk_stmt;
    std::function<void(const SyntaxTreeInterfacePtr &)> walk_block;

    walk_stmt = [&](const SyntaxTreeInterfacePtr &stmt) {
        if (!stmt) return;
        if (stmt->Type() == SyntaxTreeType::Return) {
            auto ret_node = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);
            if (ret_node && ret_node->Explist()) {
                auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(ret_node->Explist());
                if (el && !el->Exps().empty()) {
                    InferredType t = inferRetTypeFromAssumptionsImpl(el->Exps()[0], var_types);
                    if (result == T_UNKNOWN) result = t;
                    else if (IsNumericInferredType(result) && IsNumericInferredType(t)) {
                        result = (result == T_FLOAT || t == T_FLOAT) ? T_FLOAT : result;
                    } else if (IsNumericInferredType(result) && !IsNumericInferredType(t)) {
                        // 已知数值类型 + 不可推导 → 保持已知类型
                    } else if (!IsNumericInferredType(result) && IsNumericInferredType(t)) {
                        result = t;
                    }
                }
            }
            return;
        }
        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            // local name, name2,... = exp1, exp2,...  — forward-propagate local 类型
            auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            auto nl = lv->Namelist();
            auto el = lv->Explist() ? std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist()) : nullptr;
            if (!nl || nl->Type() != SyntaxTreeType::NameList) return;
            auto nlist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(nl);
            const auto &names = nlist->Names();
            const auto &exps = el ? el->Exps() : std::vector<SyntaxTreeInterfacePtr>{};
            for (size_t i = 0; i < names.size(); ++i) {
                if (i < exps.size()) {
                    InferredType t = inferRetTypeFromAssumptionsImpl(exps[i], var_types);
                    if (IsNumericInferredType(t)) var_types[names[i]] = t;
                    else var_types.erase(names[i]);
                } else {
                    var_types.erase(names[i]);
                }
            }
            return;
        }
        if (stmt->Type() == SyntaxTreeType::Assign) {
            auto as = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
            auto vl = as->Varlist() ? std::dynamic_pointer_cast<SyntaxTreeVarlist>(as->Varlist()) : nullptr;
            auto el = as->Explist() ? std::dynamic_pointer_cast<SyntaxTreeExplist>(as->Explist()) : nullptr;
            if (!vl || !el) return;
            const auto &vars = vl->Vars();
            const auto &exps = el->Exps();
            for (size_t i = 0; i < vars.size() && i < exps.size(); ++i) {
                if (vars[i]->Type() != SyntaxTreeType::Var) continue;
                auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(vars[i]);
                if (v->GetVarKind() != VarKind::kSimple) continue;
                InferredType t = inferRetTypeFromAssumptionsImpl(exps[i], var_types);
                auto it = var_types.find(v->GetName());
                if (IsNumericInferredType(t)) {
                    if (it == var_types.end()) var_types[v->GetName()] = t;
                    else if (IsNumericInferredType(it->second))
                        it->second = (it->second == T_FLOAT || t == T_FLOAT) ? T_FLOAT : it->second;
                } else {
                    var_types.erase(v->GetName());
                }
            }
            return;
        }
        // recurse into nested blocks
        switch (stmt->Type()) {
            case SyntaxTreeType::Block: walk_block(stmt); break;
            case SyntaxTreeType::If: {
                auto ifn = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
                walk_block(ifn->Block());
                if (auto eb = ifn->ElseBlock()) walk_block(eb);
                break;
            }
            default: break;
        }
    };

    walk_block = [&](const SyntaxTreeInterfacePtr &node) {
        if (!node || node->Type() != SyntaxTreeType::Block) return;
        auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
        for (auto &s : blk->Stmts()) walk_stmt(s);
    };
    walk_block(func_block);
    if (result == T_UNKNOWN) result = T_INT;
    return result;
}

// 从特化快照中推导函数特化版本的返回类型
// 当快照全部为 T_DYNAMIC（SSA use_versions 未填充）时，回退到 param_assumptions：
// 通过把参数类型代入 return 表达式 AST 来直接计算。
InferredType TypeInferencer::inferReturnTypeFromSnaps(
    const SyntaxTreeInterfacePtr &func_block,
    const std::unordered_map<const SyntaxTreeInterface*, SSATypeInfo> &snap) {

    // 先尝试从快照推导
    InferredType result = T_UNKNOWN;
    std::function<void(const SyntaxTreeInterfacePtr&)> walk;
    walk = [&](const SyntaxTreeInterfacePtr &node) {
        if (!node) return;
        if (node->Type() == SyntaxTreeType::Return) {
            auto ret_node = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            if (ret_node && ret_node->Explist()) {
                auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(ret_node->Explist());
                if (el && !el->Exps().empty()) {
                    auto it = snap.find(el->Exps()[0].get());
                    if (it != snap.end()) {
                        InferredType t = it->second.type;
                        if (result == T_UNKNOWN) result = t;
                        else if (IsNumericInferredType(result) && IsNumericInferredType(t)) {
                            result = (result == T_FLOAT || t == T_FLOAT) ? T_FLOAT : result;
                        } else {
                            result = T_DYNAMIC;
                        }
                    }
                }
            }
            return;
        }
        switch (node->Type()) {
            case SyntaxTreeType::Block: {
                auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
                for (auto &s : blk->Stmts()) walk(s);
                break;
            }
            case SyntaxTreeType::If: {
                auto ifn = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
                walk(ifn->Block());
                if (auto eb = ifn->ElseBlock()) walk(eb);
                break;
            }
            default:
                break;
        }
    };
    walk(func_block);
    if (result == T_UNKNOWN) result = T_INT; // 默认 int
    return result;
}

std::vector<TypeInferencer::FunctionSpecInfo>
TypeInferencer::CollectFunctionSpecInfos(const ParseResult &pr) const {
    std::vector<FunctionSpecInfo> infos;
    if (!pr.chunk || pr.chunk->Type() != SyntaxTreeType::Block) return infos;

    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);
    for (const auto &stmt : top_block->Stmts()) {
        std::string name;
        SyntaxTreeInterfacePtr funcbody;
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            const auto fnlist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcname->FuncNameList());
            name = fnlist->Funcnames()[0];
            funcbody = func->Funcbody();
        } else if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
            name = func->Name();
            funcbody = func->Funcbody();
        }
        if (name.empty() || !funcbody) continue;

        const auto funcbody_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
        if (!funcbody_ptr->Parlist()) continue;
        const auto pl = std::dynamic_pointer_cast<SyntaxTreeParlist>(funcbody_ptr->Parlist());
        if (!pl->Namelist()) continue;
        const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(pl->Namelist());
        if (nl->Names().empty()) continue;

        FunctionSpecInfo info;
        info.name = name;
        info.block = funcbody_ptr->Block();
        info.params = nl->Names();
        infos.push_back(std::move(info));
    }
    return infos;
}

void TypeInferencer::CollectGlobalConstVars(const ParseResult &pr, const EvalTypeSnapshot &current_map, InferResult &ir) {
    if (!pr.chunk || pr.chunk->Type() != SyntaxTreeType::Block) return;
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);
    for (const auto &stmt : block->Stmts()) {
        if (stmt->Type() != SyntaxTreeType::LocalVar) continue;
        const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
        const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
        if (!namelist) continue;

        auto explist = local_var->Explist()
            ? std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist()) : nullptr;
        const auto &names = namelist->Names();
        const auto &exps = explist ? explist->Exps() : std::vector<SyntaxTreeInterfacePtr>{};

        for (size_t i = 0; i < names.size(); ++i) {
            InferredType type = T_DYNAMIC;
            if (i < exps.size()) {
                auto it = current_map.find(exps[i].get());
                if (it != current_map.end()) type = it->second;
            }
            ir.global_const_vars[names[i]] = type;
        }
    }
}

}// namespace fakelua
