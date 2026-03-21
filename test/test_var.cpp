#include "fakelua.h"
#include "state/State.h"
#include "state/var_string_heap.h"
#include "var/var.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(var, construct) {
    auto s = std::make_shared<State>();

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

    Var v5(s, "hello");
    ASSERT_EQ(v5.Type(), VarType::String);

    Var v6(s, std::string("hello"));
    ASSERT_EQ(v6.Type(), VarType::String);

    Var v7(s, std::move(std::string("hello")));
    ASSERT_EQ(v7.Type(), VarType::String);

    std::string str("hello");
    Var v8(s, str);
    ASSERT_EQ(v8.Type(), VarType::String);

    Var v9;
    v9.SetString(s, str);
    ASSERT_EQ(v9.Type(), VarType::String);

    std::string_view StrView("hello");
    Var v10(s, StrView);
    ASSERT_EQ(v10.Type(), VarType::String);

    Var v11;
    v11.SetString(s, StrView);
    ASSERT_EQ(v11.Type(), VarType::String);
}

TEST(var, set_get) {
    auto s = std::make_shared<State>();

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

    v.SetString(s, "hello");
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "hello");

    v.SetString(s, std::string("hello"));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "hello");

    v.SetString(s, std::move(std::string("hello")));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "hello");
}

TEST(var, VarStringHeap) {
    auto s = std::make_shared<State>();

    VarStringHeap &heap = s->get_var_string_heap();
    auto ret = heap.alloc("hello");
    ASSERT_EQ(ret->Str(), "hello");

    auto ret1 = heap.alloc("hello");
    ASSERT_EQ(ret1->Str(), "hello");
    ASSERT_EQ(ret, ret1);

    ret = heap.alloc("hello");
    ASSERT_EQ(ret->Str(), "hello");
    ASSERT_EQ(ret, ret1);

    ret = heap.alloc("hello1");
    ASSERT_EQ(ret->Str(), "hello1");
    ASSERT_NE(ret, ret1);
}

TEST(var, ToString) {
    auto s = std::make_shared<State>();

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

    v.SetString(s, "hello");
    ASSERT_EQ(v.ToString(), "\"hello\"");

    v.SetTable(s);
    ASSERT_EQ(v.ToString(), std::format("table({})", (void *) v.GetTable()));
}

TEST(var, VarTable) {
    auto s = std::make_shared<State>();

    VarTable vt;

    Var k1((int64_t) 1);
    Var v1((int64_t) 2);
    vt.Set(k1, v1, false);
    ASSERT_EQ(vt.Get(k1), v1);

    Var k2((int64_t) 1);
    Var v2((int64_t) 3);
    vt.Set(k2, v2, false);
    ASSERT_EQ(vt.Get(k2), v2);

    Var k3((int64_t) 2);
    Var v3((int64_t) 4);
    vt.Set(k3, v3, false);
    ASSERT_EQ(vt.Get(k3), v3);

    Var k4(s, "hello");
    Var v4((int64_t) 5);
    vt.Set(k4, v4, false);
    ASSERT_EQ(vt.Get(k4), v4);

    vt.Set(k4, Var(), false);
    ASSERT_EQ(vt.Get(k4), const_null_var);

    Var k5(s, "hello");
    Var v5((int64_t) 6);
    vt.Set(k5, v5, false);
    ASSERT_EQ(vt.Get(k5), v5);

    Var nil;
    vt.Set(k5, nil, false);
    ASSERT_EQ(vt.Get(k5), const_null_var);

    vt.Set(k1, v1, false);
    ASSERT_EQ(vt.Get(k1), v1);

    vt.Set(k1, Var(), false);
    ASSERT_EQ(vt.Get(k1), const_null_var);
}

TEST(var, var_string_heap_reset) {
    auto s = std::make_shared<State>();

    VarStringHeap &heap = s->get_var_string_heap();
    auto str = heap.alloc("hello");
    ASSERT_EQ(str->Str(), "hello");

    auto str1 = heap.alloc("world");
    auto str2 = heap.alloc("world");

    ASSERT_EQ(str1->Str(), "world");
    ASSERT_EQ(str2->Str(), "world");

    ASSERT_EQ(str1->Str().data(), str2->Str().data());

    ASSERT_EQ(heap.size(), 2);
}

TEST(var, SetString) {
    auto s = std::make_shared<State>();

    Var v;
    const std::string str("hello");
    v.SetString(s, str);
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);

    v.SetString(s, std::move(std::string("hello")));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);

    v.SetString(s, "hello");
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);

    v.SetString(s, std::string_view("hello"));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);
}

TEST(var, var_table_keys) {
    auto s = std::make_shared<State>();

    VarTable vt;

    Var k1;
    k1.SetBool(true);
    Var k2;
    k2.SetBool(false);

    Var v1;
    v1.SetInt((int64_t) 1);
    Var v2;
    v2.SetInt((int64_t) 2);

    vt.Set(k1, v1, false);
    vt.Set(k2, v2, false);

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

    vt.Set(k3, v1, false);
    vt.Set(k4, v2, false);

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

    vt.Set(k7, v1, false);
    vt.Set(k8, v2, false);

    v = vt.Get(k7);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 1);

    v = vt.Get(k8);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);
}

TEST(var, var_table_int_float_keys) {
    auto s = std::make_shared<State>();

    VarTable vt;

    Var v1;
    v1.SetInt((int64_t) 1);
    Var v2;
    v2.SetInt((int64_t) 2);

    Var k9;
    k9.SetInt(1);
    Var k10;
    k10.SetFloat(1.0);

    vt.Set(k9, v1, false);
    vt.Set(k10, v2, false);

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

    vt.Set(k11, v1, false);
    vt.Set(k12, v2, false);

    v = vt.Get(k11);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);

    v = vt.Get(k12);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);
}

TEST(var, var_table_nan_keys) {
    auto s = std::make_shared<State>();

    VarTable vt;

    Var k1;
    k1.SetFloat(0.0 / 0.0);
    Var k2;
    k2.SetFloat(0.0 / 0.0);

    Var v1;
    v1.SetInt(1);
    Var v2;
    v2.SetInt(2);

    vt.Set(k1, v1, false);
    vt.Set(k2, v2, false);

    auto v = vt.Get(k1);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);

    v = vt.Get(k2);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 2);
}

TEST(var, var_table_size) {
    auto s = std::make_shared<State>();

    VarTable vt;

    Var v1;
    v1.SetInt((int64_t) 1);
    Var v2;
    v2.SetInt((int64_t) 2);

    Var k9;
    k9.SetInt(1);
    Var k10;
    k10.SetFloat(1.0);

    vt.Set(k9, v1, false);
    vt.Set(k10, v2, false);

    ASSERT_EQ(vt.Size(), 1);

    Var k11;
    k11.SetInt(2);

    vt.Set(k11, v2, false);
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
