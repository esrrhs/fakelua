#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(ini, newstate) {
    const auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
}
