#include "stack.h"
#include "util/common.h"

namespace fakelua {

stack::stack(StateConfig config) {
    LOG_INFO("create var_stack_size {}", config.max_stack_size);
    var_stack_.resize(config.max_stack_size);
}

CVar *stack::top() {
    DEBUG_ASSERT(stack_top_ < var_stack_.size());
    return var_stack_.data() + stack_top_;
}

CVar *stack::max() {
    return var_stack_.data() + var_stack_.size();
}

void stack::push(const CVar &v) {
    if (stack_top_ >= var_stack_.size()) {
        ThrowFakeluaException("stack overflow");
    }
    var_stack_[stack_top_] = v;
    stack_top_++;
}

void stack::PopTo(CVar * top) {
    size_t pos = top - var_stack_.data();
    DEBUG_ASSERT(pos <= stack_top_);
    stack_top_ = pos;
}

}// namespace fakelua
