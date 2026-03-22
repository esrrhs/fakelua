#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(ini, newstate) {
    const auto L = FakeluaNewState();
    ASSERT_NE(L.get(), nullptr);
}
