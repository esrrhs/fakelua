#pragma once

#include <string>
#include <map>
#include "bison/parser.h"
#include "compile/myflexer.h"

namespace fakelua {

// lua compiler class, make the code to JIT binary code
class compiler {
public:
    compiler();

    virtual ~compiler();

public:
    // compile the lua file
    void compile_file(const std::string &file);

    // compile the lua string
    void compile_string(const std::string &str);

public:
};

}