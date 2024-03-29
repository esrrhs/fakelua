#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "gcc_jit_handle.h"
#include "var/var.h"
#include "vm.h"
#include <libgccjit++.h>

namespace fakelua {

typedef std::shared_ptr<gccjit::context> gccjit_context_ptr;

class gcc_jitter {
public:
    gcc_jitter() = default;

    ~gcc_jitter();

    void compile(fakelua_state_ptr sp, compile_config cfg, const std::string &file_name, const syntax_tree_interface_ptr &chunk);

private:
    void compile_const_defines(const syntax_tree_interface_ptr &chunk);

    void compile_const_define(const syntax_tree_interface_ptr &stmt);

    void compile_functions(const syntax_tree_interface_ptr &chunk);

    void compile_function(const std::string &name, const syntax_tree_interface_ptr &funcbody);

    std::string compile_funcname(const syntax_tree_interface_ptr &ptr);

    gccjit::rvalue compile_exp(gccjit::function &func, const syntax_tree_interface_ptr &exp, bool is_const);

    std::vector<std::pair<std::string, gccjit::param>> compile_parlist(syntax_tree_interface_ptr parlist, int &is_variadic);

    void compile_block(gccjit::function &func, const syntax_tree_interface_ptr &block);

    void compile_stmt(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_return(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    std::vector<gccjit::rvalue> compile_explist(gccjit::function &func, const syntax_tree_interface_ptr &explist);

    void compile_const_defines_init_func();

    gccjit::rvalue compile_prefixexp(gccjit::function &func, const syntax_tree_interface_ptr &pe, bool is_const);

    gccjit::lvalue compile_var(const syntax_tree_interface_ptr &v, bool is_const);

    void compile_stmt_local_var(gccjit::function &function, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_assign(gccjit::function &function, const syntax_tree_interface_ptr &stmt);

    std::vector<gccjit::lvalue> compile_varlist(gccjit::function &func, const syntax_tree_interface_ptr &explist);

    gccjit::rvalue compile_tableconstructor(gccjit::function &func, const syntax_tree_interface_ptr &tc, bool is_const);

    std::vector<gccjit::rvalue> compile_fieldlist(gccjit::function &func, const syntax_tree_interface_ptr &fieldlist, bool is_const);

    std::pair<gccjit::rvalue, gccjit::rvalue> compile_field(gccjit::function &func, const syntax_tree_interface_ptr &field, bool is_const);

    gccjit::rvalue compile_binop(gccjit::function &func, const syntax_tree_interface_ptr &left, const syntax_tree_interface_ptr &right,
                                 const syntax_tree_interface_ptr &op, bool is_const);

    gccjit::rvalue compile_unop(gccjit::function &func, const syntax_tree_interface_ptr &right, const syntax_tree_interface_ptr &op,
                                bool is_const);

    gccjit::rvalue compile_functioncall(gccjit::function &func, const syntax_tree_interface_ptr &functioncall);

    std::vector<gccjit::rvalue> compile_args(gccjit::function &func, const syntax_tree_interface_ptr &args);

private:
    gccjit::location new_location(const syntax_tree_interface_ptr &ptr);

    std::string new_block_name(const std::string &name, const syntax_tree_interface_ptr &ptr);

    void call_const_defines_init_func();

    std::string location_str(const syntax_tree_interface_ptr &ptr);

    gccjit::lvalue find_lvalue_by_name(const std::string &name, const syntax_tree_interface_ptr &ptr);

    void save_stack_lvalue_by_name(const std::string &name, const gccjit::lvalue &value, const syntax_tree_interface_ptr &ptr);

    [[noreturn]] void throw_error(const std::string &msg, const syntax_tree_interface_ptr &ptr);

    void check_return_block(gccjit::function &func, const syntax_tree_interface_ptr &ptr);

private:
    fakelua_state_ptr sp_;
    std::string file_name_;
    gccjit_context_ptr gccjit_context_;
    gcc_jit_handle_ptr gcc_jit_handle_;
    struct function_info {
        int params_count = 0;
        bool is_variadic = false;
    };
    std::unordered_map<std::string, function_info> function_infos_;
    std::unordered_map<std::string, std::pair<gccjit::lvalue, syntax_tree_interface_ptr>> global_const_vars_;
    std::vector<std::string> global_const_vars_vec_;
    struct function_data {
        struct stack_frame {
            std::unordered_map<std::string, gccjit::lvalue> local_vars;
        };
        std::vector<stack_frame> stack_frames;
        std::unordered_set<gcc_jit_block *> ended_blocks;
        gccjit::block cur_block;
        int pre_index = 0;
    };
    function_data cur_function_data_;
};

typedef std::shared_ptr<gcc_jitter> gcc_jitter_ptr;

}// namespace fakelua
