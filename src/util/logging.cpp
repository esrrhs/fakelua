#include "logging.h"
#include "common.h"

namespace fakelua {

log_level g_log_level = log_level::Error;

void set_log_level(const log_level &level) {
    g_log_level = level;
}

void log(const log_level &level, const std::string_view &message, const std::source_location &source) {
    if (level > g_log_level) {
        return;
    }
    auto tz = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()};
    std::string t = std::format("{:%F %T %Z}", tz);
    std::string s = std::format("{}:{}:{}", source.file_name(), source.line(), source.column());
    std::string l = level == log_level::Error ? "ERROR" : "INFO";
    auto line = std::format("[{}] {} | {} | {}", l, t, s, message);
    if (level == log_level::Error) {
        std::cerr << line << std::endl;
    } else {
        std::cout << line << std::endl;
    }
}

}// namespace fakelua
