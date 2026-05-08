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

    // 返回最近一次编译时记录的 C 代码（仅当 CompileConfig::record_c_code 为 true 时非空）
    [[nodiscard]] const std::string &GetLastRecordedCCode() const {
        return last_recorded_c_code_;
    }

private:
    // 核心编译流程实现，由 CompileFile 和 CompileString 调用
    CompileResult Compile(MyFlexer &f, const CompileConfig &cfg);

private:
    State *s_;// FakeLua 状态指针
    std::string last_recorded_c_code_;
};

}// namespace fakelua
