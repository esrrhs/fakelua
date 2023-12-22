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
