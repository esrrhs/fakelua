#pragma once

#include <stdexcept>
#include <string>

namespace fakelua {

class FakeluaException : public std::runtime_error {
public:
    explicit FakeluaException(const std::string &msg) : std::runtime_error(msg) {
    }
};

[[noreturn]] void ThrowFakeluaException(const std::string &msg);

// 构造 FakeluaException 的错误文本（追加当前堆栈）并记录日志，但不抛出。
// 供无法抛异常、只能把错误回传给 JIT 边界的路径使用，保证两条路径的信息一致。
std::string BuildFakeluaErrorMessage(const std::string &msg);

}// namespace fakelua
