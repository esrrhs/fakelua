#include "interpreter.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "vm.h"

namespace fakelua {

void interpreter::compile(fakelua_state_ptr sp, const syntax_tree_interface_ptr &chunk) {
    // just walk through the chunk, and save the function declaration and then we can call the function by name
    // first, save the global const define if exists, the const define must be an assignment expression at the top level
    compile_const_defines(sp, chunk);
}

void interpreter::compile_const_defines(fakelua_state_ptr sp, const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    check_syntax_tree_type(chunk, {syntax_tree_type::syntax_tree_type_block});
    // walk through the block
    auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_local_var) {
            compile_const_define(sp, stmt);
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_function ||
                   stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            // skip
        } else {
            throw std::runtime_error("the chunk top level only support const define and function define");
        }
    }
}

void interpreter::compile_const_define(fakelua_state_ptr sp, const syntax_tree_interface_ptr &stmt) {
    auto local_var = std::dynamic_pointer_cast<syntax_tree_local_var>(stmt);
    check_syntax_tree_type(local_var->namelist(), {syntax_tree_type::syntax_tree_type_namelist});
    auto keys = std::dynamic_pointer_cast<syntax_tree_namelist>(local_var->namelist());
    auto &names = keys->names();
    if (!local_var->explist()) {
        throw std::runtime_error("the const define must have a value, but the value is null, it's useless");
    }
    check_syntax_tree_type(local_var->explist(), {syntax_tree_type::syntax_tree_type_explist});
    auto values = std::dynamic_pointer_cast<syntax_tree_explist>(local_var->explist());
    auto &values_exps = values->exps();

    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        if (i >= values_exps.size()) {
            throw std::runtime_error("the const define not match, the value is not enough");
        }
        auto value = values_exps[i];
        check_syntax_tree_type(value, {syntax_tree_type::syntax_tree_type_exp});

        auto exp = std::dynamic_pointer_cast<syntax_tree_exp>(value);

        auto const_value = compile_exp(sp, exp);
        // set the value to the name
        const_defines_[name] = const_value;

        LOG(INFO) << "compile const define: " << name;
    }
}

vm_runner_interface_ptr interpreter::compile_exp(fakelua_state_ptr sp, const syntax_tree_interface_ptr &exp) {
    // the chunk must be a exp
    check_syntax_tree_type(exp, {syntax_tree_type::syntax_tree_type_exp});
    // start compile the expression
    auto e = std::dynamic_pointer_cast<syntax_tree_exp>(exp);
    const auto &exp_type = e->exp_type();
    const auto &value = e->exp_value();

    if (exp_type == "nil") {
        return make_vm_runner([=](std::vector<var *> input) -> std::vector<var *> {
            auto &vp = std::dynamic_pointer_cast<state>(sp)->get_var_pool();
            auto dst = vp.alloc();
            dst->set(nullptr);
            return {dst};
        });
    } else if (exp_type == "false") {
        return make_vm_runner([=](std::vector<var *> input) -> std::vector<var *> {
            auto &vp = std::dynamic_pointer_cast<state>(sp)->get_var_pool();
            auto dst = vp.alloc();
            dst->set(false);
            return {dst};
        });
    } else if (exp_type == "true") {
        return make_vm_runner([=](std::vector<var *> input) -> std::vector<var *> {
            auto &vp = std::dynamic_pointer_cast<state>(sp)->get_var_pool();
            auto dst = vp.alloc();
            dst->set(true);
            return {dst};
        });
    } else if (exp_type == "number") {
        return make_vm_runner([=](std::vector<var *> input) -> std::vector<var *> {
            auto &vp = std::dynamic_pointer_cast<state>(sp)->get_var_pool();
            auto dst = vp.alloc();
            if (is_integer(value)) {
                dst->set((int64_t) std::stoll(value));
            } else {
                dst->set(std::stod(value));
            }
            return {dst};
        });
    } else if (exp_type == "string") {
        return make_vm_runner([=](std::vector<var *> input) -> std::vector<var *> {
            auto &vp = std::dynamic_pointer_cast<state>(sp)->get_var_pool();
            auto dst = vp.alloc();
            dst->set(sp, value);
            return {dst};
        });
    } else if (exp_type == "var_params") {
        // TODO
        return nullptr;
    } else if (exp_type == "functiondef") {
        // TODO
        return nullptr;
    } else if (exp_type == "prefixexp") {
        // TODO
        return nullptr;
    } else if (exp_type == "tableconstructor") {
        // TODO
        return nullptr;
    } else if (exp_type == "binop") {
        // TODO
        return nullptr;
    } else if (exp_type == "unop") {
        // TODO
        return nullptr;
    } else {
        throw std::runtime_error("not support exp type: " + exp_type);
    }
}

}// namespace fakelua
