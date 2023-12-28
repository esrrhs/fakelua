#pragma once

#include "util/common.h"
#include "var/var.h"
#include <libgccjit++.h>

namespace fakelua {

class   gcc_jit_handle {
public:
    gcc_jit_handle(fakelua_state *state);

    ~gcc_jit_handle();

    fakelua_state *get_state() {
        return state_;
    }

    void set_result(gcc_jit_result *result) {
        gccjit_result_ = result;
    }

    gcc_jit_result *get_result() {
        return gccjit_result_;
    }

    void set_log_fp(FILE *fp) {
        gccjit_log_fp_ = fp;
    }

    // alloc string used by the gcc_jit_result, the storage is also state's string pool
    std::string_view alloc_str(const std::string_view &name);

    // get all used string
    const std::unordered_set<std::string_view> &get_str_container_map() const {
        return str_container_map_;
    }

    // alloc const var used by the gcc_jit_result
    var *alloc_var();

private:
    fakelua_state *state_;
    gcc_jit_result *gccjit_result_ = nullptr;
    FILE *gccjit_log_fp_ = nullptr;
    std::unordered_set<std::string_view> str_container_map_;
    std::vector<var_ptr> const_vars_;
};

typedef std::shared_ptr<gcc_jit_handle> gcc_jit_handle_ptr;

}// namespace fakelua
