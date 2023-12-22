#include "exception.h"


namespace fakelua {

// std::stacktrace::current() is not implemented in libstdc++ yet, so we have to use a fake one.
std::string stacktrace_current() {
    std::string ret;
#ifndef _WIN32
    ret.reserve(1024);
    ret += "stacktrace:\n";
    void *buffer[1024];
    int size = backtrace(buffer, 1024);
    char **strings = backtrace_symbols(buffer, size);
    for (int i = 0; i < size; ++i) {
        ret += strings[i];
        ret += "\n";
    }
    free(strings);
#endif
    return ret;
}

[[noreturn]] void throw_fakelua_exception(const std::string &msg) {
    LOG_ERROR("fakelua error: {}\n{}", msg, stacktrace_current());
    throw fakelua_exception(std::format("fakelua error: {}\n{}", msg, stacktrace_current()));
}

}// namespace fakelua
