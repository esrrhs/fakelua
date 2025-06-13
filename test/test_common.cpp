#include "fakelua.h"
#include "util/common.h"
#include "util/string_util.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(common, escapse_string) {
    std::string src = "a\\ab\\bc\\fd\\ne\\rf\\tg\\vh\\\"i\\'j\\\\k";
    std::string dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "a\ab\bc\fd\ne\rf\tg\vh\"i\'j\\k");

    src = "\\a\\b\\f\\n\\r\\t\\v\\\"\\'";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "\a\b\f\n\r\t\v\"\'");

    src = "a\\z    b";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "ab");

    src = "a\\z    ";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "a");

    src = "\\z    \\z    ";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "");

    src = "\\z\\z    ";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "");

    src = "\\z\\z";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "");

    src = "a\\97b";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "aab");

    src = "a\\97";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "aa");

    src = "\\097b";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "ab");

    src = "\\097\\97";
    dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "aa");
}

TEST(common, logging) {
    set_log_level(log_level::Info);
    LOG_INFO("Hello, World!");
    LOG_ERROR("Hello, World!");
}

TEST(common, vi_sort) {
    simple_var_impl t;
    std::vector<std::pair<var_interface *, var_interface *>> kv;

    simple_var_impl k1;
    k1.vi_set_nil();
    simple_var_impl v1;
    v1.vi_set_int(1);
    kv.push_back(std::make_pair(&k1, &v1));

    simple_var_impl k2;
    k2.vi_set_nil();
    simple_var_impl v2;
    v2.vi_set_int(1);
    kv.push_back(std::make_pair(&k2, &v2));

    simple_var_impl k3;
    k3.vi_set_bool(true);
    simple_var_impl v3;
    v3.vi_set_int(3);
    kv.push_back(std::make_pair(&k3, &v3));

    simple_var_impl k4;
    k4.vi_set_bool(false);
    simple_var_impl v4;
    v4.vi_set_int(4);
    kv.push_back(std::make_pair(&k4, &v4));

    simple_var_impl k5;
    k5.vi_set_float(1.0);
    simple_var_impl v5;
    v5.vi_set_int(5);
    kv.push_back(std::make_pair(&k5, &v5));

    simple_var_impl k6;
    k6.vi_set_float(2.0);
    simple_var_impl v6;
    v6.vi_set_int(6);
    kv.push_back(std::make_pair(&k6, &v6));

    simple_var_impl k7;
    k7.vi_set_table({});
    simple_var_impl v7;
    v7.vi_set_int(7);
    kv.push_back(std::make_pair(&k7, &v7));

    simple_var_impl k8;
    k8.vi_set_table({});
    simple_var_impl v8;
    v8.vi_set_int(7);
    kv.push_back(std::make_pair(&k8, &v8));

    shuffle(kv.begin(), kv.end(), std::mt19937(std::random_device()()));

    t.vi_set_table(kv);
    t.vi_sort_table();

    ASSERT_EQ(t.vi_to_string(0), "table:\n\t[nil] = 1\n\t[nil] = 1\n\t[false] = 4\n\t[true] = 3\n\t[1.000000] = 5\n\t[2.000000] = "
                                "6\n\t[table:] = 7\n\t[table:] = 7");
}

TEST(common, is_number) {
    ASSERT_TRUE(is_number("123"));
    ASSERT_TRUE(is_number("-123"));
    ASSERT_TRUE(is_number("+123"));

    ASSERT_TRUE(is_number("123.456"));
    ASSERT_TRUE(is_number("-123.456"));
    ASSERT_TRUE(is_number("+123.456"));

    ASSERT_TRUE(is_number("123e456"));
    ASSERT_TRUE(is_number("-123e-456"));
    ASSERT_TRUE(is_number("+123E+456"));

    ASSERT_TRUE(is_number("0x123"));
    ASSERT_TRUE(is_number("-0X123"));
    ASSERT_TRUE(is_number("+0x123.456"));
    ASSERT_TRUE(is_number("-0X123.456P+789"));

    ASSERT_FALSE(is_number("abc"));
    ASSERT_FALSE(is_number("123abc"));
    ASSERT_FALSE(is_number("123.456.789"));
}

TEST(common, is_integer) {
    ASSERT_TRUE(is_integer("123"));
    ASSERT_TRUE(is_integer("-123"));
    ASSERT_TRUE(is_integer("+123"));

    ASSERT_TRUE(is_integer("0x123"));
    ASSERT_TRUE(is_integer("-0X123"));

    ASSERT_FALSE(is_integer("123.456"));
    ASSERT_FALSE(is_integer("-123.456"));
    ASSERT_FALSE(is_integer("+123.456"));
    ASSERT_FALSE(is_integer("123e456"));
    ASSERT_FALSE(is_integer("-123e-456"));
    ASSERT_FALSE(is_integer("+123E+456"));
    ASSERT_FALSE(is_integer("+0x123.456"));
    ASSERT_FALSE(is_integer("-0X123.456P+789"));
    ASSERT_FALSE(is_integer("abc"));
    ASSERT_FALSE(is_integer("123abc"));
    ASSERT_FALSE(is_integer("123.456.789"));
}

