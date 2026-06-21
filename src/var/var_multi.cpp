#include "fakelua.h"
#include "var/var_multi.h"
#include "state/state.h"
#include "var/var_type.h"
#include "util/debug.h"

namespace fakelua {

VarMulti *VarMulti::AllocTemp(State *state, uint32_t count) {
    auto ret = static_cast<VarMulti *>(state->GetHeap().GetTempAllocator().Alloc(sizeof(VarMulti) + count * sizeof(CVar)));
    ret->count = count;
    return ret;
}

namespace inter {

CVar AllocMultiCVar(State *s, int count) {
    VarMulti *m = VarMulti::AllocTemp(s, count);
    for (int i = 0; i < count; ++i) {
        m->GetVars()[i] = CVar{static_cast<int>(VarType::Nil)};
    }
    CVar result;
    result.type_ = static_cast<int>(VarType::Multi);
    result.flag_ = 0;
    result.data_.m = m;
    return result;
}

void SetMultiCVarElement(CVar &multi, int idx, CVar val) {
    DEBUG_ASSERT(multi.type_ == static_cast<int>(VarType::Multi));
    VarMulti *m = multi.data_.m;
    DEBUG_ASSERT(idx >= 0 && idx < static_cast<int>(m->GetCount()));
    m->GetVars()[idx] = val;
}

CVar GetMultiCVarElement(const CVar &multi, int idx) {
    if (multi.type_ != static_cast<int>(VarType::Multi)) {
        return (idx == 0) ? multi : CVar{static_cast<int>(VarType::Nil)};
    }
    VarMulti *m = multi.data_.m;
    if (idx < 0 || idx >= static_cast<int>(m->GetCount())) {
        return CVar{static_cast<int>(VarType::Nil)};
    }
    return m->GetVars()[idx];
}

int GetMultiCVarCount(const CVar &multi) {
    if (multi.type_ != static_cast<int>(VarType::Multi)) {
        return 0;
    }
    return static_cast<int>(multi.data_.m->GetCount());
}

}// namespace inter
}// namespace fakelua
