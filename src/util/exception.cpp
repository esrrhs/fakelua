#include "common.h"

namespace fakelua {

// std::stacktrace::current() is not implemented in libstdc++ yet, so we have to use a fake one.
std::string StacktraceCurrent() {
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

[[noreturn]] void ThrowFakeluaException(const std::string &msg) {
    LOG_ERROR("fakelua error: {}\n{}", msg, StacktraceCurrent());
    throw FakeluaException(std::format("fakelua error: {}\n{}", msg, StacktraceCurrent()));
}

}// namespace fakelua
