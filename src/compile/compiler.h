#pragma once

#include "bison/parser.h"
#include "compile/my_flexer.h"
#include "jit/gcc_jit.h"
#include "jit/preprocessor.h"
#include <string>

namespace fakelua {

// 编译结果，包含文件名和生成的语法树
struct CompileResult {
    // 源代码文件名
    std::string fileName;
    // 语法树根节点（代码块）
    SyntaxTreeInterfacePtr chunk;
};

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
    State *s_;        // FakeLua 状态指针
    PreProcessor pp_; // 预处理器，处理常量、宏等
    GccJitter jitter_;// JIT 编译器后端
};

}// namespace fakelua
