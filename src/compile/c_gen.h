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

private:
    State *s_;
    std::stringstream header_;
};

}// namespace fakelua
