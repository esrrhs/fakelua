#include <gtest/gtest.h>

#include "fakelua.h"

using namespace fakelua;

TEST(test_utf8, test_utf8_char) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_char.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_char", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_codepoint) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_codepoint.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_codepoint", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_len) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_len.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_len", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_offset) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_offset.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_offset", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_charpattern) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_charpattern.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_charpattern", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_codes) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_codes.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_codes", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_char_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_char_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_char_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_codepoint_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_codepoint_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_codepoint_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_len_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_len_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_len_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_offset_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_offset_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_offset_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_codes_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./utf8/test_utf8_codes_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_utf8_codes_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_utf8, test_utf8_boundary_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./utf8/test_utf8_boundary_error.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_utf8_char_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_utf8_codepoint_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_utf8_len_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_utf8_offset_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}
