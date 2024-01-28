#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "var/var.h"

namespace fakelua {

class pre_processor {
public:
    pre_processor() = default;

    ~pre_processor();

    void process(fakelua_state_ptr sp, compile_config cfg, const std::string &file_name, const syntax_tree_interface_ptr &chunk);

private:
    void preprocess_const(const syntax_tree_interface_ptr &chunk);

    void preprocess_const_define(const syntax_tree_interface_ptr &stmt);

    void preprocess_function(const syntax_tree_interface_ptr &chunk);

    void preprocess_function_name(const syntax_tree_interface_ptr &func);

    void save_preprocess_trunk_new_stmt(const syntax_tree_interface_ptr &chunk);

private:
    [[noreturn]] void throw_error(const std::string &msg, const syntax_tree_interface_ptr &ptr);

    std::string location_str(const syntax_tree_interface_ptr &ptr);

private:
    // the state contains the running environment we need.
    fakelua_state_ptr sp_;
    // the compile config
    std::string file_name_;
    // save the preprocess trunk new stmt
    std::vector<syntax_tree_interface_ptr> preprocess_trunk_new_stmt_;
};

}// namespace fakelua
