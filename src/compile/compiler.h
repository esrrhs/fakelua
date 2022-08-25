#pragma once

#include <string>
#include <map>
#include "bison/parser.h"
#include "compile/myflexer.h"

namespace fakelua {

struct compile_result {
    syntax_tree_interface_ptr chunk;
};

// lua compiler class, make the code to JIT binary code
class compiler {
public:
    compiler();

    virtual ~compiler();

public:
    // compile the lua file
    compile_result compile_file(const std::string &file);

    // compile the lua string
    compile_result compile_string(const std::string &str);

public:
};

}