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

    // 校验文件级（chunk 顶层）语句的合法性。
    // 必须在 PreProcessor 之前调用：预处理会把顶层语句搬进 __fakelua_init，
    // 之后就分辨不出哪些是用户写在文件级的语句、哪些是编译器生成的初始化赋值了。
    void CheckFileLevelStmts(const ParseResult &pr);

    // 运行语义分析
    AnalysisResult Analyze(const ParseResult &pr, const CompileConfig &cfg);

private:
    void AnalyzeGlobalConstNames(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar);
    void CheckUnsupportedSyntax(const SyntaxTreeInterfacePtr &chunk, const AnalysisResult &ar);
    void CheckNode(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckGotoOrLabel(const SyntaxTreeInterfacePtr &node);
    void ValidateGotoInBlock(const SyntaxTreeInterfacePtr &chunk, std::unordered_map<std::string, SyntaxTreeInterfacePtr> visible_labels, int loop_depth);
    void CollectBlockLabels(const SyntaxTreeInterfacePtr &block, std::unordered_map<std::string, SyntaxTreeInterfacePtr> &labels);
    void CheckFunctionCall(const SyntaxTreeInterfacePtr &node);
    void CheckParList(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckLocalVar(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckBlockReturnPosition(const SyntaxTreeInterfacePtr &node);
    void CheckForLoop(const SyntaxTreeInterfacePtr &node);
    void CheckForIn(const SyntaxTreeInterfacePtr &node);
    void CheckExp(const SyntaxTreeInterfacePtr &node);
    void CheckGlobalConstExp(const SyntaxTreeInterfacePtr &exp);
    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    void AnalyzeFunctionReturnCounts(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar);
    void CollectReturnsForBlock(const SyntaxTreeInterfacePtr &node, std::vector<SyntaxTreeInterfacePtr> &returns);
    std::string GetCalleeName(const SyntaxTreeInterfacePtr &exp_node);

private:
    State *s_;
    std::string file_name_;
    std::unordered_set<const SyntaxTreeInterface *> top_level_stmts_;
};

}// namespace fakelua
