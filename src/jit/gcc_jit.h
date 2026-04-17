#pragma once

#include "compile/compile_common.h"
#include "fakelua.h"

namespace fakelua {

class State;

class GccJitter {
public:
    explicit GccJitter(State *s);
    ~GccJitter() = default;

    void Compile(CompileResult &cr, const CompileConfig &cfg);

private:
    State *s_;
};

}// namespace fakelua
