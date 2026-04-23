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
    // Use portable time formatting that works across platforms
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &now_time_t);
#else
    localtime_r(&now_time_t, &tm_buf);
#endif
    char time_buf[32];
#ifdef _WIN32
    // Windows strftime doesn't support %F and %T, use explicit format
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
#else
    std::strftime(time_buf, sizeof(time_buf), "%F %T", &tm_buf);
#endif
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
