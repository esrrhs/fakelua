%require "3.2"
%language "c++"
%header

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
#include <string>
#include "glog/logging.h"
namespace fakelua {
    class myflexer;
}
}

// The parsing context.
%param { fakelua::myflexer* l }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
#include "compile/myflexer.h"

yy::parser::symbol_type yylex(fakelua::myflexer* l) {
	auto ret = l->my_yylex();
	LOG(INFO) << "[bison]: bison get token: " << ret.name() << " " << *ret.location.begin.filename
	    << "(" << ret.location.begin.line << ":" << ret.location.begin.column << ")";
	return ret;
}

}

%define api.token.prefix {TOK_}
%token
  ASSIGN  ":="
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
;

%token <std::string> IDENTIFIER "identifier"
%token <int> NUMBER "number"
%nterm <int> exp

%printer { yyo << $$; } <*>;

%%
%start unit;

unit: assignments exp  {  $2; };

assignments:
  %empty                 {}
| assignments assignment {};

assignment:
  "identifier" ":=" exp {  $3; };

%left "+" "-";
%left "*" "/";
exp:
  "number"
| "identifier"  { $$ = 0; }
| exp "+" exp   { $$ = $1 + $3; }
| exp "-" exp   { $$ = $1 - $3; }
| exp "*" exp   { $$ = $1 * $3; }
| exp "/" exp   { $$ = $1 / $3; }
| "(" exp ")"   { $$ = $2; }
%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
