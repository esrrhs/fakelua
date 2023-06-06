#include "fakelua.h"
#include "state/state.h"
#include "state/var_string_heap.h"
#include "var/var.h"
#include "gtest/gtest.h"
#include "state/var_pool.h"

using namespace fakelua;

TEST(var, construct) {
    auto s = std::make_shared<state>();

    var v;
    ASSERT_EQ(v.type(), var_type::NIL);

    var v1(nullptr);
    ASSERT_EQ(v1.type(), var_type::NIL);

    var v2(true);
    ASSERT_EQ(v2.type(), var_type::BOOL);

    var v3((int64_t) 1);
    ASSERT_EQ(v3.type(), var_type::INT);

    var v4(1.0);
    ASSERT_EQ(v4.type(), var_type::FLOAT);

    var v5(s, "hello");
    ASSERT_EQ(v5.type(), var_type::STRING);

    var v6(s, std::string("hello"));
    ASSERT_EQ(v6.type(), var_type::STRING);

    var v7(s, std::move(std::string("hello")));
    ASSERT_EQ(v7.type(), var_type::STRING);

    var v8(v7);
    ASSERT_EQ(v8.type(), var_type::STRING);

    var v9(std::move(v7));
    ASSERT_EQ(v9.type(), var_type::STRING);

    var v10;
    v10 = v9;
    ASSERT_EQ(v10.type(), var_type::STRING);

    var v11;
    v11 = std::move(v9);
    ASSERT_EQ(v11.type(), var_type::STRING);
}

TEST(var, set_get) {
    auto s = std::make_shared<state>();

    var v;
    v.set(nullptr);
    ASSERT_EQ(v.type(), var_type::NIL);

    v.set(true);
    ASSERT_EQ(v.type(), var_type::BOOL);
    ASSERT_EQ(v.get_bool(), true);

    v.set((int64_t) 1);
    ASSERT_EQ(v.type(), var_type::INT);
    ASSERT_EQ(v.get_int(), 1);

    v.set(1.0);
    ASSERT_EQ(v.type(), var_type::FLOAT);
    ASSERT_EQ(v.get_float(), 1.0);

    v.set(s, "hello");
    ASSERT_EQ(v.type(), var_type::STRING);
    ASSERT_EQ(v.get_string_view(s), "hello");

    v.set(s, std::string("hello"));
    ASSERT_EQ(v.type(), var_type::STRING);
    ASSERT_EQ(v.get_string_view(s), "hello");

    v.set(s, std::move(std::string("hello")));
    ASSERT_EQ(v.type(), var_type::STRING);
    ASSERT_EQ(v.get_string_view(s), "hello");

    var v1;
    v1.set(v);
    ASSERT_EQ(v1.type(), var_type::STRING);
    ASSERT_EQ(v1.get_string_view(s), "hello");
}

TEST(var, var_string_heap) {
    var_string_heap heap;
    auto ret = heap.alloc("hello");
    ASSERT_EQ(ret.string_index(), 1);
    ASSERT_EQ(heap.get(ret), "hello");

    ret = heap.alloc("hello");
    ASSERT_EQ(ret.string_index(), 1);
    ASSERT_EQ(heap.get(ret), "hello");

    ret = heap.alloc("hello1");
    ASSERT_EQ(ret.string_index(), 2);
    ASSERT_EQ(heap.get(ret), "hello1");

    std::string str;
    for (int i = 0; i < MAX_SHORT_STR_LEN; ++i) {
        str += "a";
    }

    ret = heap.alloc(str);
    ASSERT_EQ(ret.string_index(), 3);
    ASSERT_EQ(heap.get(ret), str);

    str += "a";

    ret = heap.alloc(str);
    ASSERT_EQ(ret.string_index(), make_long_string_index(0));
    ASSERT_EQ(heap.get(ret), str);
}

TEST(var, var_pool) {
    var_pool pool;
    auto v = pool.alloc();
    ASSERT_EQ(v->type(), var_type::NIL);

    auto v1 = pool.alloc();
    ASSERT_EQ(v1->type(), var_type::NIL);
    ASSERT_NE(v, v1);

    pool.reset();

    auto v2 = pool.alloc();
    ASSERT_EQ(v2->type(), var_type::NIL);

    auto v3 = pool.alloc();
    ASSERT_EQ(v3->type(), var_type::NIL);

    pool.reset();

    auto v4 = pool.alloc();
    ASSERT_EQ(v2->type(), var_type::NIL);
    ASSERT_EQ(v2, v4);

    auto v5 = pool.alloc();
    ASSERT_EQ(v3->type(), var_type::NIL);
    ASSERT_EQ(v3, v5);
}