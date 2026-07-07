#pragma once

#include "compile/compile_common.h"
#include <unordered_set>

namespace fakelua {

class TypeInferencer {
public:
    // 主入口: 返回 SSA/CFG/Shape 单轨的类型推导结果
    [[nodiscard]] InferResult InferTypes(const ParseResult &pr, const CompileConfig &cfg);

private:
    struct FunctionSpecInfo {
        std::string name;
        SyntaxTreeInterfacePtr block;
        std::vector<std::string> params;
    };

    // ── SSA 管线入口 ─────────────────────────────────────────────────
    void RunSSAAnalysis(const ParseResult &pr, InferResult &ir);

    // ── SSA 辅助 ──────────────────────────────────────────────────────
    [[nodiscard]] std::vector<FunctionSpecInfo> CollectFunctionSpecInfos(const ParseResult &pr) const;
};

}// namespace fakelua
