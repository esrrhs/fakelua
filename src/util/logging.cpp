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
#ifdef __APPLE__
    // Apple Clang libc++ does not support C++20 time zone facilities
    auto now_tp = std::chrono::system_clock::now();
    auto t_time_t = std::chrono::system_clock::to_time_t(now_tp);
    char t_buf[64] = {};
    std::strftime(t_buf, sizeof(t_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t_time_t));
    std::string t = t_buf;
#else
    auto tz = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()};
    std::string t = std::format("{:%F %T %Z}", tz);
#endif
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
