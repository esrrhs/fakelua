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

%type <fakelua::syntax_tree_interface_ptr> label
%type <fakelua::syntax_tree_interface_ptr> stmt
%type <fakelua::syntax_tree_interface_ptr> block
%type <fakelua::syntax_tree_interface_ptr> chunk
%type <fakelua::syntax_tree_interface_ptr> retstat
%type <fakelua::syntax_tree_interface_ptr> varlist
%type <fakelua::syntax_tree_interface_ptr> explist
%type <fakelua::syntax_tree_interface_ptr> var
%type <fakelua::syntax_tree_interface_ptr> exp
%type <fakelua::syntax_tree_interface_ptr> prefixexp
%type <fakelua::syntax_tree_interface_ptr> functioncall

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
		auto block = std::make_shared<fakelua::syntax_tree_block>(@1);
		if ($1 != nullptr) {
			block->add_stmt($1);
		}
  		$$ = block;
  	}
	|
	block stmt
	{
		LOG(INFO) << "[bison]: block: " << "block stmt";
		auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($1);
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
		$$ = $1;
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
		auto varlist = std::dynamic_pointer_cast<fakelua::syntax_tree_varlist>($1);
		auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>($3);
		if (varlist == nullptr) {
			LOG(ERROR) << "[bison]: stmt: " << "varlist is not a varlist";
			throw std::runtime_error("varlist is not a varlist");
		}
		if (explist == nullptr) {
			LOG(ERROR) << "[bison]: stmt: " << "explist is not a explist";
			throw std::runtime_error("explist is not a explist");
		}
		auto assign = std::make_shared<fakelua::syntax_tree_assign>(varlist, explist, @2);
		$$ = assign;
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
		$$ = std::make_shared<fakelua::syntax_tree_return>(nullptr, @1);
	}
	|
	RETURN explist
	{
		LOG(INFO) << "[bison]: retstat: " << "RETURN explist";
		$$ = std::make_shared<fakelua::syntax_tree_return>($2, @1);
	}
	;

label:
	GOTO_TAG IDENTIFIER GOTO_TAG
	{
		LOG(INFO) << "[bison]: bison get label: " << $2 << " loc: " << @2;
		$$ = std::make_shared<fakelua::syntax_tree_label>($2, @2);
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
		auto varlist = std::make_shared<fakelua::syntax_tree_varlist>(@1);
		varlist->add_var($1);
		$$ = varlist;
	}
	|
	varlist COMMA var
	{
		LOG(INFO) << "[bison]: varlist: " << "varlist COMMA var";
		auto varlist = std::dynamic_pointer_cast<fakelua::syntax_tree_varlist>($1);
		if (varlist == nullptr) {
			LOG(ERROR) << "[bison]: varlist: " << "varlist is not a varlist";
			throw std::runtime_error("varlist is not a varlist");
		}
		varlist->add_var($3);
		$$ = varlist;
	}
	;

var:
	IDENTIFIER
	{
		LOG(INFO) << "[bison]: var: " << "IDENTIFIER";
		auto var = std::make_shared<fakelua::syntax_tree_var>(@1);
		var->set_name($1);
		var->set_type("simple");
		$$ = var;
	}
	|
	prefixexp LSQUARE exp RSQUARE
	{
		LOG(INFO) << "[bison]: var: " << "prefixexp LSQUARE exp RSQUARE";
		auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($1);
		auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($3);
		if (prefixexp == nullptr) {
			LOG(ERROR) << "[bison]: var: " << "prefixexp is not a prefixexp";
			throw std::runtime_error("prefixexp is not a prefixexp");
		}
		if (exp == nullptr) {
			LOG(ERROR) << "[bison]: var: " << "exp is not a exp";
			throw std::runtime_error("exp is not a exp");
		}
		auto var = std::make_shared<fakelua::syntax_tree_var>(@2);
		var->set_prefixexp(prefixexp);
		var->set_exp(exp);
		var->set_type("square");
		$$ = var;
	}
	|
	prefixexp DOT IDENTIFIER
	{
		LOG(INFO) << "[bison]: var: " << "prefixexp DOT IDENTIFIER";
		auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($1);
		if (prefixexp == nullptr) {
			LOG(ERROR) << "[bison]: var: " << "prefixexp is not a prefixexp";
			throw std::runtime_error("prefixexp is not a prefixexp");
		}
		auto var = std::make_shared<fakelua::syntax_tree_var>(@2);
		var->set_prefixexp(prefixexp);
		var->set_name($3);
		var->set_type("dot");
		$$ = var;
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
		$$ = $1;
	}
	|
	functioncall
	{
		LOG(INFO) << "[bison]: prefixexp: " << "functioncall";
		$$ = $1;
	}
	|
	LPAREN exp RPAREN
	{
		LOG(INFO) << "[bison]: prefixexp: " << "LPAREN exp RPAREN";
		$$ = $2;
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
