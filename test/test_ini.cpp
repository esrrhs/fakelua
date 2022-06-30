#include "gtest/gtest.h"
#include "fakelua/fakelua.h"

using namespace fakelua;

TEST(ini, newstate) {
    fakelua_state *L = fakelua_newstate();
    ASSERT_NE(L, nullptr);
    fakelua_close(L);
}

TEST(ini, close) {
    fakelua_state *L = fakelua_newstate();
    ASSERT_NE(L, nullptr);
    fakelua_close(L);
}
