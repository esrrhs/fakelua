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
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_table();
    // push ... to ret
    va_list args;
    va_start(args, s);
    int i = 0;
    for (;;) {
        auto arg = va_arg(args, var *);
        if (!arg) {
            break;
        }
        i++;

        if (arg->type() <= var_type::VAR_INVALID || arg->type() >= var_type::VAR_MAX) {
            throw std::runtime_error(std::format("wrap_return_var: invalid arg type {}", (int) arg->type()));
        }

        auto k = dynamic_cast<state *>(s)->get_var_pool().alloc();
        k->set_int(i);
        ret->get_table().set(k, arg, true);
    }
    va_end(args);
    return ret;
}

}// namespace fakelua
