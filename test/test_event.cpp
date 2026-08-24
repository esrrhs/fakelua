#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_event, on_emit) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_on_emit.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_on_emit", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_event, multiple_handlers) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_multiple_handlers.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_multiple_handlers", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_event, off) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_off.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_event_off", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_event, once) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_once.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_event_once", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_event, clear) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_clear.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_event_clear", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_event, clear_all) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_clear_all.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_event_clear_all", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_event, no_handlers) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_no_handlers.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_event_no_handlers", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_event, off_nonexistent) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_off_nonexistent.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_event_off_nonexistent", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_event, args_forward) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_args_forward.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_args_forward", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_event, reentrant) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./event/test_event_reentrant.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "EventTest.test_event_reentrant", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
