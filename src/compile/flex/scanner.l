%{
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>
#include "compile/bison/parser.h"
#include "compile/myflexer.h"

// https://westes.github.io/flex/manual/Cxx.html

// Give Flex the prototype of yylex we want ...
# define YY_DECL \
    yy::parser::symbol_type fakelua::myflexer::my_yylex()

%}

%{

#define FLEX_VERSION (YY_FLEX_MAJOR_VERSION * 100 + YY_FLEX_MINOR_VERSION)

%}

%option c++ noyywrap nounput noinput batch debug yylineno

%{

%}

string1  \"((?:\\\"|[^\"\n])*)\"
string2  \'((\\'|[^'\n])*)\'
string3  \[\[([^\]]|(\][^\]]))*\]\]
string   {string1}|{string2}|{string3}
name     [_a-zA-Z][_a-zA-Z0-9]{0,30}
num1     -?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?
num2     -?0[xX][0-9a-fA-F]+(\.[0-9a-fA-F]+)?([pP][+-]?[0-9]+)?
number   {num1}|{num2}

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  yy::location& loc = location_;
  // Code run each time yylex is called.
  loc.step ();
%}

[ \t]+          loc.step ();
\n+             loc.lines(yyleng); loc.step();
\r+             {}

"%"             return yy::parser::make_MOD(loc);
"&"             return yy::parser::make_BITAND(loc);
"|"             return yy::parser::make_BITOR(loc);
"~"             return yy::parser::make_BITNOT(loc);
">"             return yy::parser::make_MORE(loc);
"<"             return yy::parser::make_LESS(loc);
"#"             return yy::parser::make_NUMBER_SIGN(loc);
"^"             return yy::parser::make_XOR(loc);
";"             return yy::parser::make_SEMICOLON(loc);
":"             return yy::parser::make_COLON(loc);
","             return yy::parser::make_COMMA(loc);
"."             return yy::parser::make_DOT(loc);
".."            return yy::parser::make_CONCAT(loc);
"..."           return yy::parser::make_VAR_PARAMS(loc);
"=="            return yy::parser::make_EQUAL(loc);
">="            return yy::parser::make_MORE_EQUAL(loc);
"<="            return yy::parser::make_LESS_EQUAL(loc);
"~="            return yy::parser::make_NOT_EQUAL(loc);
"<<"            return yy::parser::make_LEFT_SHIFT(loc);
">>"            return yy::parser::make_RIGHT_SHIFT(loc);
"::"            return yy::parser::make_GOTO_TAG(loc);
"="             return yy::parser::make_ASSIGN(loc);
"-"             return yy::parser::make_MINUS(loc);
"+"             return yy::parser::make_PLUS(loc);
"*"             return yy::parser::make_STAR(loc);
"/"             return yy::parser::make_SLASH(loc);
"("             return yy::parser::make_LPAREN(loc);
")"             return yy::parser::make_RPAREN(loc);
"{"             return yy::parser::make_LCURLY(loc);
"}"             return yy::parser::make_RCURLY(loc);
"["             return yy::parser::make_LSQUARE(loc);
"]"             return yy::parser::make_RSQUARE(loc);
"and"           return yy::parser::make_AND(loc);
"break"         return yy::parser::make_BREAK(loc);
"do"            return yy::parser::make_DO(loc);
"else"          return yy::parser::make_ELSE(loc);
"elseif"        return yy::parser::make_ELSEIF(loc);
"end"           return yy::parser::make_END(loc);
"false"         return yy::parser::make_FALSES(loc);
"for"           return yy::parser::make_FOR(loc);
"function"      return yy::parser::make_FUNCTION(loc);
"goto"          return yy::parser::make_GOTO(loc);
"if"            return yy::parser::make_IF(loc);
"in"            return yy::parser::make_IN(loc);
"local"         return yy::parser::make_LOCAL(loc);
"nil"           return yy::parser::make_NIL(loc);
"not"           return yy::parser::make_NOT(loc);
"or"            return yy::parser::make_OR(loc);
"repeat"        return yy::parser::make_REPEAT(loc);
"return"        return yy::parser::make_RETURN(loc);
"then"          return yy::parser::make_THEN(loc);
"true"          return yy::parser::make_TRUE(loc);
"until"         return yy::parser::make_UNTIL(loc);
"while"         return yy::parser::make_WHILE(loc);
"//"            return yy::parser::make_DOUBLE_SLASH(loc);

{number}        return yy::parser::make_NUMBER(yytext, loc);

{name}          return yy::parser::make_IDENTIFIER(yytext, loc);

{string}        return yy::parser::make_STRING(yytext, loc);

.               { throw yy::parser::syntax_error(loc, "invalid character: [" + std::string(yytext) + "]"); }

<<EOF>>         return yy::parser::make_YYEOF (loc);

%%
