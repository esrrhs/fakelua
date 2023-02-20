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
  GOTO          "goto"
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
%type <fakelua::syntax_tree_interface_ptr> args
%type <fakelua::syntax_tree_interface_ptr> tableconstructor
%type <fakelua::syntax_tree_interface_ptr> fieldlist
%type <fakelua::syntax_tree_interface_ptr> field
%type <fakelua::syntax_tree_interface_ptr> elseifs
%type <fakelua::syntax_tree_interface_ptr> namelist
%type <fakelua::syntax_tree_interface_ptr> parlist
%type <fakelua::syntax_tree_interface_ptr> funcbody
%type <fakelua::syntax_tree_interface_ptr> funcname
%type <fakelua::syntax_tree_interface_ptr> funcnamelist

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
  		LOG(INFO) << "[bison]: block: " << "empty";
  		$$ = std::make_shared<fakelua::syntax_tree_block>(@0);
  	}
	|
	stmt
  	{
		LOG(INFO) << "[bison]: block: " << "stmt";
		auto block = std::make_shared<fakelua::syntax_tree_block>(@1);
		auto stmt = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($1);
		if (stmt == nullptr) {
			LOG(ERROR) << "[bison]: block: " << "stmt is nullptr";
		}
		block->add_stmt(stmt);
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
		auto stmt = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($2);
		if (stmt == nullptr) {
			LOG(ERROR) << "[bison]: block: " << "stmt is not a stmt";
			throw std::runtime_error("stmt is not a stmt");
		}
		block->add_stmt(stmt);
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
		$$ = std::make_shared<fakelua::syntax_tree_empty>(@1);
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
		auto assign = std::make_shared<fakelua::syntax_tree_assign>(@2);
		assign->set_varlist(varlist);
		assign->set_explist(explist);
		$$ = assign;
	}
	|
  	label
  	{
  		LOG(INFO) << "[bison]: stmt: " << "label";
  		$$ = $1;
  	}
  	|
  	BREAK
  	{
  		LOG(INFO) << "[bison]: stmt: " << "break";
  		$$ = std::make_shared<fakelua::syntax_tree_break>(@1);
  	}
  	|
  	GOTO IDENTIFIER
  	{
  		LOG(INFO) << "[bison]: stmt: " << "goto IDENTIFIER";
  		auto go = std::make_shared<fakelua::syntax_tree_goto>(@2);
  		go->set_label($2);
  		$$ = go;
  	}
  	|
  	DO block END
  	{
  		LOG(INFO) << "[bison]: stmt: " << "do block end";
  		$$ = $2;
  	}
  	|
  	WHILE exp DO block END
  	{
  		LOG(INFO) << "[bison]: stmt: " << "while exp do block end";
  		auto while_stmt = std::make_shared<fakelua::syntax_tree_while>(@1);
  		while_stmt->set_exp($2);
  		while_stmt->set_block($4);
  		$$ = while_stmt;
  	}
  	|
  	REPEAT block UNTIL exp
  	{
  		LOG(INFO) << "[bison]: stmt: " << "repeat block until exp";
  		auto repeat = std::make_shared<fakelua::syntax_tree_repeat>(@1);
  		repeat->set_block($2);
  		repeat->set_exp($4);
  		$$ = repeat;
  	}
  	|
  	IF exp THEN block elseifs ELSE block END
  	{
  		LOG(INFO) << "[bison]: stmt: " << "if exp then block elseifs else block end";
  		auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(@1);
  		if_stmt->set_exp($2);
  		if_stmt->set_block($4);
  		auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>($5);
  		if (elseifs == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "elseiflist is not a elseiflist";
  			throw std::runtime_error("elseiflist is not a elseiflist");
  		}
  		if_stmt->set_elseiflist(elseifs);
  		if_stmt->set_else_block($7);
  		$$ = if_stmt;
  	}
  	|
  	IF exp THEN block elseifs END
  	{
  		LOG(INFO) << "[bison]: stmt: " << "if exp then block elseifs end";
  		auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(@1);
  		if_stmt->set_exp($2);
  		if_stmt->set_block($4);
  		auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>($5);
  		if (elseifs == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "elseiflist is not a elseiflist";
  			throw std::runtime_error("elseiflist is not a elseiflist");
  		}
  		if_stmt->set_elseiflist(elseifs);
  		$$ = if_stmt;
  	}
  	|
  	FOR IDENTIFIER ASSIGN exp COMMA exp DO block END
  	{
  		LOG(INFO) << "[bison]: stmt: " << "for IDENTIFIER assign exp COMMA exp do block end";
  		auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(@1);
  		for_loop_stmt->set_name($2);
  		for_loop_stmt->set_exp_begin($4);
  		for_loop_stmt->set_exp_end($6);
  		for_loop_stmt->set_block($8);
  		$$ = for_loop_stmt;
  	}
  	|
  	FOR IDENTIFIER ASSIGN exp COMMA exp COMMA exp DO block END
  	{
  		LOG(INFO) << "[bison]: stmt: " << "for IDENTIFIER assign exp COMMA exp COMMA exp do block end";
  		auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(@1);
  		for_loop_stmt->set_name($2);
  		for_loop_stmt->set_exp_begin($4);
  		for_loop_stmt->set_exp_end($6);
  		for_loop_stmt->set_exp_step($8);
  		for_loop_stmt->set_block($10);
  		$$ = for_loop_stmt;
  	}
  	|
  	FOR namelist IN explist DO block END
  	{
  		LOG(INFO) << "[bison]: stmt: " << "for namelist in explist do block end";
  		auto for_in_stmt = std::make_shared<fakelua::syntax_tree_for_in>(@1);
  		auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>($2);
  		if (namelist == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "namelist is not a namelist";
  			throw std::runtime_error("namelist is not a namelist");
  		}
  		auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>($4);
  		if (explist == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "explist is not a explist";
  			throw std::runtime_error("explist is not a explist");
  		}
  		for_in_stmt->set_namelist(namelist);
  		for_in_stmt->set_explist(explist);
  		for_in_stmt->set_block($6);
  		$$ = for_in_stmt;
  	}
  	|
  	FUNCTION funcname funcbody
  	{
  		LOG(INFO) << "[bison]: stmt: " << "function funcname funcbody";
  		auto func_stmt = std::make_shared<fakelua::syntax_tree_function>(@1);
  		auto funcname = std::dynamic_pointer_cast<fakelua::syntax_tree_funcname>($2);
  		if (funcname == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "funcname is not a funcname";
  			throw std::runtime_error("funcname is not a funcname");
  		}
  		auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>($3);
  		if (funcbody == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "funcbody is not a funcbody";
  			throw std::runtime_error("funcbody is not a funcbody");
  		}
  		func_stmt->set_funcname(funcname);
  		func_stmt->set_funcbody(funcbody);
  		$$ = func_stmt;
  	}
  	;

