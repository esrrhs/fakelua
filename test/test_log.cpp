#include "fakelua.h"
#include "test_jit.h"
#include "util/logging.h"
#include "gtest/gtest.h"

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace fakelua;

// 新建 State 的默认级别是 Info
TEST(test_log, init) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Trace));
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Info));
}

// 没有 State 时按 Info 判断，只打控制台
TEST(test_log, nullptr_is_info) {
    EXPECT_FALSE(CheckLogLevel(nullptr, LogLevel::Trace));
    EXPECT_FALSE(CheckLogLevel(nullptr, LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(nullptr, LogLevel::Info));
    EXPECT_TRUE(CheckLogLevel(nullptr, LogLevel::Error));
    LOG_INFO(nullptr, "test", "no state");
}

// 测试日志级别设置（本 State）
TEST(test_log, set_level) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);

    SetLogLevel(s, LogLevel::Trace);
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Trace));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Info));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Warn));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Error));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Critical));

    SetLogLevel(s, LogLevel::Off);
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Trace));
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Critical));
}

// 测试脚本侧 log.info
TEST(test_log, script_info) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_info", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试脚本侧 log.debug
TEST(test_log, script_debug) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_debug", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试脚本侧 log.warn
TEST(test_log, script_warn) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_warn", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试脚本侧 log.error
TEST(test_log, script_error) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_error", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试多参数拼接
TEST(test_log, script_multi_args) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_multi_args", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试数字格式化
TEST(test_log, script_number) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_number", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试布尔和 nil
TEST(test_log, script_bool_nil) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_bool_nil", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试 set_level
TEST(test_log, script_set_level) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_set_level", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试 pcall 兼容性
TEST(test_log, script_pcall) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_pcall", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试引擎侧日志宏
TEST(test_log, engine_macros) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetLogLevel(s, LogLevel::Debug);

    LOG_TRACE(s, "test", "trace message {}", 1);
    LOG_DEBUG(s, "test", "debug message {}", 2);
    LOG_INFO(s, "test", "info message {}", 3);
    LOG_WARN(s, "test", "warn message {}", 4);
    LOG_ERROR(s, "test", "error message {}", 5);
    LOG_CRITICAL(s, "test", "critical message {}", 6);
}

// 测试级别过滤 — Info 级别下 Debug 不输出
TEST(test_log, level_filter) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetLogLevel(s, LogLevel::Info);
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Trace));
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Info));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Warn));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Error));
}

// 测试脚本侧 log.trace
TEST(test_log, script_trace) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_trace", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试脚本侧 log.critical
TEST(test_log, script_critical) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_critical", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试脚本侧 log.set_file
TEST(test_log, script_set_file) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_set_file", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试脚本侧 log.set_level 错误参数
TEST(test_log, script_set_level_bad_arg) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_set_level_bad_arg", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试脚本侧 log.info 无参数
TEST(test_log, script_info_no_args) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./log/test_log_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "LogTest.test_log_info_no_args", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

// 测试 C++ 侧 SetLogFile 和文件输出
TEST(test_log, set_log_file) {
#ifdef _WIN32
    const char *log_path = "test_cpp_log_tmp.txt";
#else
    const char *log_path = "/tmp/test_cpp_log_tmp.txt";
#endif

    std::remove(log_path);

    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetLogFile(s, log_path, 1024 * 1024, 3);
    SetLogLevel(s, LogLevel::Trace);

    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Info));

    LOG_INFO(s, "test", "cpp file log message {}", 123);
    LOG_DEBUG(s, "test", "debug file log {}", 456);

    // 关掉文件输出，确保内容刷到磁盘
    SetLogFile(s, "", 0, 0);

    std::ifstream f(log_path);
    EXPECT_TRUE(f.is_open());
    if (f.is_open()) {
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        EXPECT_FALSE(content.empty());
        EXPECT_NE(content.find("debug file log 456"), std::string::npos);
        f.close();
    }

    std::remove(log_path);
}

