#include "fakelua.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

// 定时器队列挂在 State 上，CallAll 同 State 连跑会把上一后端留下的 delay-0
// 回调带进下一后端。每种后端单独建 State。
static void RunTimerScript(const char *file, const char *fn) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        EXPECT_NE(s, nullptr) << JitTypeName(jit_type);
        if (!s) {
            continue;
        }
        CompileFile(s, file, {});
        int64_t ret = 0;
        Call(s, jit_type, fn, ret);
        EXPECT_EQ(ret, 1) << fn << " jit=" << JitTypeName(jit_type);
        FakeluaDeleteState(s);
    }
}

TEST(test_timer, test_set_and_fire) {
    RunTimerScript("./timer/test_timer_set_and_fire.lua", "TimerTest.test_set_and_fire");
}

TEST(test_timer, test_del_before_fire) {
    RunTimerScript("./timer/test_timer_del_before_fire.lua", "TimerTest.test_del_before_fire");
}

TEST(test_timer, test_multiple_timers_order) {
    RunTimerScript("./timer/test_timer_multiple_order.lua", "TimerTest.test_multiple_timers_order");
}

TEST(test_timer, test_heartbeat) {
    RunTimerScript("./timer/test_timer_heartbeat.lua", "TimerTest.test_heartbeat");
}

TEST(test_timer, test_reenter) {
    RunTimerScript("./timer/test_timer_reenter.lua", "TimerTest.test_reenter");
}

TEST(test_timer, test_heartbeat_nested) {
    RunTimerScript("./timer/test_timer_heartbeat_nested.lua", "TimerTest.test_heartbeat_nested");
}

TEST(test_timer, test_nested_tick_noop) {
    RunTimerScript("./timer/test_timer_reenter.lua", "TimerTest.test_nested_tick_noop");
}
