#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

// store the string data. use the plain memory to store the string data.
class VarString {
public:
    // make a VarString with the given data and size
    static VarString *MakeVarString(const char *data, int size) {
        auto *s = static_cast<VarString *>(malloc(sizeof(VarString) + size));
        s->size_ = size;
        memcpy((void *) s->data_, data, size);
        return s;
    }

    // free the VarString
    static void FreeVarString(VarString *s) {
        if (s) {
            free(s);
        }
    }

    // return string view
    [[nodiscard]] std::string_view Str() const {
        return {data_, static_cast<std::string_view::size_type>(size_)};
    }

    [[nodiscard]] size_t Size() const {
        return static_cast<size_t>(size_);
    }

private:
    int size_ = 0;        // size of the string
    const char data_[0]{};// data of the string
};

// assert VarString header size is 4 bytes, the same as we defined in gccjit
static_assert(sizeof(VarString) == 4, "VarString size must be 8 bytes");

}// namespace fakelua
