#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "util/no_copy.h"
#include "var/var_type.h"

namespace fakelua {

// every state has a string heap.
// the string heap contains all the strings in the state.
// just like Lua, it has two kinds of strings, short string and long string.
// short string can compare by pointer, and long string must compare by value.
class var_string_heap : public no_copy<var_string_heap> {
public:
    var_string_heap() = default;

    ~var_string_heap() = default;

    // alloc a string, and return the stored string_view.
    std::tuple<bool, std::string_view> alloc(const std::string_view &str);

    // clear the string heap. usually called before running.
    void reset();

private:
    // the all string mem buffer, a flat memory.
    std::vector<char> str_mem_;
    // the index of the next string in the str_mem_
    uint32_t str_mem_index_ = 0;
    // the key is the input string_view, the value is the stored string_view
    std::unordered_map<std::string_view, std::string_view> short_str_to_index_map_;
    // when str_mem_ is full, we use short_str_tmp_ to store the new short strings.
    std::vector<str_container_ptr> short_str_tmp_;

    // the vector stores the long strings.
    std::vector<str_container_ptr> long_str_vec_;
};

}// namespace fakelua
