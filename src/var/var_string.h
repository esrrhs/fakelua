#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"

namespace fakelua {

// store the string data. use the plain memory to store the string data.
class var_string {
public:
    const char *data() const {
        return data_;
    }

private:
    int size_ = 0; // size of the string
    const char data_[0]; // data of the string
};

// assert var_string header size is 4 bytes, the same as we defined in gccjit
static_assert(sizeof(var_string) == 4, "var_string size must be 8 bytes");

}// namespace fakelua
