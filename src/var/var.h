#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"

namespace fakelua {

class state;
class var_string;
class var_table;

// Var is the class that holds the multiple types of data.
class var {
public:
    var() = default;

    explicit var(int64_t val) {
        set_int(val);
    }

    explicit var(bool val) {
        set_bool(val);
    }

    // get the var type
    [[nodiscard]] var_type type() const {
        return static_cast<var_type>(type_);
    }

    // get bool value
    [[nodiscard]] bool get_bool() const {
        DEBUG_ASSERT(type_ == var_type::VAR_BOOL);
        return data_.b;
    }

    // get int value
    [[nodiscard]] int64_t get_int() const {
        DEBUG_ASSERT(type_ == var_type::VAR_INT);
        return data_.i;
    }

    // get float value
    [[nodiscard]] double get_float() const {
        DEBUG_ASSERT(type_ == var_type::VAR_FLOAT);
        return data_.f;
    }

    // get string_view value
    [[nodiscard]] var_string *get_string() const {
        DEBUG_ASSERT(type_ == var_type::VAR_STRING);
        return data_.s;
    }

    // get table value
    [[nodiscard]] var_table *get_table() const {
        DEBUG_ASSERT(type_ == var_type::VAR_TABLE);
        return data_.t;
    }

public:
    // set nullptr
    void set_nil() {
        type_ = var_type::VAR_NIL;
    }

    // set bool value
    void set_bool(bool val) {
        type_ = var_type::VAR_BOOL;
        data_.b = val;
    }

    // set int value
    void set_int(int64_t val) {
        type_ = var_type::VAR_INT;
        data_.i = val;
    }

    // set float value
    void set_float(double val) {
        double int_part;
        if (std::modf(val, &int_part) != 0.0) {
            type_ = var_type::VAR_FLOAT;
            data_.f = val;
        } else {
            type_ = var_type::VAR_INT;
            data_.i = static_cast<int64_t>(val);
        }
    }

    // set string value
    void set_string(const fakelua_state_ptr &s, const std::string_view &val);

    // set string value
    void set_string(fakelua_state *s, const std::string_view &val);

    // set table value
    void set_table(const fakelua_state_ptr &s);

    // set table value
    void set_table(fakelua_state *s);

public:
    // to string
    [[nodiscard]] std::string to_string(bool has_quote = true, bool has_postfix = true) const;

    void set_const(bool val) {
        SET_FLAG_BIT(flag_, VAR_FLAG_CONST_IDX, val);
    }

    [[nodiscard]] bool is_const() const {
        return GET_FLAG_BIT(flag_, VAR_FLAG_CONST_IDX);
    }

    void set_variadic(bool val) {
        SET_FLAG_BIT(flag_, VAR_FLAG_VARIADIC_IDX, val);
    }

    [[nodiscard]] bool is_variadic() const {
        return GET_FLAG_BIT(flag_, VAR_FLAG_VARIADIC_IDX);
    }

    // get hash value
    [[nodiscard]] size_t hash() const;

    // equal
    [[nodiscard]] bool equal(const var &rhs) const;

    // is calculable
    [[nodiscard]] bool is_calculable() const;

    // is calculable integer
    [[nodiscard]] bool is_calculable_integer() const;

    // get calculable integer, only call this when is_calculable_integer() is true
    [[nodiscard]] int64_t get_calculable_int() const;

    // get calculable number value, maybe integer or float or string
    [[nodiscard]] double get_calculable_number() const;

public:
    void plus(const var &rhs, var &result) const;

    void minus(const var &rhs, var &result) const;

    void star(const var &rhs, var &result) const;

    void slash(const var &rhs, var &result) const;

    void double_slash(const var &rhs, var &result) const;

    void pow(const var &rhs, var &result) const;

    void mod(const var &rhs, var &result) const;

    void bitand_(const var &rhs, var &result) const;

    void xor_(const var &rhs, var &result) const;

    void bitor_(const var &rhs, var &result) const;

    void right_shift(const var &rhs, var &result) const;

    void left_shift(const var &rhs, var &result) const;

    void concat(fakelua_state *s, const var &rhs, var &result) const;

    void less(const var &rhs, var &result) const;

    void less_equal(const var &rhs, var &result) const;

    void more(const var &rhs, var &result) const;

    void more_equal(const var &rhs, var &result) const;

    void equal(const var &rhs, var &result) const;

    void not_equal(const var &rhs, var &result) const;

    [[nodiscard]] bool test_true() const;

    void unop_minus(var &result) const;

    void unop_not(var &result) const;

    void unop_number_sign(var &result) const;

    void unop_bitnot(var &result) const;

    void table_set(const var &key, const var &val, bool can_be_nil) const;

    [[nodiscard]] const var *table_get(const var &key) const;

    [[nodiscard]] size_t table_size() const;

    [[nodiscard]] const var *table_key_at(size_t pos) const;

    [[nodiscard]] const var *table_value_at(size_t pos) const;

private:
    var_type type_ = var_type::VAR_NIL;
    int flag_ = 0;
    union var_data {
        bool b;
        int64_t i;
        double f;
        var_string *s;
        var_table *t;
    };
    var_data data_{};
};

// assert var size is 16 bytes, the same as we defined in gccjit
static_assert(sizeof(var) == 16);

extern var const_null_var;
extern var const_true_var;
extern var const_false_var;

}// namespace fakelua
