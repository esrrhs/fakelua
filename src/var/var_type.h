#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

enum class var_type {
    VAR_NIL,
    VAR_BOOL,
    VAR_INT,
    VAR_FLOAT,
    VAR_STRING,
    VAR_TABLE,

    VAR_MIN = VAR_NIL,
    VAR_MAX = VAR_TABLE,
};

inline std::string var_type_to_string(var_type t) {
    switch (t) {
        case var_type::VAR_NIL:
            return "VAR_NIL";
        case var_type::VAR_BOOL:
            return "VAR_BOOL";
        case var_type::VAR_INT:
            return "VAR_INT";
        case var_type::VAR_FLOAT:
            return "VAR_FLOAT";
        case var_type::VAR_STRING:
            return "VAR_STRING";
        case var_type::VAR_TABLE:
            return "VAR_TABLE";
        default:
            return "UNKNOWN";
    }
}

static_assert(sizeof(var_type) == sizeof(int), "var_type must be an integer type");

}// namespace fakelua
