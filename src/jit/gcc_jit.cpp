#include "gcc_jit.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "vm.h"

namespace fakelua {

gcc_jitter::~gcc_jitter() {
    if (gccjit_context_) {
        gccjit_context_->release();
        gccjit_context_ = nullptr;
    }
}

void gcc_jitter::compile(fakelua_state_ptr sp, const syntax_tree_interface_ptr &chunk) {
    sp_ = sp;
    // init gccjit
    gccjit_context_ = std::make_shared<gccjit::context>(gccjit::context::acquire());
    /* Set some options on the context.
     Turn this on to see the code being generated, in assembler form.  */
    gccjit_context_->set_bool_option(GCC_JIT_BOOL_OPTION_DUMP_GENERATED_CODE, 0);

    // just walk through the chunk, and save the function declaration and then we can call the function by name
    // first, check the global const define, the const define must be an assignment expression at the top level
    compile_const_defines(chunk);

    // second, walk through the chunk, and save the function declaration
    compile_functions(chunk);
}

void gcc_jitter::compile_functions(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    check_syntax_tree_type(chunk, {syntax_tree_type::syntax_tree_type_block});
    // walk through the block
    auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_function) {
            auto func = std::dynamic_pointer_cast<syntax_tree_function>(stmt);
            auto name = compile_funcname(func->funcname());
            compile_function(name, func->funcbody());
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            auto func = std::dynamic_pointer_cast<syntax_tree_local_function>(stmt);
            compile_function(func->name(), func->funcbody());
        }
    }
}

std::string gcc_jitter::compile_funcname(const syntax_tree_interface_ptr &ptr) {
    check_syntax_tree_type(ptr, {syntax_tree_type::syntax_tree_type_funcname});

    std::string ret;

    auto name = std::dynamic_pointer_cast<syntax_tree_funcname>(ptr);
    auto funcnamelistptr = name->funcnamelist();

    if (funcnamelistptr) {
        check_syntax_tree_type(funcnamelistptr, {syntax_tree_type::syntax_tree_type_funcnamelist});
        auto funcnamelist = std::dynamic_pointer_cast<syntax_tree_funcnamelist>(funcnamelistptr);
        auto &names = funcnamelist->funcnames();
        // join the names with .
        ret = join_string(names, ".");
    }

    if (!name->colon_name().empty()) {
        ret += ":" + name->colon_name();
    }

    return ret;
}

void gcc_jitter::compile_function(const std::string &name, const syntax_tree_interface_ptr &funcbody) {
    // TODO
}

void gcc_jitter::compile_const_defines(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    check_syntax_tree_type(chunk, {syntax_tree_type::syntax_tree_type_block});
    // walk through the block
    auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_local_var) {
            compile_const_define(stmt);
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_function ||
                   stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            // skip
        } else {
            throw std::runtime_error("the chunk top level only support const define and function define");
        }
    }
}

void gcc_jitter::compile_const_define(const syntax_tree_interface_ptr &stmt) {
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

        LOG(INFO) << "compile const define: " << name;
    }
}

vm_runner_interface_ptr gcc_jitter::compile_exp(const syntax_tree_interface_ptr &exp) {
    auto sp = sp_;

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
