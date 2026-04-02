#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(ini, newstate) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    FakeluaDeleteState(s);
}
