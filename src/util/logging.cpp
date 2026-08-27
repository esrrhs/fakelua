#include "common.h"

#include <cinttypes>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>

namespace fakelua {

namespace {

// 默认日志格式
constexpr const char *kTimeFormat = "%Y-%m-%d %H:%M:%S";

// 级别名称
const char *LevelName(LogLevel level) {
    switch (level) {
    case LogLevel::Trace: return "TRACE";
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO ";
    case LogLevel::Warn: return "WARN ";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Critical: return "CRIT ";
    case LogLevel::Off: return "OFF  ";
    }
    return "INFO ";
}

// 级别对应的输出流
std::ostream &LevelStream(LogLevel level) {
    return (level >= LogLevel::Error) ? std::cerr : std::cout;
}

// 全局状态
struct LoggerState {
    std::mutex mutex;
    LogLevel level = LogLevel::Info;
    std::ofstream file;
    std::string file_path;
    size_t max_size = 10 * 1024 * 1024;
    size_t max_files = 5;

    static LoggerState &Get() {
        static LoggerState instance;
        return instance;
    }
};

// 获取当前时间字符串
std::string CurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &now_time_t);
#else
    localtime_r(&now_time_t, &tm_buf);
#endif

    char time_buf[64];
    std::strftime(time_buf, sizeof(time_buf), kTimeFormat, &tm_buf);
    char result[80];
    std::snprintf(result, sizeof(result), "%s.%03" PRId64, time_buf, static_cast<int64_t>(ms.count()));
    return result;
}

// 滚动日志文件
void RotateLogs(const std::string &path, size_t max_files) {
    // 删除最旧的文件
    std::string oldest = std::format("{}.{}", path, max_files);
    std::error_code ec;
    std::filesystem::remove(oldest, ec);

    // 将现有文件向后移动
    for (size_t i = max_files - 1; i >= 1; --i) {
        std::string old_name = std::format("{}.{}", path, i);
        std::string new_name = std::format("{}.{}", path, i + 1);
        std::filesystem::rename(old_name, new_name, ec);
    }

    // 将当前文件移动为 .1
    std::filesystem::rename(path, std::format("{}.1", path), ec);
}

}  // namespace

void InitLogger() {
    // LoggerState::Get() 会自动初始化
}

void SetLogLevel(LogLevel level) {
    auto &state = LoggerState::Get();
    std::lock_guard<std::mutex> lock(state.mutex);
    state.level = level;
}

void SetLogFile(const std::string &path, size_t max_size, size_t max_files) {
    auto &state = LoggerState::Get();
    std::lock_guard<std::mutex> lock(state.mutex);

    state.file_path = path;
    state.max_size = max_size;
    state.max_files = max_files;

    // 如果文件已存在且超过大小限制，先滚动
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        auto size = std::filesystem::file_size(path, ec);
        if (size >= max_size) {
            RotateLogs(path, max_files);
        }
    }

    state.file.open(path, std::ios::app);
    if (!state.file.is_open()) {
        std::cerr << "Failed to open log file: " << path << std::endl;
    }
}

bool CheckLogLevel(LogLevel level) {
    return level >= LoggerState::Get().level;
}

void Log(LogLevel level, const std::string_view &tag, const std::string_view &message,
         const std::source_location &source) {
    auto &state = LoggerState::Get();

    // 级别检查：不满足直接跳过，不做任何格式化
    if (level < state.level) {
        return;
    }

    std::lock_guard<std::mutex> lock(state.mutex);

    std::string tag_str(tag.empty() ? "none" : std::string(tag));
    std::string msg_str(message);

    // Trace/Debug 级别显示源文件位置
    if (level <= LogLevel::Debug) {
        msg_str = std::format("{} ({}:{}:{})", msg_str, source.file_name(), source.line(), source.column());
    }

    // 格式化时间戳
    std::string time_str = CurrentTime();

    // 输出到控制台
    auto &stream = LevelStream(level);
    stream << std::format("[{}] [{}] [{}] {}", time_str, LevelName(level), tag_str, msg_str) << std::endl;

    // 输出到文件
    if (state.file.is_open()) {
        state.file << std::format("[{}] [{}] [{}] {}", time_str, LevelName(level), tag_str, msg_str) << std::endl;
        state.file.flush();

        // 检查文件大小，超过限制则滚动
        auto current_pos = state.file.tellp();
        if (static_cast<size_t>(current_pos) >= state.max_size) {
            state.file.close();
            RotateLogs(state.file_path, state.max_files);
            state.file.open(state.file_path, std::ios::app);
        }
    }
}

}// namespace fakelua
