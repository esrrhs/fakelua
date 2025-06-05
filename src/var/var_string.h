#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

// store the string data. use the plain memory to store the string data.
class var_string {
public:
    // make a var_string with the given data and size
    static var_string *make_var_string(const char *data, int size) {
        auto *s = static_cast<var_string *>(malloc(sizeof(var_string) + size));
        s->size_ = size;
        memcpy((void *) s->data_, data, size);
        return s;
    }

    // return string view
    [[nodiscard]] const std::string_view &str() const {
        return {data_, static_cast<std::string_view::size_type>(size_)};
    }

    [[nodiscard]] size_t size() const {
        return static_cast<size_t>(size_);
    }

private:
    int size_ = 0;      // size of the string
    const char data_[0];// data of the string
};

// assert var_string header size is 4 bytes, the same as we defined in gccjit
static_assert(sizeof(var_string) == 4, "var_string size must be 8 bytes");

}// namespace fakelua
