#include "common.h"

namespace fakelua {

// a simple assert system, just use to debug. only work in mingw
[[noreturn]] void DebugAssertFail(const std::string &str, const std::source_location &source) {
    ThrowFakeluaException(std::format("assert fail: {} at {}:{}:{}", str, source.file_name(), source.line(), source.column()));
}

}// namespace fakelua