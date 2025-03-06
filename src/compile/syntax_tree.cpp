#include "syntax_tree.h"

namespace fakelua {

std::string syntax_tree_empty::dump(int tab) const {
    return gen_tab(tab) + "(empty)[" + loc_str() + "]\n";
}

std::string syntax_tree_block::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(block)[" + loc_str() + "]\n";
    for (auto &stmt: stmts_) {
        str += stmt->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_label::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + name_ + "(label)[" + loc_str() + "]\n";
    return str;
}

std::string syntax_tree_return::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(return)[" + loc_str() + "]\n";
    if (explist_) {
        str += explist_->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_assign::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(assign)[" + loc_str() + "]\n";
    str += varlist_->dump(tab + 1);
    str += explist_->dump(tab + 1);
    return str;
}

std::string syntax_tree_varlist::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(varlist)[" + loc_str() + "]\n";
    for (auto &var: vars_) {
        str += var->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_explist::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(explist)[" + loc_str() + "]\n";
    for (auto &exp: exps_) {
        str += exp->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_var::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(var)[" + loc_str() + "]\n";
    if (type_ == "simple") {
        str += gen_tab(tab + 1) + "type: simple\n";
        str += gen_tab(tab + 1) + "name: " + name_ + "\n";
    } else if (type_ == "square") {
        str += gen_tab(tab + 1) + "type: square\n";
        str += prefixexp_->dump(tab + 1);
        str += exp_->dump(tab + 1);
    } else if (type_ == "dot") {
        str += gen_tab(tab + 1) + "type: dot\n";
        str += prefixexp_->dump(tab + 1);
        str += gen_tab(tab + 1) + "name: " + name_ + "\n";
    } else {
        DEBUG_ASSERT(false && "unknown var type");
    }
    return str;
}

std::string syntax_tree_functioncall::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(functioncall)[" + loc_str() + "]\n";
    str += prefixexp_->dump(tab + 1);
    if (!name_.empty()) {
        str += gen_tab(tab + 1) + "name: " + name_ + "\n";
    }
    str += args_->dump(tab + 1);
    return str;
}

std::string syntax_tree_tableconstructor::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(tableconstructor)[" + loc_str() + "]\n";
    if (fieldlist_) {
        str += fieldlist_->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_fieldlist::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(fieldlist)[" + loc_str() + "]\n";
    for (auto &field: fields_) {
        str += field->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_field::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(field)[" + loc_str() + "]\n";
    if (type_ == "array") {
        str += gen_tab(tab + 1) + "type: array\n";
        if (key_) {
            str += key_->dump(tab + 1);
        }
        str += value_->dump(tab + 1);
    } else if (type_ == "object") {
        str += gen_tab(tab + 1) + "type: object\n";
        str += gen_tab(tab + 1) + "name: " + name_ + "\n";
        str += value_->dump(tab + 1);
    } else {
        DEBUG_ASSERT(false && "unknown field type");
    }
    return str;
}

std::string syntax_tree_break::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(break)[" + loc_str() + "]\n";
    return str;
}

std::string syntax_tree_goto::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(goto)[" + loc_str() + "]\n";
    str += gen_tab(tab + 1) + "label: " + label_ + "\n";
    return str;
}

std::string syntax_tree_while::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(while)[" + loc_str() + "]\n";
    str += exp_->dump(tab + 1);
    str += block_->dump(tab + 1);
    return str;
}

std::string syntax_tree_repeat::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(repeat)[" + loc_str() + "]\n";
    str += block_->dump(tab + 1);
    str += exp_->dump(tab + 1);
    return str;
}

std::string syntax_tree_if::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(if)[" + loc_str() + "]\n";
    str += exp_->dump(tab + 1);
    str += block_->dump(tab + 1);
    str += elseifs_->dump(tab + 1);
    if (elseblock_) {
        str += elseblock_->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_elseiflist::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(elseif)[" + loc_str() + "]\n";
    for (int i = 0; i < (int) exps_.size(); ++i) {
        str += exps_[i]->dump(tab + 1);
        str += blocks_[i]->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_for_loop::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(for_loop)[" + loc_str() + "]\n";
    str += gen_tab(tab + 1) + "name: " + name_ + "\n";
    str += exp_begin_->dump(tab + 1);
    str += exp_end_->dump(tab + 1);
    if (exp_step_) {
        str += exp_step_->dump(tab + 1);
    }
    str += block_->dump(tab + 1);
    return str;
}

std::string syntax_tree_for_in::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(for_in)[" + loc_str() + "]\n";
    str += namelist_->dump(tab + 1);
    str += explist_->dump(tab + 1);
    str += block_->dump(tab + 1);
    return str;
}

