#include "fakelua.h"
#include "gtest/gtest.h"
#include "util/logging.h"

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
