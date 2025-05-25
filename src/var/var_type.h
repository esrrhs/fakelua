#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

enum var_type {
    VAR_NIL,
    VAR_BOOL,
    VAR_INT,
    VAR_FLOAT,
    VAR_STRING,
    VAR_TABLE,
};

#define VAR_MIN VAR_NIL
#define VAR_MAX VAR_TABLE

}// namespace fakelua