std::string syntax_tree_namelist::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(namelist)[" + loc_str() + "]\n";
    size_t index = 0;
    for (auto &name: names_) {
        str += gen_tab(tab + 1) + "name: " + name + "\n";
        if (index < attrib_.size() && attrib_[index] != "") {
            str += gen_tab(tab + 1) + "attrib: " + attrib_[index] + "\n";
        }
        ++index;
    }
    return str;
}

std::string syntax_tree_function::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(function)[" + loc_str() + "]\n";
    str += funcname_->dump(tab + 1);
    str += funcbody_->dump(tab + 1);
    return str;
}

std::string syntax_tree_funcnamelist::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(funcnamelist)[" + loc_str() + "]\n";
    for (auto &name: funcnames_) {
        str += gen_tab(tab + 1) + "name: " + name + "\n";
    }
    return str;
}

std::string syntax_tree_funcname::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(funcname)[" + loc_str() + "]\n";
    str += funcnamelist_->dump(tab + 1);
    str += gen_tab(tab + 1) + "colon_name: " + colon_name_ + "\n";
    return str;
}

std::string syntax_tree_funcbody::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(funcbody)[" + loc_str() + "]\n";
    if (parlist_) {
        str += parlist_->dump(tab + 1);
    }
    str += block_->dump(tab + 1);
    return str;
}

std::string syntax_tree_functiondef::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(functiondef)[" + loc_str() + "]\n";
    str += funcbody_->dump(tab + 1);
    return str;
}

std::string syntax_tree_parlist::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(parlist)[" + loc_str() + "]\n";
    if (namelist_) {
        str += namelist_->dump(tab + 1);
    }
    if (var_params_) {
        str += gen_tab(tab + 1) + "var_params: " + std::to_string(var_params_) + "\n";
    }
    return str;
}

std::string syntax_tree_local_function::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(local_function)[" + loc_str() + "]\n";
    str += gen_tab(tab + 1) + "name: " + name_ + "\n";
    str += funcbody_->dump(tab + 1);
    return str;
}

std::string syntax_tree_local_var::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(local_var)[" + loc_str() + "]\n";
    str += namelist_->dump(tab + 1);
    if (explist_) {
        str += explist_->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_exp::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(exp)[" + loc_str() + "]\n";
    str += gen_tab(tab + 1) + "type: " + type_ + "\n";
    str += gen_tab(tab + 1) + "value: " + value_ + "\n";
    if (left_) {
        str += left_->dump(tab + 1);
    }
    if (op_) {
        str += op_->dump(tab + 1);
    }
    if (right_) {
        str += right_->dump(tab + 1);
    }
    return str;
}

std::string syntax_tree_binop::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(binop)[" + loc_str() + "]\n";
    str += gen_tab(tab + 1) + "op: " + op_ + "\n";
    return str;
}

std::string syntax_tree_unop::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(unop)[" + loc_str() + "]\n";
    str += gen_tab(tab + 1) + "op: " + op_ + "\n";
    return str;
}

std::string syntax_tree_args::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(args)[" + loc_str() + "]\n";
    if (type_ == "explist") {
        str += explist_->dump(tab + 1);
    } else if (type_ == "tableconstructor") {
        str += tableconstructor_->dump(tab + 1);
    } else if (type_ == "string") {
        str += string_->dump(tab + 1);
    } else if (type_ == "empty") {
        str += gen_tab(tab + 1) + "empty" + "\n";
    } else {
        DEBUG_ASSERT(false && "unknown args type");
    }
    return str;
}

std::string syntax_tree_prefixexp::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(prefixexp)[" + loc_str() + "]\n";
    str += gen_tab(tab + 1) + "type: " + type_ + "\n";
    str += value_->dump(tab + 1);
    return str;
}

