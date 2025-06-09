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

static_assert(sizeof(var_type) == sizeof(int), "var_type must be an integer type");

}// namespace fakelua
