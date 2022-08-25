#include "syntax_tree.h"

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
