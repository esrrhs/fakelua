#pragma once

#include "fakelua.h"

namespace fakelua {

struct VarMulti {
    uint32_t count;
    CVar *vars;

    static VarMulti *AllocTemp(State *state, uint32_t count);
};

} // namespace fakelua
