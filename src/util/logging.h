#pragma once

namespace fakelua {

// 一个简单的日志系统，仅用于调试

// 日志级别
enum class LogLevel {
    Off = 0,
    Error = 1,
    Info = 2,
};

// 设置日志级别，默认为 Error
void SetLogLevel(const LogLevel &level);

// 检查日志级别，如果该级别已启用则返回 true
bool CheckLogLevel(const LogLevel &level);

void Log(const LogLevel &level, const std::string_view &message, const std::source_location &source = std::source_location::current());

#define LOG_INFO(fmt, ...)                                                                                                                 \
    do {                                                                                                                                   \
        if (fakelua::CheckLogLevel(fakelua::LogLevel::Info)) {                                                                             \
            fakelua::Log(fakelua::LogLevel::Info, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                       \
        }                                                                                                                                  \
    } while (0)
#define LOG_ERROR(fmt, ...)                                                                                                                \
    do {                                                                                                                                   \
        if (fakelua::CheckLogLevel(fakelua::LogLevel::Error)) {                                                                            \
            fakelua::Log(fakelua::LogLevel::Error, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                      \
        }                                                                                                                                  \
    } while (0)

}// namespace fakelua
