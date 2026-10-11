#pragma once

#include <stdexcept>
#include <string>

namespace fakelua {

class FakeluaException : public std::runtime_error {
public:
    explicit FakeluaException(const std::string &msg) : std::runtime_error(msg) {
    }
};

// 静默的「无法按数值特化编译」信号：CGen 试探性地把表达式编译为原生数值失败时抛出，
// 由 TryCompileNativeExpr 等回退路径捕获并改走动态编译，属于正常控制流，不记录 ERROR。
// 若它逃逸到 CGen 顶层（调用点不允许回退），会在边界处转换为普通 FakeluaException
// 编译错误，保持与历史行为一致的对外报错。
class FakeluaSpecMissException : public FakeluaException {
public:
    explicit FakeluaSpecMissException(const std::string &msg) : FakeluaException(msg) {
    }
};

[[noreturn]] void ThrowFakeluaException(const std::string &msg);

// 构造 FakeluaException 的错误文本（追加当前堆栈）并记录日志，但不抛出。
// 供无法抛异常、只能把错误回传给 JIT 边界的路径使用，保证两条路径的信息一致。
std::string BuildFakeluaErrorMessage(const std::string &msg);

}// namespace fakelua
