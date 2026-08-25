#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_random, basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./random/test_random_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "RandomTest.test_random_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_random, dice) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./random/test_random_dice.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "RandomTest.test_random_dice", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_random, chance) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./random/test_random_chance.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "RandomTest.test_random_chance", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_random, weighted) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./random/test_random_weighted.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "RandomTest.test_random_weighted", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_random, deterministic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./random/test_random_deterministic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "RandomTest.test_random_deterministic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_random, save_restore) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./random/test_random_save_restore.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "RandomTest.test_random_save_restore", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_random, independent) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./random/test_random_independent.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "RandomTest.test_random_independent", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_random, destroy) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./random/test_random_destroy.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "RandomTest.test_random_destroy", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
