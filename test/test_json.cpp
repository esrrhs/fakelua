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

TEST(test_json, encode_array_9) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_array_9", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_deep_ok) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_deep_ok", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_too_deep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "JsonTest.test_decode_too_deep", ret), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_big_int) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_big_int", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_invalid_number) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "JsonTest.test_decode_invalid_number", ret), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_invalid_exp) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "JsonTest.test_decode_invalid_exp", ret), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_control_char) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_basic.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "JsonTest.test_decode_control_char", ret), std::exception);
    FakeluaDeleteState(s);
}

// JSON edge case tests
TEST(test_json, decode_trailing_garbage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_trailing_garbage", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_invalid_escape) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_invalid_escape", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_invalid_null) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_invalid_null", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_invalid_bool) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_invalid_bool", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_leading_zero) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_leading_zero", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_dot_no_digit) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_dot_no_digit", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_exp_no_digit) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_exp_no_digit", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_empty_input) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_empty_input", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_whitespace_only) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_whitespace_only", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_unterminated_string) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_unterminated_string", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_unterminated_array) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_unterminated_array", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_unterminated_object) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_unterminated_object", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_non_string_key) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_non_string_key", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_missing_colon) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_missing_colon", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_unexpected_char) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_unexpected_char", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_escape_backslash) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_escape_backslash", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_escape_slash) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_escape_slash", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_escape_cr) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_escape_cr", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_escape_bs) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_escape_bs", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_escape_ff) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_escape_ff", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_escape_unicode) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_escape_unicode", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_escape_unicode_chinese) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_escape_unicode_chinese", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_escape_unicode_surrogate) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_escape_unicode_surrogate", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_invalid_surrogate) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_invalid_surrogate", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_invalid_surrogate_low) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_invalid_surrogate_low", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, decode_lone_low_surrogate) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_decode_lone_low_surrogate", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_int_key) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_int_key", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_float_key) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_float_key", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_bool_key) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_bool_key", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_special_chars) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_special_chars", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_control_chars) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_control_chars", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_empty_object) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_empty_object", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_nested_too_deep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_nested_too_deep", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_unsupported_type) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_unsupported_type", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_cyclic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_cyclic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_sparse_array) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_sparse_array", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_json, encode_large_int_key) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./json/test_json_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "JsonTest.test_encode_large_int_key", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
