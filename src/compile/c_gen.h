#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"

namespace fakelua {

// 从语法树生成C代码
class CGen {
public:
    // 构造函数
    explicit CGen(State *s);

    ~CGen() = default;

    void Generate(CompileResult &cr, const CompileConfig &cfg);

private:
    std::string Build(CompileResult &cr, const CompileConfig &cfg);

    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    std::string GenerateHeader();

    std::string GenerateDecls(CompileResult &cr);

    std::string CompileFuncName(const SyntaxTreeInterfacePtr &ptr);

    std::vector<std::string> CompileParlist(const SyntaxTreeInterfacePtr &parlist);

private:
    State *s_;
    std::string file_name_;

    std::unordered_set<std::string> global_const_vars_;
};

}// namespace fakelua
