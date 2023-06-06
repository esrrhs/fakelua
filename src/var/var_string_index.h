#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"
#include "util/copyable.h"

namespace fakelua {

// make a short string index from the index
inline uint32_t make_short_string_index(uint32_t index) {
    assert(index > 0 && index < 0x7FFFFFFF);
    return index;
}

// make a long string index from the index
inline uint32_t make_long_string_index(uint32_t index) {
    assert(index >= 0 && index < 0x7FFFFFFF);
    return index | 0x80000000;
}

// get the index from the short string index
inline uint32_t get_short_string_index(uint32_t index) {
    return index;
}

// get the index from the long string index
inline uint32_t get_long_string_index(uint32_t index) {
    return index & 0x7FFFFFFF;
}

// check if the index is a short string index
inline bool is_short_string_index(uint32_t index) {
    return !(index & 0x80000000);
}

// check if the index is a long string index
inline bool is_long_string_index(uint32_t index) {
    return index & 0x80000000;
}

}// namespace fakelua
