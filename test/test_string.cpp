#include <gtest/gtest.h>

#include "fakelua.h"

using namespace fakelua;

TEST(test_string, test_string_len) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_len.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_len", res);
        EXPECT_EQ(res, 100);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_sub) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_sub.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_sub", res);
        EXPECT_EQ(res, 200);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_rep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_rep.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_rep", res);
        EXPECT_EQ(res, 300);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_case) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_case.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_case", res);
        EXPECT_EQ(res, 400);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_byte_char) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_byte_char.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_byte_char", res);
        EXPECT_EQ(res, 500);
    }

    FakeluaDeleteState(s);
}
