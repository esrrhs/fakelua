#include "var/var_string.h"
#include "state/state.h"

namespace fakelua {

VarString *VarString::AllocTemp(State *state, const std::string_view &str) {
    auto ret = static_cast<VarString *>(state->GetHeap().GetTempAllocator().Alloc(sizeof(VarString) + str.size()));
    new (ret) VarString(str);
    return ret;
}

}// namespace fakelua
