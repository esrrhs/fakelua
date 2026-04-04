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

    Var v3((int64_t) 1);
    ASSERT_EQ(v3.Type(), VarType::Int);

    Var v4(1.1);
    ASSERT_EQ(v4.Type(), VarType::Float);

    Var v4_1(1.00);
    ASSERT_EQ(v4_1.Type(), VarType::Int);

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
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 1);

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
    ASSERT_EQ(v.ToString(), std::format("table({})", (void *) v.GetTable()));
}

TEST(var, VarTable) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    VarTable vt;

    Var k1((int64_t) 1);
    Var v1((int64_t) 2);
    vt.Set(s, k1, v1, false);
    ASSERT_EQ(vt.Get(k1), v1);

    Var k2((int64_t) 1);
    Var v2((int64_t) 3);
    vt.Set(s, k2, v2, false);
    ASSERT_EQ(vt.Get(k2), v2);

    Var k3((int64_t) 2);
    Var v3((int64_t) 4);
    vt.Set(s, k3, v3, false);
    ASSERT_EQ(vt.Get(k3), v3);

    Var k4;
    k4.SetTempString(s, "hello");
    Var v4((int64_t) 5);
    vt.Set(s, k4, v4, false);
    ASSERT_EQ(vt.Get(k4), v4);

    vt.Set(s, k4, Var(), false);
    ASSERT_EQ(vt.Get(k4), const_null_var);

    Var k5;
    k5.SetTempString(s, "hello");
    Var v5((int64_t) 6);
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

    vt.Set(s, k1, v1, false);
    vt.Set(s, k2, v2, false);

    auto v = vt.Get(k1);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);

    v = vt.Get(k2);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);
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

std::vector<int> TEST_CALL_VAR_CALL_SEQ;
int TEST_CALL_VAR_SEQ = 0;
Var *CALL_VAR_FUNC_TEST_FUNC(...) {
    TEST_CALL_VAR_CALL_SEQ.push_back(TEST_CALL_VAR_SEQ);
    TEST_CALL_VAR_SEQ++;
    return nullptr;
}

TEST(var, call_var_func) {
    TEST_CALL_VAR_CALL_SEQ.clear();
    TEST_CALL_VAR_SEQ = 0;
    std::vector<Var *> args;
    for (int i = 0; i <= 32; i++) {
        args.resize(i);
        CallVarFunc(CALL_VAR_FUNC_TEST_FUNC, args);
    }

    ASSERT_EQ(TEST_CALL_VAR_CALL_SEQ.size(), 33);
    for (int i = 0; i <= 32; i++) {
        ASSERT_EQ(TEST_CALL_VAR_CALL_SEQ[i], i);
    }
}

TEST(var, arithmetic) {
    Var v1((int64_t) 10);
    Var v2((int64_t) 3);
    Var res;

    // Plus
    v1.Plus(v2, res);
    ASSERT_EQ(res.GetInt(), 13);
    v1.Plus(Var(2.5), res);
    ASSERT_EQ(res.GetCalculableNumber(), 12.5);

    // Minus
    v1.Minus(v2, res);
    ASSERT_EQ(res.GetInt(), 7);
    v1.Minus(Var(0.5), res);
    ASSERT_EQ(res.GetCalculableNumber(), 9.5);

    // Star
    v1.Star(v2, res);
    ASSERT_EQ(res.GetInt(), 30);
    v1.Star(Var(1.5), res);
    ASSERT_EQ(res.GetCalculableNumber(), 15.0);

    // Slash (always float)
    v1.Slash(v2, res);
    ASSERT_NEAR(res.GetCalculableNumber(), 3.333333, 0.000001);

    // DoubleSlash (floor division)
    v1.DoubleSlash(v2, res);
    ASSERT_EQ(res.GetInt(), 3);
    Var((int64_t) -10LL).DoubleSlash(Var((int64_t) 3LL), res);
    ASSERT_EQ(res.GetInt(), -3);

    // Mod
    v1.Mod(v2, res);
    ASSERT_EQ(res.GetInt(), 1);

    // Pow
    Var((int64_t) 2LL).Pow(Var((int64_t) 3LL), res);
    ASSERT_EQ(res.GetCalculableNumber(), 8.0);
}

TEST(var, bitwise) {
    Var v1((int64_t) 0b1010);
    Var v2((int64_t) 0b1100);
    Var res;

    // Bitand
    v1.Bitand(v2, res);
    ASSERT_EQ(res.GetInt(), 0b1000);

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
}

TEST(var, comparison) {
    Var v1((int64_t) 10);
    Var v2((int64_t) 20);
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
        vt.Set(s, Var((int64_t) i), Var((int64_t) (i * 10)), false);
    }

    ASSERT_EQ(vt.Size(), 100);
    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ(vt.Get(Var((int64_t) i)).GetInt(), i * 10);
    }
}
