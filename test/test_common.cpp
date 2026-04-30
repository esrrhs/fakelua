#include "fakelua.h"
#include "util/common.h"
#include "util/macro.h"
#include "util/string_util.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(common, escapse_string) {
    std::string src = "a\\ab\\bc\\fd\\ne\\rf\\tg\\vh\\\"i\\'j\\\\k";
    std::string dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "a\ab\bc\fd\ne\rf\tg\vh\"i\'j\\k");

    src = "\\a\\b\\f\\n\\r\\t\\v\\\"\\'";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "\a\b\f\n\r\t\v\"\'");

    src = "a\\z    b";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "ab");

    src = "a\\z    ";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "a");

    src = "\\z    \\z    ";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "");

    src = "\\z\\z    ";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "");

    src = "\\z\\z";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "");

    // \z should skip all whitespace characters (tabs, newlines, carriage returns, etc.)
    src = "a\\z\t\n\r b";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "ab");

    src = "hello\\z\n\t\t  world";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "helloworld");

    src = "x\\z\r\n\v\fy";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "xy");

    src = "a\\z\n";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "a");

    src = "a\\97b";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "aab");

    src = "a\\97";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "aa");

    src = "\\097b";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "ab");

    src = "\\097\\97";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "aa");
}

TEST(common, logging) {
    SetLogLevel(LogLevel::Info);
    LOG_INFO("Hello, World!");
    LOG_ERROR("Hello, World!");
}

TEST(common, vi_sort) {
    SimpleVarImpl t;
    std::vector<std::pair<VarInterface *, VarInterface *>> kv;

    SimpleVarImpl k1;
    k1.ViSetNil();
    SimpleVarImpl v1;
    v1.ViSetInt(1);
    kv.push_back(std::make_pair(&k1, &v1));

    SimpleVarImpl k2;
    k2.ViSetNil();
    SimpleVarImpl v2;
    v2.ViSetInt(1);
    kv.push_back(std::make_pair(&k2, &v2));

    SimpleVarImpl k3;
    k3.ViSetBool(true);
    SimpleVarImpl v3;
    v3.ViSetInt(3);
    kv.push_back(std::make_pair(&k3, &v3));

    SimpleVarImpl k4;
    k4.ViSetBool(false);
    SimpleVarImpl v4;
    v4.ViSetInt(4);
    kv.push_back(std::make_pair(&k4, &v4));

    SimpleVarImpl k5;
    k5.ViSetFloat(1.0);
    SimpleVarImpl v5;
    v5.ViSetInt(5);
    kv.push_back(std::make_pair(&k5, &v5));

    SimpleVarImpl k6;
    k6.ViSetFloat(2.0);
    SimpleVarImpl v6;
    v6.ViSetInt(6);
    kv.push_back(std::make_pair(&k6, &v6));

    SimpleVarImpl k7;
    k7.ViSetTable({});
    SimpleVarImpl v7;
    v7.ViSetInt(7);
    kv.push_back(std::make_pair(&k7, &v7));

    SimpleVarImpl k8;
    k8.ViSetTable({});
    SimpleVarImpl v8;
    v8.ViSetInt(7);
    kv.push_back(std::make_pair(&k8, &v8));

    std::ranges::shuffle(kv, std::mt19937(std::random_device()()));

    t.ViSetTable(kv);
    t.ViSortTable();

    ASSERT_EQ(t.ViToString(0), "table:\n\t[nil] = 1\n\t[nil] = 1\n\t[false] = 4\n\t[true] = 3\n\t[1.000000] = 5\n\t[2.000000] = "
                                "6\n\t[table:] = 7\n\t[table:] = 7");
}

TEST(common, IsNumber) {
    ASSERT_TRUE(IsNumber("123"));
    ASSERT_TRUE(IsNumber("-123"));
    ASSERT_TRUE(IsNumber("+123"));

    ASSERT_TRUE(IsNumber("123.456"));
    ASSERT_TRUE(IsNumber("-123.456"));
    ASSERT_TRUE(IsNumber("+123.456"));

    ASSERT_TRUE(IsNumber("123e456"));
    ASSERT_TRUE(IsNumber("-123e-456"));
    ASSERT_TRUE(IsNumber("+123E+456"));

    ASSERT_TRUE(IsNumber("0x123"));
    ASSERT_TRUE(IsNumber("-0X123"));
    ASSERT_TRUE(IsNumber("+0x123.456"));
    ASSERT_TRUE(IsNumber("-0X123.456P+789"));

    ASSERT_FALSE(IsNumber("abc"));
    ASSERT_FALSE(IsNumber("123abc"));
    ASSERT_FALSE(IsNumber("123.456.789"));
}

