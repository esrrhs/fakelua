#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_string.h"
#include "var_type.h"
#include "var_table.h"
#include "util/no_copy.h"

namespace fakelua {

class state;

// Var is the class that holds the multiple types of data.
// the reason why wrap the std::variant is that we can add more features and maybe someday we can replace the std::variant.
// all the data is stored in the state's var_pool. and used by simple pointer.
class var : public no_copy<var> {
public:
    var() : data_(nullptr) {}

    var(std::nullptr_t) : data_(nullptr) {}

    var(bool val) : data_(val) {}

    var(int64_t val) : data_(val) {}

    var(double val) : data_(val) {}

    var(fakelua_state_ptr s, const std::string &val);

    var(fakelua_state_ptr s, std::string &&val);

    var(fakelua_state_ptr s, const char *val);

    var(fakelua_state_ptr s, std::string_view val);

    var(const var &val) : data_(val.data_) {}

    var(var &&val) : data_(std::move(val.data_)) {}

    var(const var_table &val) : data_(val) {}

    var(var_table &&val) : data_(std::move(val)) {}

    ~var() = default;

    var &operator=(const var &val) {
        data_ = val.data_;
        return *this;
    }

    var &operator=(var &&val) {
        data_ = std::move(val.data_);
        return *this;
    }

    // get the var type
    var_type type() const {
        return static_cast<var_type>(data_.index());
    }

    // get bool value
    bool get_bool() const {
        return std::get<bool>(data_);
    }

    // get int value
    int64_t get_int() const {
        return std::get<int64_t>(data_);
    }

    // get float value
    double get_float() const {
        return std::get<double>(data_);
    }

    // get var_string value
    const var_string &get_string() const {
        return std::get<var_string>(data_);
    }

    // get string_view value
    std::string_view get_string_view(fakelua_state_ptr s) const;

    // get table value
    const var_table &get_table() const {
        return std::get<var_table>(data_);
    }

    // get table value
    var_table &get_table() {
        return std::get<var_table>(data_);
    }

    // set nullptr
    var &set(std::nullptr_t) {
        data_ = nullptr;
        return *this;
    }

    // set bool value
    var &set(bool val) {
        data_ = val;
        return *this;
    }

    // set int value
    var &set(int64_t val) {
        data_ = val;
        return *this;
    }

    // set float value
    var &set(double val) {
        data_ = val;
        return *this;
    }

    // set string value
    var &set(fakelua_state_ptr s, const std::string &val);

    // set string value
    var &set(fakelua_state_ptr s, std::string &&val);

    // set string value
    var &set(fakelua_state_ptr s, const char *val);

    // set string value
    var &set(fakelua_state_ptr s, std::string_view val);

    // set table value
    var &set(const var_table &val);

    // set table value
    var &set(var_table &&val);

    // set var value
    var &set(const var &val) {
        data_ = val.data_;
        return *this;
    }

    // set var value
    var &set(var &&val) {
        data_ = std::move(val.data_);
        return *this;
    }

private:
    // use std::variant instead of union, use more memory but more safe.
    std::variant<std::nullptr_t, bool, int64_t, double, var_string, var_table> data_;
};

extern var const_null_var;

}// namespace fakelua
