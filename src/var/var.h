#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_table.h"
#include "var_type.h"

namespace fakelua {

class state;

// Var is the class that holds the multiple types of data.
// all the data is stored in the state's var_pool. and can only be used by simple pointer.
// except the global const var, which is stored in the gcc_jit_handle.
class var {
public:
    // get the var type
    var_type type() const {
        return (var_type) type_;
    }

    // get bool value
    bool get_bool() const {
        DEBUG_ASSERT(type_ == var_type::VAR_BOOL);
        return data_.b;
    }

    // get int value
    int64_t get_int() const {
        DEBUG_ASSERT(type_ == var_type::VAR_INT);
        return data_.i;
    }

    // get float value
    double get_float() const {
        DEBUG_ASSERT(type_ == var_type::VAR_FLOAT);
        return data_.f;
    }

    // get string_view value
    var_string *get_string() const {
        DEBUG_ASSERT(type_ == var_type::VAR_STRING);
        return data_.s;
    }

    // get table value
    var_table *get_table() const {
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
        double intpart;
        if (std::modf(val, &intpart) != 0.0) {
            type_ = var_type::VAR_FLOAT;
            data_.f = val;
        } else {
            type_ = var_type::VAR_INT;
            data_.i = (int64_t) val;
        }
    }

    // set string value
    void set_string(const fakelua_state_ptr &s, const std::string &val);

    // set string value
    void set_string(const fakelua_state_ptr &s, const char *val);

    // set string value
    void set_string(const fakelua_state_ptr &s, std::string_view val);

    // set string value
    void set_string(fakelua_state *s, const std::string &val);

    // set string value
    void set_string(fakelua_state *s, const char *val);

    // set string value
    void set_string(fakelua_state *s, std::string_view val);

    // set table value
    void set_table();

public:
    // to string
    std::string to_string(bool has_quote = true, bool has_postfix = true) const;

    void set_const(bool val) {
        SET_FLAG_BIT(flag_, VAR_FLAG_CONST_IDX, val);
    }

    bool is_const() const {
        return GET_FLAG_BIT(flag_, VAR_FLAG_CONST_IDX);
    }

    void set_variadic(bool val) {
        SET_FLAG_BIT(flag_, VAR_FLAG_VARIADIC_IDX, val);
    }

    bool is_variadic() const {
        return GET_FLAG_BIT(flag_, VAR_FLAG_VARIADIC_IDX);
    }

    // get hash value
    size_t hash() const;

    // equal
    bool equal(const var &rhs) const;

    // is calculable
    bool is_calculable() const;

    // is calculable integer
    bool is_calculable_integer() const;

    // get calculable integer, only call this when is_calculable_integer() is true
    int64_t get_calculable_int() const;

    // get calculable number value, maybe integer or float or string
    double get_calculable_number() const;

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

    bool test_true() const;

    void unop_minus(var &result) const;

    void unop_not(var &result) const;

    void unop_number_sign(var &result) const;

    void unop_bitnot(var &result) const;

    void table_set(var *key, var *val);

    var *table_get(var *key) const;

    size_t table_size() const;

    var *table_key_at(size_t pos) const;

    var *table_value_at(size_t pos) const;

private:
    int type_ = var_type::VAR_NIL;
    int flag_ = 0;
    union data_ {
        bool b;
        int64_t i;
        double f;
        var_string *s;
        var_table *t;
    };
    data_ data_{};
};

// assert var size is 16 bytes, the same as we defined in gccjit
static_assert(sizeof(var) == 16);

extern var const_null_var;
extern var const_false_var;
extern var const_true_var;

typedef std::shared_ptr<var> var_ptr;

}// namespace fakelua
