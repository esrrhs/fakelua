#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"

namespace fakelua {

class var_string {
public:
    var_string() : index_(0) {}

    var_string(uint32_t index) : index_(index) {}

    ~var_string() = default;

private:
    uint32_t index_;
};

}// namespace fakelua
