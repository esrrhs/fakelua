#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"

namespace fakelua {

class var_string {
public:
    var_string() : short_str_(0) {}

    var_string(uint32_t short_str) : short_str_(short_str) {}

    var_string(const std::string_view &long_str) : short_str_(0), long_str_(long_str) {}

    ~var_string() = default;

    uint32_t short_string_index() const {
        return short_str_;
    }

    std::string_view long_string_view() const {
        return long_str_;
    }

private:
    // if short_str_ is not 0, then it's a short string. short_str_ is the string heap alloc index.
    uint32_t short_str_;
    // otherwise, it's a long string. the long_str_ holds the string.
    std::string_view long_str_;
};

}// namespace fakelua
