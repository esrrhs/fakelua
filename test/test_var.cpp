#include "fakelua.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "gtest/gtest.h"

using namespace fakelua;

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

    v.SetTable(s);
    ASSERT_EQ(v.ToString(), std::format("table({})", static_cast<void *>(v.GetTable())));
}

TEST(var, VarTable) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    Var k1(static_cast<int64_t>(1));
    Var v1(static_cast<int64_t>(2));
    vt.Set(s, k1, v1, false);
    ASSERT_EQ(vt.Get(k1), v1);

    Var k2(static_cast<int64_t>(1));
    Var v2(static_cast<int64_t>(3));
    vt.Set(s, k2, v2, false);
    ASSERT_EQ(vt.Get(k2), v2);

    Var k3(static_cast<int64_t>(2));
    Var v3(static_cast<int64_t>(4));
    vt.Set(s, k3, v3, false);
    ASSERT_EQ(vt.Get(k3), v3);

    Var k4;
    k4.SetTempString(s, "hello");
    Var v4(static_cast<int64_t>(5));
    vt.Set(s, k4, v4, false);
    ASSERT_EQ(vt.Get(k4), v4);

    vt.Set(s, k4, Var(), false);
    ASSERT_EQ(vt.Get(k4), const_null_var);

    Var k5;
    k5.SetTempString(s, "hello");
    Var v5(static_cast<int64_t>(6));
    vt.Set(s, k5, v5, false);
    ASSERT_EQ(vt.Get(k5), v5);

    Var nil;
    vt.Set(s, k5, nil, false);
    ASSERT_EQ(vt.Get(k5), const_null_var);

    vt.Set(s, k1, v1, false);
    ASSERT_EQ(vt.Get(k1), v1);

    vt.Set(s, k1, Var(), false);
    ASSERT_EQ(vt.Get(k1), const_null_var);
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

TEST(var, var_table_keys) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    Var k1;
    k1.SetBool(true);
    Var k2;
    k2.SetBool(false);

    Var v1;
    v1.SetInt((int64_t) 1);
    Var v2;
    v2.SetInt((int64_t) 2);

    vt.Set(s, k1, v1, false);
    vt.Set(s, k2, v2, false);

    auto v = vt.Get(k1);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 1);

    v = vt.Get(k2);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);

    Var k3;
    k3.SetFloat(1.2);
    Var k4;
    k4.SetFloat(2.3);

    vt.Set(s, k3, v1, false);
    vt.Set(s, k4, v2, false);

    v = vt.Get(k3);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 1);

    v = vt.Get(k4);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);

    Var k7;
    k7.SetTable(s);
    Var k8;
    k8.SetTable(s);

    vt.Set(s, k7, v1, false);
    vt.Set(s, k8, v2, false);

    v = vt.Get(k7);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 1);

    v = vt.Get(k8);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);
}

TEST(var, var_table_int_float_keys) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    Var v1;
    v1.SetInt((int64_t) 1);
    Var v2;
    v2.SetInt((int64_t) 2);

    Var k9;
    k9.SetInt(1);
    Var k10;
    k10.SetFloat(1.0);

    vt.Set(s, k9, v1, false);
    vt.Set(s, k10, v2, false);

    auto v = vt.Get(k9);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);

    v = vt.Get(k10);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);

    Var k11;
    k11.SetFloat(2.0);
    Var k12;
    k12.SetInt(2);

    vt.Set(s, k11, v1, false);
    vt.Set(s, k12, v2, false);

    v = vt.Get(k11);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);

    v = vt.Get(k12);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);
}

TEST(var, var_table_nan_keys) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    Var k1;
    k1.SetFloat(0.0 / 0.0);
    Var k2;
    k2.SetFloat(0.0 / 0.0);

    Var v1;
    v1.SetInt(1);
    Var v2;
    v2.SetInt(2);

    // In Lua, NaN ~= NaN, so each NaN key is treated as different
    // Setting with k1 and k2 creates two separate entries
    vt.Set(s, k1, v1, false);
    vt.Set(s, k2, v2, false);

    // Size should be 2 because NaN keys are not equal
    ASSERT_EQ(vt.Size(), 2);

    // Getting with NaN keys returns nil because NaN ~= NaN
    // This is the correct Lua semantics
    auto v = vt.Get(k1);
    ASSERT_EQ(v.Type(), VarType::Nil);

    v = vt.Get(k2);
    ASSERT_EQ(v.Type(), VarType::Nil);
}

TEST(var, var_table_size) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    Var v1;
    v1.SetInt((int64_t) 1);
    Var v2;
    v2.SetInt((int64_t) 2);

    Var k9;
    k9.SetInt(1);
    Var k10;
    k10.SetFloat(1.0);

    vt.Set(s, k9, v1, false);
    vt.Set(s, k10, v2, false);

    ASSERT_EQ(vt.Size(), 1);

    Var k11;
    k11.SetInt(2);

    vt.Set(s, k11, v2, false);
    ASSERT_EQ(vt.Size(), 2);
}