TEST(common, IsInteger) {
    ASSERT_TRUE(IsInteger("123"));
    ASSERT_TRUE(IsInteger("-123"));
    ASSERT_TRUE(IsInteger("+123"));

    ASSERT_TRUE(IsInteger("0x123"));
    ASSERT_TRUE(IsInteger("-0X123"));

    ASSERT_FALSE(IsInteger("123.456"));
    ASSERT_FALSE(IsInteger("-123.456"));
    ASSERT_FALSE(IsInteger("+123.456"));
    ASSERT_FALSE(IsInteger("123e456"));
    ASSERT_FALSE(IsInteger("-123e-456"));
    ASSERT_FALSE(IsInteger("+123E+456"));
    ASSERT_FALSE(IsInteger("+0x123.456"));
    ASSERT_FALSE(IsInteger("-0X123.456P+789"));
    ASSERT_FALSE(IsInteger("abc"));
    ASSERT_FALSE(IsInteger("123abc"));
    ASSERT_FALSE(IsInteger("123.456.789"));
}

TEST(common, ToInteger) {
    ASSERT_EQ(ToInteger("123"), 123);
    ASSERT_EQ(ToInteger("-123"), -123);
    ASSERT_EQ(ToInteger("+123"), 123);
    ASSERT_EQ(ToInteger("0x123"), 0x123);
    ASSERT_EQ(ToInteger("0X123"), 0x123);
    ASSERT_EQ(ToInteger("+0X123"), 0x123);
    ASSERT_EQ(ToInteger("-0X123"), -0x123);

    ASSERT_EQ(ToInteger("0"), 0);
    ASSERT_EQ(ToInteger("-0"), 0);
    ASSERT_EQ(ToInteger("0x0"), 0);
    ASSERT_EQ(ToInteger("0X0"), 0);

    ASSERT_EQ(ToInteger("9223372036854775807"), INT64_MAX);
    ASSERT_EQ(ToInteger("-9223372036854775808"), INT64_MIN);

    ASSERT_EQ(ToInteger("0xff"), 255);
    ASSERT_EQ(ToInteger("0xBEBADA"), 0xBEBADA);
}

