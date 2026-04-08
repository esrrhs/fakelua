#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"

namespace fakelua {

// 语法树预处理
class PreProcessor {
public:
    explicit PreProcessor(State *s);

    ~PreProcessor() = default;

    void Process(const CompileResult &cr, const CompileConfig &cfg);

private:
    void PreprocessSplitAssigns(const SyntaxTreeInterfacePtr &chunk);

    void PreprocessSplitAssign(const SyntaxTreeInterfacePtr &node);

    void PreprocessTableAssigns(const SyntaxTreeInterfacePtr &chunk);

    void PreprocessTableAssign(const SyntaxTreeInterfacePtr &node);

private:
    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    std::string LocationStr(const SyntaxTreeInterfacePtr &ptr);

    void DumpDebugFile(const SyntaxTreeInterfacePtr &chunk, int step);

private:
    State *s_;
    std::string file_name_;
};

}// namespace fakelua
