#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_csv, decode_simple) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_decode_simple", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, decode_single_row) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_decode_single_row", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, decode_single_column) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_decode_single_column", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, decode_quoted) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_decode_quoted", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, decode_escaped_quotes) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_decode_escaped_quotes", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, decode_numbers) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_decode_numbers", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, decode_empty_field) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_decode_empty_field", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, decode_custom_sep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_decode_custom_sep", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, encode_simple) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_encode_simple", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, encode_quotes) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_encode_quotes", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, encode_escaped_quotes) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_encode_escaped_quotes", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, encode_numbers) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_encode_numbers", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, encode_custom_sep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_encode_custom_sep", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, roundtrip_with_commas) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CsvTest.test_roundtrip_with_commas", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_csv, unterminated_quote) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./csv/test_csv_basic.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "CsvTest.test_unterminated_quote", ret), std::exception);
    FakeluaDeleteState(s);
}
