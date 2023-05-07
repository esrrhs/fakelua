#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

enum class var_type {
    INVALID = -1,
    NIL,
    BOOL,
    INT,
    FLOAT,
    STRING,
    TABLE,
    FUNCTION,
    USERDATA,
    THREAD,
    MAX,
};

}
