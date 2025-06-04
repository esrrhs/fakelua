#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var/var_string.h"

namespace fakelua {

// string heap hold all the string, string has two type: const string and tmp string.
// const string allocated when compile function and global var.
// tmp string allocated when running function, will clean up after each run.
class var_string_heap {
public:
    // alloc a string, and return the stored var_string.
    var_string *alloc(const std::string_view &str, bool is_const);

    // clear the string heap. usually called before running.
    void reset();

    // get size
    size_t size() const {
        return const_str_map_.size() + tmp_str_map_.size();
    }

private:
    // the key is the input string_view, the value is the stored string
    std::unordered_map<std::string_view, var_string *> const_str_map_;
    std::unordered_map<std::string_view, var_string *> tmp_str_map_;
};

}// namespace fakelua
