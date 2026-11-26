#include "common.h"

#include <boost/stacktrace.hpp>

namespace fakelua {

std::string CaptureStacktrace() {
    std::string ret;
    ret.reserve(1024);
    ret += "stacktrace:\n";
    ret += boost::stacktrace::to_string(boost::stacktrace::stacktrace());
    return ret;
}

FakeluaException::FakeluaException(Parts parts) : std::runtime_error(""), parts_(std::move(parts)) {
    Rebuild();
}

void FakeluaException::Rebuild() {
    text_ = "fakelua error: " + parts_.msg;
    if (!parts_.loc.empty()) {
        text_ += " at " + parts_.loc;
    }
    if (!parts_.lua_stack.empty()) {
        text_ += "\n" + parts_.lua_stack;
    }
    if (!parts_.stack.empty()) {
        text_ += "\n" + parts_.stack;
    }
}

void FakeluaException::SetLoc(const std::string &loc) {
    if (loc.empty() || HasLoc()) {
        return;
    }
    parts_.loc = loc;
    Rebuild();
}

void FakeluaException::SetLuaStack(const std::string &lua_stack) {
    if (lua_stack.empty() || !parts_.lua_stack.empty()) {
        return;
    }
    parts_.lua_stack = lua_stack;
    Rebuild();
}

std::string LogFakeluaError(const std::string &msg, const std::string &loc, const std::string &stack) {
    const std::string head = "fakelua error: " + msg + (loc.empty() ? "" : " at " + loc);
    LOG_ERROR(nullptr, "engine", "{}\n{}", head, stack);
    return head + "\n" + stack;
}

std::string BuildFakeluaErrorMessage(const std::string &msg) {
    return BuildFakeluaErrorMessage(msg, {});
}

std::string BuildFakeluaErrorMessage(const std::string &msg, const std::string &loc) {
    return LogFakeluaError(msg, loc, CaptureStacktrace());
}

[[noreturn]] void ThrowFakeluaException(const std::string &msg) {
    ThrowFakeluaExceptionAt(msg, {});
}

[[noreturn]] void ThrowFakeluaExceptionAt(const std::string &msg, const std::string &loc, const std::string &lua_stack) {
    const auto stack = CaptureStacktrace();
    std::string logged = stack;
    if (!lua_stack.empty()) {
        logged = lua_stack;
        if (logged.back() != '\n') {
            logged += '\n';
        }
        logged += stack;
    }
    LogFakeluaError(msg, loc, logged);
    throw FakeluaException(FakeluaException::Parts{msg, loc, stack, lua_stack});
}

}// namespace fakelua
