#pragma once

#include <source_location>
#include <string>

namespace fakelua {

// a simple assert system, just use to debug. only work in mingw
[[noreturn]] void debug_assert_fail(const std::string &str, const std::source_location &source = std::source_location::current());

#ifdef _WIN32
#define DEBUG_ASSERT(x)                                                                                                                    \
    if (!(x)) {                                                                                                                            \
        fakelua::debug_assert_fail(#x);                                                                   \
    }
#else
#define DEBUG_ASSERT(x)
#endif

}// namespace fakelua