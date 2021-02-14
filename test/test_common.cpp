#include "gtest/gtest.h"
#include "types.h"
#include "fakelua.h"
#include "location.h"
#include "token.h"

enum TestEnum {
    One, Two, Three
};

enum TestEnumSparse {
    SparseOne = 1, SparseTwo = 3, SparseThree = 5
};

TEST(common, enumtypename) {
    std::string_view n = enum_type_name<TestEnum>();
    DEBUG("name %s %d", n.data(), n.size());
    ASSERT_NE(n.empty(), true);
}

TEST(common, enumname) {
    std::string_view n = enum_name<TestEnum::One>();
    DEBUG("name %s %d", n.data(), n.size());
    ASSERT_NE(n.empty(), true);
}

TEST(common, enum_min_max) {
    int min = enum_min_v<TestEnum>;
    int max = enum_max_v<TestEnum>;
    DEBUG("min %d max %d", min, max);
    ASSERT_EQ(min, 0);
    ASSERT_EQ(max, 2);
}

TEST(common, enum_value) {
    auto a = TestEnum::One;
    std::string_view n = enum_name(a);
    DEBUG("name %s %d", n.data(), n.size());
    ASSERT_NE(n.empty(), true);
}

TEST(common, enum_cast) {
    auto n = enum_cast<TestEnum>("One");
    DEBUG("n %d", n);
    ASSERT_EQ(n, TestEnum::One);
}

TEST(common, enumtypename_s) {
    std::string_view n = enum_type_name<TestEnumSparse>();
    DEBUG("name %s %d", n.data(), n.size());
    ASSERT_NE(n.empty(), true);
}

TEST(common, enumname_s) {
    std::string_view n = enum_name<TestEnumSparse::SparseOne>();
    DEBUG("name %s %d", n.data(), n.size());
    ASSERT_NE(n.empty(), true);
}

TEST(common, enum_min_max_s) {
    int min = enum_min_v<TestEnumSparse>;
    int max = enum_max_v<TestEnumSparse>;
    DEBUG("min %d max %d", min, max);
    ASSERT_EQ(min, 1);
    ASSERT_EQ(max, 5);
}

TEST(common, enum_value_s) {
    auto a = TestEnumSparse::SparseOne;
    std::string_view n = enum_name(a);
    DEBUG("name %s %d", n.data(), n.size());
    ASSERT_NE(n.empty(), true);
}

TEST(common, enum_cast_s) {
    auto n = enum_cast<TestEnumSparse>("SparseOne");
    DEBUG("n %d", n);
    ASSERT_EQ(n, TestEnumSparse::SparseOne);
    ASSERT_NE(n, TestEnumSparse::SparseTwo);
}

TEST(common, loc) {
    std::string filename = "test.lua";
    location a(filename, 100, 200);
    std::string left = "test.lua:100,200";
    DEBUG("%s", a.to_string());
    ASSERT_EQ(left, a.to_string());
}

TEST(common, loc_empty) {
    std::string filename = "";
    location a(filename, 0, 0);
    std::string left = "";
    ASSERT_EQ(left, a.to_string());
}

TEST(common, token) {
    std::string filename = "test.lua";
    location a(filename, 100, 200);
    token b(TK_THEN, a);
    std::string left = "then(test.lua:100,200)";
    DEBUG("%s", b.to_string());
    ASSERT_EQ(left, b.to_string());
}

TEST(common, token_empty) {
    std::string filename = "";
    location a(filename, 0, 0);
    token b(TK_THEN, a);
    std::string left = "then";
    ASSERT_EQ(left, b.to_string());
}