TEST(var, arithmetic) {
    Var v1(static_cast<int64_t>(10));
    Var v2(static_cast<int64_t>(3));
    Var res;

    // Plus
    v1.Plus(v2, res);
    ASSERT_EQ(res.GetInt(), 13);
    v1.Plus(Var(2.5), res);
    ASSERT_EQ(res.Type(), VarType::Float);
    ASSERT_EQ(res.GetCalculableNumber(), 12.5);

    // Minus
    v1.Minus(v2, res);
    ASSERT_EQ(res.GetInt(), 7);
    v1.Minus(Var(0.5), res);
    ASSERT_EQ(res.Type(), VarType::Float);
    ASSERT_EQ(res.GetCalculableNumber(), 9.5);

    // Star
    v1.Star(v2, res);
    ASSERT_EQ(res.GetInt(), 30);
    v1.Star(Var(1.5), res);
    ASSERT_EQ(res.Type(), VarType::Float);
    ASSERT_EQ(res.GetCalculableNumber(), 15.0);

    // Slash (always float)
    v1.Slash(v2, res);
    ASSERT_EQ(res.Type(), VarType::Float);
    ASSERT_NEAR(res.GetCalculableNumber(), 3.333333, 0.000001);
    Var((int64_t) 4LL).Slash(Var((int64_t) 2LL), res);
    ASSERT_EQ(res.Type(), VarType::Float);
    ASSERT_EQ(res.GetFloat(), 2.0);

    // DoubleSlash (floor division - Lua semantics: rounds toward negative infinity)
    v1.DoubleSlash(v2, res);
    ASSERT_EQ(res.GetInt(), 3);
    // -10 // 3 = -4 (floor division, not truncation toward zero)
    Var((int64_t) -10LL).DoubleSlash(Var((int64_t) 3LL), res);
    ASSERT_EQ(res.GetInt(), -4);
    // Additional floor division tests
    Var((int64_t) 10LL).DoubleSlash(Var((int64_t) -3LL), res);
    ASSERT_EQ(res.GetInt(), -4);
    Var((int64_t) -10LL).DoubleSlash(Var((int64_t) -3LL), res);
    ASSERT_EQ(res.GetInt(), 3);
    Var(5.0).DoubleSlash(Var((int64_t) 2LL), res);
    ASSERT_EQ(res.Type(), VarType::Float);
    ASSERT_EQ(res.GetFloat(), 2.0);

    // Mod (Lua semantics: a % b = a - b * floor(a / b))
    v1.Mod(v2, res);
    ASSERT_EQ(res.GetInt(), 1);
    // -10 % 3 = 2 (Lua semantics: -10 - 3 * floor(-10/3) = -10 - 3 * (-4) = -10 + 12 = 2)
    Var((int64_t) -10LL).Mod(Var((int64_t) 3LL), res);
    ASSERT_EQ(res.GetInt(), 2);
    // 10 % -3 = -2 (Lua semantics: 10 - (-3) * floor(10/-3) = 10 - (-3) * (-4) = 10 - 12 = -2)
    Var((int64_t) 10LL).Mod(Var((int64_t) -3LL), res);
    ASSERT_EQ(res.GetInt(), -2);
    // -10 % -3 = -1 (Lua semantics: -10 - (-3) * floor(-10/-3) = -10 - (-3) * 3 = -10 + 9 = -1)
    Var((int64_t) -10LL).Mod(Var((int64_t) -3LL), res);
    ASSERT_EQ(res.GetInt(), -1);

    // Pow
    Var((int64_t) 2LL).Pow(Var((int64_t) 3LL), res);
    ASSERT_EQ(res.GetCalculableNumber(), 8.0);
}

TEST(var, bitwise) {
    Var v1(static_cast<int64_t>(0b1010));
    Var v2(static_cast<int64_t>(0b1100));
    Var res;

    // Bitand
    v1.Bitand(v2, res);
    ASSERT_EQ(res.GetInt(), 0b1000);
    // Integral float should be accepted in bitwise ops
    Var(2.0).Bitand(Var((int64_t) 3LL), res);
    ASSERT_EQ(res.GetInt(), 2);
    // Non-integral float should fail
    ASSERT_ANY_THROW(Var(2.1).Bitand(Var((int64_t) 1LL), res));

    // Bitor
    v1.Bitor(v2, res);
    ASSERT_EQ(res.GetInt(), 0b1110);

    // Xor
    v1.Xor(v2, res);
    ASSERT_EQ(res.GetInt(), 0b0110);

    // Shift
    Var((int64_t) 1LL).LeftShift(Var((int64_t) 2LL), res);
    ASSERT_EQ(res.GetInt(), 4);
    Var((int64_t) 4LL).RightShift(Var((int64_t) 2LL), res);
    ASSERT_EQ(res.GetInt(), 1);

    // UnopBitnot
    v1.UnopBitnot(res);
    ASSERT_EQ(res.GetInt(), ~0b1010LL);
    Var(2.0).UnopBitnot(res);
    ASSERT_EQ(res.GetInt(), ~2LL);
    ASSERT_ANY_THROW(Var(2.1).UnopBitnot(res));
}

