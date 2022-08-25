%require "3.2"
%language "c++"
%header

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
#include "glog/logging.h"
#include "util/common.h"
#include "compile/syntax_tree.h"

namespace fakelua {
    class myflexer;
}

// https://www.gnu.org/software/bison/manual/html_node/A-Simple-C_002b_002b-Example.html

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
    LOG(INFO) << "[bison]: bison get token: " << ret.name() << " loc:" << ret.location;
    return ret;
}

int yyFlexLexer::yylex() { return -1; }

}

%define api.token.prefix {TOK_}
%token
  ASSIGN        "="
  MINUS         "-"
  PLUS          "+"
  STAR          "*"
  SLASH         "/"
  LPAREN        "("
  RPAREN        ")"
  LCURLY        "{"
  RCURLY        "}"
  LSQUARE       "["
  RSQUARE       "]"
  AND           "and"
  BREAK         "break"
  DO            "do"
  ELSE          "else"
  ELSEIF        "elseif"
  END           "end"
  FALSE         "false"
  FOR           "for"
  FUNCTION      "function"
  goto          "goto"
  IF            "if"
  IN            "in"
  LOCAL         "local"
  NIL           "nil"
  NOT           "not"
  OR            "or"
  REPEAT        "repeat"
  RETURN        "return"
  THEN          "then"
  TRUE          "true"
  UNTIL         "until"
  WHILE         "while"
  DOUBLE_SLASH  "//"
  CONCAT        ".."
  VAR_PARAMS    "..."
  EQUAL         "=="
  MORE_EQUAL    ">="
  LESS_EQUAL    "<="
  NOT_EQUAL     "~="
  LEFT_SHIFT    "<<"
  RIGHT_SHIFT   ">>"
  GOTO_TAG      "::"
;

%token <std::string> IDENTIFIER "identifier"
%token <int> NUMBER "number"

%type <syntax_tree_interface_ptr> label
%type <syntax_tree_interface_ptr> stmt
%type <syntax_tree_interface_ptr> block
%type <syntax_tree_interface_ptr> chunk

%printer { yyo << $$; } <*>;

%%
%start chunk;

chunk:
	block
	{
  		LOG(INFO) << "[bison]: chunk: " << "block";
  		l->set_chunk($1);
	}
	;

block:
  	%empty
  	{
  	}
	|
	stmt
  	{
		LOG(INFO) << "[bison]: block: " << "stmt";
		auto block = std::make_shared<syntax_tree_block>(@1);
		block->add_stmt($1);
  		$$ = block;
  	}
	|
	block stmt
	{
		LOG(INFO) << "[bison]: block: " << "block stmt";
		auto block = std::dynamic_pointer_cast<syntax_tree_block>($1);
		if (block == nullptr) {
			LOG(ERROR) << "[bison]: block: " << "block is not a block";
			throw std::runtime_error("block is not a block");
		}
		block->add_stmt($2);
  		$$ = block;
	}
	;

stmt:
  	label
  	{
  		LOG(INFO) << "[bison]: stmt: " << "label";
  		$$ = $1;
  	}
  	;

label:
	GOTO_TAG IDENTIFIER GOTO_TAG
	{
		LOG(INFO) << "[bison]: bison get label: " << $2 << " loc: " << @2;
		auto label = std::make_shared<syntax_tree_label>($2, @2);
		$$ = label;
	}
	;

%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
