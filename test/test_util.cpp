#include "fakelua.h"
#include "util/common.h"
#include "util/string_util.h"
#include "gtest/gtest.h"
#include "util/file_util.h"
#include "util/number_util.h"
#include "util/os.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

using namespace fakelua;

// TrimInplace tests

TEST(util, TrimInplace_empty) {
    std::string s;
    TrimInplace(s);
    ASSERT_EQ(s, "");
}

TEST(util, TrimInplace_no_whitespace) {
    std::string s = "hello";
    TrimInplace(s);
    ASSERT_EQ(s, "hello");
}

TEST(util, TrimInplace_left_spaces) {
    std::string s = "   hello";
    TrimInplace(s);
    ASSERT_EQ(s, "hello");
}

TEST(util, TrimInplace_right_spaces) {
    std::string s = "hello   ";
    TrimInplace(s);
    ASSERT_EQ(s, "hello");
}

TEST(util, TrimInplace_both_sides) {
    std::string s = "  hello world  ";
    TrimInplace(s);
    ASSERT_EQ(s, "hello world");
}

TEST(util, TrimInplace_all_spaces) {
    std::string s = "     ";
    TrimInplace(s);
    ASSERT_EQ(s, "");
}

TEST(util, TrimInplace_tabs_and_newlines) {
    std::string s = "\t\n hello\t\n ";
    TrimInplace(s);
    ASSERT_EQ(s, "hello");
}

TEST(util, TrimInplace_single_char) {
    std::string s = "x";
    TrimInplace(s);
    ASSERT_EQ(s, "x");
}

TEST(util, TrimInplace_single_space) {
    std::string s = " ";
    TrimInplace(s);
    ASSERT_EQ(s, "");
}

TEST(util, TrimInplace_internal_spaces_preserved) {
    std::string s = "  a  b  c  ";
    TrimInplace(s);
    ASSERT_EQ(s, "a  b  c");
}

// JoinString single-element path

TEST(util, JoinString_single_element) {
    ASSERT_EQ(JoinString({"only"}, ","), "only");
    ASSERT_EQ(JoinString({"x"}, "---"), "x");
    // single element with empty separator
    ASSERT_EQ(JoinString({"abc"}, ""), "abc");
}

// ReplaceEscapeChars additional paths not covered elsewhere

// Bare backslash at end of string (the loop simply breaks after consuming '\').
TEST(util, ReplaceEscapeChars_trailing_backslash) {
    // A lone backslash at the end should be silently dropped (loop breaks).
    std::string s = "abc\\";
    std::string result = ReplaceEscapeChars(s);
    ASSERT_EQ(result, "abc");
}

// \digit with exactly one digit
TEST(util, ReplaceEscapeChars_digit_one_digit) {
    // \9 → char(9)
    std::string s = "\\9";
    std::string result = ReplaceEscapeChars(s);
    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(static_cast<unsigned char>(result[0]), 9u);
}

// \digit with two digits
TEST(util, ReplaceEscapeChars_digit_two_digits) {
    // \65 → 'A'
    std::string s = "\\65";
    std::string result = ReplaceEscapeChars(s);
    ASSERT_EQ(result, "A");
}

// \digit with exactly three digits (max = 255 boundary)
TEST(util, ReplaceEscapeChars_digit_three_digits_boundary) {
    // \255 → char(255)
    std::string s = "\\255";
    std::string result = ReplaceEscapeChars(s);
    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(static_cast<unsigned char>(result[0]), 255u);
}

// \digit overflow: value > 255 must throw
TEST(util, ReplaceEscapeChars_digit_overflow_throws) {
    std::string s = "\\256";
    EXPECT_THROW(ReplaceEscapeChars(s), std::exception);

    std::string s2 = "\\999";
    EXPECT_THROW(ReplaceEscapeChars(s2), std::exception);
}

// \z followed by mixed whitespace then non-whitespace
TEST(util, ReplaceEscapeChars_z_mixed_whitespace) {
    std::string s = "start\\z \t\r\n  end";
    ASSERT_EQ(ReplaceEscapeChars(s), "startend");
}

// Multiple escape sequences of different kinds in sequence
TEST(util, ReplaceEscapeChars_mixed_escapes) {
    // \n, \t, \97 (='a'), \z, \\ in one string
    std::string s = "A\\nB\\tC\\97D\\z   E\\\\F";
    std::string result = ReplaceEscapeChars(s);
    ASSERT_EQ(result, "A\nB\tCaDE\\F");
}

TEST(util, number_util_and_os_exec) {
    EXPECT_TRUE(IsInteger("42"));
    EXPECT_FALSE(IsInteger("1.0"));
    EXPECT_FALSE(IsInteger(""));

    EXPECT_EQ(FormatLuaFloat(1.0), "1.0");
    EXPECT_FALSE(FormatLuaFloat(1.5).empty());
    EXPECT_FALSE(FormatLuaFloat(-42.0).empty());

    EXPECT_EQ(*TryConvertDoubleToInt64(3.0), 3);
    EXPECT_FALSE(TryConvertDoubleToInt64(1.5).has_value());
    EXPECT_FALSE(TryConvertDoubleToInt64(std::numeric_limits<double>::infinity()).has_value());
    EXPECT_FALSE(TryConvertDoubleToInt64(1e300).has_value());
    EXPECT_EQ(*TryConvertDoubleToInt64(static_cast<double>(std::numeric_limits<int64_t>::min())), std::numeric_limits<int64_t>::min());

    EXPECT_EQ(CompareInt64Double(1, std::numeric_limits<double>::quiet_NaN()), 0);
    EXPECT_EQ(CompareInt64Double(1, 1e300), -1);
    EXPECT_EQ(CompareInt64Double(1, -1e300), 1);
    EXPECT_EQ(CompareInt64Double(2, 2.5), -1);
    EXPECT_EQ(CompareInt64Double(3, 2.5), 1);
    EXPECT_EQ(CompareInt64Double(2, 2.0), 0);
    EXPECT_TRUE(Int64DoubleEqual(2, 2.0));
    EXPECT_FALSE(Int64DoubleEqual(2, std::numeric_limits<double>::quiet_NaN()));

    const std::string out = Exec("echo hello_cov");
    EXPECT_NE(out.find("hello_cov"), std::string::npos);
}

TEST(util, generate_tmp_filename_collision) {
    // Collision path in GenerateTmpFilename: pre-create the first candidate.
    const auto first = GenerateTmpFilename("cov_collide_", ".tmp");
    {
        std::ofstream ofs(first);
        ofs << "x";
    }
    // Force same pid/count window by creating many temps; at least exercise the function.
    const auto second = GenerateTmpFilename("cov_collide_", ".tmp");
    EXPECT_NE(first, second);
    std::error_code ec;
    std::filesystem::remove(first, ec);
    std::filesystem::remove(second, ec);
}