TEST(var, comparison) {
    Var v1(static_cast<int64_t>(10));
    Var v2(static_cast<int64_t>(20));
    Var res;

    v1.Less(v2, res);
    ASSERT_TRUE(res.GetBool());
    v1.LessEqual(v1, res);
    ASSERT_TRUE(res.GetBool());
    v2.More(v1, res);
    ASSERT_TRUE(res.GetBool());
    v2.MoreEqual(v2, res);
    ASSERT_TRUE(res.GetBool());
    v1.NotEqual(v2, res);
    ASSERT_TRUE(res.GetBool());
}

TEST(var, logical_unary) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    Var v_true(true);
    Var v_false(false);
    Var v_nil;
    Var res;

    ASSERT_TRUE(v_true.TestTrue());
    ASSERT_FALSE(v_false.TestTrue());
    ASSERT_FALSE(v_nil.TestTrue());
    ASSERT_TRUE(Var((int64_t) 10LL).TestTrue());

    v_true.UnopNot(res);
    ASSERT_FALSE(res.GetBool());
    v_nil.UnopNot(res);
    ASSERT_TRUE(res.GetBool());

    Var((int64_t) 10LL).UnopMinus(res);
    ASSERT_EQ(res.GetInt(), -10);

    Var s_var;
    s_var.SetTempString(s, "abc");
    s_var.UnopNumberSign(res);
    ASSERT_EQ(res.GetInt(), 3);

    Var t_var;
    t_var.SetTable(s);
    t_var.TableSet(s, Var((int64_t) 1LL), Var((int64_t) 100LL), false);
    t_var.UnopNumberSign(res);
    ASSERT_EQ(res.GetInt(), 1);
}

TEST(var, concat) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    Var v1;
    v1.SetTempString(s, "hello ");
    Var v2;
    v2.SetTempString(s, "world");
    Var res;

    v1.Concat(s, v2, res);
    ASSERT_EQ(res.GetString()->Str(), "hello world");
}

TEST(var, table_rehash) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    VarTable vt;

    // Trigger rehash by inserting many elements
    for (int i = 0; i < 100; ++i) {
        vt.Set(s, Var(static_cast<int64_t>(i)), Var(static_cast<int64_t>(i * 10)), false);
    }

    ASSERT_EQ(vt.Size(), 100);
    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ(vt.Get(Var(static_cast<int64_t>(i))).GetInt(), i * 10);
    }
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

TEST(var, hash_table) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetTable(s);
    size_t hash = v.Hash();
    ASSERT_NE(hash, 0);
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
    Var v1(std::nan(""));
    Var v2(std::nan(""));

    // NaN == NaN should be false (IEEE 754 and Lua 5.4 semantics)
    ASSERT_FALSE(v1.Equal(v2));
    // NaN is also not equal to itself
    ASSERT_FALSE(v1.Equal(v1));

    Var v3(1.0);
    ASSERT_FALSE(v1.Equal(v3));
}

TEST(var, equal_table) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTable(s);

    // Same table pointer
    Var v2 = v1;
    ASSERT_TRUE(v1.Equal(v2));

    // Different table pointers
    Var v3;
    v3.SetTable(s);
    ASSERT_FALSE(v1.Equal(v3));
}

TEST(var, is_calculable) {
    Var v1(static_cast<int64_t>(10));
    ASSERT_TRUE(v1.IsCalculable());

    Var v2(3.14);
    ASSERT_TRUE(v2.IsCalculable());

    Var v3(true);
    ASSERT_FALSE(v3.IsCalculable());
}

TEST(var, is_calculable_integer) {
    Var v1(static_cast<int64_t>(10));
    ASSERT_TRUE(v1.IsCalculableInteger());

    Var v2(3.14);
    ASSERT_FALSE(v2.IsCalculableInteger());
}

TEST(var, get_calculable_number_from_int) {
    Var v(static_cast<int64_t>(42));
    double result = v.GetCalculableNumber();
    ASSERT_DOUBLE_EQ(result, 42.0);
}

TEST(var, plus_type_error) {
    const FakeluaStateGuard guard;

    Var v1(true);
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Plus(v2, res), std::exception);
}

TEST(var, plus_float_result) {
    Var v1(2.5);
    Var v2(3.7);
    Var res;

    v1.Plus(v2, res);
    // Result should be float since 2.5 + 3.7 = 6.2 is not an integer
    ASSERT_DOUBLE_EQ(res.GetFloat(), 6.2);
}

