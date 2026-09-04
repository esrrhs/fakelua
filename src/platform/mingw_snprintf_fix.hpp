// mingw_snprintf_fix.hpp - Workaround for MinGW GCC 16.2.0 std::snprintf issue
// MinGW doesn't put snprintf in std namespace even with __USE_MINGW_ANSI_STDIO=1.
// This header forces snprintf into std namespace when included early.

#pragma once

#if defined(__MINGW32__) && defined(__GNUC__) && (__GNUC__ >= 16)
#include <cstdio>
#include <cwchar>

namespace std {
    // Explicitly bring snprintf and vsnprintf into std namespace
    using ::snprintf;
    using ::vsnprintf;
    using ::swprintf;
    using ::vswprintf;
}
#endif
