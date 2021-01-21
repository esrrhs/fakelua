#include "gtest/gtest.h"
#include "fakelua.h"

TEST(ini, newstate) {
    fakelua_state * L = fakelua_newstate();
    ASSERT_NE(L, nullptr);
}

TEST(ini, close) {
    fakelua_state * L = fakelua_newstate();
    ASSERT_NE(L, nullptr);
    fakelua_close(L);
}

TEST(ini, dofile) {
    fakelua_state * L = fakelua_newstate();
    ASSERT_NE(L, nullptr);
    int ret = fakelua_dofile(L, "test.lua");
    ASSERT_EQ(ret, 0);
}

TEST(ini, dostring) {
    fakelua_state * L = fakelua_newstate();
    ASSERT_NE(L, nullptr);
    int ret = fakelua_dostring(L, "test.lua");
    ASSERT_EQ(ret, 0);
}
