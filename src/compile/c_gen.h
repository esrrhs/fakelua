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
    std::string Build(const CompileResult &cr, const CompileConfig &cfg);
    void GenerateHeader();
    void GenerateMacros();
    std::string GetFuncName(const SyntaxTreeInterfacePtr &ptr);
    void GenerateDecl(const std::string &name, const SyntaxTreeInterfacePtr &body);
    void GenerateFunction(const std::string &name, const SyntaxTreeInterfacePtr &body);
    void CollectLocals(const SyntaxTreeInterfacePtr &node, std::set<std::string> &locals);
    void GenerateBlock(const SyntaxTreeInterfacePtr &node, int indent);
    void GenerateStmt(const SyntaxTreeInterfacePtr &stmt, int indent);
    void GenerateExp(const SyntaxTreeInterfacePtr &node);
    void GeneratePrefixexp(const SyntaxTreeInterfacePtr &node);

private:
    State *s_;
    std::stringstream header_;
    std::stringstream decl_;
    std::stringstream impl_;
    std::map<std::string, size_t> function_param_counts_;
};

}// namespace fakelua
