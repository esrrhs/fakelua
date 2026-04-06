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
    void GenerateDecls(CompileResult &cr);

    // 收集函数定义
    void CollectFunctions(const SyntaxTreeInterfacePtr &node);
    void CollectFunctionFromStmt(const SyntaxTreeInterfacePtr &stmt);

    // 收集函数体中的嵌套函数定义
    void CollectFunctionsFromBlock(const SyntaxTreeInterfacePtr &block);

private:
    State *s_;
    std::stringstream header_;
    std::stringstream decls_;

    // 收集到的函数信息：函数名 -> 参数个数
    std::unordered_map<std::string, int> functions_;

    // 当前函数名前缀（用于嵌套函数）
    std::string func_prefix_;
    int func_counter_ = 0;
};

}// namespace fakelua
