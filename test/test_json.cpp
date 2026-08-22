#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_json, decode_null) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_null", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_bool) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_bool", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_int) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_int", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_negative) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_float) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_float", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_string) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_string", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_string_escape) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_string_escape", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_array) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_array", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_object) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_object", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_nested) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_nested", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_null) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_null", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_bool) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_bool", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_int) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_int", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_float) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_float", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_string) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_string", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_array) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_array", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_object) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_object", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
