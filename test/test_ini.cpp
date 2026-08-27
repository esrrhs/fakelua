#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_ini, decode_basic) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./ini/test_ini_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "IniTest.test_decode_basic", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_ini, decode_multiple_sections) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./ini/test_ini_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "IniTest.test_decode_multiple_sections", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_ini, decode_types) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./ini/test_ini_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "IniTest.test_decode_types", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_ini, decode_empty) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./ini/test_ini_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "IniTest.test_decode_empty", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_ini, encode_basic) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./ini/test_ini_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "IniTest.test_encode_basic", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_ini, roundtrip) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./ini/test_ini_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "IniTest.test_roundtrip", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}
