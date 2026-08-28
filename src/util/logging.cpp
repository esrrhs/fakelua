#include "common.h"
#include "fakelua.h"
#include "var/var_type.h"
#include "var/var_string.h"

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

// 获取当前日志级别（供生成的 C 代码通过宏检查）
// 使用 C 链接，以便 TCC 生成的代码可以链接到它
// 使用函数而非全局变量，避免 Windows DLL 导入需要 __declspec(dllimport) 的问题
extern "C" int GetLogLevel() { return static_cast<int>(LoggerState::Get().level); }

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

    // 构建带位置信息的消息
    // 格式: message [file:line:function]
    std::string msg_str = std::format("{} [{}:{}:{}]", message, source.file_name(), source.line(), source.function_name());

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

void LogLua(LogLevel level, const std::string_view &tag, const std::string_view &message,
            const std::string_view &source_file, int source_line, const std::string_view &function_name) {
    auto &state = LoggerState::Get();

    // 级别检查：不满足直接跳过，不做任何格式化
    if (level < state.level) {
        return;
    }

    std::lock_guard<std::mutex> lock(state.mutex);

    std::string tag_str(tag.empty() ? "none" : std::string(tag));

    // 构建带位置信息的消息
    // 格式: message [file:line:function]
    std::string msg_str = std::format("{} [{}:{}:{}]", message, source_file, source_line, function_name);

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

// C 接口供生成的代码调用（TCC 兼容）
// 接受 CVar 消息，在 C++ 侧做字符串转换和级别检查
extern "C" void FakeluaLogLua(int level, CVar msg, const char *file, int line, const char *func) {
    // 级别检查：不满足直接跳过，不做任何格式化
    if (static_cast<LogLevel>(level) < LoggerState::Get().level) {
        return;
    }

    // 将 CVar 转为字符串
    std::string msg_str;
    switch (static_cast<VarType>(msg.type_)) {
    case VarType::Nil:
        msg_str = "nil";
        break;
    case VarType::Bool:
        msg_str = (msg.data_.i != 0) ? "true" : "false";
        break;
    case VarType::Int:
        msg_str = std::to_string(msg.data_.i);
        break;
    case VarType::Float: {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.17g", msg.data_.f);
        msg_str = buf;
        break;
    }
    case VarType::String:
    case VarType::StringId:
        if (msg.data_.s) {
            msg_str = msg.data_.s->Str();
        }
        break;
    default:
        msg_str = std::format("[type:{}]", msg.type_);
        break;
    }

    fakelua::LogLua(static_cast<fakelua::LogLevel>(level), "script", msg_str, file ? file : "", line, func ? func : "");
}

}// namespace fakelua
