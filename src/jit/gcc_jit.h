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

    void compile(const fakelua_state_ptr &sp, const compile_config &cfg, const std::string &file_name,
                 const syntax_tree_interface_ptr &chunk);

private:
    void compile_const_defines(const syntax_tree_interface_ptr &chunk);

    void compile_const_define(const syntax_tree_interface_ptr &stmt);

    void compile_functions(const syntax_tree_interface_ptr &chunk);

    void compile_function(const std::string &name, const syntax_tree_interface_ptr &funcbody);

    std::string compile_funcname(const syntax_tree_interface_ptr &ptr);

    gccjit::rvalue compile_exp(gccjit::function &func, const syntax_tree_interface_ptr &exp);

    std::vector<std::pair<std::string, gccjit::param>> compile_parlist(syntax_tree_interface_ptr parlist, int &is_variadic);

    void compile_stmt(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_return(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    std::vector<gccjit::rvalue> compile_explist(gccjit::function &func, const syntax_tree_interface_ptr &explist);

    gccjit::rvalue compile_prefixexp(gccjit::function &func, const syntax_tree_interface_ptr &pe);

    gccjit::rvalue compile_var(gccjit::function &func, const syntax_tree_interface_ptr &v);

    gccjit::lvalue compile_var_lvalue(gccjit::function &func, const syntax_tree_interface_ptr &v);

    void compile_stmt_local_var(gccjit::function &function, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_assign(gccjit::function &function, const syntax_tree_interface_ptr &stmt);

    std::vector<gccjit::lvalue> compile_varlist_lvalue(gccjit::function &func, const syntax_tree_interface_ptr &explist);

    gccjit::rvalue compile_tableconstructor(gccjit::function &func, const syntax_tree_interface_ptr &tc);

    std::vector<gccjit::rvalue> compile_fieldlist(gccjit::function &func, const syntax_tree_interface_ptr &fieldlist);

    std::pair<gccjit::rvalue, gccjit::rvalue> compile_field(gccjit::function &func, const syntax_tree_interface_ptr &field);

    gccjit::rvalue compile_binop(gccjit::function &func, const syntax_tree_interface_ptr &left, const syntax_tree_interface_ptr &right,
                                 const syntax_tree_interface_ptr &op);

    gccjit::rvalue compile_unop(gccjit::function &func, const syntax_tree_interface_ptr &right, const syntax_tree_interface_ptr &op);

    gccjit::rvalue compile_functioncall(gccjit::function &func, const syntax_tree_interface_ptr &functioncall);

    std::vector<gccjit::rvalue> compile_args(gccjit::function &func, const syntax_tree_interface_ptr &args);

    void compile_stmt_functioncall(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_label(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_block(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_while(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_repeat(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_if(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_break(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_for_loop(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

    void compile_stmt_for_in(gccjit::function &func, const syntax_tree_interface_ptr &stmt);

private:
    gccjit::location new_location(const syntax_tree_interface_ptr &ptr);

    std::string new_block_name(const std::string &name, const syntax_tree_interface_ptr &ptr);

    void call_global_init_func();

    std::string location_str(const syntax_tree_interface_ptr &ptr);

    gccjit::lvalue find_lvalue_by_name(const std::string &name, const syntax_tree_interface_ptr &ptr);

    std::optional<gccjit::lvalue> try_find_lvalue_by_name(const std::string &name, const syntax_tree_interface_ptr &ptr);

    void save_stack_lvalue_by_name(const std::string &name, const gccjit::lvalue &value, const syntax_tree_interface_ptr &ptr);

    [[noreturn]] void throw_error(const std::string &msg, const syntax_tree_interface_ptr &ptr);

    void check_return_block(gccjit::function &func, const syntax_tree_interface_ptr &ptr);

    bool is_block_ended();

    void init_global_const_var(gccjit::function &func);

private:
    void prepare_compile(const fakelua_state_ptr &sp, const compile_config &cfg, const std::string &file_name);

    bool is_simple_assign(const syntax_tree_interface_ptr &vars, const syntax_tree_interface_ptr &exps);

    bool is_simple_args(const syntax_tree_interface_ptr &args);

    bool is_simple_explist(const syntax_tree_interface_ptr &explist);

    bool is_simple_exp(const syntax_tree_interface_ptr &exp);

    bool is_simple_prefixexp(const syntax_tree_interface_ptr &pe);

    bool is_simple_tableconstructor(const syntax_tree_interface_ptr &tc);

    bool is_simple_field(const syntax_tree_interface_ptr &fieldlist);

    std::string get_simple_prefixexp_name(const syntax_tree_interface_ptr &pe);

    std::string get_simple_var_name(const syntax_tree_interface_ptr &v);

    bool is_jit_builtin_function(const std::string &name);

    std::string get_jit_builtin_function_vm_name(const std::string &name);

    void set_var_int(gccjit::lvalue &var, int64_t v, bool is_const, const syntax_tree_interface_ptr &p);

    void set_var_float(gccjit::lvalue &var, double v, bool is_const, const syntax_tree_interface_ptr &p);

    void set_var_string(gccjit::lvalue &var, const std::string &v, bool is_const, const syntax_tree_interface_ptr &p);

private:
    // the helper type in gccjit
    gccjit::type void_ptr_type_;
    gccjit::type int_type_;
    gccjit::type int64_type_;
    gccjit::type double_type_;
    gccjit::type bool_type_;
    gccjit::type const_char_ptr_type_;
    gccjit::type size_t_type_;

    gccjit::field var_type_field_;
    gccjit::field var_flag_field_;
    gccjit::field var_data_b_field_;
    gccjit::field var_data_i_field_;
    gccjit::field var_data_f_field_;
    gccjit::field var_data_s_field_;
    gccjit::field var_data_t_field_;
    gccjit::field var_data_field_;
    gccjit::type var_data_type_;
    gccjit::type var_struct_;

    gccjit::lvalue global_const_null_var_;
    gccjit::lvalue global_const_false_var_;
    gccjit::lvalue global_const_true_var_;

private:
    // the state contains the running environment we need.
    fakelua_state_ptr sp_;
    // the compiler config
    std::string file_name_;
    // gccjit context
    gccjit_context_ptr gccjit_context_;
    gcc_jit_handle_ptr gcc_jit_handle_;
    // function info saves here
    struct function_info {
        int params_count = 0;
        bool is_variadic = false;
        gccjit::function func;
    };
    // function name -> function info
    std::unordered_map<std::string, function_info> function_infos_;
    // global const var name -> gcc_jit_lvalue
    std::unordered_map<std::string, std::pair<gccjit::lvalue, syntax_tree_interface_ptr>> global_const_vars_;
    // compiling function tmp data
    struct function_data {
        // save stack frame vars
        struct stack_frame {
            std::unordered_map<std::string, gccjit::lvalue> local_vars;
        };
        // every block stack frame
        std::vector<stack_frame> stack_frames;
        // record the block ended
        std::unordered_set<gcc_jit_block *> ended_blocks;
        // use to save the current block
        gccjit::block cur_block;
        // used for jmp to
        std::vector<gccjit::block> stack_end_blocks;
        // temp var name counter
        int tmp_index = 0;
        // mark cur func if is const
        bool is_const = false;
        // current compiling function name
        std::string cur_function_name;
        // current compiling function
        gccjit::function cur_gccjit_func;
    };
    // current compiling function data
    function_data cur_function_data_;
    // save the preprocess trunk new stmt
    std::vector<syntax_tree_interface_ptr> preprocess_trunk_new_stmt_;
};

typedef std::shared_ptr<gcc_jitter> gcc_jitter_ptr;

}// namespace fakelua