TEST(var, minus_type_error) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Minus(v2, res), std::exception);
}

TEST(var, star_type_error) {
    Var v1(true);
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Star(v2, res), std::exception);
}

TEST(var, star_float_result) {
    Var v1(2.5);
    Var v2(4.1);
    Var res;

    v1.Star(v2, res);
    // Result should be float since 2.5 * 4.1 = 10.25 is not an integer
    ASSERT_DOUBLE_EQ(res.GetFloat(), 10.25);
}

TEST(var, slash_type_error) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Slash(v2, res), std::exception);
}

TEST(var, doubleslash_type_error) {
    Var v1(true);
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.DoubleSlash(v2, res), std::exception);
}

TEST(var, doubleslash_float_result) {
    Var v1(7.5);
    Var v2(2.0);
    Var res;

    v1.DoubleSlash(v2, res);
    // floor(7.5 / 2.0) = 3.0, float operands keep float result in Lua
    ASSERT_EQ(res.Type(), VarType::Float);
    ASSERT_EQ(res.GetFloat(), 3.0);

    Var v3(-7.5);
    Var v4(2.0);
    v3.DoubleSlash(v4, res);
    // floor(-7.5 / 2.0) = -4.0, float operands keep float result in Lua
    ASSERT_EQ(res.Type(), VarType::Float);
    ASSERT_EQ(res.GetFloat(), -4.0);
}

TEST(var, hash_nil) {
    Var v;
    size_t hash = v.Hash();
    ASSERT_EQ(hash, 0);
}

TEST(var, pow_type_error) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Pow(v2, res), std::exception);
}

TEST(var, mod_type_error) {
    Var v1(true);
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Mod(v2, res), std::exception);
}

TEST(var, mod_float_result) {
    Var v1(7.5);
    Var v2(2.5);
    Var res;

    v1.Mod(v2, res);
    ASSERT_DOUBLE_EQ(res.GetFloat(), 0.0);

    Var v3(7.7);
    Var v4(2.5);
    v3.Mod(v4, res);
    ASSERT_NEAR(res.GetFloat(), 0.2, 0.0001);
}

TEST(var, doubleslash_division_by_zero) {
    Var v1(static_cast<int64_t>(10));
    Var v2(static_cast<int64_t>(0));
    Var res;

    EXPECT_THROW(v1.DoubleSlash(v2, res), std::exception);
}

TEST(var, mod_division_by_zero) {
    Var v1(static_cast<int64_t>(10));
    Var v2(static_cast<int64_t>(0));
    Var res;

    EXPECT_THROW(v1.Mod(v2, res), std::exception);
}

TEST(var, bitand_type_error) {
    Var v1(3.14);
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Bitand(v2, res), std::exception);
}

TEST(var, xor_type_error) {
    Var v1(3.14);
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Xor(v2, res), std::exception);
}

TEST(var, bitor_type_error) {
    Var v1(3.14);
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Bitor(v2, res), std::exception);
}

TEST(var, rightshift_type_error) {
    Var v1(3.14);
    Var v2(static_cast<int64_t>(2));
    Var res;

    EXPECT_THROW(v1.RightShift(v2, res), std::exception);
}

TEST(var, rightshift_negative_shift) {
    Var v1(static_cast<int64_t>(8));
    Var v2(static_cast<int64_t>(-2));
    Var res;

    v1.RightShift(v2, res);
    ASSERT_EQ(res.GetInt(), 32);
}

TEST(var, leftshift_type_error) {
    Var v1(3.14);
    Var v2(static_cast<int64_t>(2));
    Var res;

    EXPECT_THROW(v1.LeftShift(v2, res), std::exception);
}

TEST(var, leftshift_negative_shift) {
    Var v1(static_cast<int64_t>(8));
    Var v2(static_cast<int64_t>(-2));
    Var res;

    v1.LeftShift(v2, res);
    ASSERT_EQ(res.GetInt(), 2);
}

TEST(var, less_type_error) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.Less(v2, res), std::exception);
}

TEST(var, less_float_result) {
    Var v1(2.5);
    Var v2(3.5);
    Var res;

    v1.Less(v2, res);
    ASSERT_TRUE(res.GetBool());
}

TEST(var, less_equal_type_error) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.LessEqual(v2, res), std::exception);
}

TEST(var, less_equal_float_result) {
    Var v1(3.5);
    Var v2(3.5);
    Var res;

    v1.LessEqual(v2, res);
    ASSERT_TRUE(res.GetBool());
}

TEST(var, more_type_error) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.More(v2, res), std::exception);
}

TEST(var, more_float_result) {
    Var v1(3.5);
    Var v2(2.5);
    Var res;

    v1.More(v2, res);
    ASSERT_TRUE(res.GetBool());
}

TEST(var, more_equal_type_error) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    Var v2(static_cast<int64_t>(10));
    Var res;

    EXPECT_THROW(v1.MoreEqual(v2, res), std::exception);
}

