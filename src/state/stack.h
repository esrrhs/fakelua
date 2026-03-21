#pragma once

#include "fakelua.h"

namespace fakelua {

// the stack is the runtime stack of fakelua state
class stack {
public:
    stack(StateConfig config);

    ~stack() = default;

    void reset() {
        stack_top_ = 0;
    }

    // get current stack top
    CVar *top();

    // get max stack pos
    CVar *max();

    // push a var to stack
    void push(const CVar &v);

    // pop stack to pos
    void PopTo(CVar * top);

private:
    // var stack
    std::vector<CVar> var_stack_;
    // current stack top
    size_t stack_top_ = 0;
};

}// namespace fakelua
