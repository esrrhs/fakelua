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
};

}// namespace fakelua