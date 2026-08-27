#pragma once

#include <source_location>
#include <string>
#include <string_view>

namespace fakelua {

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

// 初始化日志系统（首次调用时自动初始化，线程安全）
void InitLogger();

// 设置全局最低日志级别
void SetLogLevel(LogLevel level);

// 设置日志文件（同时输出到控制台和滚动文件）
// path: 日志文件路径
// max_size: 单个文件最大大小（默认 10MB）
// max_files: 最大保留文件数（默认 5）
void SetLogFile(const std::string &path, size_t max_size = 10 * 1024 * 1024, size_t max_files = 5);

// 检查某级别是否启用
bool CheckLogLevel(LogLevel level);

// 核心日志函数
void Log(LogLevel level, const std::string_view &tag, const std::string_view &message,
         const std::source_location &source = std::source_location::current());

// 便捷宏 — 带 tag 参数
#define LOG_TRACE(tag, fmt, ...)                                                                                                                                                                           \
    do {                                                                                                                                                                                                   \
        if (fakelua::CheckLogLevel(fakelua::LogLevel::Trace)) {                                                                                                                                            \
            fakelua::Log(fakelua::LogLevel::Trace, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                                 \
        }                                                                                                                                                                                                  \
    } while (0)
#define LOG_DEBUG(tag, fmt, ...)                                                                                                                                                                           \
    do {                                                                                                                                                                                                   \
        if (fakelua::CheckLogLevel(fakelua::LogLevel::Debug)) {                                                                                                                                            \
            fakelua::Log(fakelua::LogLevel::Debug, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                                 \
        }                                                                                                                                                                                                  \
    } while (0)
#define LOG_INFO(tag, fmt, ...)                                                                                                                                                                            \
    do {                                                                                                                                                                                                   \
        if (fakelua::CheckLogLevel(fakelua::LogLevel::Info)) {                                                                                                                                             \
            fakelua::Log(fakelua::LogLevel::Info, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                                  \
        }                                                                                                                                                                                                  \
    } while (0)
#define LOG_WARN(tag, fmt, ...)                                                                                                                                                                            \
    do {                                                                                                                                                                                                   \
        if (fakelua::CheckLogLevel(fakelua::LogLevel::Warn)) {                                                                                                                                             \
            fakelua::Log(fakelua::LogLevel::Warn, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                                  \
        }                                                                                                                                                                                                  \
    } while (0)
#define LOG_ERROR(tag, fmt, ...)                                                                                                                                                                           \
    do {                                                                                                                                                                                                   \
        if (fakelua::CheckLogLevel(fakelua::LogLevel::Error)) {                                                                                                                                            \
            fakelua::Log(fakelua::LogLevel::Error, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                                 \
        }                                                                                                                                                                                                  \
    } while (0)
#define LOG_CRITICAL(tag, fmt, ...)                                                                                                                                                                        \
    do {                                                                                                                                                                                                   \
        if (fakelua::CheckLogLevel(fakelua::LogLevel::Critical)) {                                                                                                                                         \
            fakelua::Log(fakelua::LogLevel::Critical, tag, std::format(fmt, ##__VA_ARGS__), std::source_location::current());                                                                              \
        }                                                                                                                                                                                                  \
    } while (0)

}// namespace fakelua
