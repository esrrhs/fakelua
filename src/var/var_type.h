#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

enum class var_type {
    VAR_INVALID = -1,
    VAR_NIL,
    VAR_BOOL,
    VAR_INT,
    VAR_FLOAT,
    VAR_STRING,
    VAR_TABLE,
    VAR_MAX,
};

}