TEST(common, ToFloat) {
    ASSERT_TRUE(std::isnan(ToFloat("nan")));
    ASSERT_TRUE(std::isnan(ToFloat("NaN")));
    ASSERT_TRUE(std::isnan(ToFloat("NAN")));
    ASSERT_TRUE(std::isinf(ToFloat("inf")));
    ASSERT_TRUE(std::isinf(ToFloat("Inf")));
    ASSERT_TRUE(std::isinf(ToFloat("INF")));
    ASSERT_TRUE(std::isinf(ToFloat("-inf")));
    ASSERT_TRUE(std::isinf(ToFloat("-Inf")));
    ASSERT_TRUE(std::isinf(ToFloat("-INF")));

    ASSERT_EQ(ToFloat("0"), 0);

    ASSERT_EQ(ToFloat("12.3"), 12.3);
    ASSERT_EQ(ToFloat("-12.3"), -12.3);
    ASSERT_EQ(ToFloat("+12.3"), 12.3);

    ASSERT_EQ(ToFloat("12.3e45"), 12.3e45);
    ASSERT_EQ(ToFloat("12.3e+45"), 12.3e+45);
    ASSERT_EQ(ToFloat("-12.3e-45"), -12.3e-45);

    ASSERT_EQ(ToFloat("0x123"), 0x123);
    ASSERT_EQ(ToFloat("0X12.3"), 18.1875);
    ASSERT_EQ(ToFloat("0X12.3P45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("0X12.3P+45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("0X12.3P-45"), 18.1875 * std::pow(2, -45));
    ASSERT_EQ(ToFloat("0X12.3P+0"), 18.1875);

    ASSERT_EQ(ToFloat("+0x123"), 0x123);
    ASSERT_EQ(ToFloat("+0X12.3"), 18.1875);
    ASSERT_EQ(ToFloat("+0X12.3P45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("+0X12.3P+45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("+0X12.3P-45"), 18.1875 * std::pow(2, -45));
    ASSERT_EQ(ToFloat("+0X12.3P+0"), 18.1875);

    ASSERT_EQ(ToFloat("-0x123"), -0x123);
    ASSERT_EQ(ToFloat("-0X12.3"), -18.1875);
    ASSERT_EQ(ToFloat("-0X12.3P45"), -18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("-0X12.3P+45"), -18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("-0X12.3P-45"), -18.1875 * std::pow(2, -45));
    ASSERT_EQ(ToFloat("-0X12.3P+0"), -18.1875);

    ASSERT_EQ(ToFloat("1.7976931348623157e+308"), std::numeric_limits<double>::max());
    ASSERT_EQ(ToFloat("2.2250738585072014e-308"), std::numeric_limits<double>::min());

    ASSERT_EQ(ToFloat("3.1416"), 3.1416);
    ASSERT_EQ(ToFloat("314.16e-2"), 314.16e-2);
    ASSERT_EQ(ToFloat("0.31416E1"), 0.31416E1);
    ASSERT_EQ(ToFloat("34e1"), 34e1);
    ASSERT_EQ(ToFloat("0x0.1E"), 0.1171875);
    ASSERT_EQ(ToFloat("0xA23p-4"), 0xA23p-4);
    ASSERT_EQ(ToFloat("0X1.921FB54442D18P+1"), 0X1.921FB54442D18P+1);
}

// Bug 1: ToInteger must parse INT64_MIN in hex form without throwing.
TEST(common, ToInteger_hex_int64_min) {
    ASSERT_EQ(ToInteger("-0x8000000000000000"), INT64_MIN);
    ASSERT_EQ(ToInteger("-0X8000000000000000"), INT64_MIN);
    // Largest positive hex value (INT64_MAX)
    ASSERT_EQ(ToInteger("0x7FFFFFFFFFFFFFFF"), INT64_MAX);
    ASSERT_EQ(ToInteger("-0x7FFFFFFFFFFFFFFF"), -INT64_MAX);
}

// Bug 2: ToFloat must handle hex integers >= 0x8000000000000000 without overflowing.
TEST(common, ToFloat_hex_large) {
    // 0x8000000000000000 == 9223372036854775808.0
    ASSERT_EQ(ToFloat("0x8000000000000000"), static_cast<double>(0x8000000000000000ULL));
    // All-ones: 0xFFFFFFFFFFFFFFFF == 18446744073709551615.0
    ASSERT_EQ(ToFloat("0xFFFFFFFFFFFFFFFF"), static_cast<double>(0xFFFFFFFFFFFFFFFFULL));
    // Negative large hex
    ASSERT_EQ(ToFloat("-0x8000000000000000"), -static_cast<double>(0x8000000000000000ULL));
}

// Bug 3: JoinString must not crash / UB on an empty vector.
TEST(common, JoinString_empty) {
    ASSERT_EQ(JoinString({}, ","), "");
    ASSERT_EQ(JoinString({"a"}, ","), "a");
    ASSERT_EQ(JoinString({"a", "b", "c"}, ","), "a,b,c");
    ASSERT_EQ(JoinString({"x", "y"}, ""), "xy");
}

// Bug 7: SET_FLAG_BIT / GET_FLAG_BIT must not invoke UB for any valid bit position.
TEST(common, flag_bit_macros) {
    // Basic set / clear / get
    uint32_t f = 0U;
    SET_FLAG_BIT(f, 0, true);
    ASSERT_EQ(GET_FLAG_BIT(f, 0), 1U);
    SET_FLAG_BIT(f, 0, false);
    ASSERT_EQ(GET_FLAG_BIT(f, 0), 0U);

    // High bit positions (would overflow with signed int)
    SET_FLAG_BIT(f, 31, true);
    ASSERT_EQ(GET_FLAG_BIT(f, 31), 1U);
    SET_FLAG_BIT(f, 31, false);
    ASSERT_EQ(GET_FLAG_BIT(f, 31), 0U);

    // Multiple bits
    SET_FLAG_BIT(f, 7, true);
    SET_FLAG_BIT(f, 15, true);
    SET_FLAG_BIT(f, 23, true);
    ASSERT_EQ(GET_FLAG_BIT(f, 7), 1U);
    ASSERT_EQ(GET_FLAG_BIT(f, 15), 1U);
    ASSERT_EQ(GET_FLAG_BIT(f, 23), 1U);
    ASSERT_EQ(GET_FLAG_BIT(f, 8), 0U);
}
