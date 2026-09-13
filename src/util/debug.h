#pragma once

#include <source_location>

namespace fakelua {

// 一个简单的断言系统，仅用于调试。
// 在 Debug 模式下（未定义 NDEBUG），断言失败时抛出异常；
// 在 Release 模式下（定义了 NDEBUG），断言被完全忽略（no-op）。
[[noreturn]] void DebugAssertFail(const std::string &str, const std::source_location &source = std::source_location::current());

#ifndef NDEBUG
#define DEBUG_ASSERT(x)                                                                                                                                                                                \
    if (!(x)) {                                                                                                                                                                                        \
        fakelua::DebugAssertFail(#x);                                                                                                                                                                  \
    }
#else
#define DEBUG_ASSERT(x) ((void) 0)
#endif

}// namespace fakelua