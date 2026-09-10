#include "common.h"
#include "fakelua.h"
#include "state/state.h"
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

// ─────────────────────────────────────────────────────────────────────────────
// LogSink —— 一个日志输出目标（文件句柄 + 轮转配置 + 保护它们的锁）
//
// 每个 State 各有一份。State 是单线程的，这把锁主要防的是析构和写并发这种边角；
// 两个 State 各写各的文件不会互相阻塞。
// ─────────────────────────────────────────────────────────────────────────────
class LogSink {
public:
    void Configure(const std::string &path, size_t max_size, size_t max_files) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            file_.close();
        }
        file_path_ = path;
        max_size_ = max_size;
        max_files_ = max_files;
        if (path.empty()) {
            return;
        }

        // 如果文件已存在且超过大小限制，先滚动
        std::error_code ec;
        if (std::filesystem::exists(path, ec) && std::filesystem::file_size(path, ec) >= max_size) {
            RotateLogs(path, max_files);
        }
        file_.open(path, std::ios::app);
        if (!file_.is_open()) {
            std::cerr << "Failed to open log file: " << path << std::endl;
        }
    }

    void Write(const std::string &line) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!file_.is_open()) {
            return;
        }
        file_ << line << std::endl;
        file_.flush();

        // 检查文件大小，超过限制则滚动
        if (static_cast<size_t>(file_.tellp()) >= max_size_) {
            file_.close();
            RotateLogs(file_path_, max_files_);
            file_.open(file_path_, std::ios::app);
        }
    }

private:
    std::mutex mutex_;
    std::ofstream file_;
    std::string file_path_;
    size_t max_size_ = 10 * 1024 * 1024;
    size_t max_files_ = 5;
};

namespace {

// 控制台是全进程共享的 stdout/stderr，单独一把锁串行化，免得多个 State 的输出交错成乱码。
// 这不是日志配置，只是保护操作系统的标准流。文件输出由各自 sink 负责，和这把锁无关。
std::mutex g_console_mutex;

constexpr LogLevel kNoStateLevel = LogLevel::Info;

LogLevel LevelOf(State *s) {
    return s != nullptr ? s->GetLogLevel() : kNoStateLevel;
}

// 拼出一条日志的最终形态
std::string FormatLine(LogLevel level, const std::string_view &tag, const std::string_view &message, const std::string &loc_str) {
    const std::string_view tag_str = tag.empty() ? std::string_view("none") : tag;
    return std::format("[{}] [{}] [{}] {} {}", CurrentTime(), LevelName(level), tag_str, loc_str, message);
}

// 输出到控制台；有文件目标再写文件
void Emit(State *s, LogLevel level, const std::string &line) {
    {
        std::lock_guard<std::mutex> lock(g_console_mutex);
        LevelStream(level) << line << std::endl;
    }
    if (s != nullptr) {
        if (LogSink *sink = s->GetLogSink()) {
            sink->Write(line);
        }
    }
}

}  // namespace

// 获取本 State 的日志级别（供生成的 C 代码通过宏检查）
// 使用 C 链接，以便 TCC 生成的代码可以链接到它
extern "C" int GetLogLevel(State *s) { return static_cast<int>(LevelOf(s)); }

void SetLogLevel(State *s, LogLevel level) {
    if (s != nullptr) {
        s->SetLogLevel(level);
    }
}

void SetLogFile(State *s, const std::string &path, size_t max_size, size_t max_files) {
    if (s != nullptr) {
        s->SetLogFile(path, max_size, max_files);
    }
}

bool CheckLogLevel(State *s, LogLevel level) {
    return level >= LevelOf(s);
}

LogSink *CreateLogSink(const std::string &path, size_t max_size, size_t max_files) {
    auto *sink = new LogSink();
    sink->Configure(path, max_size, max_files);
    return sink;
}

void DestroyLogSink(LogSink *sink) {
    delete sink;
}

void Log(State *s, LogLevel level, const std::string_view &tag, const std::string_view &message,
         const std::source_location &source) {
    if (!CheckLogLevel(s, level)) {
        return;
    }
    const std::string loc_str = std::format("[{}:{}:{}]", source.file_name(), source.line(), source.function_name());
    Emit(s, level, FormatLine(level, tag, message, loc_str));
}

void LogLua(State *s, LogLevel level, const std::string_view &tag, const std::string_view &message,
            const std::string_view &source_file, int source_line, const std::string_view &function_name) {
    if (!CheckLogLevel(s, level)) {
        return;
    }
    const std::string loc_str = std::format("[{}:{}:{}]", source_file, source_line, function_name);
    Emit(s, level, FormatLine(level, tag, message, loc_str));
}

// C 接口供生成的代码调用（TCC 兼容）。s 是生成代码里的 _S。
extern "C" void FakeluaLogLua(State *s, int level, CVar msg, const char *file, int line, const char *func) {
    if (static_cast<LogLevel>(level) < LevelOf(s)) {
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

    fakelua::LogLua(s, static_cast<fakelua::LogLevel>(level), "script", msg_str, file ? file : "", line, func ? func : "");
}

}// namespace fakelua
