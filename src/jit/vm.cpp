#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

extern "C" __attribute__((used)) var *new_var_nil(fakelua_state *s) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    return ret;
}

extern "C" __attribute__((used)) var *wrap_return_var(fakelua_state *s, ...) {
    return 0;
}

}// namespace fakelua
