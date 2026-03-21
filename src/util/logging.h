#pragma once

namespace fakelua {

// a simple logging system, just use to debug

// log level
enum class LogLevel {
    Off = 0,
    Error = 1,
    Info = 2,
};

// set the log level, default is Error
void SetLogLevel(const LogLevel &level);

// check the log level, return true if the level is enabled
bool CheckLogLevel(const LogLevel &level);

void Log(const LogLevel &level, const std::string_view &message, const std::source_location &source = std::source_location::current());

#define LOG_INFO(fmt, ...) if (fakelua::CheckLogLevel(fakelua::LogLevel::Info)) { fakelua::Log(fakelua::LogLevel::Info, std::format(fmt, ##__VA_ARGS__), std::source_location::current()); }
#define LOG_ERROR(fmt, ...) if (fakelua::CheckLogLevel(fakelua::LogLevel::Error)) { fakelua::Log(fakelua::LogLevel::Error, std::format(fmt, ##__VA_ARGS__), std::source_location::current()); }

}// namespace fakelua
