#pragma once

#include "common.h"

namespace fakelua {

// define the str_container_ptr
typedef std::shared_ptr<std::string> str_container_ptr;

inline bool is_integer(const std::string &s) {
    return std::regex_match(s, std::regex("[(-|+)|][0-9]+"));
}

inline std::string join_string(const std::vector<std::string> &strs, const std::string &sep) {
    return std::accumulate(std::next(strs.begin()), strs.end(), strs[0],
                           [&](std::string a, std::string b) { return std::move(a) + sep + b; });
}

std::string replace_escape_chars(const std::string &str);

}// namespace fakelua
