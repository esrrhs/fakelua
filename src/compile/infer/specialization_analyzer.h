#pragma once

#include "compile/compile_common.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua {

class SpecializationAnalyzer {
public:
    void Analyze(const ParseResult &pr, InferResult &ir);

private:
    struct MathFuncInfo {
        SyntaxTreeInterfacePtr block;
        std::vector<std::string> params;
        std::vector<int> param_versions;
    };
    using MathFuncInfoMap = std::unordered_map<std::string, MathFuncInfo>;

    struct FuncRetInfo {
        bool ends_with_return = false;
        std::vector<SyntaxTreeInterfacePtr> ret_exps;
    };

    MathFuncInfoMap IdentifyMathParams(const ParseResult &pr, InferResult &ir);

    std::vector<int> FindMathParamIndices(const std::string &name, const MathFuncInfo &info,
                                          const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &baseline,
                                          const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &all_int,
                                          const std::unordered_map<std::string, std::vector<int>> &known_math_positions,
                                          InferResult &ir);

    void InferSpecializationReturnTypes(InferResult &ir, const MathFuncInfoMap &math_func_info,
                                        const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache);

    bool CheckArithmeticTypeChanges(const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                    const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                    const SyntaxTreeInterfacePtr &func_block, bool improvement_mode,
                                    const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    bool CheckArithmeticNodeChange(const SyntaxTreeInterfacePtr &node,
                                   const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                   const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                   bool improvement_mode) const;

    bool CheckComparisonNodeChange(const SyntaxTreeInterfacePtr &node,
                                   const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                   const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                   bool improvement_mode) const;

    bool CheckForLoopNodeChange(const SyntaxTreeInterfacePtr &node,
                                const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                bool improvement_mode) const;

    bool CheckCallNodeChange(const SyntaxTreeInterfacePtr &node,
                             const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                             const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                             const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    bool IsArithmeticExpr(const SyntaxTreeInterfacePtr &node) const;
    bool IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const;

    std::unordered_map<int, SSATypeInfo> MakeAssumedParamTypes(const std::vector<int> &param_versions, int special_idx,
                                                               InferredType special_type, InferredType default_type) const;

    std::unordered_map<int, SSATypeInfo> MakeSpecializedParamTypes(const std::vector<int> &param_versions,
                                                                   const std::vector<int> &math_indices, int bitmask) const;

    std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> RunTrialInference(
        const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, const CFGFunction &cfg, SSAFunction &ssa,
        const std::unordered_map<int, SSATypeInfo> &assumed_types, InferResult &ir);

    bool AllPathsReturn(const SyntaxTreeInterfacePtr &block_node) const;
    bool CollectReturnExps(const SyntaxTreeInterfacePtr &block_node, std::vector<SyntaxTreeInterfacePtr> &ret_exps) const;

    InferredType ComputeReturnTypeFromSnapshot(const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &snapshot,
                                               const FuncRetInfo &ret_info) const;

    static constexpr int kMaxSpecIterations = 16;
    static constexpr int kMaxMathSpecializedParams = 5;
};

} // namespace fakelua
