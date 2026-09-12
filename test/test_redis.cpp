#include "fakelua.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

// Redis 模块测试（对齐 MySQL：失败路径无需服务器，集成测试连 127.0.0.1:6379）

// 测试 1: 连接失败时回调能收到错误（无需真实 Redis 服务器）
TEST(test_redis, connect_failure_catchable) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_connect_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试 2: 连接失败时错误信息包含 connect/refus/unable/fail 等关键字
TEST(test_redis, connect_failure_message) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis_error_message.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_error_message", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_redis, close_in_connect_callback) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_close_in_connect_cb", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：需要本地 Redis（127.0.0.1:6379，无密码）

TEST(test_redis, integration_ping_set_get) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis_integration.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_redis_integration", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：命令错误后连接仍可继续使用
TEST(test_redis, integration_command_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis_cmd_error.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_cmd_error", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：nil / 整数 / 数组 / map 的 RESP3 映射
TEST(test_redis, integration_datatypes) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis_types.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_types", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 集成测试：重复关闭幂等与关闭后 command 防护
TEST(test_redis, integration_lifecycle) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis_lifecycle.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_lifecycle", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