TEST(var, more_equal_float_result) {
    Var v1(3.5);
    Var v2(3.5);
    Var res;

    v1.MoreEqual(v2, res);
    ASSERT_TRUE(res.GetBool());
}

TEST(var, equal_with_result) {
    Var v1(static_cast<int64_t>(10));
    Var v2(static_cast<int64_t>(10));
    Var res;

    v1.Equal(v2, res);
    ASSERT_TRUE(res.GetBool());

    Var v3(static_cast<int64_t>(20));
    v1.Equal(v3, res);
    ASSERT_FALSE(res.GetBool());
}

TEST(var, unop_minus_type_error) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    Var res;

    EXPECT_THROW(v1.UnopMinus(res), std::exception);
}

TEST(var, unop_minus_float_result) {
    Var v1(3.14);
    Var res;

    v1.UnopMinus(res);
    ASSERT_DOUBLE_EQ(res.GetFloat(), -3.14);
}

TEST(var, unop_numbersign_type_error) {
    Var v1(static_cast<int64_t>(42));
    Var res;

    EXPECT_THROW(v1.UnopNumberSign(res), std::exception);
}

TEST(var, unop_bitnot_type_error) {
    Var v1(3.14);
    Var res;

    EXPECT_THROW(v1.UnopBitnot(res), std::exception);
}

TEST(var, table_set_type_error) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    Var key(static_cast<int64_t>(1));
    Var val(static_cast<int64_t>(100));

    EXPECT_THROW(v1.TableSet(s, key, val, false), std::exception);
}

TEST(var, table_get) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTable(s);
    Var key(static_cast<int64_t>(1));
    Var val(static_cast<int64_t>(100));
    v1.TableSet(s, key, val, false);

    Var result = v1.TableGet(key);
    ASSERT_EQ(result.GetInt(), 100);
}

TEST(var, table_get_type_error) {
    Var v1;
    Var key(static_cast<int64_t>(1));

    EXPECT_THROW(v1.TableGet(key), std::exception);
}

TEST(var, table_size) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTable(s);
    ASSERT_EQ(v1.TableSize(), 0);

    Var key(static_cast<int64_t>(1));
    Var val(static_cast<int64_t>(100));
    v1.TableSet(s, key, val, false);
    ASSERT_EQ(v1.TableSize(), 1);
}

TEST(var, table_size_type_error) {
    Var v1;

    EXPECT_THROW(v1.TableSize(), std::exception);
}

TEST(var, hash_default_branch) {
    Var v;
    int *type_ptr = reinterpret_cast<int *>(&v);
    *type_ptr = 100;
    size_t hash = v.Hash();
    ASSERT_EQ(hash, 0);
}

TEST(var, equal_default_branch) {
    Var v1(static_cast<int64_t>(10));
    Var v2(static_cast<int64_t>(10));

    int *type_ptr1 = reinterpret_cast<int *>(&v1);
    int *type_ptr2 = reinterpret_cast<int *>(&v2);
    *type_ptr1 = 100;
    *type_ptr2 = 100;

    ASSERT_FALSE(v1.Equal(v2));
}

// VarTable tests for full coverage

TEST(var, vartable_get_empty) {
    const FakeluaStateGuard guard;

    VarTable vt;
    Var key(static_cast<int64_t>(1));
    Var result = vt.Get(key);
    ASSERT_EQ(result.Type(), VarType::Nil);
}

TEST(var, vartable_get_quick_data_all_positions) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert 8 elements to fill quick_data_
    for (int i = 0; i < 8; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Get from all positions including position 6 and 7
    for (int i = 0; i < 8; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 100);
    }
}

TEST(var, vartable_get_hash_mode_not_found) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert more than 8 elements to trigger hash mode
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Get a key that doesn't exist (should traverse hash chain)
    Var key(static_cast<int64_t>(1000));
    Var result = vt.Get(key);
    ASSERT_EQ(result.Type(), VarType::Nil);
}

TEST(var, vartable_set_nil_key_exception) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;
    Var key;
    Var val(static_cast<int64_t>(100));

    EXPECT_THROW(vt.Set(s, key, val, false), std::exception);
}

TEST(var, vartable_get_nil_key_exception) {
    VarTable vt;
    Var key;

    EXPECT_THROW(vt.Get(key), std::exception);
}

TEST(var, vartable_delete_empty) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;
    Var key(static_cast<int64_t>(1));
    Var nil_val;

    // Delete from empty table should return without error
    vt.Set(s, key, nil_val, false);
    ASSERT_EQ(vt.Size(), 0);
}

