#include "fakelua.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_string.h"
#include "gtest/gtest.h"

using namespace fakelua;

namespace {

[[nodiscard]] VarType MakeInvalidVarTypeForTest() {
    volatile int invalid = 999;
    return static_cast<VarType>(invalid);
}

}// namespace

TEST(var, construct) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    ASSERT_EQ(v.Type(), VarType::Nil);

    Var v1;
    ASSERT_EQ(v1.Type(), VarType::Nil);

    Var v2(true);
    ASSERT_EQ(v2.Type(), VarType::Bool);

    Var v3(static_cast<int64_t>(1));
    ASSERT_EQ(v3.Type(), VarType::Int);

    Var v4(1.1);
    ASSERT_EQ(v4.Type(), VarType::Float);

    Var v4_1(1.00);
    ASSERT_EQ(v4_1.Type(), VarType::Float);

    Var v5;
    v5.SetTempString(s, "hello");
    ASSERT_EQ(v5.Type(), VarType::String);

    Var v6;
    v6.SetTempString(s, std::string("hello"));
    ASSERT_EQ(v6.Type(), VarType::String);

    Var v7;
    v7.SetTempString(s, std::move(std::string("hello")));
    ASSERT_EQ(v7.Type(), VarType::String);

    std::string str("hello");
    Var v8;
    v8.SetTempString(s, str);
    ASSERT_EQ(v8.Type(), VarType::String);

    Var v9;
    v9.SetTempString(s, str);
    ASSERT_EQ(v9.Type(), VarType::String);

    std::string_view StrView("hello");
    Var v10;
    v10.SetTempString(s, StrView);
    ASSERT_EQ(v10.Type(), VarType::String);

    Var v11;
    v11.SetTempString(s, StrView);
    ASSERT_EQ(v11.Type(), VarType::String);
}

TEST(var, set_get) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetNil();
    ASSERT_EQ(v.Type(), VarType::Nil);

    v.SetBool(true);
    ASSERT_EQ(v.Type(), VarType::Bool);
    ASSERT_EQ(v.GetBool(), true);

    v.SetInt((int64_t) 1);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 1);

    v.SetFloat(1.1);
    ASSERT_EQ(v.Type(), VarType::Float);
    ASSERT_EQ(v.GetFloat(), 1.1);

    v.SetFloat(1.0);
    ASSERT_EQ(v.Type(), VarType::Float);
    ASSERT_EQ(v.GetFloat(), 1.0);

    v.SetTempString(s, "hello");
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "hello");

    v.SetTempString(s, std::string("hello"));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "hello");

    v.SetTempString(s, std::move(std::string("hello")));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "hello");
}

TEST(var, ToString) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetNil();
    ASSERT_EQ(v.ToString(), "nil");

    v.SetBool(true);
    ASSERT_EQ(v.ToString(), "true");

    v.SetInt((int64_t) 12345);
    ASSERT_EQ(v.ToString(), "12345");

    v.SetFloat(12345.0);
    ASSERT_EQ(v.ToString(), "12345");

    v.SetFloat(12345.1);
    ASSERT_EQ(v.ToString(), "12345.1");

    v.SetFloat(2.1245e-10);
    ASSERT_EQ(v.ToString(), "2.1245e-10");

    v.SetTempString(s, "hello");
    ASSERT_EQ(v.ToString(), "\"hello\"");
}

TEST(var, SetTempString) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    const std::string str("hello");
    v.SetTempString(s, str);
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);

    v.SetTempString(s, std::move(std::string("hello")));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);

    v.SetTempString(s, "hello");
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);

    v.SetTempString(s, std::string_view("hello"));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);
}

TEST(var, test_true) {
    Var v_true(true);
    Var v_false(false);
    Var v_nil;

    ASSERT_TRUE(v_true.TestTrue());
    ASSERT_FALSE(v_false.TestTrue());
    ASSERT_FALSE(v_nil.TestTrue());
    ASSERT_TRUE(Var((int64_t) 10LL).TestTrue());
}

TEST(var, get_string_string_type) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetTempString(s, "test string");
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "test string");
}

TEST(var, get_string_stringid_type) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetConstString(s, "const string");
    ASSERT_EQ(v.Type(), VarType::StringId);
    ASSERT_EQ(v.GetString()->Str(), "const string");
}

TEST(var, tostring_stringid) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetConstString(s, "const");
    ASSERT_EQ(v.ToString(), "\"const\"");

    // Test without quote
    ASSERT_EQ(v.ToString(false), "const");
}