elseifs:
	%empty
	{
		LOG(INFO) << "[bison]: elseifs: " << "empty";
		$$ = std::make_shared<fakelua::syntax_tree_elseiflist>(@0);
	}
	|
	ELSEIF exp THEN block
	{
		LOG(INFO) << "[bison]: elseifs: " << "elseif exp then block";
		auto elseifs = std::make_shared<fakelua::syntax_tree_elseiflist>(@1);
		elseifs->add_elseif($2, $4);
		$$ = elseifs;
	}
	|
	elseifs ELSEIF exp THEN block
	{
		LOG(INFO) << "[bison]: elseifs: " << "elseifs elseif exp then block";
		auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>($1);
		if (elseifs == nullptr) {
			LOG(ERROR) << "[bison]: elseifs: " << "elseifs is not a elseifs";
			throw std::runtime_error("elseifs is not a elseifs");
		}
		elseifs->add_elseif($3, $5);
		$$ = elseifs;
	}
	;

retstat:
	RETURN
	{
		LOG(INFO) << "[bison]: retstat: " << "RETURN";
		auto ret = std::make_shared<fakelua::syntax_tree_return>(@1);
		ret->set_explist(nullptr);
		$$ = ret;
	}
	|
	RETURN explist
	{
		LOG(INFO) << "[bison]: retstat: " << "RETURN explist";
		auto ret = std::make_shared<fakelua::syntax_tree_return>(@1);
		ret->set_explist($2);
		$$ = ret;
	}
	;

label:
	GOTO_TAG IDENTIFIER GOTO_TAG
	{
		LOG(INFO) << "[bison]: bison get label: " << $2 << " loc: " << @2;
		auto ret = std::make_shared<fakelua::syntax_tree_label>(@2);
		ret->set_name($2);
		$$ = ret;
	}
	;

