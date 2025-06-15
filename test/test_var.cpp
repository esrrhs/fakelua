#include "fakelua.h"
#include "state/state.h"
#include "state/var_string_heap.h"
#include "var/var.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(var, construct) {
    auto s = std::make_shared<state>();

    var v;
    ASSERT_EQ(v.type(), var_type::VAR_NIL);

    var v1;
    ASSERT_EQ(v1.type(), var_type::VAR_NIL);

    var v2(true);
    ASSERT_EQ(v2.type(), var_type::VAR_BOOL);

    var v3((int64_t) 1);
    ASSERT_EQ(v3.type(), var_type::VAR_INT);

    var v4(1.1);
    ASSERT_EQ(v4.type(), var_type::VAR_FLOAT);

    var v4_1(1.00);
    ASSERT_EQ(v4_1.type(), var_type::VAR_INT);

    var v5(s, "hello");
    ASSERT_EQ(v5.type(), var_type::VAR_STRING);

    var v6(s, std::string("hello"));
    ASSERT_EQ(v6.type(), var_type::VAR_STRING);

    var v7(s, std::move(std::string("hello")));
    ASSERT_EQ(v7.type(), var_type::VAR_STRING);

    std::string str("hello");
    var v8(s, str);
    ASSERT_EQ(v8.type(), var_type::VAR_STRING);

    var v9;
    v9.set_string(s, str);
    ASSERT_EQ(v9.type(), var_type::VAR_STRING);

    std::string_view str_view("hello");
    var v10(s, str_view);
    ASSERT_EQ(v10.type(), var_type::VAR_STRING);

    var v11;
    v11.set_string(s, str_view);
    ASSERT_EQ(v11.type(), var_type::VAR_STRING);
}

TEST(var, set_get) {
    auto s = std::make_shared<state>();

    var v;
    v.set_nil();
    ASSERT_EQ(v.type(), var_type::VAR_NIL);

    v.set_bool(true);
    ASSERT_EQ(v.type(), var_type::VAR_BOOL);
    ASSERT_EQ(v.get_bool(), true);

    v.set_int((int64_t) 1);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 1);

    v.set_float(1.1);
    ASSERT_EQ(v.type(), var_type::VAR_FLOAT);
    ASSERT_EQ(v.get_float(), 1.1);

    v.set_float(1.0);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 1);

    v.set_string(s, "hello");
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string()->str(), "hello");

    v.set_string(s, std::string("hello"));
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string()->str(), "hello");

    v.set_string(s, std::move(std::string("hello")));
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string()->str(), "hello");
}

TEST(var, var_string_heap) {
    auto s = std::make_shared<state>();

    var_string_heap &heap = s->get_var_string_heap();
    auto ret = heap.alloc("hello", false);
    ASSERT_EQ(ret->str(), "hello");

    auto ret1 = heap.alloc("hello", true);
    ASSERT_EQ(ret1->str(), "hello");
    ASSERT_EQ(ret, ret1);

    heap.reset();

    ret = heap.alloc("hello", true);
    ASSERT_EQ(ret->str(), "hello");
    ASSERT_EQ(ret, ret1);

    ret = heap.alloc("hello1", false);
    ASSERT_EQ(ret->str(), "hello1");
    ASSERT_NE(ret, ret1);
}

TEST(var, to_string) {
    auto s = std::make_shared<state>();

    var v;
    v.set_nil();
    ASSERT_EQ(v.to_string(), "nil");

    v.set_bool(true);
    ASSERT_EQ(v.to_string(), "true");

    v.set_int((int64_t) 12345);
    ASSERT_EQ(v.to_string(), "12345");

    v.set_float(12345.0);
    ASSERT_EQ(v.to_string(), "12345");

    v.set_float(12345.1);
    ASSERT_EQ(v.to_string(), "12345.1");

    v.set_float(2.1245e-10);
    ASSERT_EQ(v.to_string(), "2.1245e-10");

    v.set_string(s, "hello");
    ASSERT_EQ(v.to_string(), "\"hello\"");

    v.set_table(s);
    ASSERT_EQ(v.to_string(), std::format("table({})", (void *) v.get_table()));
}

TEST(var, var_table) {
    auto s = std::make_shared<state>();

    var_table vt;

    var k1((int64_t) 1);
    var v1((int64_t) 2);
    vt.set(k1, v1, false);
    ASSERT_EQ(vt.get(k1), v1);

    var k2((int64_t) 1);
    var v2((int64_t) 3);
    vt.set(k2, v2, false);
    ASSERT_EQ(vt.get(k2), v2);

    var k3((int64_t) 2);
    var v3((int64_t) 4);
    vt.set(k3, v3, false);
    ASSERT_EQ(vt.get(k3), v3);

    var k4(s, "hello");
    var v4((int64_t) 5);
    vt.set(k4, v4, false);
    ASSERT_EQ(vt.get(k4), v4);

    vt.set(k4, var(), false);
    ASSERT_EQ(vt.get(k4), const_null_var);

    var k5(s, "hello");
    var v5((int64_t) 6);
    vt.set(k5, v5, false);
    ASSERT_EQ(vt.get(k5), v5);

    var nil;
    vt.set(k5, nil, false);
    ASSERT_EQ(vt.get(k5), const_null_var);

    vt.set(k1, v1, false);
    ASSERT_EQ(vt.get(k1), v1);

    vt.set(k1, var(), false);
    ASSERT_EQ(vt.get(k1), const_null_var);
}

