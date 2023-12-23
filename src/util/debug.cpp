#include "common.h"

namespace fakelua {

// a simple assert system, just use to debug. only work in mingw
[[noreturn]] void debug_assert_fail(const std::string &str, const std::source_location &source) {
    throw_fakelua_exception(std::format("assert fail: {} at {}:{}:{}", str, source.file_name(), source.line(), source.column()));
}

}// namespace fakelua