funcnamelist:
	IDENTIFIER
	{
		LOG(INFO) << "[bison]: funcnamelist: " << "IDENTIFIER";
		auto funcnamelist = std::make_shared<fakelua::syntax_tree_funcnamelist>(@1);
		funcnamelist->add_name($1);
		$$ = funcnamelist;
	}
	|
	funcnamelist DOT IDENTIFIER
	{
		LOG(INFO) << "[bison]: funcnamelist: " << "funcnamelist DOT IDENTIFIER";
		auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>($1);
		if (funcnamelist == nullptr) {
			LOG(ERROR) << "[bison]: funcnamelist: " << "funcnamelist is not a funcnamelist";
			throw std::runtime_error("funcnamelist is not a funcnamelist");
		}
		funcnamelist->add_name($3);
		$$ = funcnamelist;
	}
	;

funcname:
	funcnamelist
	{
		LOG(INFO) << "[bison]: funcname: " << "funcnamelist";
		auto funcname = std::make_shared<fakelua::syntax_tree_funcname>(@1);
		auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>($1);
		if (funcnamelist == nullptr) {
			LOG(ERROR) << "[bison]: funcname: " << "funcnamelist is not a funcnamelist";
			throw std::runtime_error("funcnamelist is not a funcnamelist");
		}
		funcname->set_funcnamelist(funcnamelist);
		$$ = funcname;
	}
	|
	funcnamelist COLON IDENTIFIER
	{
		LOG(INFO) << "[bison]: funcname: " << "funcnamelist COLON IDENTIFIER";
		auto funcname = std::make_shared<fakelua::syntax_tree_funcname>(@1);
		auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>($1);
		if (funcnamelist == nullptr) {
			LOG(ERROR) << "[bison]: funcname: " << "funcnamelist is not a funcnamelist";
			throw std::runtime_error("funcnamelist is not a funcnamelist");
		}
		funcname->set_funcnamelist(funcnamelist);
		funcname->set_colon_name($3);
		$$ = funcname;
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
		auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($1);
		auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($2);
		if (prefixexp == nullptr) {
			LOG(ERROR) << "[bison]: functioncall: " << "prefixexp is not a prefixexp";
			throw std::runtime_error("prefixexp is not a prefixexp");
		}
		if (args == nullptr) {
			LOG(ERROR) << "[bison]: functioncall: " << "args is not a args";
			throw std::runtime_error("args is not a args");
		}
		auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(@1);
		functioncall->set_prefixexp(prefixexp);
		functioncall->set_args(args);
		$$ = functioncall;
	}
	|
	prefixexp COLON IDENTIFIER args
	{
		LOG(INFO) << "[bison]: functioncall: " << "prefixexp COLON IDENTIFIER args";
		auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($1);
		if (prefixexp == nullptr) {
			LOG(ERROR) << "[bison]: functioncall: " << "prefixexp is not a prefixexp";
			throw std::runtime_error("prefixexp is not a prefixexp");
		}
		auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(@1);
		functioncall->set_prefixexp(prefixexp);
		functioncall->set_name($3);
		auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($4);
		if (args == nullptr) {
			LOG(ERROR) << "[bison]: functioncall: " << "args is not a args";
			throw std::runtime_error("args is not a args");
		}
		functioncall->set_args(args);
		$$ = functioncall;
	}
	;

args:
	LPAREN explist RPAREN
	{
		LOG(INFO) << "[bison]: args: " << "LPAREN explist RPAREN";
		$$ = $2;
	}
	|
	LPAREN RPAREN
	{
		LOG(INFO) << "[bison]: args: " << "LPAREN RPAREN";
		$$ = std::make_shared<fakelua::syntax_tree_empty>(@1);
	}
	|
	tableconstructor
	{
		LOG(INFO) << "[bison]: args: " << "tableconstructor";
		$$ = $1;
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
		auto parlist = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($2);
		if (parlist == nullptr) {
			LOG(ERROR) << "[bison]: funcbody: " << "parlist is not a parlist";
			throw std::runtime_error("parlist is not a parlist");
		}
		auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($4);
		if (block == nullptr) {
			LOG(ERROR) << "[bison]: funcbody: " << "block is not a block";
			throw std::runtime_error("block is not a block");
		}
		auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(@1);
		funcbody->set_parlist(parlist);
		funcbody->set_block(block);
		$$ = funcbody;
	}
	|
	LPAREN RPAREN block END
	{
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN RPAREN block END";
		auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($3);
		if (block == nullptr) {
			LOG(ERROR) << "[bison]: funcbody: " << "block is not a block";
			throw std::runtime_error("block is not a block");
		}
		auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(@1);
		funcbody->set_block(block);
		$$ = funcbody;
	}
	;

