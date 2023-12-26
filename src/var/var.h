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

    var(std::nullptr_t) : type_(var_type::VAR_NIL) {
    }

    var(bool val) {
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
    std::string to_string() const;

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
