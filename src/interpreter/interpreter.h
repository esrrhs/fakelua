#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "var/var.h"
#include "vm.h"

namespace fakelua {

class interpreter {
public:
    interpreter() = default;

    ~interpreter() = default;

    void compile(fakelua_state_ptr sp, const syntax_tree_interface_ptr &chunk);

private:
    void compile_const_defines(fakelua_state_ptr sp, const syntax_tree_interface_ptr &chunk);

    void compile_const_define(fakelua_state_ptr sp, const syntax_tree_interface_ptr &stmt);

    vm_runner_interface_ptr compile_exp(fakelua_state_ptr sp, const syntax_tree_interface_ptr &exp);

private:
    std::unordered_map<std::string, vm_runner_interface_ptr> const_defines_;
};

typedef std::shared_ptr<interpreter> interpreter_ptr;

}// namespace fakelua
