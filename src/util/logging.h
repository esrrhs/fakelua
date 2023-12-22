#pragma once

#include <source_location>
#include <string_view>

namespace fakelua {

// a simple logging system, just use to debug

// log level
enum class log_level {
    Off = 0,
    Error = 1,
    Info = 2,
};

// set the log level, default is Error
void set_log_level(const log_level &level);

void log(const log_level &level, const std::string_view &message, const std::source_location &source = std::source_location::current());

#define LOG_INFO(fmt, ...) fakelua::log(fakelua::log_level::Info, std::format(fmt, ##__VA_ARGS__), std::source_location::current())
#define LOG_ERROR(fmt, ...) fakelua::log(fakelua::log_level::Error, std::format(fmt, ##__VA_ARGS__), std::source_location::current())

}// namespace fakelua
