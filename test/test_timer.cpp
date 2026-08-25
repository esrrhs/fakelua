#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// ─────────────────────────────────────────────────────────────────────────────
// 全局定时器 Lua 绑定测试
// ─────────────────────────────────────────────────────────────────────────────

// 测试 1: 设置定时器并让它触发
TEST(test_timer, test_set_and_fire) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./timer/test_timer_set_and_fire.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "TimerTest.test_set_and_fire", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// 测试 2: 删除未触发的定时器
TEST(test_timer, test_del_before_fire) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./timer/test_timer_del_before_fire.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "TimerTest.test_del_before_fire", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// 测试 3: 多个定时器全部触发后自动清除
TEST(test_timer, test_multiple_timers_order) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./timer/test_timer_multiple_order.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "TimerTest.test_multiple_timers_order", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// 测试 4: 心跳注册与触发
TEST(test_timer, test_heartbeat) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./timer/test_timer_heartbeat.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "TimerTest.test_heartbeat", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

TEST(test_timer, test_reenter) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./timer/test_timer_reenter.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "TimerTest.test_reenter", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

TEST(test_timer, test_heartbeat_nested) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./timer/test_timer_heartbeat_nested.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "TimerTest.test_heartbeat_nested", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

TEST(test_timer, test_nested_tick_noop) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./timer/test_timer_reenter.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "TimerTest.test_nested_tick_noop", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}
