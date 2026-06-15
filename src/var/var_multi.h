#pragma once

#include "fakelua.h"

namespace fakelua {

class VarMulti {
public:
    uint32_t count;
    CVar vars[0];

    static VarMulti *AllocTemp(State *state, uint32_t count);
};

} // namespace fakelua
