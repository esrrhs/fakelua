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
    var_string_heap(fakelua_state *state) : state_(state) {
    }

    ~var_string_heap() = default;

    // alloc a string, and return the stored string_view.
    std::string_view alloc(const std::string_view &str);

    // clear the string heap. usually called before running.
    void reset();

private:
    fakelua_state *state_ = nullptr;

    // the key is the input string_view, the value is the stored string
    std::unordered_map<std::string_view, str_container_ptr> short_str_to_index_map_;

    // the vector stores the long strings.
    std::vector<str_container_ptr> long_str_vec_;
};

}// namespace fakelua
