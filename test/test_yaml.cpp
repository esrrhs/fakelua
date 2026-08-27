#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_yaml, decode_scalar_int) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_decode_scalar_int", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, decode_scalar_float) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_decode_scalar_float", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, decode_scalar_bool) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_decode_scalar_bool", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, decode_scalar_null) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_decode_scalar_null", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, decode_scalar_string) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_decode_scalar_string", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, decode_map) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_decode_map", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, decode_array) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_decode_array", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, decode_nested) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_decode_nested", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, decode_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_decode_error", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, encode_scalar) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_encode_scalar", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, encode_map) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_encode_map", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_yaml, roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./yaml/test_yaml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "YamlTest.test_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
