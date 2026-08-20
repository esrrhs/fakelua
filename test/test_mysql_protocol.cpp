#include "fakelua.h"
#include "gtest/gtest.h"
#include "native/mysql/mysql_protocol.h"

using namespace fakelua;
using namespace fakelua::mysql;

// ── caching_sha2_password 单元测试 ──

// 验证 SHA256-based auth hash 长度正确
TEST(test_mysql_protocol, caching_sha2_password_hash_len) {
    std::string password = "test";
    std::string scramble(20, 'A');  // 20 bytes scramble
    auto hash = caching_sha2_password_hash(password, scramble);
    EXPECT_EQ(hash.size(), 32);  // SHA256 = 32 bytes
}

// 相同输入产生相同输出
TEST(test_mysql_protocol, caching_sha2_password_hash_deterministic) {
    std::string password = "hello";
    std::string scramble(20, 'X');
    auto hash1 = caching_sha2_password_hash(password, scramble);
    auto hash2 = caching_sha2_password_hash(password, scramble);
    EXPECT_EQ(hash1, hash2);
}

// 不同密码产生不同 hash
TEST(test_mysql_protocol, caching_sha2_password_hash_different) {
    std::string scramble(20, 'A');
    auto hash1 = caching_sha2_password_hash("pass1", scramble);
    auto hash2 = caching_sha2_password_hash("pass2", scramble);
    EXPECT_NE(hash1, hash2);
}

// scramble 长度不为 20 应该抛异常
TEST(test_mysql_protocol, caching_sha2_password_hash_invalid_scramble) {
    EXPECT_THROW(caching_sha2_password_hash("test", "short"), std::runtime_error);
}

// ── 预处理语句协议测试 ──

// 解析 COM_STMT_PREPARE 响应
TEST(test_mysql_protocol, parse_prepare_response) {
    // status(1) + stmt_id(4) + num_cols(2) + num_params(2) + filler(1) + warnings(2)
    std::vector<char> payload;
    payload.push_back(0x00);  // status OK
    // statement_id = 1
    payload.push_back(0x01); payload.push_back(0x00); payload.push_back(0x00); payload.push_back(0x00);
    // num_columns = 2
    payload.push_back(0x02); payload.push_back(0x00);
    // num_params = 1
    payload.push_back(0x01); payload.push_back(0x00);
    // filler
    payload.push_back(0x00);
    // warnings = 0
    payload.push_back(0x00); payload.push_back(0x00);

    auto result = parse_prepare_response(payload);
    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.statement_id, 1);
    EXPECT_EQ(result.num_columns, 2);
    EXPECT_EQ(result.num_params, 1);
    EXPECT_EQ(result.num_warnings, 0);
}

// 构建 COM_STMT_EXECUTE 包
TEST(test_mysql_protocol, build_stmt_execute) {
    std::vector<std::string> params;
    params.push_back("hello");
    params.push_back("world");
    auto payload = build_stmt_execute(1, params);

    // 验证结构: cmd(1) + stmt_id(4) + flags(1) + iter(4) + null_bitmap(1) + bind_flag(1) + types(4) + values
    EXPECT_GE(payload.size(), 20);  // 至少头部
    EXPECT_EQ(static_cast<uint8_t>(payload[0]), COM_STMT_EXECUTE);
    // statement_id = 1 (LE uint32 at offset 1)
    EXPECT_EQ(static_cast<uint8_t>(payload[1]), 0x01);
    // flags = 0 (offset 5)
    EXPECT_EQ(static_cast<uint8_t>(payload[5]), 0x00);
    // iteration_count = 1 (LE uint32 at offset 6)
    EXPECT_EQ(static_cast<uint8_t>(payload[6]), 0x01);
}

// 无参数时也能构建
TEST(test_mysql_protocol, build_stmt_execute_no_params) {
    std::vector<std::string> params;
    auto payload = build_stmt_execute(42, params);
    EXPECT_GE(payload.size(), 10);
    EXPECT_EQ(static_cast<uint8_t>(payload[0]), COM_STMT_EXECUTE);
    // statement_id = 42
    EXPECT_EQ(static_cast<uint8_t>(payload[1]), 42);
}

// 解析二进制行
TEST(test_mysql_protocol, parse_binary_row) {
    // MySQL binary row format:
    // header(1) + null_bitmap(ceil((num_cols+2)/8)) + values...
    // null bitmap: bit (col+2) set = column col is NULL
    std::vector<char> payload;
    payload.push_back(0x00);  // packet header

    // null bitmap for 3 columns: (3+2+7)/8 = 1 byte
    // bit 2 = col 0, bit 3 = col 1, bit 4 = col 2
    // set bit 3 to make column 1 NULL: 00001000 = 0x08
    payload.push_back(0x08);

    // column 0: "abc"
    payload.push_back(0x03);  // length 3
    payload.push_back('a'); payload.push_back('b'); payload.push_back('c');

    // column 1: NULL (no value bytes)

    // column 2: "xy"
    payload.push_back(0x02);  // length 2
    payload.push_back('x'); payload.push_back('y');

    auto row = parse_binary_row(payload, 3);
    ASSERT_EQ(row.size(), 3);

    EXPECT_FALSE(row[0].first);  // not null
    EXPECT_EQ(row[0].second, "abc");

    EXPECT_TRUE(row[1].first);   // null
    EXPECT_EQ(row[1].second, "");

    EXPECT_FALSE(row[2].first);  // not null
    EXPECT_EQ(row[2].second, "xy");
}

// ── mysql_native_password 回归测试 ──

TEST(test_mysql_protocol, native_password_hash_len) {
    std::string password = "test";
    std::string scramble(20, 'A');
    auto hash = native_password_hash(password, scramble);
    EXPECT_EQ(hash.size(), 20);  // SHA1 = 20 bytes
}