TEST(var, hash_string_types) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    size_t hash1 = v1.Hash();
    ASSERT_NE(hash1, 0);

    Var v2;
    v2.SetConstString(s, "hello");
    size_t hash2 = v2.Hash();
    ASSERT_NE(hash2, 0);

    // Same string content should have same hash
    ASSERT_EQ(hash1, hash2);
}

TEST(var, hash_int_float) {
    Var v1(static_cast<int64_t>(12345));
    size_t hash1 = v1.Hash();
    ASSERT_NE(hash1, 0);

    Var v2(12345.0);
    size_t hash2 = v2.Hash();
    ASSERT_NE(hash2, 0);
}

TEST(var, equal_string_stringid) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");

    Var v2;
    v2.SetConstString(s, "hello");

    // String == StringId should compare content
    ASSERT_TRUE(v1.Equal(v2));
    ASSERT_TRUE(v2.Equal(v1));

    Var v3;
    v3.SetConstString(s, "world");
    ASSERT_FALSE(v1.Equal(v3));
}

TEST(var, equal_string) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");

    Var v2;
    v2.SetTempString(s, "hello");

    // Same string content, different pointers
    ASSERT_TRUE(v1.Equal(v2));

    Var v3;
    v3.SetTempString(s, "world");
    ASSERT_FALSE(v1.Equal(v3));
}

TEST(var, equal_stringid) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetConstString(s, "hello");

    Var v2;
    v2.SetConstString(s, "hello");

    // Same StringId should be equal
    ASSERT_TRUE(v1.Equal(v2));

    Var v3;
    v3.SetConstString(s, "world");
    ASSERT_FALSE(v1.Equal(v3));
}

TEST(var, equal_float_nan) {
    Var v1(std::numeric_limits<double>::quiet_NaN());
    Var v2(std::numeric_limits<double>::quiet_NaN());

    // NaN != NaN in IEEE 754
    ASSERT_FALSE(v1.Equal(v2));
}

TEST(var, hash_nil) {
    Var v;
    size_t hash = v.Hash();
    ASSERT_EQ(hash, 0);
}

TEST(var, hash_default_branch) {
    // Test that Hash() handles all types without crashing.
    // The invalid type falls through to the default case (return 0).
    Var v;
    // Force an invalid type on the Var to exercise the default branch.
    {
        volatile int invalid = 999;
        v.type_ = invalid;
    }
    size_t hash = v.Hash();
    ASSERT_EQ(hash, 0);
}

TEST(var, equal_default_branch) {
    // Test that Equal() with mismatched types returns false.
    Var v1(static_cast<int64_t>(42));
    Var v2;
    {
        volatile int invalid = 999;
        v2.type_ = invalid;
    }
    ASSERT_FALSE(v1.Equal(v2));
    ASSERT_FALSE(v2.Equal(v1));
}

TEST(var, VarTypeToString_all_types) {
    // Verify all valid VarType values produce non-empty strings.
    for (int t = static_cast<int>(VarType::Min); t <= static_cast<int>(VarType::Max); ++t) {
        std::string s = VarTypeToString(static_cast<VarType>(t));
        ASSERT_FALSE(s.empty()) << "VarTypeToString returned empty for type " << t;
    }
}

TEST(var, equal_int_float_cross_type) {
    // Int and Float with the same mathematical value are equal (Lua semantics).
    Var v1(static_cast<int64_t>(42));
    Var v2(42.0);
    ASSERT_TRUE(v1.Equal(v2));
    ASSERT_TRUE(v2.Equal(v1));

    Var v3(static_cast<int64_t>(42));
    Var v4(42.5);
    ASSERT_FALSE(v1.Equal(v4));
    ASSERT_FALSE(v3.Equal(v4));
}

TEST(var, test_true_float) {
    // Non-zero float is truthy.
    ASSERT_TRUE(Var(1.5).TestTrue());
    ASSERT_TRUE(Var(-0.5).TestTrue());
    // Zero float is also truthy in Lua (only nil and false are falsy).
    ASSERT_TRUE(Var(0.0).TestTrue());
}

TEST(var, test_true_string) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetTempString(s, "hello");
    ASSERT_TRUE(v.TestTrue());

    Var empty_str;
    empty_str.SetTempString(s, "");
    ASSERT_TRUE(empty_str.TestTrue());// empty string is still truthy in Lua
}
