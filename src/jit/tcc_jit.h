#pragma once

#include "compile/compile_common.h"
#include "fakelua.h"
#include "jit/vm_function.h"

namespace fakelua {

class State;

class TccJitter {
public:
    explicit TccJitter(State *s);
    ~TccJitter() = default;

    void Compile(CompileResult &cr, const CompileConfig &cfg);

private:
    State *s_;
};

} // namespace fakelua
