#include "gtest/gtest.h"
#include "types.h"
#include "fakelua.h"
#include "location.h"
#include "token.h"
#include "tester.h"

TEST(lex, token_string) {
    tester t;
    std::vector<std::tuple<std::string, int, int>> left;
    auto ret = t.token_string("");
    ASSERT_EQ(left.size(), ret.size());
}