// 测试每个 State 各自的日志文件（StateConfig::log_file）
TEST(test_log, per_state_log_file) {
#ifdef _WIN32
    const char *path_a = "test_log_state_a_tmp.txt";
    const char *path_b = "test_log_state_b_tmp.txt";
#else
    const char *path_a = "/tmp/test_log_state_a_tmp.txt";
    const char *path_b = "/tmp/test_log_state_b_tmp.txt";
#endif
    for (const char *p: {path_a, path_b}) {
        std::remove(p);
    }

    const auto read_all = [](const char *path) {
        std::ifstream f(path);
        if (!f.is_open()) return std::string();
        return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    };

    StateConfig cfg_a;
    cfg_a.log_file = path_a;
    StateConfig cfg_b;
    cfg_b.log_file = path_b;

    State *a = FakeluaNewState(cfg_a);
    State *b = FakeluaNewState(cfg_b);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);

    CompileConfig config;
    CompileFile(a, "./log/test_log_basic.lua", config);
    CompileFile(b, "./log/test_log_basic.lua", config);

    int64_t ret = 0;
    CallAll(a, "LogTest.test_log_info", ret);
    EXPECT_EQ(ret, 1);
    CallAll(b, "LogTest.test_log_info", ret);
    EXPECT_EQ(ret, 1);

    LOG_INFO(a, "test", "cpp from state A");
    LOG_INFO(b, "test", "cpp from state B");

    FakeluaDeleteState(a);
    FakeluaDeleteState(b);

    const std::string content_a = read_all(path_a);
    const std::string content_b = read_all(path_b);
    EXPECT_NE(content_a.find("hello from lua"), std::string::npos);
    EXPECT_NE(content_b.find("hello from lua"), std::string::npos);
    EXPECT_NE(content_a.find("cpp from state A"), std::string::npos);
    EXPECT_NE(content_b.find("cpp from state B"), std::string::npos);
    EXPECT_EQ(content_a.find("cpp from state B"), std::string::npos);
    EXPECT_EQ(content_b.find("cpp from state A"), std::string::npos);

    for (const char *p: {path_a, path_b}) {
        std::remove(p);
    }
}

// 不指定 log_file 的 State 只打控制台，不写文件
TEST(test_log, state_without_log_file_is_console_only) {
#ifdef _WIN32
    const char *sentinel = "test_log_no_file_tmp.txt";
#else
    const char *sentinel = "/tmp/test_log_no_file_tmp.txt";
#endif
    std::remove(sentinel);

    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./log/test_log_basic.lua", config);
    int64_t ret = 0;
    CallAll(s, "LogTest.test_log_info", ret);
    EXPECT_EQ(ret, 1);
    LOG_INFO(s, "test", "should not create a log file");
    FakeluaDeleteState(s);

    std::ifstream f(sentinel);
    EXPECT_FALSE(f.is_open());
    std::remove(sentinel);
}

// 两个 State 的日志级别互不影响
TEST(test_log, per_state_log_level) {
    StateConfig cfg_a;
    cfg_a.log_level = static_cast<int>(LogLevel::Trace);
    StateConfig cfg_b;
    cfg_b.log_level = static_cast<int>(LogLevel::Error);

    State *a = FakeluaNewState(cfg_a);
    State *b = FakeluaNewState(cfg_b);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);

    EXPECT_TRUE(CheckLogLevel(a, LogLevel::Trace));
    EXPECT_TRUE(CheckLogLevel(a, LogLevel::Debug));
    EXPECT_FALSE(CheckLogLevel(b, LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(b, LogLevel::Error));

    SetLogLevel(a, LogLevel::Off);
    EXPECT_FALSE(CheckLogLevel(a, LogLevel::Critical));
    EXPECT_TRUE(CheckLogLevel(b, LogLevel::Error));

    FakeluaDeleteState(a);
    FakeluaDeleteState(b);
}

namespace {

#ifdef _WIN32
constexpr const char *kLogTmpDir = ".";
#else
constexpr const char *kLogTmpDir = "/tmp";
#endif

std::string ReadLogFile(const std::string &path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        return {};
    }
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

void CompileLogIso(State *s) {
    CompileFile(s, "./log/test_log_isolation.lua", {});
}

void CallIso(State *s, const char *name, int64_t arg) {
    int64_t ret = 0;
    CallAll(s, name, ret, arg);
    if (ret != 1) {
        throw std::runtime_error(std::string(name) + " returned " + std::to_string(ret));
    }
}

void CallIso(State *s, const char *name, const char *arg) {
    int64_t ret = 0;
    CallAll(s, name, ret, arg);
    if (ret != 1) {
        throw std::runtime_error(std::string(name) + " returned " + std::to_string(ret));
    }
}

}// namespace

