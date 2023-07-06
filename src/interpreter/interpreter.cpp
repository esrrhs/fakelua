#include "interpreter.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

void interpreter::compile(const syntax_tree_interface_ptr &chunk) {
    // just walk through the chunk, and save the function declaration and then we can call the function by name
    // first, save the global const define if exists, the const define must be an assignment expression at the top level
    compile_const_define(chunk);
}

void interpreter::compile_const_define(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    if (chunk->type() != syntax_tree_type::syntax_tree_type_block) { throw std::runtime_error("the chunk must be a block"); }

    // walk through the block
    auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->get_stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_local_var) {
            auto local_var = std::dynamic_pointer_cast<syntax_tree_local_var>(stmt);
            // TODO
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_function ||
                   stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            // skip
        } else {
            throw std::runtime_error("the chunk top level only support const define and function define");
        }
    }
}

}// namespace fakelua
