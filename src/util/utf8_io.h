#pragma once

// UTF-8 file/env/process helpers via Boost.Nowide.
// On Windows this uses the wide APIs; on POSIX it is a passthrough.
// Lua names stay the same — only the C++ path is replaced.

#include <boost/nowide/cstdio.hpp>
#include <boost/nowide/cstdlib.hpp>
#include <boost/nowide/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <mutex>

namespace fakelua::utf8_io {

inline void Init() {
    static std::once_flag once;
    std::call_once(once, []() { boost::nowide::nowide_filesystem(); });
}

inline FILE *Fopen(const char *path, const char *mode) {
    return boost::nowide::fopen(path, mode);
}

inline int System(const char *cmd) {
    return boost::nowide::system(cmd);
}

inline const char *Getenv(const char *name) {
    return boost::nowide::getenv(name);
}

using ifstream = boost::nowide::ifstream;
using ofstream = boost::nowide::ofstream;

}// namespace fakelua::utf8_io
