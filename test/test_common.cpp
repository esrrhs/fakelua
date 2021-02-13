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

TEST(common, enum_value) {
    auto a = TestEnum::One;
    std::string_view n = enum_name(a);
    ASSERT_NE(n.empty(), true);
    DEBUG("name %s %d", n.data(), n.size());
}

TEST(common, enum_cast) {
    auto n = enum_cast<TestEnum>("One");
    ASSERT_EQ(n, TestEnum::One);
    DEBUG("n %d", n);
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

TEST(common, enum_value_s) {
    auto a = TestEnumSparse::SparseOne;
    std::string_view n = enum_name(a);
    ASSERT_NE(n.empty(), true);
    DEBUG("name %s %d", n.data(), n.size());
}

TEST(common, enum_cast_s) {
    auto n = enum_cast<TestEnumSparse>("SparseOne");
    ASSERT_EQ(n, TestEnumSparse::SparseOne);
    ASSERT_NE(n, TestEnumSparse::SparseTwo);
    DEBUG("n %d", n);
}

TEST(common, loc) {
    location a("test.lua", 100, 200);
    std::string left = "test.lua:100,200";
    ASSERT_EQ(left, std::string(a.to_string()));
    DEBUG("%s", a.to_string());
}

TEST(common, loc_empty) {
    location a;
    std::string left = "";
    ASSERT_EQ(left, std::string(a.to_string()));
}

TEST(common, token) {
    location a("test.lua", 100, 200);
    token b(TK_THEN, a);
    std::string left = "then(test.lua:100,200)";
    ASSERT_EQ(left, std::string(b.to_string()));
    DEBUG("%s", b.to_string());
}

TEST(common, token_empty) {
    location a;
    token b(TK_THEN, a);
    std::string left = "then";
    ASSERT_EQ(left, std::string(b.to_string()));
}
