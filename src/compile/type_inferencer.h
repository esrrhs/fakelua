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

    // ── legacy + SSA 辅助 ────────────────────────────────────────────
    [[nodiscard]] std::vector<FunctionSpecInfo> CollectFunctionSpecInfos(const ParseResult &pr) const;
    void WalkAST(const SyntaxTreeInterfacePtr &node, EvalTypeSnapshot &map);
    void CollectGlobalConstVars(const ParseResult &pr, const EvalTypeSnapshot &current_map, InferResult &ir);
};

}// namespace fakelua
