#include "gtest/gtest.h"
#include "types.h"
#include "fakelua.h"
#include "location.h"
#include "token.h"
#include "tester.h"

TEST(lex, token) {
    std::string filename = "test.lua";
    location a(filename, 100, 200);
    token b(TK_THEN, a);
    std::string left = "then(test.lua:100,200)";
    DEBUG("%s", b.to_string());
    ASSERT_EQ(left, b.to_string());
    ASSERT_EQ(left.size(), b.to_string().size());
}

TEST(lex, token_empty) {
    std::string filename = "";
    location a(filename, 0, 0);
    token b(TK_THEN, a);
    std::string left = "then";
    ASSERT_EQ(left, b.to_string());
    ASSERT_EQ(left.size(), b.to_string().size());
}

TEST(lex, change_comment_to_space) {
    tester t;
    std::string content = "-- aa\n";
    std::string left = "     \n";
    ASSERT_EQ(left, t.change_comment_to_space(content));
    content = "-- aa\naa";
    left = "     \n  ";
    ASSERT_EQ(left, t.change_comment_to_space(content));
    content = "-- aa--\n";
    left = "       \n";
    ASSERT_EQ(left, t.change_comment_to_space(content));
    content = "-- aa--";
    left = "       ";
    ASSERT_EQ(left, t.change_comment_to_space(content));
}

TEST(lex, replace_comment) {
    tester t;
    std::string content = "-- aa\n";
    std::string left = "     \n";
    ASSERT_EQ(left, t.replace_comment(content));
    content = "-- aa\naa";
    left = "     \naa";
    ASSERT_EQ(left, t.replace_comment(content));
    content = "-- aa--\n";
    left = "       \n";
    ASSERT_EQ(left, t.replace_comment(content));
    content = "-- aa--";
    left = "       ";
    ASSERT_EQ(left, t.replace_comment(content));
}

TEST(lex, replace_multi_comment) {
    tester t;
    std::string content = "--[[ aa ]]\n";
    std::string left = "          \n";
    ASSERT_EQ(left, t.replace_multi_comment(content));
    content = "--[[ a\n ]]\n";
    left = "      \n   \n";
    ASSERT_EQ(left, t.replace_multi_comment(content));
    content = "--[[ a\n ]]]]\n";
    left = "      \n   ]]\n";
    ASSERT_EQ(left, t.replace_multi_comment(content));
    content = "---[[ a\n ]]]]\n";
    left = "-      \n   ]]\n";
    ASSERT_EQ(left, t.replace_multi_comment(content));
    content = "----[[ a\n ]]]]\n";
    left = "--      \n   ]]\n";
    ASSERT_EQ(left, t.replace_multi_comment(content));
    content = "--[[--[[ a\n ]]]]\n";
    left = "          \n   ]]\n";
    ASSERT_EQ(left, t.replace_multi_comment(content));
}

TEST(lex, replace_all_comment) {
    tester t;
    std::string content = "--[[ a\n ]]]]\nlocal a\n-- test\nlocal b\n-- test";
    std::string left = "      \n   ]]\nlocal a\n       \nlocal b\n       ";
    content = t.replace_multi_comment(content);
    content = t.replace_comment(content);
    ASSERT_EQ(left, content);
}

TEST(lex, token_string) {
    tester t;
    std::vector<std::tuple<std::string, int, int>> left;
    auto ret = t.token_string("");
    ASSERT_EQ(left.size(), ret.size());
}
