#include "preprocessor.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

pre_processor::~pre_processor() {
}

void pre_processor::process(fakelua_state_ptr sp, compile_config cfg, const std::string &file_name,
                            const syntax_tree_interface_ptr &chunk) {
    LOG_INFO("start pre_processor::process {}", file_name);

    sp_ = sp;
    file_name_ = file_name;

    // make the const define 'a = 1' to 'a = nil', and add a new stmt 'a = 1' in function __fakelua_global_init__
    // because the const value is not supported function, so we need to make it to nil. and then assign.
    preprocess_const(chunk);

    // make the function name like 'function a.b.c()' to a temp name 'function __fakelua_temp_1__()',
    // and add a new stmt 'xxx.yyy.zzz = "__fakelua_temp_1__"' in function __fakelua_global_init__
    // also, we need to add 'self' in the front of the params if the function is a member function.
    preprocess_functions_name(chunk);

    // now we have funtion __fakelua_global_init__, we need to add it to the chunk. maybe later we will insert more stmts to it.
    save_preprocess_trunk_new_stmt(chunk);

    // change the table assign stmt like 'a.b.c = some_value' lvalue to temp name.
    // like 'local __fakelua_temp_2__; __fakelua_temp_2__ = some_value; __fakelua_set_table__(a.b, "c", __fakelua_temp_2__)'
    // so we can easily always get the value of a.b.c as rvalue.
    preprocess_table_assigns(chunk);

    LOG_INFO("end gcc_jitter::compile {}", file_name);
}

void pre_processor::save_preprocess_trunk_new_stmt(const syntax_tree_interface_ptr &chunk) {
    // add new function __fakelua_global_init__
    auto funcname = std::make_shared<syntax_tree_funcname>(chunk->loc());
    auto funcnamelist = std::make_shared<syntax_tree_funcnamelist>(chunk->loc());
    funcnamelist->add_name("__fakelua_global_init__");
    funcname->set_funcnamelist(funcnamelist);
    auto funcbody = std::make_shared<syntax_tree_funcbody>(chunk->loc());
    auto block = std::make_shared<syntax_tree_block>(chunk->loc());
    for (auto &stmt: preprocess_trunk_new_stmt_) {
        block->add_stmt(stmt);
    }
    funcbody->set_block(block);
    auto func = std::make_shared<syntax_tree_function>(chunk->loc());
    func->set_funcname(funcname);
    func->set_funcbody(funcbody);
    auto chunk_ptr = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    chunk_ptr->add_stmt(func);
    preprocess_trunk_new_stmt_.clear();
}

void pre_processor::preprocess_const(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    check_syntax_tree_type(chunk, {syntax_tree_type::syntax_tree_type_block});
    // walk through the block
    auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_local_var) {
            preprocess_const_define(stmt);
        }
    }
}

void pre_processor::preprocess_const_define(const syntax_tree_interface_ptr &stmt) {
    auto local_var = std::dynamic_pointer_cast<syntax_tree_local_var>(stmt);
    check_syntax_tree_type(local_var->namelist(), {syntax_tree_type::syntax_tree_type_namelist});
    auto keys = std::dynamic_pointer_cast<syntax_tree_namelist>(local_var->namelist());
    auto &names = keys->names();
    if (!local_var->explist()) {
        throw_error("the const define must have a value, but the value is null, it's useless", local_var);
    }
    check_syntax_tree_type(local_var->explist(), {syntax_tree_type::syntax_tree_type_explist});
    auto values = std::dynamic_pointer_cast<syntax_tree_explist>(local_var->explist());
    auto &values_exps = values->exps();

    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        if (i >= values_exps.size()) {
            throw_error("the const define not match, the value is not enough", values);
        }

        auto assign_stmt = std::make_shared<syntax_tree_assign>(stmt->loc());
        auto varlist = std::make_shared<syntax_tree_varlist>(stmt->loc());
        auto var = std::make_shared<syntax_tree_var>(stmt->loc());
        var->set_type("simple");
        var->set_name(name);
        varlist->add_var(var);
        assign_stmt->set_varlist(varlist);
        auto explist = std::make_shared<syntax_tree_explist>(stmt->loc());
        explist->add_exp(values_exps[i]);
        assign_stmt->set_explist(explist);

        auto nil_exp = std::make_shared<syntax_tree_exp>(stmt->loc());
        nil_exp->set_type("nil");
        values_exps[i] = nil_exp;

        preprocess_trunk_new_stmt_.push_back(assign_stmt);
    }
}

void pre_processor::preprocess_functions_name(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    check_syntax_tree_type(chunk, {syntax_tree_type::syntax_tree_type_block});
    // walk through the block
    auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_function) {
            auto func = std::dynamic_pointer_cast<syntax_tree_function>(stmt);
            preprocess_function_name(func);
        }
    }
}

