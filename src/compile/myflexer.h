#pragma once

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif
#include "bison/parser.h"
#include <sstream>
#include <fstream>

namespace fakelua {

// create our flexer to use new yylex function my_yylex()
class myflexer : public yyFlexLexer {
public:
    myflexer();

    virtual ~myflexer();

public:
    // implement in scanner.cpp
    yy::parser::symbol_type my_yylex();

    int yylex() { return 0; }

    // set the input lua file
    void input_file(const std::string &file);

    // set the input lua string
    void input_string(const std::string &str);

private:
    // The token's location used by the scanner.
    yy::location location_;
    // Whether to generate parser debug traces.
    bool trace_parsing_;
    // Whether to generate scanner debug traces.
    bool trace_scanning_;
    // The name of the file being parsed.
    std::string filename_;
    // The file being parsed.
    std::ifstream file_;
    // The string being parsed.
    std::istringstream string_;
};

}


