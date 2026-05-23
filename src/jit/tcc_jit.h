#pragma once

#include "compile/compile_common.h"
#include "fakelua.h"
#include <libtcc.h>

namespace fakelua {

class State;
struct TCCState;

class TccJitter {
public:
    explicit TccJitter(State *s);
    ~TccJitter() = default;

    void Compile(const ParseResult &pr, const GenResult &gr, const CompileConfig &cfg);

private:
    State *s_;
};

}// namespace fakelua
