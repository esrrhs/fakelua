#include "var/var_string.h"
#include "state/state.h"
#include <limits>

namespace fakelua {

VarString *VarString::AllocTemp(State *state, const std::string_view &str) {
    auto ret = static_cast<VarString *>(state->GetHeap().GetAllocator(false).Alloc(sizeof(VarString) + str.size()));
    new (ret) VarString(str);
    return ret;
}

VarString *VarString::AllocTempRaw(State *state, size_t size) {
    DEBUG_ASSERT(size <= static_cast<size_t>(std::numeric_limits<int>::max()));
    auto ret = static_cast<VarString *>(state->GetHeap().GetAllocator(false).Alloc(sizeof(VarString) + size));
    ret->size_ = static_cast<int>(size);
    ret->hash_ = 0;
    return ret;
}

}// namespace fakelua