void walk_syntax_tree(const syntax_tree_interface_ptr &node, walk_syntax_tree_func func) {
    if (!node) {
        return;
    }
    switch (node->type()) {
        case syntax_tree_type::syntax_tree_type_empty: {
            auto empty = std::dynamic_pointer_cast<syntax_tree_empty>(node);
            func(empty);
            break;
        }
        case syntax_tree_type::syntax_tree_type_block: {
            auto block = std::dynamic_pointer_cast<syntax_tree_block>(node);
            func(block);
            for (auto &stmt: block->stmts()) {
                walk_syntax_tree(stmt, func);
            }
            break;
        }
        case syntax_tree_type::syntax_tree_type_label: {
            auto label = std::dynamic_pointer_cast<syntax_tree_label>(node);
            func(label);
            break;
        }
        case syntax_tree_type::syntax_tree_type_return: {
            auto ret = std::dynamic_pointer_cast<syntax_tree_return>(node);
            func(ret);
            walk_syntax_tree(ret->explist(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_assign: {
            auto assign = std::dynamic_pointer_cast<syntax_tree_assign>(node);
            func(assign);
            walk_syntax_tree(assign->varlist(), func);
            walk_syntax_tree(assign->explist(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_varlist: {
            auto varlist = std::dynamic_pointer_cast<syntax_tree_varlist>(node);
            func(varlist);
            for (auto &var: varlist->vars()) {
                walk_syntax_tree(var, func);
            }
            break;
        }
        case syntax_tree_type::syntax_tree_type_explist: {
            auto explist = std::dynamic_pointer_cast<syntax_tree_explist>(node);
            func(explist);
            for (auto &exp: explist->exps()) {
                walk_syntax_tree(exp, func);
            }
            break;
        }
        case syntax_tree_type::syntax_tree_type_var: {
            auto var = std::dynamic_pointer_cast<syntax_tree_var>(node);
            func(var);
            walk_syntax_tree(var->get_exp(), func);
            walk_syntax_tree(var->get_prefixexp(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_functioncall: {
            auto functioncall = std::dynamic_pointer_cast<syntax_tree_functioncall>(node);
            func(functioncall);
            walk_syntax_tree(functioncall->prefixexp(), func);
            walk_syntax_tree(functioncall->args(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_tableconstructor: {
            auto tableconstructor = std::dynamic_pointer_cast<syntax_tree_tableconstructor>(node);
            func(tableconstructor);
            walk_syntax_tree(tableconstructor->fieldlist(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_fieldlist: {
            auto fieldlist = std::dynamic_pointer_cast<syntax_tree_fieldlist>(node);
            func(fieldlist);
            for (auto &field: fieldlist->fields()) {
                walk_syntax_tree(field, func);
            }
            break;
        }
        case syntax_tree_type::syntax_tree_type_field: {
            auto field = std::dynamic_pointer_cast<syntax_tree_field>(node);
            func(field);
            walk_syntax_tree(field->key(), func);
            walk_syntax_tree(field->value(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_break: {
            auto bre = std::dynamic_pointer_cast<syntax_tree_break>(node);
            func(bre);
            break;
        }
        case syntax_tree_type::syntax_tree_type_goto: {
            auto go = std::dynamic_pointer_cast<syntax_tree_goto>(node);
            func(go);
            break;
        }
        case syntax_tree_type::syntax_tree_type_while: {
            auto wh = std::dynamic_pointer_cast<syntax_tree_while>(node);
            func(wh);
            walk_syntax_tree(wh->exp(), func);
            walk_syntax_tree(wh->block(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_repeat: {
            auto rep = std::dynamic_pointer_cast<syntax_tree_repeat>(node);
            func(rep);
            walk_syntax_tree(rep->block(), func);
            walk_syntax_tree(rep->exp(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_if: {
            auto i = std::dynamic_pointer_cast<syntax_tree_if>(node);
            func(i);
            walk_syntax_tree(i->exp(), func);
            walk_syntax_tree(i->block(), func);
            walk_syntax_tree(i->elseifs(), func);
            walk_syntax_tree(i->elseblock(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_elseiflist: {
            auto elseifs = std::dynamic_pointer_cast<syntax_tree_elseiflist>(node);
            func(elseifs);
            for (auto &exp: elseifs->elseif_exps()) {
                walk_syntax_tree(exp, func);
            }
            for (auto &block: elseifs->elseif_blocks()) {
                walk_syntax_tree(block, func);
            }
            break;
        }
        case syntax_tree_type::syntax_tree_type_for_loop: {
            auto for_loop = std::dynamic_pointer_cast<syntax_tree_for_loop>(node);
            func(for_loop);
            walk_syntax_tree(for_loop->exp_begin(), func);
            walk_syntax_tree(for_loop->exp_end(), func);
            walk_syntax_tree(for_loop->exp_step(), func);
            walk_syntax_tree(for_loop->block(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_for_in: {
            auto for_in = std::dynamic_pointer_cast<syntax_tree_for_in>(node);
            func(for_in);
            walk_syntax_tree(for_in->namelist(), func);
            walk_syntax_tree(for_in->explist(), func);
            walk_syntax_tree(for_in->block(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_namelist: {
            auto namelist = std::dynamic_pointer_cast<syntax_tree_namelist>(node);
            func(namelist);
            break;
        }
        case syntax_tree_type::syntax_tree_type_function: {
            auto f = std::dynamic_pointer_cast<syntax_tree_function>(node);
            func(f);
            walk_syntax_tree(f->funcname(), func);
            walk_syntax_tree(f->funcbody(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_funcnamelist: {
            auto funcnamelist = std::dynamic_pointer_cast<syntax_tree_funcnamelist>(node);
            func(funcnamelist);
            break;
        }
        case syntax_tree_type::syntax_tree_type_funcname: {
            auto funcname = std::dynamic_pointer_cast<syntax_tree_funcname>(node);
            func(funcname);
            walk_syntax_tree(funcname->funcnamelist(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_funcbody: {
            auto funcbody = std::dynamic_pointer_cast<syntax_tree_funcbody>(node);
            func(funcbody);
            walk_syntax_tree(funcbody->parlist(), func);
            walk_syntax_tree(funcbody->block(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_functiondef: {
            auto functiondef = std::dynamic_pointer_cast<syntax_tree_functiondef>(node);
            func(functiondef);
            walk_syntax_tree(functiondef->funcbody(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_parlist: {
            auto parlist = std::dynamic_pointer_cast<syntax_tree_parlist>(node);
            func(parlist);
            walk_syntax_tree(parlist->namelist(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_local_function: {
            auto local_function = std::dynamic_pointer_cast<syntax_tree_local_function>(node);
            func(local_function);
            walk_syntax_tree(local_function->funcbody(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_local_var: {
            auto local_var = std::dynamic_pointer_cast<syntax_tree_local_var>(node);
            func(local_var);
            walk_syntax_tree(local_var->namelist(), func);
            walk_syntax_tree(local_var->explist(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_exp: {
            auto exp = std::dynamic_pointer_cast<syntax_tree_exp>(node);
            func(exp);
            walk_syntax_tree(exp->left(), func);
            walk_syntax_tree(exp->op(), func);
            walk_syntax_tree(exp->right(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_binop: {
            auto binop = std::dynamic_pointer_cast<syntax_tree_binop>(node);
            func(binop);
            break;
        }
        case syntax_tree_type::syntax_tree_type_unop: {
            auto unop = std::dynamic_pointer_cast<syntax_tree_unop>(node);
            func(unop);
            break;
        }
        case syntax_tree_type::syntax_tree_type_args: {
            auto args = std::dynamic_pointer_cast<syntax_tree_args>(node);
            func(args);
            walk_syntax_tree(args->explist(), func);
            walk_syntax_tree(args->tableconstructor(), func);
            walk_syntax_tree(args->string(), func);
            break;
        }
        case syntax_tree_type::syntax_tree_type_prefixexp: {
            auto prefixexp = std::dynamic_pointer_cast<syntax_tree_prefixexp>(node);
            func(prefixexp);
            walk_syntax_tree(prefixexp->get_value(), func);
            break;
        }
        default: {
            DEBUG_ASSERT(false && "unknown syntax tree type");
        }
    }
}

}// namespace fakelua
