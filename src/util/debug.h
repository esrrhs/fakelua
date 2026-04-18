#pragma once

#include <source_location>

namespace fakelua {

// 一个简单的断言系统，仅用于调试。仅在 mingw 下生效
[[noreturn]] void DebugAssertFail(const std::string &str, const std::source_location &source = std::source_location::current());

#if defined(_WIN32) && defined(_DEBUG)
#define DEBUG_ASSERT(x)                                                                                                                    \
    if (!(x)) {                                                                                                                            \
        fakelua::DebugAssertFail(#x);                                                                   \
    }
#else
#define DEBUG_ASSERT(x)
#endif

}// namespace fakelua