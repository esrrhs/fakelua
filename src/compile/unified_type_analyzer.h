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

    // ── Phase 3a: 计算 widest shapes ────────────────────────────────────
    void ComputeVarFinalShapes(const SSAFunction &ssa, InferResult &ir);
    void ComputeCtorTargetShapes(const SyntaxTreeInterfacePtr &func_block,
                                 const SSAFunction &ssa,
                                 InferResult &ir);

    // ── §8 逃逸分析 ────────────────────────────────────────────────────
    void ComputeEscape(const std::string &func_name,
                       const SyntaxTreeInterfacePtr &func_block,
                       const SSAFunction &ssa,
                       InferResult &ir);

    // ── §7 函数摘要 ────────────────────────────────────────────────────
    void BuildSummary(const std::string &func_name,
                      const SyntaxTreeInterfacePtr &func_block,
                      const SSAFunction &ssa,
                      const std::unordered_map<int, SSATypeInfo> &version_types,
                      InferResult &ir);

    SSATypeInfo ApplyCallSummary(const std::string &callee_name,
                                  const InferResult &ir) const;

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

    static void LinkExprToTargetShape(const SyntaxTreeInterfacePtr &node,
                                      const std::string &target_name,
                                      const std::unordered_map<std::string, int> &var_final_shapes,
                                      std::unordered_map<const SyntaxTreeInterface *, int> &ctor_target_shapes);

    void WalkEscape(const std::string &func_name,
                    const SyntaxTreeInterfacePtr &node,
                    const SSAFunction &ssa,
                    const std::unordered_map<int, SSATypeInfo> &version_types,
                    std::unordered_map<std::string, bool> &escape_vars,
                    InferResult &ir);

    ShapeRegistry *registry_ = nullptr;
    std::string cur_func_name_;
};

}// namespace fakelua
