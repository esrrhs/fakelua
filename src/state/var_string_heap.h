#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var/var_string.h"
#include "var/var_type.h"

namespace fakelua {

typedef std::shared_ptr<std::string> str_container_ptr;

// every state has a string heap
// the string heap contains all the strings in the state
class var_string_heap {
public:
    var_string_heap() = default;

    ~var_string_heap() = default;

    var_string alloc(const std::string &str) {

    }

private:
    std::unordered_map<std::string, var_string> str_set_;
};

}// namespace fakelua
