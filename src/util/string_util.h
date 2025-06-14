#pragma once

#include "common.h"

namespace fakelua {

// define the str_container_ptr
typedef std::shared_ptr<std::string> str_container_ptr;

static const auto g_number_regex =
        std::regex("^[+-]?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?$|^[+-]?0[xX][0-9a-fA-F]+(\\.[0-9a-fA-F]+)?([pP][+-]?[0-9]+)?$");

static const auto g_integer_regex = std::regex("^[+-]?[0-9]+$|^[+-]?0[xX][0-9a-fA-F]+$");

inline bool is_number(const std::string_view &s) {
    return std::regex_match(s.begin(), s.end(), g_number_regex);
}

inline bool is_integer(const std::string_view &s) {
    return std::regex_match(s.begin(), s.end(), g_integer_regex);
}

int64_t to_integer(const std::string_view &s);

double to_float(const std::string_view &s);

inline std::string join_string(const std::vector<std::string> &strs, const std::string &sep) {
    return std::accumulate(std::next(strs.begin()), strs.end(), strs[0],
                           [&](std::string a, const std::string& b) { return std::move(a) + sep + b; });
}

std::string replace_escape_chars(const std::string &str);

}// namespace fakelua
