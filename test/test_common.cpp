#include "gtest/gtest.h"
#include "types.h"
#include "fakelua.h"

enum TestEnum {
    One, Two, Three
};

enum TestEnumSparse {
    SparseOne = 1, SparseTwo = 3, SparseThree = 5
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

TEST(common, enum_min_max) {
    int min = enum_min_v<TestEnum>;
    int max = enum_max_v<TestEnum>;
    ASSERT_EQ(min, 0);
    ASSERT_EQ(max, 2);
    DEBUG("min %d max %d", min, max);
}

TEST(common, enumtypename_s) {
    std::string_view n = enum_type_name<TestEnumSparse>();
    ASSERT_NE(n.empty(), true);
    DEBUG("name %s %d", n.data(), n.size());
}

TEST(common, enumname_s) {
    std::string_view n = enum_name<TestEnumSparse::SparseOne>();
    ASSERT_NE(n.empty(), true);
    DEBUG("name %s %d", n.data(), n.size());
}

TEST(common, enum_min_max_s) {
    int min = enum_min_v<TestEnumSparse>;
    int max = enum_max_v<TestEnumSparse>;
    ASSERT_EQ(min, 1);
    ASSERT_EQ(max, 5);
    DEBUG("min %d max %d", min, max);
}