TEST(common, to_integer) {
    ASSERT_EQ(to_integer("123"), 123);
    ASSERT_EQ(to_integer("-123"), -123);
    ASSERT_EQ(to_integer("+123"), 123);
    ASSERT_EQ(to_integer("0x123"), 0x123);
    ASSERT_EQ(to_integer("0X123"), 0x123);
    ASSERT_EQ(to_integer("+0X123"), 0x123);
    ASSERT_EQ(to_integer("-0X123"), -0x123);

    ASSERT_EQ(to_integer("0"), 0);
    ASSERT_EQ(to_integer("-0"), 0);
    ASSERT_EQ(to_integer("0x0"), 0);
    ASSERT_EQ(to_integer("0X0"), 0);

    ASSERT_EQ(to_integer("9223372036854775807"), INT64_MAX);
    ASSERT_EQ(to_integer("-9223372036854775808"), INT64_MIN);

    ASSERT_EQ(to_integer("0xff"), 255);
    ASSERT_EQ(to_integer("0xBEBADA"), 0xBEBADA);
}

TEST(common, to_float) {
    ASSERT_TRUE(std::isnan(to_float("nan")));
    ASSERT_TRUE(std::isnan(to_float("NaN")));
    ASSERT_TRUE(std::isnan(to_float("NAN")));
    ASSERT_TRUE(std::isinf(to_float("inf")));
    ASSERT_TRUE(std::isinf(to_float("Inf")));
    ASSERT_TRUE(std::isinf(to_float("INF")));
    ASSERT_TRUE(std::isinf(to_float("-inf")));
    ASSERT_TRUE(std::isinf(to_float("-Inf")));
    ASSERT_TRUE(std::isinf(to_float("-INF")));

    ASSERT_EQ(to_float("0"), 0);

    ASSERT_EQ(to_float("12.3"), 12.3);
    ASSERT_EQ(to_float("-12.3"), -12.3);
    ASSERT_EQ(to_float("+12.3"), 12.3);

    ASSERT_EQ(to_float("12.3e45"), 12.3e45);
    ASSERT_EQ(to_float("12.3e+45"), 12.3e+45);
    ASSERT_EQ(to_float("-12.3e-45"), -12.3e-45);

    ASSERT_EQ(to_float("0x123"), 0x123);
    ASSERT_EQ(to_float("0X12.3"), 18.1875);
    ASSERT_EQ(to_float("0X12.3P45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(to_float("0X12.3P+45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(to_float("0X12.3P-45"), 18.1875 * std::pow(2, -45));
    ASSERT_EQ(to_float("0X12.3P+0"), 18.1875);

    ASSERT_EQ(to_float("+0x123"), 0x123);
    ASSERT_EQ(to_float("+0X12.3"), 18.1875);
    ASSERT_EQ(to_float("+0X12.3P45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(to_float("+0X12.3P+45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(to_float("+0X12.3P-45"), 18.1875 * std::pow(2, -45));
    ASSERT_EQ(to_float("+0X12.3P+0"), 18.1875);

    ASSERT_EQ(to_float("-0x123"), -0x123);
    ASSERT_EQ(to_float("-0X12.3"), -18.1875);
    ASSERT_EQ(to_float("-0X12.3P45"), -18.1875 * std::pow(2, 45));
    ASSERT_EQ(to_float("-0X12.3P+45"), -18.1875 * std::pow(2, 45));
    ASSERT_EQ(to_float("-0X12.3P-45"), -18.1875 * std::pow(2, -45));
    ASSERT_EQ(to_float("-0X12.3P+0"), -18.1875);

    ASSERT_EQ(to_float("1.7976931348623157e+308"), std::numeric_limits<double>::max());
    ASSERT_EQ(to_float("2.2250738585072014e-308"), std::numeric_limits<double>::min());

    ASSERT_EQ(to_float("3.1416"), 3.1416);
    ASSERT_EQ(to_float("314.16e-2"), 314.16e-2);
    ASSERT_EQ(to_float("0.31416E1"), 0.31416E1);
    ASSERT_EQ(to_float("34e1"), 34e1);
    ASSERT_EQ(to_float("0x0.1E"), 0.1171875);
    ASSERT_EQ(to_float("0xA23p-4"), 0xA23p-4);
    ASSERT_EQ(to_float("0X1.921FB54442D18P+1"), 0X1.921FB54442D18P+1);
}
