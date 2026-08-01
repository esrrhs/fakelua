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

TEST(test_string, test_string_format) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_format.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_format", res);
        EXPECT_EQ(res, 600);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_dump) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_dump.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_dump", res);
        EXPECT_EQ(res, 700);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_find) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_find.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_find", res);
        EXPECT_EQ(res, 1000);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_match) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_match.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_match", res);
        EXPECT_EQ(res, 2000);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_gmatch) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_gmatch.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_gmatch", res);
        EXPECT_EQ(res, 3000);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_gsub) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_gsub.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_gsub", res);
        EXPECT_EQ(res, 4000);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_pack_unpack) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_pack_unpack.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_string_pack_unpack", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}


TEST(test_string, test_string_charpattern) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_charpattern.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_charpattern", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_loadfile) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_loadfile.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_loadfile", res);
        EXPECT_NEAR(res, 7000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_string, test_string_format_p) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./string/test_string_format_p.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_format_p", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}
