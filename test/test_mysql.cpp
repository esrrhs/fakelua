#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// ─────────────────────────────────────────────────────────────────────────────
// MySQL 模块测试
// ─────────────────────────────────────────────────────────────────────────────

// 测试 1: 连接失败时 pcall 能捕获错误（无需真实 MySQL 服务器）
TEST(test_mysql, connect_failure_catchable) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_connect_fail.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MysqlTest.test_connect_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试 2: 连接失败时错误信息包含 "connect"（验证错误传播）
TEST(test_mysql, connect_failure_message) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_error_message.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MysqlTest.test_error_message", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
