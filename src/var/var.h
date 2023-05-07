#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"

namespace fakelua {

// Var is the class that holds the multiple types of data.
// the reason why wrap the std::variant is that we can add more features and maybe someday we can replace the std::variant.
class var {
public:
    var() : data_(nullptr) {}

    var(std::nullptr_t) : data_(nullptr) {}

    var(bool val) : data_(val) {}

    var(int64_t val) : data_(val) {}

    var(double val) : data_(val) {}

    var(const std::string &val) : data_(val) {}

    var(std::string &&val) : data_(std::move(val)) {}

    var(const char *val) : data_(std::string(val)) {}

    var(const var &val) : data_(val.data_) {}

    var(var &&val) : data_(std::move(val.data_)) {}

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

    // get string value
    const std::string &get_string() const {
        return std::get<std::string>(data_);
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
    var &set(const std::string &val) {
        data_ = val;
        return *this;
    }

    // set string value
    var &set(std::string &&val) {
        data_ = std::move(val);
        return *this;
    }

    // set string value
    var &set(const char *val) {
        data_ = std::string(val);
        return *this;
    }

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
    // we just put the std::string in data, no string heap like lua. because most of the string is short.
    // and the std::string is already use the small string optimization. so the performance is good.
    // and the string heap will cause every new string has a hash compare, but mostly the string will not compare each other.
    std::variant<std::nullptr_t, bool, int64_t, double, std::string> data_;
};

}// namespace fakelua
