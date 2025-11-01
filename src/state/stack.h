#pragma once

#include "fakelua.h"

namespace fakelua {

// the stack is the runtime stack of fakelua state
class stack {
public:
    stack(state_config config);

    ~stack() = default;

    void reset() {
        stack_top_ = 0;
    }

    // get current stack top
    cvar *top();

    // get max stack pos
    cvar *max();

    // push a var to stack
    void push(const cvar &v);

    // pop stack to pos
    void pop_to(cvar * top);

private:
    // var stack
    std::vector<cvar> var_stack_;
    // current stack top
    size_t stack_top_ = 0;
};

}// namespace fakelua
