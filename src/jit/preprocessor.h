#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "var/var.h"

namespace fakelua {

class pre_processor {
public:
    pre_processor() = default;

    ~pre_processor() = default;

    void process(const fakelua_state_ptr &sp, const compile_config &cfg, const std::string &file_name,
                 const syntax_tree_interface_ptr &chunk);

private:
    void preprocess_const(const syntax_tree_interface_ptr &chunk);

    void preprocess_const_define(const syntax_tree_interface_ptr &stmt);

    void preprocess_functions_name(const syntax_tree_interface_ptr &chunk);

    void preprocess_function_name(const syntax_tree_interface_ptr &func);

    void save_preprocess_global_init(const syntax_tree_interface_ptr &chunk);

    void preprocess_table_assigns(const syntax_tree_interface_ptr &chunk);

    void preprocess_table_assign(const syntax_tree_interface_ptr &funcbody);

    void preprocess_extracts_literal_constants(const syntax_tree_interface_ptr &chunk);

private:
    [[noreturn]] void throw_error(const std::string &msg, const syntax_tree_interface_ptr &ptr);

    std::string location_str(const syntax_tree_interface_ptr &ptr);

    void dump_debug_file(const syntax_tree_interface_ptr &chunk, int step);

private:
    // the state contains the running environment we need.
    fakelua_state_ptr sp_;
    // the compiler config
    std::string file_name_;
    // save the preprocess trunk new stmt in global_init func
    std::vector<syntax_tree_interface_ptr> global_init_new_stmt_;
    // temp var name counter
    int pre_index_ = 0;
};

}// namespace fakelua
