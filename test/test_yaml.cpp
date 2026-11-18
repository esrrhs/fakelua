#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_yaml, decode_scalar_int) {
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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
    for (auto jit_type: AllJitTypes()) {
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

TEST(test_yaml, decode_cycle) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_cycle", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, decode_too_deep) {
    for (auto jit_type: AllJitTypes()) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./yaml/test_yaml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "YamlTest.test_decode_too_deep", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_yaml, scalar_encoding_and_quoted_nesting_tokens) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./yaml/test_yaml_encoding_boundaries.lua", {});
    for (auto backend: AllJitTypes()) {
        int64_t result = 0;
        Call(state.GetState(), backend, "test_yaml_scalar_encoding", result);
        EXPECT_EQ(result, 1) << JitTypeName(backend);
    }
}

TEST(test_yaml, encode_cycles_depth_and_unsupported_values) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./yaml/test_yaml_encoding_boundaries.lua", {});
    for (auto backend: AllJitTypes()) {
        int64_t result = 0;
        Call(state.GetState(), backend, "test_yaml_encoding_rejections", result);
        EXPECT_EQ(result, 1) << JitTypeName(backend);
    }
}
