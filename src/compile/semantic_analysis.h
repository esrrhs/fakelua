#pragma once

#include "compile/compile_common.h"
#include "fakelua.h"
#include <unordered_map>
#include <vector>

namespace fakelua {

class State;

// SemanticAnalysis —— 独立的语义与控制流分析器阶段。
class SemanticAnalysis {
public:
    explicit SemanticAnalysis(State *s);

    // 运行语义分析
    AnalysisResult Analyze(const ParseResult &pr, const CompileConfig &cfg);

private:
    void AnalyzeFunctionReturnCounts(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar);
    void CollectReturnsForBlock(const SyntaxTreeInterfacePtr &node, std::vector<SyntaxTreeInterfacePtr> &returns);
    bool IsFunctionCallExp(const SyntaxTreeInterfacePtr &exp_node);
    std::string GetCalleeName(const SyntaxTreeInterfacePtr &exp_node);

private:
    State *s_;
};

} // namespace fakelua
