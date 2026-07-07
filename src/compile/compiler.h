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
//
// 关于 CompileFileTo / CompileStringTo:
//   这两个新接口返回完整的 CompileResult（C 代码 + InferResult）。
//   原 CompileFile 接口保留（返回 ParseResult），用于不需要中间结果的
//   纯执行路径（如 REPL、快速 benchmark 等）。
// ─────────────────────────────────────────────────────────────────────────
class Compiler {
public:
    // 构造函数
    explicit Compiler(State *s);


public:
    // ── 旧接口（返回仅解析结果） ─────────────────────────────────────────
    // 兼容入口。新代码应优先使用 CompileFileTo / CompileStringTo 以获取
    // 完整的管线产物。
    // 保留返回 ParseResult 是为了不破坏 REPL 等不需要 ir 的调用点。
    [[deprecated("Use CompileFileTo for full pipeline results")]]
    ParseResult CompileFile(const std::string &file, const CompileConfig &cfg);

    [[deprecated("Use CompileStringTo for full pipeline results")]]
    ParseResult CompileString(const std::string &str, const CompileConfig &cfg);

    // ── 新接口（返回完整管线结果） ────────────────────────────────────────
    // 编译 Lua 文件 → 返回 CCode + InferResult + AR + GR。
    // 返回值中的 InferResult 持有 AST 生命周期（见 InferResult::chunk）。
    CompileResult CompileFileTo(const std::string &file, const CompileConfig &cfg);

    // 编译 Lua 源代码字符串 → 返回 CCode + InferResult + AR + GR。
    // 与 CompileFileTo 唯一的区别是 source 来自字符串而非文件。
    CompileResult CompileStringTo(const std::string &str, const CompileConfig &cfg);

private:
    // 核心编译管线实现，由所有 Compile* 调用。
    // 参数 out_result 允许调用方获取完整管线结果；传 nullptr 时仅执行管线
    // 不产出结果（用于兼容旧的 CompileFile 接口）。
    ParseResult Compile(MyFlexer &f, const CompileConfig &cfg,
                        CompileResult *out_result = nullptr);

private:
    State *s_;  // FakeLua 状态指针（非拥有，生命周期由调用方管理）
};

}// namespace fakelua