TEST(var, vartable_delete_quick_data_all_positions) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert 8 elements
    for (int i = 0; i < 8; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }
    ASSERT_EQ(vt.Size(), 8);

    Var nil_val;

    // Delete from position 0
    vt.Set(s, Var(static_cast<int64_t>(0)), nil_val, false);
    ASSERT_EQ(vt.Size(), 7);

    // Delete from position 1 (now has different value due to swap)
    vt.Set(s, Var(static_cast<int64_t>(1)), nil_val, false);
    ASSERT_EQ(vt.Size(), 6);

    // Delete from position 2
    vt.Set(s, Var(static_cast<int64_t>(2)), nil_val, false);
    ASSERT_EQ(vt.Size(), 5);

    // Delete from position 3
    vt.Set(s, Var(static_cast<int64_t>(3)), nil_val, false);
    ASSERT_EQ(vt.Size(), 4);

    // Delete from position 4
    vt.Set(s, Var(static_cast<int64_t>(4)), nil_val, false);
    ASSERT_EQ(vt.Size(), 3);

    // Delete from position 5
    vt.Set(s, Var(static_cast<int64_t>(5)), nil_val, false);
    ASSERT_EQ(vt.Size(), 2);

    // Delete from position 6
    vt.Set(s, Var(static_cast<int64_t>(6)), nil_val, false);
    ASSERT_EQ(vt.Size(), 1);

    // Delete from position 7
    vt.Set(s, Var(static_cast<int64_t>(7)), nil_val, false);
    ASSERT_EQ(vt.Size(), 0);
}

TEST(var, vartable_update_quick_data_all_positions) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert 8 elements
    for (int i = 0; i < 8; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Update each position
    for (int i = 0; i < 8; ++i) {
        Var key(static_cast<int64_t>(i));
        Var new_val(static_cast<int64_t>(i * 1000));
        vt.Set(s, key, new_val, false);
    }

    // Verify updates
    for (int i = 0; i < 8; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 1000);
    }
}

TEST(var, vartable_delete_hash_mode_not_found) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert more than 8 elements to trigger hash mode
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    Var nil_val;

    // Delete a key that doesn't exist
    vt.Set(s, Var(static_cast<int64_t>(1000)), nil_val, false);
    ASSERT_EQ(vt.Size(), 20);
}

TEST(var, vartable_delete_hash_mode_found) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert more than 8 elements to trigger hash mode
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    Var nil_val;

    // Delete existing keys
    vt.Set(s, Var(static_cast<int64_t>(0)), nil_val, false);
    ASSERT_EQ(vt.Size(), 19);

    vt.Set(s, Var(static_cast<int64_t>(10)), nil_val, false);
    ASSERT_EQ(vt.Size(), 18);

    vt.Set(s, Var(static_cast<int64_t>(19)), nil_val, false);
    ASSERT_EQ(vt.Size(), 17);
}

TEST(var, vartable_delete_with_collision) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert many elements to ensure hash collisions
    for (int i = 0; i < 100; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Delete various keys to test collision chain handling
    for (int i = 0; i < 50; ++i) {
        Var nil_val;
        vt.Set(s, Var(static_cast<int64_t>(i)), nil_val, false);
    }

    ASSERT_EQ(vt.Size(), 50);

    // Verify remaining keys still work
    for (int i = 50; i < 100; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 100);
    }
}

TEST(var, vartable_rehash_expand) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert exactly 8 elements (fills quick_data_)
    for (int i = 0; i < 8; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }
    ASSERT_EQ(vt.Size(), 8);

    // Insert 9th element triggers rehash to hash mode
    Var key9(static_cast<int64_t>(8));
    Var val9(static_cast<int64_t>(800));
    vt.Set(s, key9, val9, false);
    ASSERT_EQ(vt.Size(), 9);

    // Verify all elements still accessible
    for (int i = 0; i < 9; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 100);
    }
}

TEST(var, vartable_large_scale) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert many elements to trigger multiple rehashes
    for (int i = 0; i < 1000; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    ASSERT_EQ(vt.Size(), 1000);

    // Verify all elements
    for (int i = 0; i < 1000; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 100);
    }

    // Delete half
    for (int i = 0; i < 500; ++i) {
        Var nil_val;
        vt.Set(s, Var(static_cast<int64_t>(i)), nil_val, false);
    }

    ASSERT_EQ(vt.Size(), 500);

    // Verify remaining
    for (int i = 500; i < 1000; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 100);
    }
}

TEST(var, vartable_string_keys) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Test with string keys
    Var key1;
    key1.SetTempString(s, "hello");
    Var val1(static_cast<int64_t>(100));
    vt.Set(s, key1, val1, false);

    Var key2;
    key2.SetTempString(s, "world");
    Var val2(static_cast<int64_t>(200));
    vt.Set(s, key2, val2, false);

    ASSERT_EQ(vt.Get(key1).GetInt(), 100);
    ASSERT_EQ(vt.Get(key2).GetInt(), 200);

    Var nil_val;
    vt.Set(s, key1, nil_val, false);
    ASSERT_EQ(vt.Get(key1).Type(), VarType::Nil);
    ASSERT_EQ(vt.Size(), 1);
}

