#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
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
    void AnalyzeGlobalConsts(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar);
    void CheckUnsupportedSyntax(const SyntaxTreeInterfacePtr &chunk, const AnalysisResult &ar);
    void CheckNode(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckGlobalConstExp(const SyntaxTreeInterfacePtr &exp);
    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);
    std::string LocationStr(const SyntaxTreeInterfacePtr &ptr);

    void AnalyzeFunctionReturnCounts(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar);
    void CollectReturnsForBlock(const SyntaxTreeInterfacePtr &node, std::vector<SyntaxTreeInterfacePtr> &returns);
    bool IsFunctionCallExp(const SyntaxTreeInterfacePtr &exp_node);
    std::string GetCalleeName(const SyntaxTreeInterfacePtr &exp_node);

private:
    State *s_;
    std::string file_name_;
    std::unordered_set<const SyntaxTreeInterface *> top_level_stmts_;
};

} // namespace fakelua
