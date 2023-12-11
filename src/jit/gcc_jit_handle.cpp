#include "gcc_jit_handle.h"

namespace fakelua {

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

str_container_ptr gcc_jit_handle::alloc_str(const std::string_view &name) {
    auto it = str_container_map_.find(name);
    if (it != str_container_map_.end()) {
        return it->second;
    }

    auto ret = std::make_shared<std::string>(name);
    str_container_map_.insert({name, ret});
    return ret;
}

}// namespace fakelua