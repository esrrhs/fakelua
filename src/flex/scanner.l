%{
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>
#include "bison/parser.h"
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

string  \"[^\n"]+\"
ws      [ \t]+
alpha   [A-Za-z]
dig     [0-9]
name    ({alpha}|{dig}|\$)({alpha}|{dig}|[_.\-/$])*
num1    [-+]?{dig}+\.?([eE][-+]?{dig}+)?
num2    [-+]?{dig}*\.{dig}+([eE][-+]?{dig}+)?
number  {num1}|{num2}

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

ws      loc.step ();
\n+     loc.lines(yyleng); loc.step();

"="        return yy::parser::make_ASSIGN(loc);
"-"        return yy::parser::make_MINUS(loc);
"+"        return yy::parser::make_PLUS(loc);
"*"        return yy::parser::make_STAR(loc);
"/"        return yy::parser::make_SLASH(loc);
"("        return yy::parser::make_LPAREN(loc);
")"        return yy::parser::make_RPAREN(loc);

{number}     return yy::parser::make_NUMBER(0, loc);

{name}       return yy::parser::make_IDENTIFIER(yytext, loc);

.          {
             throw yy::parser::syntax_error
               (loc, "invalid character: " + std::string(yytext));
}

<<EOF>>    return yy::parser::make_YYEOF (loc);

%%
