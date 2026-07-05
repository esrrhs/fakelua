#include "compile/type_inferencer.h"

#include <algorithm>
#include <fstream>
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
// 主入口（legacy walker + SSA 补充）
// ─────────────────────────────────────────────────────────────────────────
InferResult TypeInferencer::InferTypes(const ParseResult &pr, const CompileConfig &cfg) {
    InferResult ir;

    // ── Phase 1: legacy flow-insensitive walker（主路径，保证现有测试绿）─
    EvalTypeSnapshot legacy_map;
    WalkAST(pr.chunk, legacy_map);
    ir.main_eval_types = legacy_map;
    CollectGlobalConstVars(pr, legacy_map, ir);

    // ── Phase 2: SSA/CFG/Shape 管线（补充类型数据，不干扰 legacy）────
    RunSSAAnalysis(pr, ir);
    RunSSASpecialization(pr, ir);

    // debug 输出
    if (cfg.debug_mode) {
        const auto dumpfile = GenerateTmpFilename("fakelua_infer_", ".txt");
        if (std::ofstream ofs(dumpfile); ofs.is_open()) {
            ofs << "=== Main Evaluation Types (Legacy) ===\n";
            for (auto &[node, ty] : ir.main_eval_types) {
                ofs << "  node=" << node << " type=" << InferredTypeToString(ty) << "\n";
            }
            ofs.close();
            LOG_INFO("Type inference results generated: {}", dumpfile);
        }
    }

    return ir;
}

// 简化 AST walk for legacy main_eval_types
void TypeInferencer::WalkAST(const SyntaxTreeInterfacePtr &node, EvalTypeSnapshot &map) {
    if (!node) return;

    switch (node->Type()) {
        case SyntaxTreeType::Exp: {
            auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            switch (e->GetExpKind()) {
                case ExpKind::kNumber: {
                    const auto &v = e->ExpValue();
                    if (v.find('.') == std::string::npos && v.find('e') == std::string::npos && v.find('E') == std::string::npos)
                        map[node.get()] = T_INT;
                    else
                        map[node.get()] = T_FLOAT;
                    break;
                }
                case ExpKind::kString: map[node.get()] = T_STRING; break;
                case ExpKind::kNil: map[node.get()] = T_NIL; break;
                case ExpKind::kTrue:
                case ExpKind::kFalse: map[node.get()] = T_BOOL; break;
                case ExpKind::kBinop: {
                    WalkAST(e->Left(), map);
                    WalkAST(e->Right(), map);
                    auto lt = map.count(e->Left().get()) ? map[e->Left().get()] : T_DYNAMIC;
                    auto rt = map.count(e->Right().get()) ? map[e->Right().get()] : T_DYNAMIC;
                    map[node.get()] = ShapeRegistry::MeetType(lt, rt);
                    break;
                }
                case ExpKind::kUnop: {
                    WalkAST(e->Right(), map);
                    auto ot = map.count(e->Right().get()) ? map[e->Right().get()] : T_DYNAMIC;
                    map[node.get()] = ot;
                    break;
                }
                case ExpKind::kPrefixExp: {
                    WalkAST(e->Right(), map);
                    map[node.get()] = map.count(e->Right().get()) ? map[e->Right().get()] : T_DYNAMIC;
                    break;
                }
                default: map[node.get()] = T_DYNAMIC; break;
            }
            break;
        }
        case SyntaxTreeType::PrefixExp: {
            auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
            WalkAST(pe->GetValue(), map);
            map[node.get()] = map.count(pe->GetValue().get()) ? map[pe->GetValue().get()] : T_DYNAMIC;
            break;
        }
        case SyntaxTreeType::Var: {
            auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
            if (v->GetVarKind() == VarKind::kSimple) {
                // 简单变量——未知（运行时确定）
            } else if (v->GetVarKind() == VarKind::kDot) {
                WalkAST(v->GetPrefixexp(), map);
            } else if (v->GetVarKind() == VarKind::kSquare) {
                WalkAST(v->GetPrefixexp(), map);
                WalkAST(v->GetExp(), map);
            }
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            WalkAST(fc->prefixexp(), map);
            WalkAST(fc->Args(), map);
            break;
        }
        case SyntaxTreeType::ExpList: {
            auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            for (auto &e : el->Exps()) WalkAST(e, map);
            break;
        }
        case SyntaxTreeType::Block: {
            auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (auto &s : blk->Stmts()) WalkAST(s, map);
            break;
        }
        case SyntaxTreeType::LocalVar: {
            auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            WalkAST(lv->Explist(), map);
            break;
        }
        default: break;
    }
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
