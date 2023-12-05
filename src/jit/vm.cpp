#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

extern "C" var *new_var_nil(fakelua_state *s) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    return ret;
}

extern "C" var *wrap_return_var(fakelua_state *s, var *v...) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    // TODO
    return ret;
}

}// namespace fakelua
