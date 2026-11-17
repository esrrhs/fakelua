#include "fakelua.h"
#include "jit/vm.h"
#include "native/object/native_object.h"
#include "state/state.h"
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

TEST(test_redis, bad_port) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_bad_port", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_redis, bad_timeout_ms) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_bad_timeout_ms", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_redis, bad_db) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_bad_db", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// Pub/Sub 订阅族命令被拒绝（防 use-after-free，无需真实服务器）
TEST(test_redis, subscribe_rejected) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./redis/test_redis.lua", config);
    int64_t ret = 0;
    CallAll(s, "RedisTest.test_subscribe_rejected", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_redis, authentication_errors_database_selection_and_resp3_scalars) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./redis/test_redis_handshake_contracts.lua", {});
    for (auto backend: AllJitTypes()) {
        int64_t result = 0;
        Call(state.GetState(), backend, "RedisHandshake.test_handshake_contracts", result);
        EXPECT_EQ(result, 1) << JitTypeName(backend);
    }
}

TEST(test_redis, pending_connection_and_deep_reply_contracts) {
    CompileConfig config;
    config.disable_jit[JIT_TCC] = true;
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./redis/test_redis_handshake_contracts.lua", config);
    int64_t result = 0;
    Call(state.GetState(), JIT_GCC, "RedisHandshake.test_pending_connection_and_deep_reply_contracts", result);
    EXPECT_EQ(result, 1);
}

TEST(test_redis, explicit_group_destruction_unregisters_live_wrapper) {
    FakeluaStateGuard state;
    CompileString(state.GetState(), R"(function live_wrapper_factory() return redis.connect({host = "127.0.0.1", port = 6379}, "unused_callback") end)", {});
    CVar value = CallByNameArgs(state.GetState(), JIT_INTERP, "live_wrapper_factory", 0, nullptr);
    auto *object = NativeObject::Unwrap(value);
    ASSERT_NE(object, nullptr);
    const auto group = object->GetGroupId();
    EXPECT_EQ(state.GetState()->GetNativeObjectManager().DestroyGroup(group), 1u);
    EXPECT_EQ(state.GetState()->GetNativeObjectManager().DestroyGroup(group), 0u);
}
