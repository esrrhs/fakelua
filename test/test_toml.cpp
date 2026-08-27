#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_toml, decode_int) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./toml/test_toml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "TomlTest.test_decode_int", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_toml, decode_float) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./toml/test_toml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "TomlTest.test_decode_float", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_toml, decode_bool) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./toml/test_toml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "TomlTest.test_decode_bool", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_toml, decode_string) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./toml/test_toml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "TomlTest.test_decode_string", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_toml, decode_array) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./toml/test_toml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "TomlTest.test_decode_array", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_toml, decode_table) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./toml/test_toml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "TomlTest.test_decode_table", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_toml, decode_error) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./toml/test_toml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "TomlTest.test_decode_error", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_toml, encode_basic) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./toml/test_toml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "TomlTest.test_encode_basic", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_toml, roundtrip) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./toml/test_toml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "TomlTest.test_roundtrip", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}
