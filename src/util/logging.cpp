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
    // libc++ (macOS) doesn't fully support std::chrono::zoned_time yet
    // Use a simpler approach that works across platforms
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_r(&now_time_t, &tm_buf);
    char time_buf[32];
    std::strftime(time_buf, sizeof(time_buf), "%F %T", &tm_buf);
    std::string t = time_buf;
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
