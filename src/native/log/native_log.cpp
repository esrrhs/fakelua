#include "native/log/native_log.h"
#include "native/native_common.h"
#include "util/logging.h"

#include <string>
#include <string_view>

namespace fakelua::log {

// 脚本侧日志统一 tag
static constexpr const char *kScriptTag = "script";

// 将 Lua 参数格式化为字符串（仿 print 风格）
static std::string FormatArgs(State *s, CVar *args, int n) {
    std::string result;
    for (int i = 0; i < n; ++i) {
        if (i > 0) result += "\t";
        CVar arg = inter::GetNativeArg(s, args, n, i);

        switch (arg.type_) {
        case static_cast<int>(VarType::Nil):
            result += "nil";
            break;
        case static_cast<int>(VarType::Bool):
            result += (arg.data_.i != 0) ? "true" : "false";
            break;
        case static_cast<int>(VarType::Int):
            result += std::to_string(arg.data_.i);
            break;
        case static_cast<int>(VarType::Float): {
            char buf[64];
            // Use ::snprintf to avoid std::snprintf not being declared on MinGW
            ::snprintf(buf, sizeof(buf), "%.17g", arg.data_.f);
            result += buf;
            break;
        }
        case static_cast<int>(VarType::String):
        case static_cast<int>(VarType::StringId):
            result += inter::FakeluaToNativeString(s, arg);
            break;
        default:
            result += std::format("[{}]", static_cast<int>(arg.type_));
            break;
        }
    }
    return result;
}

// 可变参数日志函数的通用实现
static CVar LogVarArgs(State *s, CVar *args, int n, LogLevel level) {
    if (n < 1) ThrowBadArgument(1, "log.xxx", "message expected");
    std::string msg = FormatArgs(s, args, n);
    Log(level, kScriptTag, msg);
    return inter::NativeToFakeluaNil(s);
}

// log.trace(msg, ...)
static CVar log_trace(State *s, CVar *args, int n) {
    return LogVarArgs(s, args, n, LogLevel::Trace);
}

// log.debug(msg, ...)
static CVar log_debug(State *s, CVar *args, int n) {
    return LogVarArgs(s, args, n, LogLevel::Debug);
}

// log.info(msg, ...)
static CVar log_info(State *s, CVar *args, int n) {
    return LogVarArgs(s, args, n, LogLevel::Info);
}

// log.warn(msg, ...)
static CVar log_warn(State *s, CVar *args, int n) {
    return LogVarArgs(s, args, n, LogLevel::Warn);
}

// log.error(msg, ...)
static CVar log_error(State *s, CVar *args, int n) {
    return LogVarArgs(s, args, n, LogLevel::Error);
}

// log.critical(msg, ...)
static CVar log_critical(State *s, CVar *args, int n) {
    return LogVarArgs(s, args, n, LogLevel::Critical);
}

// log.set_level(level)
static CVar log_set_level(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "log.set_level", "level expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    if (a0.type_ != static_cast<int>(VarType::Int)) {
        ThrowBadArgument(1, "log.set_level", "integer expected");
    }
    auto level = static_cast<LogLevel>(a0.data_.i);
    SetLogLevel(level);
    return inter::NativeToFakeluaNil(s);
}

// log.set_file(path)
static CVar log_set_file(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "log.set_file", "path expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string path = inter::FakeluaToNativeString(s, a0);
    SetLogFile(path);
    return inter::NativeToFakeluaNil(s);
}

void RegisterLogLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "log.trace", 1, true, log_trace);
    RegisterNativeFunction(s, "log.debug", 1, true, log_debug);
    RegisterNativeFunction(s, "log.info", 1, true, log_info);
    RegisterNativeFunction(s, "log.warn", 1, true, log_warn);
    RegisterNativeFunction(s, "log.error", 1, true, log_error);
    RegisterNativeFunction(s, "log.critical", 1, true, log_critical);
    RegisterNativeFunction(s, "log.set_level", 1, false, log_set_level);
    RegisterNativeFunction(s, "log.set_file", 1, false, log_set_file);
}

}  // namespace fakelua::log
