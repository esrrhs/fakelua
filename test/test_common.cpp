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

    ASSERT_EQ(t.vi_to_string(), "table:\n\t[nil] = 1\n\t[nil] = 1\n\t[false] = 4\n\t[true] = 3\n\t[1.000000] = 5\n\t[2.000000] = "
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