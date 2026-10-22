#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace fakelua {

// 错误文本由三段拼成：原始消息、Lua 源码位置（file:line）、C++ 堆栈。
// 分开存是因为 Lua 位置只在离错误最近的 Lua 边界才拿得到——解释器帧看自己的当前指令，
// JIT 那边看生成代码写在 State 上的位置槽——补上位置时要重算 what()。
class FakeluaException : public std::runtime_error {
public:
    struct Parts {
        std::string msg;
        std::string loc;
        std::string stack;
    };

    explicit FakeluaException(Parts parts);

    const char *what() const noexcept override {
        return text_.c_str();
    }

    // 不带 "fakelua error:" 前缀、位置与堆栈的原始消息。跨边界传递时只带它，
    // 避免前缀被重复追加。
    [[nodiscard]] const std::string &Msg() const noexcept {
        return parts_.msg;
    }
    [[nodiscard]] const std::string &Loc() const noexcept {
        return parts_.loc;
    }
    [[nodiscard]] const std::string &Stack() const noexcept {
        return parts_.stack;
    }
    [[nodiscard]] bool HasLoc() const noexcept {
        return !parts_.loc.empty();
    }

    // 由最近的 Lua 边界补位置；已有位置的（内层帧已补）保持不动。
    void SetLoc(const std::string &loc);

protected:
    void Rebuild();

    Parts parts_;
    std::string text_;
};

// 静默的「无法按数值特化编译」信号：CGen 试探性地把表达式编译为原生数值失败时抛出，
// 由 TryCompileNativeExpr 等回退路径捕获并改走动态编译，属于正常控制流，不记录 ERROR。
// 若它逃逸到 CGen 顶层（调用点不允许回退），会在边界处转换为普通 FakeluaException
// 编译错误，保持与历史行为一致的对外报错。
class FakeluaSpecMissException : public FakeluaException {
public:
    explicit FakeluaSpecMissException(const std::string &msg) : FakeluaException(Parts{msg, {}, {}}) {
    }
};

// 采集当前 C++ 堆栈：错误路径专用，代价高，不要在控制流异常上使用。
std::string CaptureStacktrace();

// 记录 ERROR 日志并返回完整错误文本（前缀 + 消息 + 位置 + 堆栈）。
std::string LogFakeluaError(const std::string &msg, const std::string &loc, const std::string &stack);

// 构造完整错误文本（含当前堆栈），供只能回传字符串、无法抛异常的路径复用。
std::string BuildFakeluaErrorMessage(const std::string &msg);
std::string BuildFakeluaErrorMessage(const std::string &msg, const std::string &loc);

[[noreturn]] void ThrowFakeluaException(const std::string &msg);
// loc 为 Lua 源码位置文本，形如 "main.lua:12"；未知时传空串。
[[noreturn]] void ThrowFakeluaExceptionAt(const std::string &msg, const std::string &loc);

}// namespace fakelua