void pre_processor::preprocess_function_name(const syntax_tree_interface_ptr &func) {
    check_syntax_tree_type(func, {syntax_tree_type::syntax_tree_type_function});
    auto func_ptr = std::dynamic_pointer_cast<syntax_tree_function>(func);

    auto funcname = std::dynamic_pointer_cast<syntax_tree_funcname>(func_ptr->funcname());
    auto funcnamelistptr = funcname->funcnamelist();

    auto newfuncnamelistptr = std::make_shared<syntax_tree_funcnamelist>(funcnamelistptr->loc());
    std::vector<std::string> newnamelist;

    check_syntax_tree_type(funcnamelistptr, {syntax_tree_type::syntax_tree_type_funcnamelist});
    auto funcnamelist = std::dynamic_pointer_cast<syntax_tree_funcnamelist>(funcnamelistptr);
    auto &names = funcnamelist->funcnames();
    newnamelist.insert(newnamelist.end(), names.begin(), names.end());

    if (!funcname->colon_name().empty()) {
        newnamelist.push_back(funcname->colon_name());
        // insert self in the front of params
        auto funcbody = func_ptr->funcbody();
        check_syntax_tree_type(funcbody, {syntax_tree_type::syntax_tree_type_funcbody});
        auto funcbody_ptr = std::dynamic_pointer_cast<syntax_tree_funcbody>(funcbody);
        auto parlist = funcbody_ptr->parlist();
        auto new_parlist = std::make_shared<syntax_tree_parlist>(funcbody_ptr->loc());
        auto namelist = std::make_shared<syntax_tree_namelist>(funcbody_ptr->loc());
        namelist->add_name("self");
        if (parlist) {
            auto old_namelist = std::dynamic_pointer_cast<syntax_tree_parlist>(parlist)->namelist();
            for (auto &name: std::dynamic_pointer_cast<syntax_tree_namelist>(old_namelist)->names()) {
                namelist->add_name(name);
            }
        }
        std::dynamic_pointer_cast<syntax_tree_parlist>(new_parlist)->set_namelist(namelist);
        parlist = new_parlist;
        funcbody_ptr->set_parlist(parlist);

        funcname->set_colon_name("");
    }

    DEBUG_ASSERT(!newnamelist.empty());

    if (newnamelist.size() == 1) {
        // global function
        newfuncnamelistptr->add_name(newnamelist[0]);
        funcname->set_funcnamelist(newfuncnamelistptr);
    } else {
        // member function
        // xxx.yyy(), we need alloc special function name, and set the name to xxx.yyy
        auto name = std::dynamic_pointer_cast<state>(sp_)->get_vm().alloc_temp_name();
        newfuncnamelistptr->add_name(name);
        funcname->set_funcnamelist(newfuncnamelistptr);

        // set the name to xxx.yyy.zzz = name in preprocess_trunk_new_stmt_
        auto assign_stmt = std::make_shared<syntax_tree_assign>(funcname->loc());
        auto varlist = std::make_shared<syntax_tree_varlist>(funcname->loc());

        auto prefixexp = std::make_shared<syntax_tree_prefixexp>(funcname->loc());
        auto first_var = std::make_shared<syntax_tree_var>(funcname->loc());
        first_var->set_type("simple");
        first_var->set_name(newnamelist[0]);
        prefixexp->set_type("var");
        prefixexp->set_value(first_var);

        syntax_tree_interface_ptr last_prefixexp = prefixexp;
        syntax_tree_interface_ptr last_var = first_var;
        for (size_t i = 1; i < newnamelist.size(); ++i) {
            auto var = std::make_shared<syntax_tree_var>(funcname->loc());
            var->set_type("dot");
            var->set_name(newnamelist[i]);
            var->set_prefixexp(last_prefixexp);

            auto cur_prefixexp = std::make_shared<syntax_tree_prefixexp>(funcname->loc());
            cur_prefixexp->set_type("var");
            cur_prefixexp->set_value(var);

            last_var = var;
            last_prefixexp = cur_prefixexp;
        }

        varlist->add_var(last_var);
        assign_stmt->set_varlist(varlist);

        auto explist = std::make_shared<syntax_tree_explist>(funcname->loc());
        auto exp = std::make_shared<syntax_tree_exp>(funcname->loc());
        exp->set_type("string");
        exp->set_value(name);
        explist->add_exp(exp);

        assign_stmt->set_explist(explist);

        preprocess_trunk_new_stmt_.push_back(assign_stmt);
    }
}

[[noreturn]] void pre_processor::throw_error(const std::string &msg, const syntax_tree_interface_ptr &ptr) {
    throw_fakelua_exception(std::format("{} at {}", msg, location_str(ptr)));
}

std::string pre_processor::location_str(const syntax_tree_interface_ptr &ptr) {
    return std::format("{}:{}:{}", file_name_, ptr->loc().begin.line, ptr->loc().begin.column);
}

void pre_processor::preprocess_table_assigns(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    check_syntax_tree_type(chunk, {syntax_tree_type::syntax_tree_type_block});
    // walk through the block
    auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_function) {
            auto func = std::dynamic_pointer_cast<syntax_tree_function>(stmt);
            preprocess_table_assign(func->funcbody());
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            auto func = std::dynamic_pointer_cast<syntax_tree_local_function>(stmt);
            preprocess_table_assign(func->funcbody());
        }
    }
}

void pre_processor::preprocess_table_assign(const syntax_tree_interface_ptr &funcbody) {
    check_syntax_tree_type(funcbody, {syntax_tree_type::syntax_tree_type_funcbody});
    auto funcbody_ptr = std::dynamic_pointer_cast<syntax_tree_funcbody>(funcbody);

    auto block = funcbody_ptr->block();
    check_syntax_tree_type(block, {syntax_tree_type::syntax_tree_type_block});
    auto block_ptr = std::dynamic_pointer_cast<syntax_tree_block>(block);


}

}// namespace fakelua
