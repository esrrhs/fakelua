#include "fakelua.h"
#include "util/rich_hashmap.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(common, rich_hashmap) {
    rich_hashmap<int, int, 2> map1(2);
    map1.set(1, 1);
    map1.set(2, 2);
    map1.set(3, 3);
    map1.set(4, 4);

    int v;
    auto ret = map1.get(1, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 1);

    ret = map1.get(2, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 2);

    ret = map1.get(3, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 3);

    ret = map1.get(4, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 4);

    auto size = map1.size();
    ASSERT_EQ(size, 4);

    map1.set(1, 2);
    ret = map1.get(1, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 2);

    auto bucket_size = map1.bucket_size();
    ASSERT_EQ(bucket_size, 2);

    map1.set(5, 5);
    ret = map1.get(5, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 5);

    size = map1.size();
    ASSERT_EQ(size, 5);

    bucket_size = map1.bucket_size();
    ASSERT_EQ(bucket_size, 8);

    map1.remove(1);
    ret = map1.get(1, v);
    ASSERT_EQ(ret, false);

    size = map1.size();
    ASSERT_EQ(size, 4);
}

TEST(common, escapse_string) {
    std::string src = "a\\ab\\bc\\fd\\ne\\rf\\tg\\vh\\\"i\\'j";
    std::string dst = replace_escape_chars(src);
    ASSERT_EQ(dst, "a\ab\bc\fd\ne\rf\tg\vh\"i\'j");

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
