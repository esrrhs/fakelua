#pragma once

#include "util/exception.h"
#include <setjmp.h>
#include <string>
#include <utility>

// 边界设在每次 C++ 进入 JIT 代码的位置，属于调用热路径，因此避开 setjmp/longjmp
// 对信号掩码的保存与恢复（这里不需要，实测每次调用要多花十几纳秒）。
#if defined(_WIN32)
#define FAKELUA_SETJMP(buf) setjmp(buf)
#define FAKELUA_LONGJMP(buf, val) longjmp(buf, val)
#else
#define FAKELUA_SETJMP(buf) _setjmp(buf)
#define FAKELUA_LONGJMP(buf, val) _longjmp(buf, val)
#endif

namespace fakelua {

// JIT 代码帧与 C++ 异常
// ─────────────────────────────────────────────────────────────────────────────
// TCC 把代码生成到自己的内存代码页里，这些帧没有 DWARF 展开表（.eh_frame）。
// 一旦 C++ 异常需要穿过它们，展开器找不到处理器就直接 std::terminate()，
// 进程当场崩溃——`pcall`、`dofile` 乃至调用方的 try/catch 全都失效。
//
// 因此错误不再依赖异常穿越 JIT 帧：在每个"C++ 调用 JIT 代码"的位置用 setjmp
// 埋一个边界，JIT 代码里产生的错误 longjmp 回到边界，再由边界以 FakeluaException
// 的形式抛给调用方，之后就是普通的 C++ 传播。
//
// longjmp 只跨越 JIT 生成的帧（纯 C，没有析构函数）。凡是能被 JIT 代码直接调用
// 的 C++ 入口（FakeluaThrowError / FakeluaCallByName / FakeluaAlloc 等）都先在
// 自己这一层把异常接住，让沿途 C++ 帧的析构函数正常执行，之后才跳转。
struct JitErrorBoundary {
    jmp_buf buf;
    JitErrorBoundary *prev = nullptr;
    // 错误信息由抛出方在跳转前填好（含堆栈），边界拿它构造异常
    std::string msg;
};

// 当前线程最内层的边界；为空表示当前不在 JIT 代码执行过程中。
// 与 Vm 一致地按"一个 State 只被一个线程持有"设计，故用 thread_local 而非全局。
extern thread_local JitErrorBoundary *g_jit_error_boundary __attribute__((tls_model("initial-exec")));

inline bool InJitFrame() {
    return g_jit_error_boundary != nullptr;
}

// 把错误交回最近的边界。仅在 InJitFrame() 为真时可调用。
[[noreturn]] void JumpToJitErrorBoundary(std::string msg);

// 边界的入栈与出栈。出栈必须走析构：fn 也可能直接抛出普通 C++ 异常（例如参数个数
// 不合法），那种情况下不会回到 setjmp 点，仅靠顺序代码复位会留下悬空指针。
class JitErrorBoundaryScope {
public:
    explicit JitErrorBoundaryScope(JitErrorBoundary *boundary) : prev_(g_jit_error_boundary) {
        boundary->prev = prev_;
        g_jit_error_boundary = boundary;
    }

    ~JitErrorBoundaryScope() {
        g_jit_error_boundary = prev_;
    }

    JitErrorBoundaryScope(const JitErrorBoundaryScope &) = delete;
    JitErrorBoundaryScope &operator=(const JitErrorBoundaryScope &) = delete;

private:
    JitErrorBoundary *prev_;
};

// 在边界内执行 fn（fn 会调用 JIT 代码）：JIT 代码里的错误会以 FakeluaException 抛出。
template<typename Fn>
auto RunWithJitErrorBoundary(Fn &&fn) -> decltype(fn()) {
    JitErrorBoundary boundary;
    JitErrorBoundaryScope scope(&boundary);

    if (FAKELUA_SETJMP(boundary.buf) == 0) {
        return fn();
    }
    throw FakeluaException(std::move(boundary.msg));
}

// 供可被 JIT 代码直接调用的 C++ 入口使用：正常路径零开销，出错时先让本层及以下的
// C++ 析构函数按常规展开执行，再把错误 longjmp 给最近的边界。不在 JIT 代码里时
// 保持原有的异常传播语义不变。
template<typename Fn>
auto GuardJitEntry(Fn &&fn) -> decltype(fn()) {
    try {
        return fn();
    } catch (const FakeluaException &e) {
        if (!InJitFrame()) throw;
        JumpToJitErrorBoundary(e.what());
    } catch (const std::exception &e) {
        if (!InJitFrame()) throw;
        JumpToJitErrorBoundary(std::string("fakelua error: ") + e.what());
    } catch (...) {
        if (!InJitFrame()) throw;
        JumpToJitErrorBoundary("fakelua error: unknown exception thrown from native code");
    }
}

}// namespace fakelua
