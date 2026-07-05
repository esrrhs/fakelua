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
    PopulateLegacyReturnTypes(ir);
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

// 填充 legacy specialization_snapshots / specialization_return_types（CGen 兼容）
void TypeInferencer::PopulateLegacyReturnTypes(InferResult &ir) {
    for (auto &[func, rets] : ir.spec_ssa_return_types) {
        auto &legacy = ir.specialization_return_types[func];
        legacy.resize(rets.size());
        for (size_t i = 0; i < rets.size(); ++i)
            legacy[i] = rets[i].type;
    }
    for (auto &[func, snaps] : ir.spec_ssa_snapshots) {
        auto &legacy = ir.specialization_snapshots[func];
        legacy.resize(snaps.size());
        for (size_t i = 0; i < snaps.size(); ++i) {
            for (const auto &[node, ssa_info] : snaps[i])
                legacy[i][const_cast<SyntaxTreeInterface *>(node)] = ssa_info.type;
        }
    }
}

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
// 给定一个 SyntaxTreeExp 节点，返回它的 InferredType（可能是 literal）
InferredType TypeOfScalar(const SyntaxTreeInterfacePtr &exp, fakelua::EvalTypeSnapshot &map) {
    if (!exp) return {T_DYNAMIC};
    auto it = map.find(exp.get());
    if (it != map.end()) return it->second;
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
                    t = TypeOfScalar(exps[i], map);
                }
                // 退化：如果变量已存在，meet 两者的类型
                auto it = cur_type.find(names[i]);
                if (it != cur_type.end()) {
                    t = MeetFlow(it->second, t);
                }
                cur_type[names[i]] = t;
                if (i < exps.size()) decl_node_by_name[names[i]] = exps[i];
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
                auto rt = TypeOfScalar(exps[i], map);
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
            // Restore outer state (unchanged: cursor is fresh in inner block)
            if (saved_outer) cur_type[cname] = *saved_outer;
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
    }
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
