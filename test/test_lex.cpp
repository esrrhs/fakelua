#include "ftest.h"
#include "types.h"
#include "fakelua.h"
#include "location.h"
#include "token.h"
#include "tester.h"

TEST(lex, token) {
    std::string filename = "test.lua";
    location a(filename, 100, 200);
    token b(TK_THEN, a);
    std::string right = "then(test.lua:100,200)";
    DEBUG("%s", b.to_string().c_str());
    ASSERT_EQ(b.to_string(), right);
    ASSERT_EQ(b.to_string().size(), right.size());
}

TEST(lex, token_empty) {
    std::string filename = "";
    location a(filename, 0, 0);
    token b(TK_THEN, a);
    std::string right = "then";
    ASSERT_EQ(b.to_string(), right);
    ASSERT_EQ(b.to_string().size(), right.size());
}

TEST(lex, change_comment_to_space) {
    tester t;
    std::string content = "-- aa\n";
    std::string right = "     \n";
    ASSERT_EQ(t.change_comment_to_space(content), right);
    content = "-- aa\naa";
    right = "     \n  ";
    ASSERT_EQ(t.change_comment_to_space(content), right);
    content = "-- aa--\n";
    right = "       \n";
    ASSERT_EQ(t.change_comment_to_space(content), right);
    content = "-- aa--";
    right = "       ";
    ASSERT_EQ(t.change_comment_to_space(content), right);
}

TEST(lex, replace_comment) {
    tester t;
    std::string content = "-- aa\n";
    std::string right = "     \n";
    ASSERT_EQ(t.replace_comment(content), right);
    content = "-- aa\naa";
    right = "     \naa";
    ASSERT_EQ(t.replace_comment(content), right);
    content = "-- aa--\n";
    right = "       \n";
    ASSERT_EQ(t.replace_comment(content), right);
    content = "-- aa--";
    right = "       ";
    ASSERT_EQ(t.replace_comment(content), right);
}

TEST(lex, replace_multi_comment) {
    tester t;
    std::string content = "--[[ aa ]]\n";
    std::string right = "          \n";
    ASSERT_EQ(t.replace_multi_comment(content), right);
    content = "--[[ a\n ]]\n";
    right = "      \n   \n";
    ASSERT_EQ(t.replace_multi_comment(content), right);
    content = "--[[ a\n ]]]]\n";
    right = "      \n   ]]\n";
    ASSERT_EQ(t.replace_multi_comment(content), right);
    content = "---[[ a\n ]]]]\n";
    right = "-      \n   ]]\n";
    ASSERT_EQ(t.replace_multi_comment(content), right);
    content = "----[[ a\n ]]]]\n";
    right = "--      \n   ]]\n";
    ASSERT_EQ(t.replace_multi_comment(content), right);
    content = "--[[--[[ a\n ]]]]\n";
    right = "          \n   ]]\n";
    ASSERT_EQ(t.replace_multi_comment(content), right);
}

TEST(lex, replace_all_comment) {
    tester t;
    std::string content = "--[[ a\n ]]]]\nlocal a\n-- test\nlocal b\n-- test";
    std::string right = "      \n   ]]\nlocal a\n       \nlocal b\n       ";
    content = t.replace_multi_comment(content);
    content = t.replace_comment(content);
    ASSERT_EQ(content, right);
}

TEST(lex, split_string) {
    std::string file_name = "test.lua";
    tester t;
    {
        auto[err, ret] = t.split_string(file_name, "");
        ASSERT_EQ(err.empty(), true);
        ASSERT_EQ(ret.empty(), true);
    }
    {
        auto[err, ret] = t.split_string(file_name, "abc def");
        ASSERT_EQ(err.empty(), true);
        ASSERT_EQ((int) ret.size(), 2);
        {
            auto[str, line, col]=ret[0];
            ASSERT_EQ(str, "abc");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 1);
        }
        {
            auto[str, line, col]=ret[1];
            ASSERT_EQ(str, "def");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 5);
        }
    }
    {
        auto[err, ret] = t.split_string(file_name, "\n\n abc ");
        ASSERT_EQ(err.empty(), true);
        ASSERT_EQ((int) ret.size(), 1);
        {
            auto[str, line, col]=ret[0];
            ASSERT_EQ(str, "abc");
            ASSERT_EQ(line, 3);
            ASSERT_EQ(col, 2);
        }
    }
    {
        auto[err, ret] = t.split_string(file_name, "'\"ab'c ");
        ASSERT_EQ(err.empty(), true);
        ASSERT_EQ((int) ret.size(), 2);
        {
            auto[str, line, col]=ret[0];
            ASSERT_EQ(str, "\"ab");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 1);
        }
        {
            auto[str, line, col]=ret[1];
            ASSERT_EQ(str, "c");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 6);
        }
    }
    {
        auto[err, ret] = t.split_string(file_name, "\"'ab\"c ");
        ASSERT_EQ(err.empty(), true);
        ASSERT_EQ((int) ret.size(), 2);
        {
            auto[str, line, col]=ret[0];
            ASSERT_EQ(str, "'ab");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 1);
        }
        {
            auto[str, line, col]=ret[1];
            ASSERT_EQ(str, "c");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 6);
        }
    }
    {
        auto[err, ret] = t.split_string(file_name, "\"'ab\"c ''");
        ASSERT_EQ(err.empty(), true);
        ASSERT_EQ((int) ret.size(), 3);
        {
            auto[str, line, col]=ret[0];
            ASSERT_EQ(str, "'ab");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 1);
        }
        {
            auto[str, line, col]=ret[1];
            ASSERT_EQ(str, "c");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 6);
        }
        {
            auto[str, line, col]=ret[2];
            ASSERT_EQ(str, "");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 8);
        }
    }
    {
        auto[err, ret] = t.split_string(file_name, "''''");
        ASSERT_EQ(err.empty(), true);
        ASSERT_EQ((int) ret.size(), 2);
        {
            auto[str, line, col]=ret[0];
            ASSERT_EQ(str, "");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 1);
        }
        {
            auto[str, line, col]=ret[1];
            ASSERT_EQ(str, "");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 3);
        }
    }
    {
        auto[err, ret] = t.split_string(file_name, "a\"cd\"d");
        ASSERT_EQ(err.empty(), true);
        ASSERT_EQ((int) ret.size(), 3);
        {
            auto[str, line, col]=ret[0];
            ASSERT_EQ(str, "a");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 1);
        }
        {
            auto[str, line, col]=ret[1];
            ASSERT_EQ(str, "cd");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 3);
        }
        {
            auto[str, line, col]=ret[2];
            ASSERT_EQ(str, "d");
            ASSERT_EQ(line, 1);
            ASSERT_EQ(col, 6);
        }
    }
    {
        auto[err, ret] = t.split_string(file_name, "a\"cd");
        ASSERT_EQ(err.empty(), false);
        ASSERT_EQ(err.code(), FAKELUA_LEX_FAIL);
        DEBUG("%s", err.str().c_str());
        ASSERT_EQ(err.str(), "unfinished string in file test.lua:1,2");
    }
}