// 同一条线程上轮流跑两个 State：级别、文件、脚本 log.set_* 都不该串台
TEST(test_log, same_thread_states_isolated) {
    const std::string path_a = std::string(kLogTmpDir) + "/test_log_iso_st_a.txt";
    const std::string path_b = std::string(kLogTmpDir) + "/test_log_iso_st_b.txt";
    std::remove(path_a.c_str());
    std::remove(path_b.c_str());

    State *a = FakeluaNewState();
    State *b = FakeluaNewState();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    CompileLogIso(a);
    CompileLogIso(b);

    CallIso(a, "LogIso.set_file", path_a.c_str());
    CallIso(b, "LogIso.set_file", path_b.c_str());
    CallIso(a, "LogIso.set_level", static_cast<int64_t>(LogLevel::Trace));
    CallIso(b, "LogIso.set_level", static_cast<int64_t>(LogLevel::Error));

    EXPECT_TRUE(CheckLogLevel(a, LogLevel::Trace));
    EXPECT_FALSE(CheckLogLevel(b, LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(b, LogLevel::Error));

    CallIso(a, "LogIso.debug_msg", "lua-debug-A");
    CallIso(b, "LogIso.debug_msg", "lua-debug-B");
    CallIso(a, "LogIso.error_msg", "lua-error-A");
    CallIso(b, "LogIso.error_msg", "lua-error-B");

    LOG_DEBUG(a, "test", "cpp-debug-A");
    LOG_DEBUG(b, "test", "cpp-debug-B");
    LOG_ERROR(a, "test", "cpp-error-A");
    LOG_ERROR(b, "test", "cpp-error-B");

    SetDebugLogLevel(a, static_cast<int>(LogLevel::Off));
    EXPECT_FALSE(CheckLogLevel(a, LogLevel::Error));
    EXPECT_TRUE(CheckLogLevel(b, LogLevel::Error));
    LOG_ERROR(a, "test", "cpp-after-off-A");
    LOG_ERROR(b, "test", "cpp-after-off-B");

    FakeluaDeleteState(a);
    FakeluaDeleteState(b);

    const std::string content_a = ReadLogFile(path_a);
    const std::string content_b = ReadLogFile(path_b);
    EXPECT_NE(content_a.find("lua-debug-A"), std::string::npos);
    EXPECT_NE(content_a.find("lua-error-A"), std::string::npos);
    EXPECT_NE(content_a.find("cpp-debug-A"), std::string::npos);
    EXPECT_NE(content_a.find("cpp-error-A"), std::string::npos);
    EXPECT_EQ(content_a.find("cpp-after-off-A"), std::string::npos);
    EXPECT_EQ(content_a.find("-B"), std::string::npos);

    EXPECT_EQ(content_b.find("lua-debug-B"), std::string::npos);
    EXPECT_EQ(content_b.find("cpp-debug-B"), std::string::npos);
    EXPECT_NE(content_b.find("lua-error-B"), std::string::npos);
    EXPECT_NE(content_b.find("cpp-error-B"), std::string::npos);
    EXPECT_NE(content_b.find("cpp-after-off-B"), std::string::npos);
    EXPECT_EQ(content_b.find("-A"), std::string::npos);

    std::remove(path_a.c_str());
    std::remove(path_b.c_str());
}

// 多条线程各跑自己的 State：并发改级别、写文件，互不影响
TEST(test_log, multi_thread_states_isolated) {
    constexpr int kThreads = 4;
    std::vector<std::string> paths(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        paths[i] = std::format("{}/test_log_iso_mt_{}.txt", kLogTmpDir, i);
        std::remove(paths[i].c_str());
    }

    std::vector<std::string> errors(kThreads);
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i] {
            try {
                const bool verbose = (i % 2) == 0;
                StateConfig cfg;
                cfg.log_file = paths[i];
                cfg.log_level = static_cast<int>(verbose ? LogLevel::Trace : LogLevel::Error);
                State *s = FakeluaNewState(cfg);
                if (s == nullptr) {
                    errors[i] = "FakeluaNewState returned null";
                    return;
                }
                CompileLogIso(s);
                if (CheckLogLevel(s, LogLevel::Debug) != verbose) {
                    errors[i] = "initial log level leaked across states";
                    FakeluaDeleteState(s);
                    return;
                }

                const auto marker = std::to_string(i);
                CallIso(s, "LogIso.debug_msg", ("lua-debug-" + marker).c_str());
                CallIso(s, "LogIso.error_msg", ("lua-error-" + marker).c_str());
                LOG_DEBUG(s, "test", "cpp-debug-{}", i);
                LOG_ERROR(s, "test", "cpp-error-{}", i);

                SetLogLevel(s, verbose ? LogLevel::Off : LogLevel::Trace);
                if (CheckLogLevel(s, LogLevel::Debug) == verbose) {
                    errors[i] = "SetLogLevel did not stick on this state";
                }
                FakeluaDeleteState(s);
            } catch (const std::exception &e) {
                errors[i] = e.what();
            }
        });
    }
    for (auto &t: threads) {
        t.join();
    }

    for (int i = 0; i < kThreads; ++i) {
        EXPECT_TRUE(errors[i].empty()) << "thread " << i << ": " << errors[i];
        const std::string content = ReadLogFile(paths[i]);
        const auto marker = std::to_string(i);
        if ((i % 2) == 0) {
            EXPECT_NE(content.find("lua-debug-" + marker), std::string::npos);
            EXPECT_NE(content.find("cpp-debug-" + marker), std::string::npos);
        } else {
            EXPECT_EQ(content.find("lua-debug-" + marker), std::string::npos);
            EXPECT_EQ(content.find("cpp-debug-" + marker), std::string::npos);
        }
        EXPECT_NE(content.find("lua-error-" + marker), std::string::npos);
        EXPECT_NE(content.find("cpp-error-" + marker), std::string::npos);
        for (int j = 0; j < kThreads; ++j) {
            if (j == i) {
                continue;
            }
            EXPECT_EQ(content.find("lua-error-" + std::to_string(j)), std::string::npos);
            EXPECT_EQ(content.find("cpp-error-" + std::to_string(j)), std::string::npos);
        }
        std::remove(paths[i].c_str());
    }
}

