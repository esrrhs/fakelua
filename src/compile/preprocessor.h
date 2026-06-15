#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"

namespace fakelua {

// 语法树预处理
class PreProcessor {
public:
    explicit PreProcessor(State *s);


    void Process(const ParseResult &pr, const CompileConfig &cfg);

private:
    void PreprocessSplitAssigns(const SyntaxTreeInterfacePtr &chunk);

    void PreprocessSplitAssign(const SyntaxTreeInterfacePtr &node);

    void PreprocessTableAssigns(const SyntaxTreeInterfacePtr &chunk);

    void PreprocessTableAssign(const SyntaxTreeInterfacePtr &node);

    // 将顶层 "local f = function(...) ... end" 转换为 "local function f(...) ... end"，
    // 使其可被特化发现流程处理，行为与 LocalFunction 语句完全一致。
    void PreprocessFunctiondefLocalVars(const SyntaxTreeInterfacePtr &chunk);

private:
    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    std::string LocationStr(const SyntaxTreeInterfacePtr &ptr);

    void DumpDebugFile(const SyntaxTreeInterfacePtr &chunk, int step);

    bool IsFunctionCallExp(const SyntaxTreeInterfacePtr &exp_node);

    std::shared_ptr<SyntaxTreePrefixexp> MakeSimpleVarPrefixexp(const SyntaxTreeLocation &loc, const std::string &name);

    std::shared_ptr<SyntaxTreeExp> MakePrefixexpExp(const SyntaxTreeLocation &loc, const SyntaxTreeInterfacePtr &pe);

    std::shared_ptr<SyntaxTreeExp> MakeStringExp(const SyntaxTreeLocation &loc, const std::string &val);

private:
    State *s_;
    std::string file_name_;
    int tmp_var_counter_ = 0;
};

}// namespace fakelua
