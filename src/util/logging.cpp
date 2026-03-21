#include "common.h"

namespace fakelua {

auto g_log_level = LogLevel::Error;

void SetLogLevel(const LogLevel &level) {
    g_log_level = level;
}

bool CheckLogLevel(const LogLevel &level) {
    return level <= g_log_level;
}

void Log(const LogLevel &level, const std::string_view &message, const std::source_location &source) {
    auto tz = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()};
    std::string t = std::format("{:%F %T %Z}", tz);
    std::string s = std::format("{}:{}:{}", source.file_name(), source.line(), source.column());
    std::string l = level == LogLevel::Error ? "ERROR" : "INFO";
    const auto line = std::format("[{}] {} | {} | {}", l, t, s, message);
    if (level == LogLevel::Error) {
        std::cerr << line << std::endl;
    } else {
        std::cout << line << std::endl;
    }
}

}// namespace fakelua
