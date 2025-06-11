#pragma once

#include "bison/parser.h"
#include "compile/myflexer.h"
#include "compile_common.h"
#include "jit/gcc_jit.h"
#include <map>
#include <string>

namespace fakelua {

struct compile_result {
    // the file name
    std::string file_name;
    // the main syntax tree
    syntax_tree_interface_ptr chunk;
};

// lua compiler class, parse lua code to a syntax tree, and then compile to toy-interpreter runtime, and JIT binary code
class compiler {
public:
    compiler() = default;

    ~compiler() = default;

public:
    // compile the lua file
    compile_result compile_file(const fakelua_state_ptr &sp, const std::string &file, const compile_config &cfg);

    // compile the lua string
    compile_result compile_string(const fakelua_state_ptr &sp, const std::string &str, const compile_config &cfg);

private:
    // compile the myflexer which already input the file or string
    compile_result compile(const fakelua_state_ptr &sp, myflexer &f, const compile_config &cfg);
};

}// namespace fakelua