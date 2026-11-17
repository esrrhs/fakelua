#include "fakelua.h"
#include "jit/vm.h"
#include "native/timer/heap_timer.h"
#include "native/timer/native_timer.h"
#include "test_jit.h"
#include "var/var.h"
#include "gtest/gtest.h"
#include <algorithm>

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

TEST(test_timer, heap_timer_growth_deletion_and_deadline_order) {
    timer::HeapTimer heap;
    const auto past = timer::HeapTimer::Clock::now() - std::chrono::hours(1);
    std::vector<timer::HeapTimer::TimerId> ids;
    for (int i = 127; i >= 0; --i) ids.push_back(heap.AddAt(past + std::chrono::milliseconds(i)));
    ASSERT_EQ(heap.Size(), 128u);
    EXPECT_FALSE(heap.Del(0));
    std::vector<timer::HeapTimer::TimerId> expected;
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i % 3 == 0) {
            EXPECT_TRUE(heap.Del(ids[i]));
            EXPECT_FALSE(heap.Del(ids[i]));
        } else
            expected.push_back(ids[i]);
    }
    std::reverse(expected.begin(), expected.end());
    EXPECT_EQ(heap.Update(), expected);
    EXPECT_EQ(heap.Size(), 0u);
    EXPECT_TRUE(heap.Update().empty());
    const auto future = heap.AddAt(timer::HeapTimer::Clock::now() + std::chrono::hours(24));
    EXPECT_TRUE(heap.Update().empty());
    EXPECT_EQ(heap.Size(), 1u);
    EXPECT_TRUE(heap.Del(future));
    EXPECT_EQ(heap.Size(), 0u);
}

TEST(test_timer, heap_timer_move_preserves_pending_ids_and_reuses_source) {
    timer::HeapTimer source;
    const auto past = timer::HeapTimer::Clock::now() - std::chrono::hours(1);
    const auto first = source.AddAt(past);
    const auto second = source.AddAt(past + std::chrono::seconds(1));
    timer::HeapTimer moved(std::move(source));
    EXPECT_EQ(source.Size(), 0u);
    EXPECT_EQ(moved.Size(), 2u);
    EXPECT_TRUE(moved.Del(first));
    timer::HeapTimer assigned;
    assigned.AddAt(past);
    assigned = std::move(moved);
    EXPECT_EQ(moved.Size(), 0u);
    EXPECT_EQ(assigned.Update(), std::vector<timer::HeapTimer::TimerId>{second});
    const auto reused = source.AddAt(past);
    EXPECT_EQ(source.Update(), std::vector<timer::HeapTimer::TimerId>{reused});
}

TEST(test_timer, native_callbacks_validation_and_exception_recovery) {
    FakeluaStateGuard state;
    State *s = state.GetState();
    int calls = 0;
    RegisterNativeFunction(s, "cpp_timer_record", 2, false, [&](State *st, CVar *args, int n) {
        EXPECT_EQ(n, 2);
        EXPECT_EQ(inter::CVarToString(args[0], ""), "timer");
        EXPECT_GT(inter::CVarToInteger(args[1]), 0);
        ++calls;
        return inter::NativeToFakeluaNil(st);
    });
    RegisterNativeFunction(s, "cpp_timer_throw", 2, false, [](State *, CVar *, int) -> CVar { throw std::runtime_error("timer dispatch failure"); });
    auto set = [&](const char *name) {
        CVar args[] = {inter::NativeToFakeluaInt(s, 0), inter::NativeToFakeluaString(s, name)};
        return CallByNameArgs(s, JIT_INTERP, "timer.set", 2, args);
    };
    set("cpp_timer_throw");
    EXPECT_THROW(timer::TickAll(s), std::runtime_error);
    set("cpp_timer_record");
    timer::TickAll(s);
    EXPECT_EQ(calls, 1);
    set("missing_timer_callback");
    EXPECT_NO_THROW(timer::TickAll(s));
    CompileConfig config;
    config.disable_jit[JIT_TCC] = true;
    CompileString(s, "function gcc_timer_callback(kind, id) cpp_timer_record(kind, id) end", config);
    set("gcc_timer_callback");
    timer::TickAll(s);
    EXPECT_EQ(calls, 2);
    for (const char *name: {"timer.set", "timer.set_heartbeat"}) {
        CVar args[] = {inter::NativeToFakeluaInt(s, -1), inter::NativeToFakeluaString(s, "cpp_timer_record")};
        EXPECT_THROW(CallByNameArgs(s, JIT_INTERP, name, 2, args), std::exception);
        args[0] = inter::NativeToFakeluaInt(s, 4294967296LL);
        EXPECT_THROW(CallByNameArgs(s, JIT_INTERP, name, 2, args), std::exception);
        args[0] = inter::NativeToFakeluaInt(s, 1);
        args[1] = inter::NativeToFakeluaBool(s, false);
        EXPECT_THROW(CallByNameArgs(s, JIT_INTERP, name, 2, args), std::exception);
    }
    CVar invalid = inter::NativeToFakeluaInt(s, -1);
    EXPECT_THROW(CallByNameArgs(s, JIT_INTERP, "timer.del", 1, &invalid), std::exception);
    EXPECT_THROW(CallByNameArgs(s, JIT_INTERP, "timer.register_obj_methods", 1, &invalid), std::exception);
}
