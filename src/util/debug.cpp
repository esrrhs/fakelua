#include "common.h"

namespace fakelua {

// 一个简单的断言系统，仅用于调试。仅在 mingw 下生效
[[noreturn]] void DebugAssertFail(const std::string &str, const std::source_location &source) {
    ThrowFakeluaException(std::format("assert fail: {} at {}:{}:{}", str, source.file_name(), source.line(), source.column()));
}

}// namespace fakelua