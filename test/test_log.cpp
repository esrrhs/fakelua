#include "fakelua.h"
#include "gtest/gtest.h"
#include "util/logging.h"

#include <cstdio>
#include <fstream>

using namespace fakelua;

// 测试日志系统初始化
TEST(test_log, init) {
    InitLogger();
    SetLogLevel(LogLevel::Info);
    EXPECT_TRUE(true);
}

// 测试日志级别设置
TEST(test_log, set_level) {
    SetLogLevel(LogLevel::Trace);
    EXPECT_TRUE(CheckLogLevel(LogLevel::Trace));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Info));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Warn));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Error));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Critical));

    SetLogLevel(LogLevel::Off);
    EXPECT_FALSE(CheckLogLevel(LogLevel::Trace));
    EXPECT_FALSE(CheckLogLevel(LogLevel::Critical));

    // 恢复默认
    SetLogLevel(LogLevel::Info);
}

// 测试脚本侧 log.info
TEST(test_log, script_info) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    SetLogLevel(LogLevel::Debug);

    LOG_TRACE("test", "trace message {}", 1);
    LOG_DEBUG("test", "debug message {}", 2);
    LOG_INFO("test", "info message {}", 3);
    LOG_WARN("test", "warn message {}", 4);
    LOG_ERROR("test", "error message {}", 5);
    LOG_CRITICAL("test", "critical message {}", 6);

    EXPECT_TRUE(true);
    SetLogLevel(LogLevel::Info);
}

// 测试级别过滤 — Info 级别下 Debug 不输出
TEST(test_log, level_filter) {
    SetLogLevel(LogLevel::Info);
    EXPECT_FALSE(CheckLogLevel(LogLevel::Trace));
    EXPECT_FALSE(CheckLogLevel(LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Info));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Warn));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Error));
}

// 测试脚本侧 log.trace
TEST(test_log, script_trace) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    // 使用跨平台临时路径
#ifdef _WIN32
    const char *log_path = "test_cpp_log_tmp.txt";
#else
    const char *log_path = "/tmp/test_cpp_log_tmp.txt";
#endif

    // 删除旧文件，确保干净状态
    std::remove(log_path);

    // 设置日志文件
    SetLogFile(log_path, 1024 * 1024, 3);
    SetLogLevel(LogLevel::Trace);

    // 验证日志级别设置正确
    EXPECT_TRUE(CheckLogLevel(LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Info));

    // 写入日志
    LOG_INFO("test", "cpp file log message {}", 123);
    LOG_DEBUG("test", "debug file log {}", 456);

    // 关闭日志文件，确保内容写入磁盘
    SetLogFile("", 0, 0);

    // 验证文件存在
    std::ifstream f(log_path);
    EXPECT_TRUE(f.is_open());
    if (f.is_open()) {
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        EXPECT_FALSE(content.empty());
        // 至少 DEBUG 消息应该被写入
        EXPECT_NE(content.find("debug file log 456"), std::string::npos);
        f.close();
    }

    // 清理临时文件
    std::remove(log_path);

    // 恢复默认日志级别
    SetLogLevel(LogLevel::Info);
}

// 测试 C++ 侧日志级别 Off
TEST(test_log, level_off) {
    SetLogLevel(LogLevel::Off);
    EXPECT_FALSE(CheckLogLevel(LogLevel::Trace));
    EXPECT_FALSE(CheckLogLevel(LogLevel::Debug));
    EXPECT_FALSE(CheckLogLevel(LogLevel::Info));
    EXPECT_FALSE(CheckLogLevel(LogLevel::Warn));
    EXPECT_FALSE(CheckLogLevel(LogLevel::Error));
    EXPECT_FALSE(CheckLogLevel(LogLevel::Critical));
    // 恢复
    SetLogLevel(LogLevel::Info);
}

// 测试 C++ 侧日志级别 Trace（最低级别，全部通过）
TEST(test_log, level_trace) {
    SetLogLevel(LogLevel::Trace);
    EXPECT_TRUE(CheckLogLevel(LogLevel::Trace));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Debug));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Info));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Warn));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Error));
    EXPECT_TRUE(CheckLogLevel(LogLevel::Critical));
    // 恢复
    SetLogLevel(LogLevel::Info);
}

