#include "fakelua.h"
#include "gtest/gtest.h"
#include "native/mysql/mysql_connection.h"
#include "native/mysql/mysql_protocol.h"

using namespace fakelua;
using namespace fakelua::mysql;

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

// ─────────────────────────────────────────────────────────────────────────────
// MySQL 协议单元测试（无需真实 MySQL 服务器）
// ─────────────────────────────────────────────────────────────────────────────

// ── caching_sha2_password 认证测试 ──

TEST(test_mysql, caching_sha2_password_hash_len) {
    std::string password = "test";
    std::string scramble(20, 'A');
    auto hash = caching_sha2_password_hash(password, scramble);
    EXPECT_EQ(hash.size(), 32);  // SHA256 = 32 bytes
}

TEST(test_mysql, caching_sha2_password_hash_deterministic) {
    std::string password = "hello";
    std::string scramble(20, 'X');
    auto hash1 = caching_sha2_password_hash(password, scramble);
    auto hash2 = caching_sha2_password_hash(password, scramble);
    EXPECT_EQ(hash1, hash2);
}

TEST(test_mysql, caching_sha2_password_hash_different) {
    std::string scramble(20, 'A');
    auto hash1 = caching_sha2_password_hash("pass1", scramble);
    auto hash2 = caching_sha2_password_hash("pass2", scramble);
    EXPECT_NE(hash1, hash2);
}

TEST(test_mysql, caching_sha2_password_hash_invalid_scramble) {
    EXPECT_THROW(caching_sha2_password_hash("test", "short"), std::runtime_error);
}

// ── 预处理语句协议测试 ──

TEST(test_mysql, parse_prepare_response) {
    std::vector<char> payload;
    payload.push_back(0x00);  // status OK
    payload.push_back(0x01); payload.push_back(0x00); payload.push_back(0x00); payload.push_back(0x00);  // stmt_id = 1
    payload.push_back(0x02); payload.push_back(0x00);  // num_columns = 2
    payload.push_back(0x01); payload.push_back(0x00);  // num_params = 1
    payload.push_back(0x00);  // filler
    payload.push_back(0x00); payload.push_back(0x00);  // warnings = 0

    auto result = parse_prepare_response(payload);
    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.statement_id, 1);
    EXPECT_EQ(result.num_columns, 2);
    EXPECT_EQ(result.num_params, 1);
    EXPECT_EQ(result.num_warnings, 0);
}

TEST(test_mysql, build_stmt_execute) {
    std::vector<std::string> params;
    params.push_back("hello");
    params.push_back("world");
    auto payload = build_stmt_execute(1, params);

    EXPECT_GE(payload.size(), 20);
    EXPECT_EQ(static_cast<uint8_t>(payload[0]), COM_STMT_EXECUTE);
    EXPECT_EQ(static_cast<uint8_t>(payload[1]), 0x01);  // statement_id
    EXPECT_EQ(static_cast<uint8_t>(payload[5]), 0x00);  // flags
    EXPECT_EQ(static_cast<uint8_t>(payload[6]), 0x01);  // iteration_count
}

TEST(test_mysql, build_stmt_execute_no_params) {
    std::vector<std::string> params;
    auto payload = build_stmt_execute(42, params);
    EXPECT_GE(payload.size(), 10);
    EXPECT_EQ(static_cast<uint8_t>(payload[0]), COM_STMT_EXECUTE);
    EXPECT_EQ(static_cast<uint8_t>(payload[1]), 42);
}

TEST(test_mysql, parse_binary_row) {
    std::vector<char> payload;
    payload.push_back(0x00);  // packet header

    // null bitmap for 3 columns: set bit 3 to make column 1 NULL
    payload.push_back(0x08);

    // column 0: "abc"
    payload.push_back(0x03);
    payload.push_back('a'); payload.push_back('b'); payload.push_back('c');

    // column 1: NULL

    // column 2: "xy"
    payload.push_back(0x02);
    payload.push_back('x'); payload.push_back('y');

    auto row = parse_binary_row(payload, 3);
    ASSERT_EQ(row.size(), 3);

    EXPECT_FALSE(row[0].first);
    EXPECT_EQ(row[0].second, "abc");

    EXPECT_TRUE(row[1].first);
    EXPECT_EQ(row[1].second, "");

    EXPECT_FALSE(row[2].first);
    EXPECT_EQ(row[2].second, "xy");
}

// ── mysql_native_password 回归测试 ──

TEST(test_mysql, native_password_hash_len) {
    std::string password = "test";
    std::string scramble(20, 'A');
    auto hash = native_password_hash(password, scramble);
    EXPECT_EQ(hash.size(), 20);  // SHA1 = 20 bytes
}

// ─────────────────────────────────────────────────────────────────────────────
// 集成测试：需要本地 MySQL 服务（root@127.0.0.1:3306, 密码 root, 数据库 test）
// ─────────────────────────────────────────────────────────────────────────────

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
