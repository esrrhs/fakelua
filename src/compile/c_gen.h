#pragma once
#include "compile_common.h"
#include "fakelua.h"

namespace fakelua {

// 从语法树生成C代码
class CGen {
public:
    // 构造函数
    explicit CGen(State *s);

    ~CGen() = default;

    void Generate(const CompileResult &cr, const CompileConfig &cfg);

public:

private:
    State *s_;
};

}// namespace fakelua
