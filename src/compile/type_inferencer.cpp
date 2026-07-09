#include "compile/type_inferencer.h"

#include <fstream>

#include "compile/infer/cfg.h"
#include "compile/infer/shape_type.h"
#include "compile/infer/ssa.h"
#include "compile/infer/unified_type_analyzer.h"
#include "compile/infer/specialization_analyzer.h"
#include "compile/syntax_tree.h"
#include "util/file_util.h"

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────
// 主入口 — SSA/CFG/Shape 单轨
// ─────────────────────────────────────────────────────────────────────────
InferResult TypeInferencer::InferTypes(const ParseResult &pr, const CompileConfig &cfg) {
    InferResult ir;
    ir.shape_registry = std::make_shared<ShapeRegistry>();
    // 预绑定 AST 生命周期（后续测试通过 main_ssa_types 的 raw pointer 访问 AST）
    ir.chunk = pr.chunk;

    RunSSAAnalysis(pr, ir);

    SpecializationAnalyzer sa;
    sa.Analyze(pr, ir);

    // Debug dump
    if (cfg.debug_mode) {
        const auto dumpfile = GenerateTmpFilename("fakelua_infer_", ".txt");
        if (std::ofstream ofs(dumpfile); ofs.is_open()) {
            ofs << "=== main_ssa_types ===\n";
            for (auto &[node, ssa]: ir.main_ssa_types) {
                ofs << "  node=" << node << " type=" << InferredTypeToString(ssa.type) << " shape=" << ssa.shape_id << "\n";
            }
            ofs.close();
            LOG_INFO("Type inference results generated: {}", dumpfile);
        }
    }

    return ir;
}

// ─────────────────────────────────────────────────────────────────────────
// SSA 管线入口
// ─────────────────────────────────────────────────────────────────────────
void TypeInferencer::RunSSAAnalysis(const ParseResult &pr, InferResult &ir) {
    if (!ir.shape_registry) return;

    UnifiedTypeAnalyzer uta(ir.shape_registry.get());
    CFGBuilder cfg_builder;
    SSABuilder ssa_builder;

    const auto function_infos = CollectFunctionSpecInfos(pr);

    // 分析所有函数
    for (const auto &func_info: function_infos) {
        auto cfg = std::make_shared<CFGFunction>(cfg_builder.Build(func_info.block, func_info.params, func_info.name, /*is_vararg=*/false));
        auto ssa = std::make_shared<SSAFunction>(ssa_builder.Build(*cfg));

        ir.cfg_functions[func_info.name] = cfg;
        ir.ssa_functions[func_info.name] = ssa;

        ir.func_summaries[func_info.name].being_built = true;
        ir.func_summaries[func_info.name].func_name = func_info.name;

        uta.Analyze(func_info.name, func_info.block, *cfg, *ssa, ir);
        uta.ComputeCtorTargetShapes(func_info.block, *ssa, ir);
        uta.BuildSummary(func_info.name, func_info.block, *ssa, *cfg, ir.ssa_version_types, ir);

        ir.func_summaries[func_info.name].being_built = false;
    }

    // 顶层 chunk
    {
        const std::string init_name = kInitFunctionName;
        std::vector<std::string> empty_params;
        auto cfg = std::make_shared<CFGFunction>(cfg_builder.Build(pr.chunk, empty_params, init_name, /*is_vararg=*/false));
        auto ssa = std::make_shared<SSAFunction>(ssa_builder.Build(*cfg));
        ir.cfg_functions[init_name] = cfg;
        ir.ssa_functions[init_name] = ssa;
        uta.Analyze(init_name, pr.chunk, *cfg, *ssa, ir);
    }
}

std::vector<TypeInferencer::FunctionSpecInfo> TypeInferencer::CollectFunctionSpecInfos(const ParseResult &pr) const {
    std::vector<FunctionSpecInfo> infos;
    if (!pr.chunk || pr.chunk->Type() != SyntaxTreeType::Block) return infos;

    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);
    for (const auto &stmt: top_block->Stmts()) {
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
        std::vector<std::string> params;
        if (funcbody_ptr->Parlist()) {
            const auto pl = std::dynamic_pointer_cast<SyntaxTreeParlist>(funcbody_ptr->Parlist());
            if (pl && pl->Namelist()) {
                const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(pl->Namelist());
                if (nl) {
                    params = nl->Names();
                }
            }
        }

        FunctionSpecInfo info;
        info.name = name;
        info.block = funcbody_ptr->Block();
        info.params = params;
        infos.push_back(std::move(info));
    }
    return infos;
}

}// namespace fakelua
