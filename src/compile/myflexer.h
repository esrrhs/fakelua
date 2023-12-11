#pragma once

#if !defined(yyFlexLexerOnce)

#include <FlexLexer.h>

#endif

#include "bison/parser.h"
#include "syntax_tree.h"
#include <fstream>
#include <sstream>

namespace fakelua {

// create our flexer to use new yylex function my_yylex(), and receive the syntax tree
class myflexer : public yyFlexLexer {
public:
    myflexer();

    virtual ~myflexer();

public:
    // implement in scanner.cpp
    yy::parser::symbol_type my_yylex();

    int yylex() {
        return 0;
    }

    // set the input lua file
    void input_file(const std::string &file);

    // set the input lua string
    void input_string(const std::string &str);

    // set the main syntax tree from parser
    void set_chunk(const syntax_tree_interface_ptr &chunk);

    // get the main syntax tree
    syntax_tree_interface_ptr get_chunk() const;

    // remove string quotes
    std::string remove_quotes(const std::string &str);

    // get the filename
    std::string get_filename() const {
        return filename_;
    }

private:
    // replace escape chars
    std::string replace_escape_chars(const std::string &str);

    std::string generate_tmp_file(const std::string &str);

private:
    // The token's location used by the scanner.
    yy::location location_;
    // The name of the file being parsed.
    std::string filename_;
    // The file being parsed.
    std::ifstream file_;
    // The string being parsed.
    std::istringstream string_;
    // The syntax tree from parser.
    syntax_tree_interface_ptr chunk_;
};

}// namespace fakelua
