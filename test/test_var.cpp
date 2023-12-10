#include "fakelua.h"
#include "state/state.h"
#include "state/var_pool.h"
#include "state/var_string_heap.h"
#include "var/var.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(var, construct) {
    auto s = std::make_shared<state>();

    var v;
    ASSERT_EQ(v.type(), var_type::VAR_NIL);

    var v1(nullptr);
    ASSERT_EQ(v1.type(), var_type::VAR_NIL);

    var v2(true);
    ASSERT_EQ(v2.type(), var_type::VAR_BOOL);

    var v3((int64_t) 1);
    ASSERT_EQ(v3.type(), var_type::VAR_INT);

    var v4(1.0);
    ASSERT_EQ(v4.type(), var_type::VAR_FLOAT);

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

    v.set_float(1.0);
    ASSERT_EQ(v.type(), var_type::VAR_FLOAT);
    ASSERT_EQ(v.get_float(), 1.0);

    v.set_string(s, "hello");
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string(), "hello");

    v.set_string(s, std::string("hello"));
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string(), "hello");

    v.set_string(s, std::move(std::string("hello")));
    ASSERT_EQ(v.type(), var_type::VAR_STRING);
    ASSERT_EQ(v.get_string(), "hello");
}

TEST(var, var_string_heap) {
    var_string_heap heap;
    auto ret = heap.alloc("hello");
    ASSERT_EQ(ret.length() <= MAX_SHORT_STR_LEN, true);
    ASSERT_EQ(ret, "hello");

    auto ret1 = heap.alloc("hello");
    ASSERT_EQ(ret.length() <= MAX_SHORT_STR_LEN, true);
    ASSERT_EQ(ret1, "hello");
    ASSERT_EQ(ret.data(), ret1.data());

    heap.reset();

    ret = heap.alloc("hello");
    ASSERT_EQ(ret.length() <= MAX_SHORT_STR_LEN, true);
    ASSERT_EQ(ret, "hello");

    ret1 = heap.alloc("hello");
    ASSERT_EQ(ret.length() <= MAX_SHORT_STR_LEN, true);
    ASSERT_EQ(ret1, "hello");
    ASSERT_EQ(ret.data(), ret1.data());

    ret = heap.alloc("hello1");
    ASSERT_EQ(ret.length() <= MAX_SHORT_STR_LEN, true);
    ASSERT_EQ(ret, "hello1");

    std::string str;
    for (int i = 0; i < MAX_SHORT_STR_LEN; ++i) {
        str += "a";
    }

    ret = heap.alloc(str);
    ASSERT_EQ(ret.length() <= MAX_SHORT_STR_LEN, true);
    ASSERT_EQ(ret, str);

    str += "a";

    ret = heap.alloc(str);
    ASSERT_EQ(ret.length() <= MAX_SHORT_STR_LEN, false);
    ASSERT_EQ(ret, str);
}

TEST(var, var_pool) {
    var_pool pool;
    auto v = pool.alloc();
    ASSERT_EQ(v->type(), var_type::VAR_NIL);

    auto v1 = pool.alloc();
    ASSERT_EQ(v1->type(), var_type::VAR_NIL);
    ASSERT_NE(v, v1);

    pool.reset();

    auto v2 = pool.alloc();
    ASSERT_EQ(v2->type(), var_type::VAR_NIL);

    auto v3 = pool.alloc();
    ASSERT_EQ(v3->type(), var_type::VAR_NIL);

    pool.reset();

    auto v4 = pool.alloc();
    ASSERT_EQ(v2->type(), var_type::VAR_NIL);
    ASSERT_EQ(v2, v4);

    auto v5 = pool.alloc();
    ASSERT_EQ(v3->type(), var_type::VAR_NIL);
    ASSERT_EQ(v3, v5);
}

TEST(var, to_string) {
    auto s = std::make_shared<state>();

    var_pool pool;
    auto v = pool.alloc();
    v->set_nil();
    ASSERT_EQ(v->to_string(), "nil");

    v->set_bool(true);
    ASSERT_EQ(v->to_string(), "true");

    v->set_int((int64_t) 12345);
    ASSERT_EQ(v->to_string(), "12345");

    v->set_float(12345.0);
    ASSERT_EQ(v->to_string(), "12345.000000");

    v->set_string(s, "hello");
    ASSERT_EQ(v->to_string(), "\"hello\"");

    v->set_table();
    ASSERT_EQ(v->to_string(), std::format("table({})", (void *) v));
}

TEST(var, var_table) {
    auto s = std::make_shared<state>();

    var_table vt;

    var k1((int64_t) 1);
    var v1((int64_t) 2);
    vt.set(&k1, &v1);
    ASSERT_EQ(vt.get(&k1), &v1);

    var k2((int64_t) 1);
    var v2((int64_t) 3);
    vt.set(&k2, &v2);
    ASSERT_EQ(vt.get(&k2), &v2);

    var k3((int64_t) 2);
    var v3((int64_t) 4);
    vt.set(&k3, &v3);
    ASSERT_EQ(vt.get(&k3), &v3);

    var k4(s, "hello");
    var v4((int64_t) 5);
    vt.set(&k4, &v4);
    ASSERT_EQ(vt.get(&k4), &v4);

    vt.set(&k4, nullptr);
    ASSERT_EQ(vt.get(&k4), &const_null_var);

    var k5(s, "hello");
    var v5((int64_t) 6);
    vt.set(&k5, &v5);
    ASSERT_EQ(vt.get(&k5), &v5);

    var nil;
    vt.set(&k5, &nil);
    ASSERT_EQ(vt.get(&k5), &const_null_var);

    vt.set(&k1, &v1);
    ASSERT_EQ(vt.get(&k1), &v1);

    vt.set(&k1, nullptr);
    ASSERT_EQ(vt.get(&k1), &const_null_var);
}
