#include "common.h"

#include <boost/stacktrace.hpp>

namespace fakelua {

std::string StacktraceCurrent() {
    std::string ret;
    ret.reserve(1024);
    ret += "stacktrace:\n";
    ret += boost::stacktrace::to_string(boost::stacktrace::stacktrace());
    return ret;
}

std::string BuildFakeluaErrorMessage(const std::string &msg) {
    const auto stack = StacktraceCurrent();
    LOG_ERROR(nullptr, "engine", "fakelua error: {}\n{}", msg, stack);
    return std::format("fakelua error: {}\n{}", msg, stack);
}

[[noreturn]] void ThrowFakeluaException(const std::string &msg) {
    throw FakeluaException(BuildFakeluaErrorMessage(msg));
}

}// namespace fakelua
