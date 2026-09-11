#pragma once

#include <source_location>
#include <string>
#include <string_view>

namespace fakelua {

class State;

// 日志级别
enum class LogLevel : int {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Critical = 5,
    Off = 6,
};

// 一个日志输出目标（文件 + 轮转配置），完整定义在 logging.cpp 里。每个 State 各有一份。
class LogSink;

// 新建 / 销毁一个独立的输出目标
LogSink *CreateLogSink(const std::string &path, size_t max_size, size_t max_files);
void DestroyLogSink(LogSink *sink);

// 设置本 State 的最低日志级别。s 为 nullptr 时无操作。
void SetLogLevel(State *s, LogLevel level);

// 设置本 State 的日志文件。path 为空则关掉文件输出，只剩控制台。
void SetLogFile(State *s, const std::string &path, size_t max_size = 10 * 1024 * 1024, size_t max_files = 5);

// 检查本 State 是否启用某级别。s 为 nullptr 时按 Info 判断（给没有 State 的异常路径用）。
bool CheckLogLevel(State *s, LogLevel level);

// 核心日志函数。有 StateConfig::log_file / log.set_file 就写那个文件，否则只打控制台。
// s 为 nullptr 表示没有关联的 State（例如抛异常时），只打控制台。
void Log(State *s, LogLevel level, const std::string_view &tag, const std::string_view &message, const std::source_location &source = std::source_location::current());

// Lua 侧日志函数（需手动传入源文件位置信息）
void LogLua(State *s, LogLevel level, const std::string_view &tag, const std::string_view &message, const std::string_view &source_file, int source_line, const std::string_view &function_name);

// 便捷宏 — 第一个参数是 State*
#define LOG_TRACE(s, tag, fmt, ...)                                                                                                                                                                    \
    do {                                                                                                                                                                                               \
        if (fakelua::CheckLogLevel((s), fakelua::LogLevel::Trace)) {                                                                                                                                   \
            fakelua::Log((s), fakelua::LogLevel::Trace, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                        \
        }                                                                                                                                                                                              \
    } while (0)
#define LOG_DEBUG(s, tag, fmt, ...)                                                                                                                                                                    \
    do {                                                                                                                                                                                               \
        if (fakelua::CheckLogLevel((s), fakelua::LogLevel::Debug)) {                                                                                                                                   \
            fakelua::Log((s), fakelua::LogLevel::Debug, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                        \
        }                                                                                                                                                                                              \
    } while (0)
#define LOG_INFO(s, tag, fmt, ...)                                                                                                                                                                     \
    do {                                                                                                                                                                                               \
        if (fakelua::CheckLogLevel((s), fakelua::LogLevel::Info)) {                                                                                                                                    \
            fakelua::Log((s), fakelua::LogLevel::Info, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                         \
        }                                                                                                                                                                                              \
    } while (0)
#define LOG_WARN(s, tag, fmt, ...)                                                                                                                                                                     \
    do {                                                                                                                                                                                               \
        if (fakelua::CheckLogLevel((s), fakelua::LogLevel::Warn)) {                                                                                                                                    \
            fakelua::Log((s), fakelua::LogLevel::Warn, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                         \
        }                                                                                                                                                                                              \
    } while (0)
#define LOG_ERROR(s, tag, fmt, ...)                                                                                                                                                                    \
    do {                                                                                                                                                                                               \
        if (fakelua::CheckLogLevel((s), fakelua::LogLevel::Error)) {                                                                                                                                   \
            fakelua::Log((s), fakelua::LogLevel::Error, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                        \
        }                                                                                                                                                                                              \
    } while (0)
#define LOG_CRITICAL(s, tag, fmt, ...)                                                                                                                                                                 \
    do {                                                                                                                                                                                               \
        if (fakelua::CheckLogLevel((s), fakelua::LogLevel::Critical)) {                                                                                                                                \
            fakelua::Log((s), fakelua::LogLevel::Critical, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                     \
        }                                                                                                                                                                                              \
    } while (0)

}// namespace fakelua
