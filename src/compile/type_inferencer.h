#pragma once

#include "compile/compile_common.h"
#include <optional>
#include <unordered_set>

namespace fakelua {

class TypeInferencer {
public:
    // 运行全局类型推断（legacy + SSA 双轨）
    InferResult InferTypes(const ParseResult &pr, const CompileConfig &cfg);

private:
    struct FunctionSpecInfo {
        std::string name;
        SyntaxTreeInterfacePtr block;
        std::vector<std::string> params;
    };

    // ── SSA 管线入口 ─────────────────────────────────────────────────
    void RunSSAAnalysis(const ParseResult &pr, InferResult &ir);
    void RunSSASpecialization(const ParseResult &pr, InferResult &ir);

    // ── Legacy 兼容桥（由 SSA 快照派生）──────────────────────────────
    void PopulateMainEvalTypesFromSSA(const SyntaxTreeInterfacePtr &chunk, InferResult &ir);
    void PopulateGlobalConstVarsFromSSA(const SyntaxTreeInterfacePtr &chunk, InferResult &ir);
    void PopulateLegacyReturnTypes(InferResult &ir);
    void PopulateMathParamPositionsFromSSA(InferResult &ir);
    void PopulateTableSpecInfosFromSSA(const SyntaxTreeInterfacePtr &chunk, InferResult &ir);

    // ── 简化版 flow-sensitive local 类型修复（覆盖 UTA 的 AST-walk
    //    单遍局限）。对每个顶层/函数内的 local 声明，若后续被赋值非数值，
    //    其 local_eval_type 在 main_eval_types 中退化为 T_DYNAMIC。
    //    作用：使 Legacy CGen a = 2 → CVar a = 2 (当 a 后续被赋字符串时)，
    //    生成正确可执行的代码。
    void PopulateLocalFlowSensitiveTypes(const SyntaxTreeInterfacePtr &chunk, InferResult &ir);

    // ── SSA 辅助 ──────────────────────────────────────────────────────
    [[nodiscard]] std::vector<FunctionSpecInfo> CollectFunctionSpecInfos(const ParseResult &pr) const;
    void CollectGlobalConstVars(const ParseResult &pr, const EvalTypeSnapshot &current_map, InferResult &ir);
    void AnnotateSimpleConstants(const SyntaxTreeInterfacePtr &node, InferResult &ir);
};

}// namespace fakelua
