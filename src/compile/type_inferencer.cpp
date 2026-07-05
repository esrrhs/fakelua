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