TEST(var, vartable_update_existing_in_hash_mode) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert more than 8 elements to trigger hash mode
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Update existing values
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var new_val(static_cast<int64_t>(i * 1000));
        vt.Set(s, key, new_val, false);
    }

    // Verify updates
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 1000);
    }
}

TEST(var, vartable_delete_quick_data_position_4_7) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;
    Var nil_val;

    // Insert 5 elements and delete position 4
    for (int i = 0; i < 5; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }
    vt.Set(s, Var(static_cast<int64_t>(4)), nil_val, false);
    ASSERT_EQ(vt.Size(), 4);

    // Reset and test position 5
    VarTable vt2;
    for (int i = 0; i < 6; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt2.Set(s, key, val, false);
    }
    vt2.Set(s, Var(static_cast<int64_t>(5)), nil_val, false);
    ASSERT_EQ(vt2.Size(), 5);

    // Reset and test position 6
    VarTable vt3;
    for (int i = 0; i < 7; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt3.Set(s, key, val, false);
    }
    vt3.Set(s, Var(static_cast<int64_t>(6)), nil_val, false);
    ASSERT_EQ(vt3.Size(), 6);

    // Reset and test position 7 (last element, no swap needed)
    VarTable vt4;
    for (int i = 0; i < 8; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt4.Set(s, key, val, false);
    }
    vt4.Set(s, Var(static_cast<int64_t>(7)), nil_val, false);
    ASSERT_EQ(vt4.Size(), 7);
}

TEST(var, vartable_get_hash_mode_chain_traversal) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert enough elements to ensure hash collisions and chain traversal
    for (int i = 0; i < 50; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Get non-existent key to force chain traversal
    Var key(static_cast<int64_t>(1000));
    Var result = vt.Get(key);
    ASSERT_EQ(result.Type(), VarType::Nil);
}

TEST(var, vartable_delete_hash_mode_chain_node) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;
    Var nil_val;

    // Insert many elements to ensure hash collisions
    for (int i = 0; i < 50; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Delete from hash mode - should cover various deletion paths
    // Delete first element (may be at different position after rehash)
    vt.Set(s, Var(static_cast<int64_t>(0)), nil_val, false);

    // Delete middle elements
    vt.Set(s, Var(static_cast<int64_t>(25)), nil_val, false);
    vt.Set(s, Var(static_cast<int64_t>(30)), nil_val, false);

    ASSERT_EQ(vt.Size(), 47);
}

TEST(var, vartable_delete_hash_mode_main_bucket_with_next) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;
    Var nil_val;

    // Insert elements to create hash collisions
    // Using specific values to increase chance of collisions
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i * 16));// Multiples of 16 may collide
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Delete some to test different deletion paths
    vt.Set(s, Var(static_cast<int64_t>(0)), nil_val, false);
    vt.Set(s, Var(static_cast<int64_t>(16)), nil_val, false);
    vt.Set(s, Var(static_cast<int64_t>(32)), nil_val, false);

    ASSERT_EQ(vt.Size(), 17);
}

TEST(var, vartable_insert_raw_chain) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert many elements to trigger InsertRaw chain behavior
    for (int i = 0; i < 100; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    ASSERT_EQ(vt.Size(), 100);

    // Verify all inserted correctly via chain
    for (int i = 0; i < 100; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 100);
    }
}

TEST(var, vartable_rehash_from_hash_mode) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert enough to get into hash mode
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Insert more to trigger rehash from hash mode (line 314)
    for (int i = 20; i < 100; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    ASSERT_EQ(vt.Size(), 100);

    // Verify all elements
    for (int i = 0; i < 100; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 100);
    }
}

TEST(var, vartable_delete_from_full_quick_data) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Fill quick_data_ completely
    for (int i = 0; i < 8; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Delete each one by one to cover all delete paths in quick mode
    for (int i = 7; i >= 0; --i) {
        Var nil_val;
        vt.Set(s, Var(static_cast<int64_t>(i)), nil_val, false);
        ASSERT_EQ(vt.Size(), static_cast<size_t>(i));
    }
}

TEST(var, vartable_update_after_rehash) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert to trigger rehash
    for (int i = 0; i < 50; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Update values after rehash (covers InsertRaw update path)
    for (int i = 0; i < 50; ++i) {
        Var key(static_cast<int64_t>(i));
        Var new_val(static_cast<int64_t>(i * 1000));
        vt.Set(s, key, new_val, false);
    }

    // Verify
    for (int i = 0; i < 50; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 1000);
    }
}

TEST(var, vartable_delete_nonexistent_from_hash_mode) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;
    Var nil_val;

    // Insert elements to enter hash mode
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Try to delete non-existent key from hash mode
    vt.Set(s, Var(static_cast<int64_t>(1000)), nil_val, false);
    ASSERT_EQ(vt.Size(), 20);
}

