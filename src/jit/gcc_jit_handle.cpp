#include "gcc_jit_handle.h"
#include "state/state.h"

namespace fakelua {

gcc_jit_handle::gcc_jit_handle(fakelua_state *state) : state_(state) {
}

gcc_jit_handle::~gcc_jit_handle() {
    if (gccjit_result_) {
        gcc_jit_result_release(gccjit_result_);
        gccjit_result_ = nullptr;
    }
    if (gccjit_log_fp_) {
        fclose(gccjit_log_fp_);
        gccjit_log_fp_ = nullptr;
    }
}

std::string_view gcc_jit_handle::alloc_str(const std::string_view &name) {
    auto it = str_container_map_.find(name);
    if (it != str_container_map_.end()) {
        return *it;
    }

    const auto &ret = dynamic_cast<state *>(state_)->get_var_string_heap().alloc(name);
    str_container_map_.insert(ret);
    return ret;
}

var *gcc_jit_handle::alloc_var() {
    auto ret = std::make_shared<var>();
    ret->set_const(true);
    const_vars_.push_back(ret);
    return ret.get();
}

}// namespace fakelua