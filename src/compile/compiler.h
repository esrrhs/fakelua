#pragma once

#include "compile/compile_common.h"
#include "compile/my_flexer.h"
#include "fakelua.h"
#include <string>

namespace fakelua {

class State;

// ─────────────────────────────────────────────────────────────────────────
// 编译器类
//
// 驱动从源码到 JIT 可执行代码的完整管线:
//   源码 → Lexer/Parser → PreProcessor → SemanticAnalysis
//        → TypeInferencer(SSA+Shape) → CGen → TCC/GCC JIT
//
// 该类的实例方法不持有任何跨调用的状态——每次 Compile* 调用会构建并返回
// 独立的 CompileResult，全局唯一残留是 s_（虚拟机状态，用于访问常量池和
// JIT 编译器）。
// ─────────────────────────────────────────────────────────────────────────
class Compiler {
public:
    // 构造函数
    explicit Compiler(State *s);

public:
    // ── 编译管线接口（返回完整管线结果） ────────────────────────────────
    //
    // CompileResult 包含所有阶段的中间产物 (parse/infer/analysis/gen),
    // 调用方可以从中读取任意阶段的结果用于验证或调试。
    //
    // 示例:
    //   auto result = compiler.CompileFile("test.lua", cfg);
    //   const auto& types = result.GetNodeTypes();  // 获取 AST 节点类型
    //   const std::string& code = result.GetCCode();  // 获取 C 代码

    // 编译 Lua 文件 → 返回 CompileResult。
    CompileResult CompileFile(const std::string &file, const CompileConfig &cfg);

    // 编译 Lua 源代码字符串 → 返回 CompileResult。
    // 与 CompileFile 唯一的区别是 source 来自字符串而非文件。
    CompileResult CompileString(const std::string &str, const CompileConfig &cfg);

private:
    // 核心编译管线实现，由 CompileFile / CompileString 调用。
    // 将各阶段产物写入 result。
    CompileResult Build(MyFlexer &f, const CompileConfig &cfg);

private:
    State *s_;// FakeLua 状态指针（非拥有，生命周期由调用方管理）
};

}// namespace fakelua
