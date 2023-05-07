#include "fakelua.h"
#include "var/var.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(var, construct) {
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

    var v5("hello");
    ASSERT_EQ(v5.type(), var_type::STRING);

    var v6(std::string("hello"));
    ASSERT_EQ(v6.type(), var_type::STRING);

    var v7(std::move(std::string("hello")));
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

    v.set("hello");
    ASSERT_EQ(v.type(), var_type::STRING);
    ASSERT_EQ(v.get_string(), "hello");

    v.set(std::string("hello"));
    ASSERT_EQ(v.type(), var_type::STRING);
    ASSERT_EQ(v.get_string(), "hello");

    v.set(std::move(std::string("hello")));
    ASSERT_EQ(v.type(), var_type::STRING);
    ASSERT_EQ(v.get_string(), "hello");

    var v1;
    v1.set(v);
    ASSERT_EQ(v1.type(), var_type::STRING);
    ASSERT_EQ(v1.get_string(), "hello");
}
