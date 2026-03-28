#pragma once

#include "compile/compile_common.h"
#include "compile/my_flexer.h"
#include "fakelua.h"
#include <string>

namespace fakelua {

class State;

// 编译器类，负责驱动词法分析、语法解析、预处理和 JIT 编译
class Compiler {
public:
    // 构造函数
    explicit Compiler(State *s);

    ~Compiler() = default;

public:
    // 编译指定的 Lua 文件
    CompileResult CompileFile(const std::string &file, const CompileConfig &cfg);

    // 编译 Lua 源代码字符串
    CompileResult CompileString(const std::string &str, const CompileConfig &cfg);

private:
    // 核心编译流程实现，由 CompileFile 和 CompileString 调用
    CompileResult Compile(MyFlexer &f, const CompileConfig &cfg);

private:
    State *s_;// FakeLua 状态指针
};

}// namespace fakelua
