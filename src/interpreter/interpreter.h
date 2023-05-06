#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"

namespace fakelua {

class interpreter {
public:
    interpreter() = default;

    ~interpreter() = default;

    void compile(const syntax_tree_interface_ptr &chunk);

private:
    void compile_const_define(const syntax_tree_interface_ptr &chunk);
};

typedef std::shared_ptr<interpreter> interpreter_ptr;

}// namespace fakelua
