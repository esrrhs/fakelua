#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

// 为了性能考虑，字符串只是字符串，并不能自动转成数字参与计算。
// 字符串也分为了常量字符串和临时字符串，常量字符串是在编译期确定的，会自动分配0-N的顺序编号，方便快速查找。
// VarString是临时字符串，运行期间生成，直接从池子里切出来，最后一口气释放。
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
    int hash_ = 0;
    const char data_[0]{};// data of the string
};

// assert VarString header size is 8 bytes, the same as we defined in gccjit
static_assert(sizeof(VarString) == 8, "VarString size must be 8 bytes");

}// namespace fakelua
