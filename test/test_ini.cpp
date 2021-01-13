#include "gtest/gtest.h"
#include "fakelua.h"

TEST(ini, newstate) {
    fakelua_State * ret = fakelua_newstate();
    ASSERT_NE(ret, nullptr);
}