parlist:
	namelist
	{
		LOG(INFO) << "[bison]: parlist: " << "namelist";
		auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($1);
		if (namelist == nullptr) {
			LOG(ERROR) << "[bison]: parlist: " << "namelist is not a namelist";
			throw std::runtime_error("namelist is not a namelist");
		}
		auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(@1);
		parlist->set_namelist(namelist);
		$$ = parlist;
	}
	|
	namelist COMMA VAR_PARAMS
	{
		LOG(INFO) << "[bison]: parlist: " << "namelist COMMA VAR_PARAMS";
		auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($1);
		if (namelist == nullptr) {
			LOG(ERROR) << "[bison]: parlist: " << "namelist is not a namelist";
			throw std::runtime_error("namelist is not a namelist");
		}
		auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(@1);
		parlist->set_namelist(namelist);
		parlist->set_var_params(true);
		$$ = parlist;
	}
	|
	VAR_PARAMS
	{
		LOG(INFO) << "[bison]: parlist: " << "VAR_PARAMS";
		auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(@1);
		parlist->set_var_params(true);
		$$ = parlist;
	}
	;

tableconstructor:
	LCURLY fieldlist RCURLY
	{
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY fieldlist RCURLY";
		auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(@1);
		tableconstructor->set_fieldlist($2);
		$$ = tableconstructor;
	}
	|
	LCURLY RCURLY
	{
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY RCURLY";
		auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(@1);
		tableconstructor->set_fieldlist(std::make_shared<fakelua::syntax_tree_empty>(@1));
		$$ = tableconstructor;
	}
	;

fieldlist:
	field
	{
		LOG(INFO) << "[bison]: fieldlist: " << "field";
		auto fieldlist = std::make_shared<fakelua::syntax_tree_fieldlist>(@1);
		auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($1);
		if (field == nullptr) {
			LOG(ERROR) << "[bison]: fieldlist: " << "field is not a field";
			throw std::runtime_error("field is not a field");
		}
		fieldlist->add_field(field);
		$$ = fieldlist;
	}
	|
	fieldlist fieldsep field
	{
		LOG(INFO) << "[bison]: fieldlist: " << "fieldlist fieldsep field";
		auto fieldlist = std::dynamic_pointer_cast<fakelua::syntax_tree_fieldlist>($1);
		if (fieldlist == nullptr) {
			LOG(ERROR) << "[bison]: fieldlist: " << "fieldlist is not a fieldlist";
			throw std::runtime_error("fieldlist is not a fieldlist");
		}
		auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($3);
		if (field == nullptr) {
			LOG(ERROR) << "[bison]: fieldlist: " << "field is not a field";
			throw std::runtime_error("field is not a field");
		}
		fieldlist->add_field(field);
		$$ = fieldlist;
	}
	;

field:
	LSQUARE exp RSQUARE ASSIGN exp
	{
		LOG(INFO) << "[bison]: field: " << "LSQUARE exp RSQUARE ASSIGN exp";
		auto assignment = std::make_shared<fakelua::syntax_tree_fieldassignment>(@1);
		auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($2);
		if (field == nullptr) {
			LOG(ERROR) << "[bison]: field: " << "field is not a field";
			throw std::runtime_error("field is not a field");
		}
		assignment->set_field(field);
		auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($5);
		if (exp == nullptr) {
			LOG(ERROR) << "[bison]: field: " << "exp is not a exp";
			throw std::runtime_error("exp is not a exp");
		}
		assignment->set_exp(exp);
		$$ = assignment;
	}
	|
	IDENTIFIER ASSIGN exp
	{
		LOG(INFO) << "[bison]: field: " << "IDENTIFIER ASSIGN exp";
		auto assignment = std::make_shared<fakelua::syntax_tree_fieldassignment>(@1);
		auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($3);
		if (exp == nullptr) {
			LOG(ERROR) << "[bison]: field: " << "exp is not a exp";
			throw std::runtime_error("exp is not a exp");
		}
		assignment->set_name($1);
		assignment->set_exp(exp);
		$$ = assignment;
	}
	|
	exp
	{
		LOG(INFO) << "[bison]: field: " << "exp";
		$$ = $1;
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
