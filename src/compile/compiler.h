#pragma once

#include "bison/parser.h"
#include "compile/my_flexer.h"
#include "compile_common.h"
#include "jit/gcc_jit.h"
#include <map>
#include <string>

namespace fakelua {

struct CompileResult {
    // the file name
    std::string fileName;
    // the main syntax tree
    SyntaxTreeInterfacePtr chunk;
};

// lua Compiler class, parse lua code to a syntax tree, and then compile to toy-interpreter runtime, and JIT binary code
class Compiler {
public:
    Compiler() = default;

    ~Compiler() = default;

public:
    // compile the lua file
    CompileResult CompileFile(const FakeluaStatePtr &sp, const std::string &file, const CompileConfig &cfg);

    // compile the lua string
    CompileResult CompileString(const FakeluaStatePtr &sp, const std::string &str, const CompileConfig &cfg);

private:
    // compile the myflexer which already input the file or string
    CompileResult Compile(const FakeluaStatePtr &sp, MyFlexer &f, const CompileConfig &cfg);
};

}// namespace fakelua