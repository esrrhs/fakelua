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
    void AnalyzeGlobalConstNames(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar);
    void CheckUnsupportedSyntax(const SyntaxTreeInterfacePtr &chunk, const AnalysisResult &ar);
    void CheckNode(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckGotoOrLabel(const SyntaxTreeInterfacePtr &node);
    void ValidateGotoInBlock(const SyntaxTreeInterfacePtr &chunk, std::unordered_map<std::string, SyntaxTreeInterfacePtr> visible_labels);
    void CollectBlockLabels(const SyntaxTreeInterfacePtr &block, std::unordered_map<std::string, SyntaxTreeInterfacePtr> &labels);
    void CheckFunctionCall(const SyntaxTreeInterfacePtr &node);
    void CheckParList(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckLocalVar(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckAssign(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckForLoop(const SyntaxTreeInterfacePtr &node);
    void CheckForIn(const SyntaxTreeInterfacePtr &node);
    void CheckExp(const SyntaxTreeInterfacePtr &node);
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
    std::unordered_set<const SyntaxTreeInterface *> init_assign_nodes_;
    std::unordered_set<std::string> assigned_global_consts_;
};

}// namespace fakelua
