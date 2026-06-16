#pragma once

#include "fakelua.h"

namespace fakelua {

class VarMulti {
public:
    static VarMulti *AllocTemp(State *state, uint32_t count);

    [[nodiscard]] uint32_t GetCount() const {
        return count;
    }

    [[nodiscard]] const CVar *GetVars() const {
        return vars;
    }

    [[nodiscard]] CVar *GetVars() {
        return vars;
    }

private:
    uint32_t count;
    CVar vars[0];
};

}// namespace fakelua
