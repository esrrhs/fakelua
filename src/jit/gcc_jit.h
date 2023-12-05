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

    ~gcc_jitter();

    void compile(fakelua_state_ptr sp, const std::string &file_name, const syntax_tree_interface_ptr &chunk);

private:
    void compile_const_defines(const syntax_tree_interface_ptr &chunk);

    void compile_const_define(const syntax_tree_interface_ptr &stmt);

    void compile_functions(const syntax_tree_interface_ptr &chunk);

    void compile_function(const std::string &name, const syntax_tree_interface_ptr &funcbody);

    std::string compile_funcname(const syntax_tree_interface_ptr &ptr);

    gccjit::rvalue compile_exp(gccjit::function &func, gccjit::block &the_block, const syntax_tree_interface_ptr &exp);

    std::vector<gccjit::param> compile_parlist(syntax_tree_interface_ptr parlist, int &is_variadic);

    void compile_block(gccjit::function &func, const syntax_tree_interface_ptr &block);

    void compile_stmt(gccjit::function &func, gccjit::block &the_block, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_return(gccjit::function &func, gccjit::block &the_block, const syntax_tree_interface_ptr &stmt);

    std::vector<gccjit::rvalue> compile_explist(gccjit::function &func, gccjit::block &the_block, const syntax_tree_interface_ptr &explist);

private:
    gccjit::location new_location(const syntax_tree_interface_ptr &ptr);

private:
    fakelua_state_ptr sp_;
    std::string file_name_;
    gccjit_context_ptr gccjit_context_;
};

typedef std::shared_ptr<gcc_jitter> gcc_jitter_ptr;

}// namespace fakelua
