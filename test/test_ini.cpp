#include "gtest/gtest.h"
#include "fakelua/fakelua.h"

using namespace fakelua;

TEST(ini, newstate) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
}
