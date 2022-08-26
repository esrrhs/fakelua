#include "syntax_tree.h"

namespace fakelua {

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


}
