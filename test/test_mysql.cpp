#include "fakelua.h"
#include "gtest/gtest.h"
#include "native/mysql/mysql_connection.h"

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

// 集成测试：需要本地 MySQL 服务（root@127.0.0.1:3306, 密码 root, 数据库 test）
TEST(test_mysql, integration_callback_api) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_integration.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MysqlTest.test_mysql_integration", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：预处理语句
TEST(test_mysql, integration_prepared_statements) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_stmt.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MysqlTest.test_stmt", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：多结果集
TEST(test_mysql, integration_multi_result) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_multi_result.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MysqlTest.test_multi_result", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：连接池
TEST(test_mysql, integration_pool) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./mysql/test_mysql_pool.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MysqlTest.test_pool", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 错误分类单元测试
TEST(test_mysql, error_classification) {
    // Test classify_error_code via the public is_retryable interface
    using namespace fakelua::mysql;
    EXPECT_TRUE(MysqlConnection::is_retryable(MysqlErrorType::Connection));
    EXPECT_TRUE(MysqlConnection::is_retryable(MysqlErrorType::Timeout));
    EXPECT_FALSE(MysqlConnection::is_retryable(MysqlErrorType::Authentication));
    EXPECT_FALSE(MysqlConnection::is_retryable(MysqlErrorType::Syntax));
    EXPECT_FALSE(MysqlConnection::is_retryable(MysqlErrorType::Server));
    EXPECT_FALSE(MysqlConnection::is_retryable(MysqlErrorType::Protocol));
    EXPECT_FALSE(MysqlConnection::is_retryable(MysqlErrorType::Unknown));
}
