#pragma once

namespace fakelua {

// a simple assert system, just use to debug. only work in mingw
[[noreturn]] void DebugAssertFail(const std::string &str, const std::source_location &source = std::source_location::current());

#ifdef _WIN32
#define DEBUG_ASSERT(x)                                                                                                                    \
    if (!(x)) {                                                                                                                            \
        fakelua::DebugAssertFail(#x);                                                                   \
    }
#else
#define DEBUG_ASSERT(x)
#endif

}// namespace fakelua