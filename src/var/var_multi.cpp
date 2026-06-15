#include "var/var_multi.h"
#include "state/state.h"

namespace fakelua {

VarMulti *VarMulti::AllocTemp(State *state, uint32_t count) {
    auto ret = static_cast<VarMulti *>(state->GetHeap().GetTempAllocator().Alloc(sizeof(VarMulti)));
    ret->count = count;
    ret->vars = static_cast<CVar *>(state->GetHeap().GetTempAllocator().Alloc(count * sizeof(CVar)));
    return ret;
}

} // namespace fakelua
