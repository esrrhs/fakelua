#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

extern "C" var *new_var_nil(fakelua_state *s) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    return ret;
}

}// namespace fakelua

extern "C" void *wrap_return_var(void *s, ...) {
    return 0;
}
