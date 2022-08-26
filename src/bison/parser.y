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
  SEMICOLON     ";"
  COLON         ":"
  COMMA         ","
  DOT           "."
  XOR           "^"
  MOD           "%"
  BITAND        "&"
  BITOR         "|"
  BITNOT        "~"
  MORE          ">"
  LESS          "<"
  NUMBER_SIGN   "#"
;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> STRING "string"
%token <double> NUMBER "number"

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
		if ($1 != nullptr) {
			block->add_stmt($1);
		}
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
		if ($2 != nullptr) {
			block->add_stmt($2);
		}
  		$$ = block;
	}
	;

stmt:
        retstat
        {
        	LOG(INFO) << "[bison]: stmt: " << "retstat";
        	$$ = nullptr;
        }
        |
	SEMICOLON
	{
		LOG(INFO) << "[bison]: stmt: " << "SEMICOLON";
		$$ = nullptr;
	}
	|
	varlist ASSIGN explist
	{
		LOG(INFO) << "[bison]: stmt: " << "varlist ASSIGN explist";
		$$ = nullptr;
	}
	|
  	label
  	{
  		LOG(INFO) << "[bison]: stmt: " << "label";
  		$$ = $1;
  	}
  	;

retstat:
	RETURN
	{
		LOG(INFO) << "[bison]: retstat: " << "RETURN";
	}
	|
	RETURN explist
	{
		LOG(INFO) << "[bison]: retstat: " << "RETURN explist";
	}
	;

label:
	GOTO_TAG IDENTIFIER GOTO_TAG
	{
		LOG(INFO) << "[bison]: bison get label: " << $2 << " loc: " << @2;
		$$ = std::make_shared<syntax_tree_label>($2, @2);
	}
	;

funcnamelist:
	IDENTIFIER
	{
		LOG(INFO) << "[bison]: funcnamelist: " << "IDENTIFIER";
	}
	|
	funcnamelist DOT IDENTIFIER
	{
		LOG(INFO) << "[bison]: funcnamelist: " << "funcnamelist DOT IDENTIFIER";
	}
	;

funcname:
	funcnamelist
	{
		LOG(INFO) << "[bison]: funcname: " << "funcnamelist";
	}
	|
	funcnamelist COLON IDENTIFIER
	{
		LOG(INFO) << "[bison]: funcname: " << "funcnamelist COLON IDENTIFIER";
	}
	;

varlist:
	var
	{
		LOG(INFO) << "[bison]: varlist: " << "var";
	}
	|
	varlist COMMA var
	{
		LOG(INFO) << "[bison]: varlist: " << "varlist COMMA var";
	}
	;

var:
	IDENTIFIER
	{
		LOG(INFO) << "[bison]: var: " << "IDENTIFIER";
	}
	|
	prefixexp LSQUARE exp RSQUARE
	{
		LOG(INFO) << "[bison]: var: " << "prefixexp LSQUARE exp RSQUARE";
	}
	|
	prefixexp DOT IDENTIFIER
	{
		LOG(INFO) << "[bison]: var: " << "prefixexp DOT IDENTIFIER";
	}
	;

namelist:
	IDENTIFIER
	{
		LOG(INFO) << "[bison]: namelist: " << "IDENTIFIER";
	}
	|
	namelist COMMA IDENTIFIER
	{
		LOG(INFO) << "[bison]: namelist: " << "namelist COMMA IDENTIFIER";
	}
	;

explist:
	exp
	{
		LOG(INFO) << "[bison]: explist: " << "exp";
	}
	|
	explist COMMA exp
	{
		LOG(INFO) << "[bison]: explist: " << "explist COMMA exp";
	}
	;

exp:
	NIL
	{
		LOG(INFO) << "[bison]: exp: " << "NIL";
	}
	|
	TRUE
	{
		LOG(INFO) << "[bison]: exp: " << "TRUE";
	}
	|
	FALSE
	{
		LOG(INFO) << "[bison]: exp: " << "FALSE";
	}
	|
	NUMBER
	{
		LOG(INFO) << "[bison]: exp: " << "NUMBER";
	}
	|
	STRING
	{
		LOG(INFO) << "[bison]: exp: " << "STRING";
	}
	|
	VAR_PARAMS
	{
		LOG(INFO) << "[bison]: exp: " << "VAR_PARAMS";
	}
	|
	functiondef
	{
		LOG(INFO) << "[bison]: exp: " << "functiondef";
	}
	|
	prefixexp
	{
		LOG(INFO) << "[bison]: exp: " << "prefixexp";
	}
	|
	tableconstructor
	{
		LOG(INFO) << "[bison]: exp: " << "tableconstructor";
	}
	|
	exp binop exp
	{
		LOG(INFO) << "[bison]: exp: " << "exp binop exp";
	}
	|
	unop exp
	{
		LOG(INFO) << "[bison]: exp: " << "unop exp";
	}
	;

prefixexp:
	var
	{
		LOG(INFO) << "[bison]: prefixexp: " << "var";
	}
	|
	functioncall
	{
		LOG(INFO) << "[bison]: prefixexp: " << "functioncall";
	}
	|
	LPAREN exp RPAREN
	{
		LOG(INFO) << "[bison]: prefixexp: " << "LPAREN exp RPAREN";
	}

functioncall:
	prefixexp args
	{
		LOG(INFO) << "[bison]: functioncall: " << "prefixexp args";
	}
	|
	prefixexp COLON IDENTIFIER args
	{
		LOG(INFO) << "[bison]: functioncall: " << "prefixexp COLON IDENTIFIER args";
	}
	;