// 测试 C++ 侧日志级别 Off
TEST(test_log, level_off) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetLogLevel(s, LogLevel::Off);
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Trace));
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Debug));
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Info));
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Warn));
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Error));
    EXPECT_FALSE(CheckLogLevel(s, LogLevel::Critical));
}

// 测试 C++ 侧日志级别 Trace（最低级别，全部通过）
TEST(test_log, level_trace) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetLogLevel(s, LogLevel::Trace);
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Trace));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Info));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Warn));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Error));
    EXPECT_TRUE(CheckLogLevel(s, LogLevel::Critical));
}

// 测试脚本侧 log 格式化各种类型（GCC 后端以生成覆盖率）
TEST(test_log, script_format_types) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./log/test_log_format_types.lua", config);
    int64_t ret = 0;
    CallAll(s, "LogFormatTypes.test_log_info_nil", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_info_bool", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_info_integer", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_info_float", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_info_string", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_info_mixed", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_debug_format", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_warn_format", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_error_format", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_trace_format", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_critical_format", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试脚本侧 log 错误参数（GCC 后端）
TEST(test_log, script_error_args) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./log/test_log_format_types.lua", config);
    int64_t ret = 0;
    CallAll(s, "LogFormatTypes.test_log_set_level_bad_arg", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_set_level_no_arg", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_set_file_no_arg", ret);
    EXPECT_EQ(ret, 1);
    CallAll(s, "LogFormatTypes.test_log_info_no_args", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试所有日志级别带各种参数类型（覆盖 FormatArgs）
// Normal tests use TCC backend
TEST(test_log, all_levels_with_types) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./log/test_log_all_levels.lua", config);
    int64_t ret = 0;
    // All normal tests use TCC
    CallAll(s, "LogAllLevels.test_log_trace_various_types", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_debug_various_types", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_warn_various_types", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_error_various_types", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_critical_various_types", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_info_mixed_types", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_info_booleans", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_info_nil_only", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_info_float_only", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_info_integer_only", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试日志级别切换和错误参数
// Normal tests use TCC, exception tests use GCC (TCC doesn't support C++ exception propagation)
TEST(test_log, level_transitions_and_errors) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./log/test_log_all_levels.lua", config);
    int64_t ret = 0;
    // Normal tests use TCC
    CallAll(s, "LogAllLevels.test_log_all_level_transitions", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "LogAllLevels.test_log_set_level_boundary", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    // Exception tests use GCC
    CallThrow(s, "LogAllLevels.test_log_set_level_invalid", ret);
    ret = 0;
    CallThrow(s, "LogAllLevels.test_log_set_level_no_arg", ret);
    ret = 0;
    CallThrow(s, "LogAllLevels.test_log_set_file_invalid", ret);
    ret = 0;
    CallThrow(s, "LogAllLevels.test_log_set_file_no_arg", ret);
    ret = 0;
    CallThrow(s, "LogAllLevels.test_log_info_no_args", ret);
    FakeluaDeleteState(s);
}
