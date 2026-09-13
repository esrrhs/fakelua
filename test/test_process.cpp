#include "fakelua.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_process, echo) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProcessTest.test_echo", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_process, stdin_cat) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProcessTest.test_stdin", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_process, timeout) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProcessTest.test_timeout", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_process, missing_exe) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ProcessTest.test_missing", ret);
    FakeluaDeleteState(s);
}
