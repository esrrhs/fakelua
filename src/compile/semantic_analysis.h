#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"
#include <unordered_map>
#include <unordered_set>
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
    void CollectGotosInStmt(const SyntaxTreeInterfacePtr &stmt, std::vector<std::pair<std::string, SyntaxTreeInterfacePtr>> &out);
    void CollectGotosInBlock(const SyntaxTreeInterfacePtr &block, std::vector<std::pair<std::string, SyntaxTreeInterfacePtr>> &out);
    void ValidateConstAssignInBlock(const SyntaxTreeInterfacePtr &chunk, std::unordered_set<std::string> const_names);
    void CollectBlockLabels(const SyntaxTreeInterfacePtr &block, std::unordered_map<std::string, SyntaxTreeInterfacePtr> &labels);
    void CheckFunctionCall(const SyntaxTreeInterfacePtr &node);
    void CheckParList(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckLocalVar(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar);
    void CheckBlockReturnPosition(const SyntaxTreeInterfacePtr &node);
    void CheckForLoop(const SyntaxTreeInterfacePtr &node);
    void CheckForIn(const SyntaxTreeInterfacePtr &node);
    void CheckExp(const SyntaxTreeInterfacePtr &node);
    void CheckGlobalConstExp(const SyntaxTreeInterfacePtr &exp);
    // fakelua 刻意不支持 Lua 的隐式全局：每个简单名必须能解析到 local/形参/upvalue、
    // 文件级 local 或文件级函数，或宿主注册的原生函数。未声明名字在编译期直接报错
    // （读返回 nil、写改写 const kNil 的旧行为都是隐患）。豁免：直接调用位 f(...) 的
    // 整条点号调用链（未知名维持运行时 FakeluaCallByName 的 "not found" 报错，且原生
    // 函数允许编译后注册）、math/string 等点号原生库的模块根名，以及文件首行 package
    // 声明（package "X" / package = "X"）的名字。
    void CheckUndeclaredVars(const SyntaxTreeInterfacePtr &chunk, const AnalysisResult &ar);
    // CheckUndeclaredVars 的词法作用域递归，作用域栈语义与 CGen::ResolveScopes 对齐。
    void CheckVarScopes(const SyntaxTreeInterfacePtr &node, std::vector<std::unordered_set<std::string>> &scopes,
                        const std::unordered_set<std::string> &file_level_names,
                        const std::unordered_set<const SyntaxTreeInterface *> &exempt_vars,
                        const std::unordered_set<const SyntaxTreeInterface *> &lvalue_vars);
    [[nodiscard]] bool IsDeclaredSimpleName(const std::string &name,
                                            const std::vector<std::unordered_set<std::string>> &scopes,
                                            const std::unordered_set<std::string> &file_level_names) const;
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
