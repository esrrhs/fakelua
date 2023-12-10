#pragma once

#include "util/common.h"
#include <libgccjit++.h>

namespace fakelua {

class gcc_jit_handle {
public:
    gcc_jit_handle() = default;

    ~gcc_jit_handle();

    void set_result(gcc_jit_result *result) {
        gccjit_result_ = result;
    }

    gcc_jit_result *get_result() {
        return gccjit_result_;
    }

    void set_log_fp(FILE *fp) {
        gccjit_log_fp_ = fp;
    }

    FILE *get_log_fp() {
        return gccjit_log_fp_;
    }

private:
    gcc_jit_result *gccjit_result_ = nullptr;
    FILE *gccjit_log_fp_ = nullptr;
};

typedef std::shared_ptr<gcc_jit_handle> gcc_jit_handle_ptr;

}// namespace fakelua
