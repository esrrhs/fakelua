#include "gtest/gtest.h"
#include "fakelua.h"

TEST(ini, newstate) {
    fakelua_state * ret = fakelua_newstate();
    ASSERT_NE(ret, nullptr);
}