TEST(var, var_string_heap_reset) {
    auto s = std::make_shared<state>();

    var_string_heap &heap = s->get_var_string_heap();
    auto str = heap.alloc("hello", false);
    ASSERT_EQ(str->str(), "hello");

    auto str1 = heap.alloc("world", false);
    auto str2 = heap.alloc("world", true);

    ASSERT_EQ(str1->str(), "world");
    ASSERT_EQ(str2->str(), "world");

    ASSERT_EQ(str1->str().data(), str2->str().data());

    ASSERT_EQ(heap.size(), 2);

    heap.reset();

    ASSERT_EQ(heap.size(), 1);
}

TEST(var, set_string) {
    auto s = std::make_shared<state>();

    var v;
    const std::string str("hello");
    v.set_string(s.get(), str);
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string()->str(), str);

    v.set_string(s.get(), std::move(std::string("hello")));
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string()->str(), str);

    v.set_string(s.get(), "hello");
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string()->str(), str);

    v.set_string(s.get(), std::string_view("hello"));
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string()->str(), str);

    v.set_const(true);
    ASSERT_EQ(v.to_string(), "\"hello\"(const)");

    v.set_variadic(true);
    ASSERT_EQ(v.to_string(), "\"hello\"(const)(variadic)");
}

TEST(var, var_table_keys) {
    auto s = std::make_shared<state>();

    var_table vt;

    var k1;
    k1.set_bool(true);
    var k2;
    k2.set_bool(false);

    var v1;
    v1.set_int((int64_t) 1);
    var v2;
    v2.set_int((int64_t) 2);

    vt.set(k1, v1, false);
    vt.set(k2, v2, false);

    auto v = vt.get(k1);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 1);

    v = vt.get(k2);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 2);

    var k3;
    k3.set_float(1.2);
    var k4;
    k4.set_float(2.3);

    vt.set(k3, v1, false);
    vt.set(k4, v2, false);

    v = vt.get(k3);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 1);

    v = vt.get(k4);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 2);

    var k7;
    k7.set_table(s);
    var k8;
    k8.set_table(s);

    vt.set(k7, v1, false);
    vt.set(k8, v2, false);

    v = vt.get(k7);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 1);

    v = vt.get(k8);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 2);
}

TEST(var, var_table_int_float_keys) {
    auto s = std::make_shared<state>();

    var_table vt;

    var v1;
    v1.set_int((int64_t) 1);
    var v2;
    v2.set_int((int64_t) 2);

    var k9;
    k9.set_int(1);
    var k10;
    k10.set_float(1.0);

    vt.set(k9, v1, false);
    vt.set(k10, v2, false);

    auto v = vt.get(k9);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 2);

    v = vt.get(k10);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 2);

    var k11;
    k11.set_float(2.0);
    var k12;
    k12.set_int(2);

    vt.set(k11, v1, false);
    vt.set(k12, v2, false);

    v = vt.get(k11);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 2);

    v = vt.get(k12);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 2);
}

TEST(var, var_table_nan_keys) {
    auto s = std::make_shared<state>();

    var_table vt;

    var k1;
    k1.set_float(0.0 / 0.0);
    var k2;
    k2.set_float(0.0 / 0.0);

    var v1;
    v1.set_int(1);
    var v2;
    v2.set_int(2);

    vt.set(k1, v1, false);
    vt.set(k2, v2, false);

    auto v = vt.get(k1);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 2);

    v = vt.get(k2);
    ASSERT_EQ(v.type(), var_type::VAR_INT);
    ASSERT_EQ(v.get_int(), 2);
}

TEST(var, var_table_size) {
    auto s = std::make_shared<state>();

    var_table vt;

    var v1;
    v1.set_int((int64_t) 1);
    var v2;
    v2.set_int((int64_t) 2);

    var k9;
    k9.set_int(1);
    var k10;
    k10.set_float(1.0);

    vt.set(k9, v1, false);
    vt.set(k10, v2, false);

    ASSERT_EQ(vt.size(), 1);

    var k11;
    k11.set_int(2);

    vt.set(k11, v2, false);
    ASSERT_EQ(vt.size(), 2);
}

std::vector<int> TEST_CALL_VAR_CALL_SEQ;
int TEST_CALL_VAR_SEQ = 0;
var *CALL_VAR_FUNC_TEST_FUNC(...) {
    TEST_CALL_VAR_CALL_SEQ.push_back(TEST_CALL_VAR_SEQ);
    TEST_CALL_VAR_SEQ++;
    return nullptr;
}

TEST(var, call_var_func) {
    TEST_CALL_VAR_CALL_SEQ.clear();
    TEST_CALL_VAR_SEQ = 0;
    std::vector<var *> args;
    for (int i = 0; i <= 32; i++) {
        args.resize(i);
        call_var_func(CALL_VAR_FUNC_TEST_FUNC, args);
    }

    ASSERT_EQ(TEST_CALL_VAR_CALL_SEQ.size(), 33);
    for (int i = 0; i <= 32; i++) {
        ASSERT_EQ(TEST_CALL_VAR_CALL_SEQ[i], i);
    }
}
