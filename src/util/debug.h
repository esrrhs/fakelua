#pragma once

#include <source_location>

namespace fakelua {

// 一个简单的断言系统，仅用于调试。断言失败时抛出异常。
[[noreturn]] void DebugAssertFail(const std::string &str, const std::source_location &source = std::source_location::current());

#define DEBUG_ASSERT(x)                                                                                                                    \
    if (!(x)) {                                                                                                                            \
        fakelua::DebugAssertFail(#x);                                                                   \
    }

}// namespace fakelua