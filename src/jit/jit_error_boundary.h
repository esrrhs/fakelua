#pragma once

#include "state/state.h"
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
// TCC 把代码生成到自己的内存代码页里，这些帧没有 DWARF 展开表（.eh_frame）。
// 一旦 C++ 异常需要穿过它们，展开器找不到处理器就直接 std::terminate()，
// 进程当场崩溃——`pcall`、`dofile` 乃至调用方的 try/catch 全都失效。
// 因此错误不再依赖异常穿越 JIT 帧：在每个"C++ 调用 JIT 代码"的位置用 setjmp
// 埋一个边界，JIT 代码里产生的错误 longjmp 回到边界，再由边界以 FakeluaException
// 的形式抛给调用方，之后就是普通的 C++ 传播。
// longjmp 只跨越 JIT 生成的帧（纯 C，没有析构函数）。凡是能被 JIT 代码直接调用
// 的 C++ 入口（FakeluaThrowError / FakeluaCallByName / FakeluaAlloc 等）都先在
// 自己这一层把异常接住，让沿途 C++ 帧的析构函数正常执行，之后才跳转。
struct JitErrorBoundary {
    jmp_buf buf;
    JitErrorBoundary *prev = nullptr;
    // 错误信息分三段由跳转方填好（原始消息 / Lua 位置 / C++ 堆栈），
    // 边界只负责把它还原成异常，不重复采集堆栈或追加前缀。
    std::string msg;
    std::string loc;
    std::string stack;
    std::string lua_stack;
};

// 边界链的栈顶存在 State 上（State::GetJitErrorBoundary），不用线程局部变量：一个 State
// 只被一个线程持有，脚本执行也只发生在它自己的栈上，所以链顶天然是每 State 一份。
inline bool InJitFrame(State *s) {
    return s->GetJitErrorBoundary() != nullptr;
}

// 把错误交回最近的边界。仅在 InJitFrame(s) 为真时可调用。
[[noreturn]] void JumpToJitErrorBoundary(State *s, std::string msg, std::string loc = {}, std::string stack = {}, std::string lua_stack = {});

// 边界的入栈与出栈。出栈必须走析构：fn 也可能直接抛出普通 C++ 异常（例如参数个数
// 不合法），那种情况下不会回到 setjmp 点，仅靠顺序代码复位会留下悬空指针。
class JitErrorBoundaryScope {
public:
    JitErrorBoundaryScope(State *s, JitErrorBoundary *boundary) : s_(s), prev_(s->GetJitErrorBoundary()) {
        boundary->prev = prev_;
        s_->SetJitErrorBoundary(boundary);
    }

    ~JitErrorBoundaryScope() {
        s_->SetJitErrorBoundary(prev_);
    }

    JitErrorBoundaryScope(const JitErrorBoundaryScope &) = delete;
    JitErrorBoundaryScope &operator=(const JitErrorBoundaryScope &) = delete;

private:
    State *s_;
    JitErrorBoundary *prev_;
};

// 位置槽只在「这条 Lua 语句还在栈上」时有效：进入被调代码前清空、返回后恢复，
// 免得上一趟调用的残留被当成出错位置。O(1) 栈上保存/恢复，无堆分配与遍历开销。
class LuaLocScope {
public:
    explicit LuaLocScope(State *s) : s_(s), jit_(s->CurrentJit()), saved_(s->GetActiveLuaLoc(jit_)) {
        s_->GetActiveLuaLoc(jit_) = LuaLoc{nullptr, 0};
    }

    ~LuaLocScope() {
        s_->GetActiveLuaLoc(jit_) = saved_;
    }

    LuaLocScope(const LuaLocScope &) = delete;
    LuaLocScope &operator=(const LuaLocScope &) = delete;

private:
    State *s_;
    JITType jit_;
    LuaLoc saved_;
};

// 异常或 longjmp 会跳过生成代码里的 FL_LUA_POP。这里记下进入时的深度与溢出顶槽，离开时一并恢复；
// 调用栈文本已经在抛出点拷进异常，恢复深度不会丢掉报错信息。
class LuaStackScope {
public:
    explicit LuaStackScope(State *s) : s_(s), saved_(s->LuaDepth()), saved_overflow_(s->OverflowLuaFrame()) {
    }

    ~LuaStackScope() {
        s_->RestoreLuaStack(saved_, saved_overflow_);
    }

    LuaStackScope(const LuaStackScope &) = delete;
    LuaStackScope &operator=(const LuaStackScope &) = delete;

private:
    State *s_;
    int saved_;
    LuaFrame saved_overflow_;
};

// 在边界内执行 fn（fn 会调用 JIT 代码）：JIT 代码里的错误会以 FakeluaException 抛出。
template<typename Fn>
auto RunWithJitErrorBoundary(State *s, Fn &&fn) -> decltype(fn()) {
    JitErrorBoundary boundary;
    JitErrorBoundaryScope scope(s, &boundary);

    if (FAKELUA_SETJMP(boundary.buf) == 0) {
        return fn();
    }
    throw FakeluaException(FakeluaException::Parts{std::move(boundary.msg), std::move(boundary.loc), std::move(boundary.stack), std::move(boundary.lua_stack)});
}

// 供可被 JIT 代码直接调用的 C++ 入口使用：正常路径零开销，出错时先让本层及以下的
// C++ 析构函数按常规展开执行，再把错误 longjmp 给最近的边界。不在 JIT 代码里时
// 保持原有的异常传播语义不变。
template<typename Fn>
auto GuardJitEntry(State *s, Fn &&fn) -> decltype(fn()) {
    try {
        return fn();
    } catch (FakeluaException &e) {
        // 原生函数的报错没有 Lua 栈可读，在离错误最近的一层一次补齐位置和调用栈；外层已有则不再重复展开。
        if (!e.HasLoc() || e.LuaStack().empty()) {
            std::string loc;
            std::string lua_stack;
            s->CaptureLuaError(loc, lua_stack);
            e.SetLoc(loc);
            e.SetLuaStack(lua_stack);
        }
        if (!InJitFrame(s)) throw;
        JumpToJitErrorBoundary(s, e.Msg(), e.Loc(), e.Stack(), e.LuaStack());
    } catch (const std::exception &e) {
        if (!InJitFrame(s)) throw;
        std::string loc;
        std::string lua_stack;
        s->CaptureLuaError(loc, lua_stack);
        JumpToJitErrorBoundary(s, e.what(), std::move(loc), CaptureStacktrace(), std::move(lua_stack));
    } catch (...) {
        if (!InJitFrame(s)) throw;
        std::string loc;
        std::string lua_stack;
        s->CaptureLuaError(loc, lua_stack);
        JumpToJitErrorBoundary(s, "unknown exception thrown from native code", std::move(loc), CaptureStacktrace(), std::move(lua_stack));
    }
}

}// namespace fakelua
