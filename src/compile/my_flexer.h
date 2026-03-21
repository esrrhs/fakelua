#pragma once

#if !defined(yyFlexLexerOnce)

#include <FlexLexer.h>

#endif

#include "bison/parser.h"
#include "syntax_tree.h"
#include <fstream>
#include <sstream>

namespace fakelua {

// create our flexer to use new yylex function MyYylex(), and receive the syntax tree
class MyFlexer : public yyFlexLexer {
public:
    MyFlexer() = default;

    ~MyFlexer() override = default;

public:
    // implement in scanner.cpp
    yy::parser::SymbolType MyYylex();

    // set the input lua file
    void InputFile(const std::string &file);

    // set the input lua string
    void InputString(const std::string &str);

    // set the main syntax tree from parser
    void SetChunk(const SyntaxTreeInterfacePtr &chunk);

    // get the main syntax tree
    SyntaxTreeInterfacePtr GetChunk() const;

    // remove string quotes
    std::string RemoveQuotes(const std::string &str);

    // get the filename
    std::string GetFilename() const {
        return filename_;
    }

private:
    std::string GenerateTmpFile(const std::string &str);

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
    SyntaxTreeInterfacePtr chunk_;
};

}// namespace fakelua
