#include "stack.h"
#include "util/common.h"

namespace fakelua {

stack::stack(state_config config) {
    LOG_INFO("create var_stack_size {}", config.max_stack_size);
    var_stack_.resize(config.max_stack_size);
}

cvar *stack::top() {
    DEBUG_ASSERT(stack_top_ < var_stack_.size());
    return var_stack_.data() + stack_top_;
}

cvar *stack::max() {
    return var_stack_.data() + var_stack_.size();
}

void stack::push(const cvar &v) {
    if (stack_top_ >= var_stack_.size()) {
        throw_fakelua_exception("stack overflow");
    }
    var_stack_[stack_top_] = v;
    stack_top_++;
}

void stack::pop_to(cvar * top) {
    size_t pos = top - var_stack_.data();
    DEBUG_ASSERT(pos <= stack_top_);
    stack_top_ = pos;
}

}// namespace fakelua
