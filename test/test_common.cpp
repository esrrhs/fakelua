#include "gtest/gtest.h"
#include "types.h"
#include "fakelua.h"

enum TestEnum {
    One, Two, Three
};

TEST(common, enumtypename) {
    std::string_view n = enum_type_name<TestEnum>();
    ASSERT_NE(n.empty(), true);
    DEBUG("name %s %d", n.data(), n.size());
}

TEST(common, enumname) {
    std::string_view n = enum_name<TestEnum::One>();
    ASSERT_NE(n.empty(), true);
    DEBUG("name %s %d", n.data(), n.size());
}
