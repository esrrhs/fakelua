#pragma once

#include "compile/cfg.h"
#include "compile/compile_common.h"
#include "compile/shape_type.h"
#include "compile/ssa.h"
#include "compile/syntax_tree.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

class UnifiedTypeAnalyzer {
public:
    using SSATypeInfo = fakelua::SSATypeInfo;
    using SpecParam = fakelua::SpecParam;

    // 参数类型假设（用于特化版本分析）
    using ParamAssumption = std::unordered_map<int, SSATypeInfo>;

    UnifiedTypeAnalyzer(ShapeRegistry *registry) : registry_(registry) {}

    // ── 主分析入口 ──────────────────────────────────────────────────────
    void Analyze(const std::string &func_name,
                 const SyntaxTreeInterfacePtr &func_block,
                 const CFGFunction &cfg,
                 SSAFunction &ssa,
                 InferResult &ir,
                 int bitmask = -1,
                 const ParamAssumption &param_assumptions = {});

    // ── 发现可特化参数 ──────────────────────────────────────────────────
    std::vector<SpecParam> FindSpecializableParams(const SyntaxTreeInterfacePtr &func_block,
                                                   const CFGFunction &cfg,
                                                   const SSAFunction &ssa,
                                                   const InferResult &ir);

private:
    using TypeEnv = std::unordered_map<int, SSATypeInfo>;

    void RunWorklist(const SSAFunction &ssa,
                     const ParamAssumption &param_assumptions,
                     TypeEnv &version_types,
                     const InferResult &ir);

    SSATypeInfo InferExprType(const SyntaxTreeInterfacePtr &expr,
                              const SSAFunction &ssa,
                              const TypeEnv &version_types,
                              const InferResult &ir);

    int BuildShapeFromCtor(const SyntaxTreeInterfacePtr &tc,
                           const SSAFunction &ssa,
                           const TypeEnv &version_types,
                           const InferResult &ir);

    SSATypeInfo Meet(const SSATypeInfo &a, const SSATypeInfo &b);

    void ComputeVarFinalShapes(const SSAFunction &ssa, InferResult &ir);
    void ComputeCtorTargetShapes(const SyntaxTreeInterfacePtr &func_block,
                                 const SSAFunction &ssa,
                                 InferResult &ir);
    static void LinkExprToTargetShape(const SyntaxTreeInterfacePtr &node,
                                      const std::string &target_name,
                                      const std::unordered_map<std::string, int> &var_final_shapes,
                                      std::unordered_map<const SyntaxTreeInterface *, int> &ctor_target_shapes);

    void BuildSummary(const std::string &func_name,
                      const SyntaxTreeInterfacePtr &func_block,
                      const SSAFunction &ssa,
                      const TypeEnv &version_types,
                      InferResult &ir);

    ShapeRegistry *registry_ = nullptr;
    std::string cur_func_name_;
};

}// namespace fakelua
