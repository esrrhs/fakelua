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
    str += explist_->dump(tab + 1);
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
        str += gen_tab(tab + 1) + "type: unknown\n";
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
    str += fieldlist_->dump(tab + 1);
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

std::string syntax_tree_fieldassignment::dump(int tab) const {
    std::string str;
    str += gen_tab(tab) + "(fieldassignment)[" + loc_str() + "]\n";
    if (name_.empty()) {
        str += field_->dump(tab + 1);
    } else {
        str += gen_tab(tab + 1) + "name: " + name_ + "\n";
    }
    str += exp_->dump(tab + 1);
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
    for (auto &name: names_) {
        str += gen_tab(tab + 1) + "name: " + name + "\n";
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

}
