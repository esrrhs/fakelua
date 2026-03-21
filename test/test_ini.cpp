#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(ini, newstate) {
    const auto L = FakeluaNewstate();
    ASSERT_NE(L.get(), nullptr);
}
