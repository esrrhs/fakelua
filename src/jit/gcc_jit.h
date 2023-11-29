#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "var/var.h"
#include "vm.h"
#include <libgccjit++.h>

namespace fakelua {

typedef std::shared_ptr<gccjit::context> gccjit_context_ptr;

class gcc_jitter {
public:
    gcc_jitter() = default;

    ~gcc_jitter() = default;

    void compile(fakelua_state_ptr sp, const syntax_tree_interface_ptr &chunk);

private:
    void compile_const_defines(const syntax_tree_interface_ptr &chunk);

    void compile_const_define(const syntax_tree_interface_ptr &stmt);

    void compile_functions(const syntax_tree_interface_ptr &chunk);

    void compile_function(const std::string &name, const syntax_tree_interface_ptr &funcbody);

    std::string compile_funcname(const syntax_tree_interface_ptr &ptr);

    vm_runner_interface_ptr compile_exp(const syntax_tree_interface_ptr &exp);

private:
    fakelua_state_ptr sp_;
    gccjit_context_ptr gccjit_context_;
};

typedef std::shared_ptr<gcc_jitter> gcc_jitter_ptr;

}// namespace fakelua