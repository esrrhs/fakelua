#include "var/var_multi.h"
#include "fakelua.h"
#include "state/state.h"

namespace fakelua {

VarMulti *VarMulti::AllocTemp(State *state, uint32_t count) {
    auto ret = static_cast<VarMulti *>(state->GetHeap().GetAllocator(false).Alloc(sizeof(VarMulti) + count * sizeof(CVar)));
    ret->count = count;
    return ret;
}

}// namespace fakelua