TEST(var, vartable_stress_test) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert many
    for (int i = 0; i < 500; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Update many
    for (int i = 0; i < 500; ++i) {
        Var key(static_cast<int64_t>(i));
        Var new_val(static_cast<int64_t>(i * 1000));
        vt.Set(s, key, new_val, false);
    }

    // Delete half
    for (int i = 0; i < 250; ++i) {
        Var nil_val;
        vt.Set(s, Var(static_cast<int64_t>(i)), nil_val, false);
    }

    // Verify remaining
    for (int i = 250; i < 500; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 1000);
    }

    ASSERT_EQ(vt.Size(), 250);
}

TEST(var, vartable_delete_quick_data_swap_paths) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var nil_val;

    // Test delete position 4 with count > 5 (line 113)
    {
        VarTable vt;
        for (int i = 0; i < 6; ++i) {
            Var key(static_cast<int64_t>(i));
            Var val(static_cast<int64_t>(i * 100));
            vt.Set(s, key, val, false);
        }
        vt.Set(s, Var(static_cast<int64_t>(4)), nil_val, false);
        ASSERT_EQ(vt.Size(), 5);
    }

    // Test delete position 5 with count > 6 (line 120)
    {
        VarTable vt;
        for (int i = 0; i < 7; ++i) {
            Var key(static_cast<int64_t>(i));
            Var val(static_cast<int64_t>(i * 100));
            vt.Set(s, key, val, false);
        }
        vt.Set(s, Var(static_cast<int64_t>(5)), nil_val, false);
        ASSERT_EQ(vt.Size(), 6);
    }

    // Test delete position 6 with count > 7 (line 127)
    {
        VarTable vt;
        for (int i = 0; i < 8; ++i) {
            Var key(static_cast<int64_t>(i));
            Var val(static_cast<int64_t>(i * 100));
            vt.Set(s, key, val, false);
        }
        vt.Set(s, Var(static_cast<int64_t>(6)), nil_val, false);
        ASSERT_EQ(vt.Size(), 7);
    }
}

TEST(var, vartable_get_empty_bucket_in_hash_mode) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert some elements to get into hash mode
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Get a key that will hash to an empty bucket
    // We use a large key value that likely hashes differently
    Var key(static_cast<int64_t>(99999999));
    Var result = vt.Get(key);
    ASSERT_EQ(result.Type(), VarType::Nil);
}

TEST(var, vartable_delete_empty_bucket_in_hash_mode) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;
    Var nil_val;

    // Insert some elements to get into hash mode
    for (int i = 0; i < 20; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Delete a key that will hash to an empty bucket (line 142)
    vt.Set(s, Var(static_cast<int64_t>(99999999)), nil_val, false);
    ASSERT_EQ(vt.Size(), 20);
}

TEST(var, vartable_get_with_chain_traversal) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert elements with similar hash values to create chains
    for (int i = 0; i < 100; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Get a key that requires chain traversal (line 64)
    Var key(static_cast<int64_t>(50));
    Var result = vt.Get(key);
    ASSERT_EQ(result.GetInt(), 5000);

    // Get non-existent key requiring full chain traversal
    Var key2(static_cast<int64_t>(1000));
    Var result2 = vt.Get(key2);
    ASSERT_EQ(result2.Type(), VarType::Nil);
}

TEST(var, vartable_find_key_in_chain) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    // Insert enough elements to ensure hash collisions and chain formation
    // The hash function uses the key value, so we insert sequential keys
    // to potentially create collisions
    for (int i = 0; i < 200; ++i) {
        Var key(static_cast<int64_t>(i));
        Var val(static_cast<int64_t>(i * 100));
        vt.Set(s, key, val, false);
    }

    // Now try to get keys that might be deep in a chain
    // This should trigger line 64 when the key is found after traversing chain
    for (int i = 0; i < 200; ++i) {
        Var key(static_cast<int64_t>(i));
        Var result = vt.Get(key);
        ASSERT_EQ(result.GetInt(), i * 100);
    }
}

TEST(var, VarTypeToString_all_types) {
    EXPECT_EQ(VarTypeToString(VarType::Nil), "Nil");
    EXPECT_EQ(VarTypeToString(VarType::Bool), "Bool");
    EXPECT_EQ(VarTypeToString(VarType::Int), "Int");
    EXPECT_EQ(VarTypeToString(VarType::Float), "Float");
    EXPECT_EQ(VarTypeToString(VarType::String), "String");
    EXPECT_EQ(VarTypeToString(VarType::StringId), "StringId");
    EXPECT_EQ(VarTypeToString(VarType::Table), "Table");

    // Test default case with invalid value
    auto invalid_type = static_cast<VarType>(999);
    EXPECT_EQ(VarTypeToString(invalid_type), "UNKNOWN");
}