args:
	LPAREN explist RPAREN
	{
		LOG(INFO) << "[bison]: args: " << "LPAREN explist RPAREN";
	}
	|
	LPAREN RPAREN
	{
		LOG(INFO) << "[bison]: args: " << "LPAREN RPAREN";
	}
	|
	tableconstructor
	{
		LOG(INFO) << "[bison]: args: " << "tableconstructor";
	}
	|
	STRING
	{
		LOG(INFO) << "[bison]: args: " << "STRING";
	}
	;

functiondef:
	FUNCTION funcbody
	{
		LOG(INFO) << "[bison]: functiondef: " << "FUNCTION funcbody";
	}
	;

funcbody:
	LPAREN parlist RPAREN block END
	{
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN parlist RPAREN block END";
	}
	|
	LPAREN RPAREN block END
	{
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN RPAREN block END";
	}
	;

parlist:
	namelist
	{
		LOG(INFO) << "[bison]: parlist: " << "namelist";
	}
	|
	namelist COMMA VAR_PARAMS
	{
		LOG(INFO) << "[bison]: parlist: " << "namelist COMMA VAR_PARAMS";
	}
	|
	VAR_PARAMS
	{
		LOG(INFO) << "[bison]: parlist: " << "VAR_PARAMS";
	}
	;

tableconstructor:
	LCURLY fieldlist RCURLY
	{
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY fieldlist RCURLY";
	}
	|
	LCURLY RCURLY
	{
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY RCURLY";
	}
	;

fieldlist:
	field
	{
		LOG(INFO) << "[bison]: fieldlist: " << "field";
	}
	|
	fieldlist fieldsep field
	{
		LOG(INFO) << "[bison]: fieldlist: " << "fieldlist fieldsep field";
	}
	;


field:
	LSQUARE exp RSQUARE ASSIGN exp
	{
		LOG(INFO) << "[bison]: field: " << "LSQUARE exp RSQUARE ASSIGN exp";
	}
	|
	IDENTIFIER ASSIGN exp
	{
		LOG(INFO) << "[bison]: field: " << "IDENTIFIER ASSIGN exp";
	}
	|
	exp
	{
		LOG(INFO) << "[bison]: field: " << "exp";
	}
	;


fieldsep:
	COMMA
	{
		LOG(INFO) << "[bison]: fieldsep: " << "COMMA";
	}
	|
	SEMICOLON
	{
		LOG(INFO) << "[bison]: fieldsep: " << "SEMICOLON";
	}
	;

binop:
	PLUS
	{
		LOG(INFO) << "[bison]: binop: " << "PLUS";
	}
	|
	MINUS
	{
		LOG(INFO) << "[bison]: binop: " << "MINUS";
	}
	|
	STAR
	{
		LOG(INFO) << "[bison]: binop: " << "STAR";
	}
	|
	SLASH
	{
		LOG(INFO) << "[bison]: binop: " << "SLASH";
	}
	|
	DOUBLE_SLASH
	{
		LOG(INFO) << "[bison]: binop: " << "DOUBLE_SLASH";
	}
	|
	XOR
	{
		LOG(INFO) << "[bison]: binop: " << "XOR";
	}
	|
	MOD
	{
		LOG(INFO) << "[bison]: binop: " << "MOD";
	}
	|
	BITAND
	{
		LOG(INFO) << "[bison]: binop: " << "BITAND";
	}
	|
	BITNOT
	{
		LOG(INFO) << "[bison]: binop: " << "BITNOT";
	}
	|
	BITOR
	{
		LOG(INFO) << "[bison]: binop: " << "BITOR";
	}
	|
	RIGHT_SHIFT
	{
		LOG(INFO) << "[bison]: binop: " << "RIGHT_SHIFT";
	}
	|
	LEFT_SHIFT
	{
		LOG(INFO) << "[bison]: binop: " << "LEFT_SHIFT";
	}
	|
	CONCAT
	{
		LOG(INFO) << "[bison]: binop: " << "CONCAT";
	}
	|
	LESS
	{
		LOG(INFO) << "[bison]: binop: " << "LESS";
	}
	|
	LESS_EQUAL
	{
		LOG(INFO) << "[bison]: binop: " << "LESS_EQUAL";
	}
	|
	MORE
	{
		LOG(INFO) << "[bison]: binop: " << "MORE";
	}
	|
	MORE_EQUAL
	{
		LOG(INFO) << "[bison]: binop: " << "MORE_EQUAL";
	}
	|
	EQUAL
	{
		LOG(INFO) << "[bison]: binop: " << "EQUAL";
	}
	|
	NOT_EQUAL
	{
		LOG(INFO) << "[bison]: binop: " << "NOT_EQUAL";
	}
	|
	AND
	{
		LOG(INFO) << "[bison]: binop: " << "AND";
	}
	|
	OR
	{
		LOG(INFO) << "[bison]: binop: " << "OR";
	}
	;

unop:
	MINUS
	{
		LOG(INFO) << "[bison]: unop: " << "MINUS";
	}
	|
	NOT
	{
		LOG(INFO) << "[bison]: unop: " << "NOT";
	}
	|
	NUMBER_SIGN
	{
		LOG(INFO) << "[bison]: unop: " << "NUMBER_SIGN";
	}
	|
	BITNOT
	{
		LOG(INFO) << "[bison]: unop: " << "BITNOT";
	}
	;
%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
