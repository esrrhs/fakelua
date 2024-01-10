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
    var() : type_(var_type::VAR_NIL) {
    }

    var(std::nullptr_t, bool is_const = false) : type_(var_type::VAR_NIL), is_const_(is_const) {
    }

    var(bool val, bool is_const = false) : is_const_(is_const) {
        set_bool(val);
    }

    var(int64_t val) {
        set_int(val);
    }

    var(double val) {
        set_float(val);
    }

    var(const fakelua_state_ptr &s, const std::string &val);

    var(const fakelua_state_ptr &s, std::string &&val);

    var(const fakelua_state_ptr &s, const char *val);

    var(const fakelua_state_ptr &s, std::string_view val);

    var(var &&val) = default;

    ~var() = default;

public:
    var &operator=(const var &val) = default;

    var &operator=(var &&val) = default;

public:
    // get the var type
    var_type type() const {
        return type_;
    }

    // get bool value
    bool get_bool() const {
        return bool_;
    }

    // get int value
    int64_t get_int() const {
        return int_;
    }

    // get float value
    double get_float() const {
        return float_;
    }

    // get string_view value
    const std::string_view &get_string() const {
        return string_;
    }

    // get string_view value
    bool is_short_string() const {
        return string_.length() <= MAX_SHORT_STR_LEN;
    }

    // get table value
    const var_table &get_table() const {
        return table_;
    }

    // get table value
    var_table &get_table() {
        return table_;
    }

public:
    // set nullptr
    void set_nil() {
        type_ = var_type::VAR_NIL;
    }

    // set bool value
    void set_bool(bool val) {
        type_ = var_type::VAR_BOOL;
        bool_ = val;
    }

    // set int value
    void set_int(int64_t val) {
        type_ = var_type::VAR_INT;
        int_ = val;
    }

    // set float value
    void set_float(double val) {
        double intpart;
        if (std::modf(val, &intpart) != 0.0) {
            type_ = var_type::VAR_FLOAT;
            float_ = val;
        } else {
            type_ = var_type::VAR_INT;
            int_ = (int64_t) val;
        }
    }

    // set string value
    void set_string(const fakelua_state_ptr &s, const std::string &val);

    // set string value
    void set_string(const fakelua_state_ptr &s, std::string &&val);

    // set string value
    void set_string(const fakelua_state_ptr &s, const char *val);

    // set string value
    void set_string(const fakelua_state_ptr &s, std::string_view val);

    // set string value
    void set_string(fakelua_state *s, const std::string &val);

    // set string value
    void set_string(fakelua_state *s, std::string &&val);

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
        is_const_ = val;
    }

    bool is_const() const {
        return is_const_;
    }

    void set_variadic(bool val) {
        is_variadic_ = val;
    }

    bool is_variadic() const {
        return is_variadic_;
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

private:
    // use class members instead of union, use more memory but more safe and fast.
    var_type type_ = var_type::VAR_NIL;
    bool is_const_ = false;
    bool is_variadic_ = false;
    bool bool_ = false;
    int64_t int_ = 0;
    double float_ = 0;
    std::string_view string_;
    var_table table_;
};

extern var const_null_var;
extern var const_false_var;
extern var const_true_var;

typedef std::shared_ptr<var> var_ptr;

}// namespace fakelua
