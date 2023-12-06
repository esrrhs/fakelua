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
    // the jitter
    gcc_jitter_ptr jitter;
};

// lua compiler class, parse lua code to syntax tree, and then compile to toy-interpreter runtime, and JIT binary code
class compiler {
public:
    compiler() = default;

    ~compiler() = default;

public:
    // compile the lua file
    compile_result compile_file(fakelua_state_ptr sp, const std::string &file, compile_config cfg);

    // compile the lua string
    compile_result compile_string(fakelua_state_ptr sp, const std::string &str, compile_config cfg);

private:
    // compile the myflexer which already input the file or string
    compile_result compile(fakelua_state_ptr sp, myflexer &f, compile_config cfg);
};

}// namespace fakelua