#include "common.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace fakelua {

namespace {

// 默认日志格式
constexpr const char *kLogPattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%L%$] [%-7s] %v";

// 将 LogLevel 映射到 spdlog::level
spdlog::level::level_enum ToSpdlogLevel(LogLevel level) {
    switch (level) {
    case LogLevel::Trace: return spdlog::level::trace;
    case LogLevel::Debug: return spdlog::level::debug;
    case LogLevel::Info: return spdlog::level::info;
    case LogLevel::Warn: return spdlog::level::warn;
    case LogLevel::Error: return spdlog::level::err;
    case LogLevel::Critical: return spdlog::level::critical;
    case LogLevel::Off: return spdlog::level::off;
    }
    return spdlog::level::info;
}

// 将 spdlog::level 映射回 LogLevel
LogLevel FromSpdlogLevel(spdlog::level::level_enum level) {
    switch (level) {
    case spdlog::level::trace: return LogLevel::Trace;
    case spdlog::level::debug: return LogLevel::Debug;
    case spdlog::level::info: return LogLevel::Info;
    case spdlog::level::warn: return LogLevel::Warn;
    case spdlog::level::err: return LogLevel::Error;
    case spdlog::level::critical: return LogLevel::Critical;
    case spdlog::level::off: return LogLevel::Off;
    default: return LogLevel::Info;
    }
}

// 获取或创建全局 logger（线程安全，spdlog 内部保证）
std::shared_ptr<spdlog::logger> GetLogger() {
    auto logger = spdlog::get("fakelua");
    if (!logger) {
        // 默认：带颜色的控制台输出
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern(kLogPattern);
        console_sink->set_level(spdlog::level::trace);  // 控制台接受所有级别，由 logger 级别统一过滤

        logger = std::make_shared<spdlog::logger>("fakelua", console_sink);
        logger->set_level(spdlog::level::info);  // 默认 Info 级别
        logger->flush_on(spdlog::level::err);    // Error 及以上自动 flush
        spdlog::register_logger(logger);
    }
    return logger;
}

}  // namespace

void InitLogger() {
    GetLogger();
}

void SetLogLevel(LogLevel level) {
    GetLogger()->set_level(ToSpdlogLevel(level));
}

void SetLogFile(const std::string &path, size_t max_size, size_t max_files) {
    auto logger = GetLogger();

    try {
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, max_size, max_files);
        file_sink->set_pattern(kLogPattern);
        file_sink->set_level(spdlog::level::trace);  // 文件接受所有级别，由 logger 级别统一过滤
        logger->sinks().push_back(file_sink);
    } catch (const spdlog::spdlog_ex &ex) {
        // 文件创建失败时回退到仅控制台
        std::cerr << "Failed to create log file: " << ex.what() << std::endl;
    }
}

bool CheckLogLevel(LogLevel level) {
    return ToSpdlogLevel(level) >= GetLogger()->level();
}

void Log(LogLevel level, const std::string_view &tag, const std::string_view &message, const std::source_location &source) {
    auto logger = GetLogger();
    auto spd_level = ToSpdlogLevel(level);

    // 级别检查：不满足直接跳过，不做任何格式化
    if (spd_level < logger->level()) {
        return;
    }

    // 格式化 tag 和消息
    std::string tag_str(tag.empty() ? "none" : std::string(tag));
    std::string msg_str(message);

    // 添加源文件位置（Trace/Debug 级别显示）
    if (spd_level <= spdlog::level::debug) {
        msg_str = std::format("{} ({}:{}:{})", msg_str, source.file_name(), source.line(), source.column());
    }

    logger->log(spd_level, "[{}] {}", tag_str, msg_str);
}

}// namespace fakelua
