#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_yaml, decode_scalar_int) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_scalar_int", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, decode_scalar_float) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_scalar_float", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, decode_scalar_bool) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_scalar_bool", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, decode_scalar_null) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_scalar_null", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, decode_scalar_string) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_scalar_string", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, decode_map) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_map", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, decode_array) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_array", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, decode_nested) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_nested", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, decode_error) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_error", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, encode_scalar) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_encode_scalar", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, encode_map) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_encode_map", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, roundtrip) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_roundtrip", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}
