#pragma once

namespace fakelua {

// JIT类型
enum JITType {
    // TinyCC 是一个小型的 C 语言编译器，支持即时编译（JIT）。它的特点是编译速度快，适合于需要快速生成和执行代码的场景
    JIT_TCC = 0,
    // GCC 后端：将生成 C 代码通过系统 gcc 编译为动态库并加载执行
    JIT_GCC,
    JIT_MAX,
};

// 控制编译器的配置项
struct CompileConfig {
    // 跳过 JIT 编译。仅进行词法分析和语法解析。
    bool skip_jit = false;
    // 调试模式。如果为 true，JIT 代码将被转储到文件中。
    bool debug_mode = true;
    // 是否使用 JIT 编译，默认都开启
    bool disable_jit[JIT_MAX] = {false};
    // 记录生成的 C 代码（全局变量、函数声明、函数实现，不含公共头部）。
    // 开启后可通过 GetLastRecordedCCode(State*) 获取最近一次编译产生的代码片段。
    bool record_c_code = false;
};

} // namespace fakelua
