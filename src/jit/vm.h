#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "var/var.h"

namespace fakelua {

class vm_runner_interface {
public:
    vm_runner_interface() = default;
    virtual ~vm_runner_interface() = default;
    virtual std::vector<var *> run(std::vector<var *> input) {
        return {};
    }
};

typedef std::shared_ptr<vm_runner_interface> vm_runner_interface_ptr;

template<typename F>
class vm_runner : public vm_runner_interface {
public:
    vm_runner(F f) : f_(f) {
    }

    virtual ~vm_runner() = default;

    virtual std::vector<var *> run(std::vector<var *> input) override {
        return f_(input);
    }

private:
    F f_;
};

template<typename F>
vm_runner_interface_ptr make_vm_runner(F f) {
    return std::make_shared<vm_runner<F>>(f);
}

extern "C" var *new_var_nil(fakelua_state *s);

extern "C" var *wrap_return_var(fakelua_state *s, var *v...);

}// namespace fakelua
