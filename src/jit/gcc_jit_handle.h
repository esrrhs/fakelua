#pragma once

#include "util/common.h"
#include "var/var.h"
#include <libgccjit++.h>

namespace fakelua {

class GccJitHandle {
public:
    GccJitHandle() = default;

    ~GccJitHandle();

    void SetResult(gcc_jit_result *result) {
        gccjit_result_ = result;
    }

    [[nodiscard]] gcc_jit_result *get_result() const {
        return gccjit_result_;
    }

    void SetLogFp(FILE *fp) {
        gccjit_log_fp_ = fp;
    }

private:
    FakeluaState *state_;
    gcc_jit_result *gccjit_result_ = nullptr;
    FILE *gccjit_log_fp_ = nullptr;
};

typedef std::shared_ptr<GccJitHandle> GccJitHandlePtr;

}// namespace fakelua
