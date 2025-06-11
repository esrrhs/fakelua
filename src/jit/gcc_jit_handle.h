#pragma once

#include "util/common.h"
#include "var/var.h"
#include <libgccjit++.h>

namespace fakelua {

class gcc_jit_handle {
public:
    gcc_jit_handle() = default;

    ~gcc_jit_handle();

    void set_result(gcc_jit_result *result) {
        gccjit_result_ = result;
    }

    [[nodiscard]] gcc_jit_result *get_result() const {
        return gccjit_result_;
    }

    void set_log_fp(FILE *fp) {
        gccjit_log_fp_ = fp;
    }

private:
    fakelua_state *state_;
    gcc_jit_result *gccjit_result_ = nullptr;
    FILE *gccjit_log_fp_ = nullptr;
};

typedef std::shared_ptr<gcc_jit_handle> gcc_jit_handle_ptr;

}// namespace fakelua
