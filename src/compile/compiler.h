#pragma once

#include "bison/parser.h"
#include "compile/myflexer.h"
#include "interpreter/interpreter.h"
#include <map>
#include <string>

namespace fakelua {

struct compile_result {
    // the chunk name
    std::string chunk_name;
    // the main syntax tree
    syntax_tree_interface_ptr chunk;
    // the interpreter
    interpreter_ptr interpreter;
};

// lua compiler class, parse lua code to syntax tree, and then compile to toy-interpreter runtime, and JIT binary code
class compiler {
public:
    compiler();

    virtual ~compiler();

public:
    // compile the lua file
    compile_result compile_file(const std::string &file);

    // compile the lua string
    compile_result compile_string(const std::string &str);

private:
    // compile the myflexer which already input the file or string
    compile_result compile(myflexer &f);
};

}// namespace fakelua