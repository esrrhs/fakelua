%{
#include "semantic.h"
#include "types.h"
#include "fuck.h"

#define YYPARSE_PARAM   parm
#define YYLEX_PARAM     parm

#define yyerror(msg) my_yyerror(msg, YYPARSE_PARAM)

int yylex(YYSTYPE *lvalp, void * parm)
{
	myflexer *l = (myflexer *)parm;
	int ret = l->yylex(lvalp);
	FKLOG("bison get token[%s] str[%s]", fkget_token_name(ret).c_str(), lvalp->str.c_str());
	return ret;
}

int my_yyerror(const char *s, void * parm)
{
    myflexer *l = (myflexer *)parm;
    l->LexerError(s);
    return 1;
}

%}

%pure_parser

%token VAR_BEGIN
%token RETURN
%token BREAK
%token FUNC
%token WHILE
%token TRUE
%token FALSE
%token IF
%token THEN
%token ELSE
%token END
%token STRING_DEFINITION
%token IDENTIFIER
%token NUMBER
%token SINGLE_LINE_COMMENT
%token DIVIDE_MOD
%token ARG_SPLITTER
%token PLUS
%token MINUS
%token DIVIDE
%token MULTIPLY
%token ASSIGN
%token MORE
%token LESS
%token MORE_OR_EQUAL
%token LESS_OR_EQUAL
%token EQUAL
%token NOT_EQUAL
%token OPEN_BRACKET
%token CLOSE_BRACKET

%right PLUS
%right MINUS
%right DIVIDE
%right MULTIPLY

%expect 11

%type<str> IDENTIFIER  
%type<str> NUMBER
%type<str> STRING_DEFINITION
%type<str> PLUS
%type<str> MINUS
%type<str> DIVIDE
%type<str> MULTIPLY
%type<str> DIVIDE_MOD
%type<str> MORE LESS MORE_OR_EQUAL LESS_OR_EQUAL EQUAL NOT_EQUAL
%type<str> TRUE FALSE
%type<str> ASSIGN

%type<oper> break
%type<oper> function_declaration
%type<oper> block
%type<oper> stmt
%type<oper> body

%type<expr> expr
%type<expr> math_expr
%type<expr> function_call
%type<expr> explicit_value
%type<expr> var
%type<expr> arg

%type<args> function_call_arguments
%type<args> function_declaration_arguments
%%

/* Top level rules */
program: body
	;

body:
	function_declaration
	|
	body function_declaration
	;

/* function declaration begin */

function_declaration:
	FUNC IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET block END
	{
		FKLOG("function_declaration %s", $2.c_str());
		// todo
	}
	|
	FUNC IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET END
	{
		FKLOG("function_declaration %s", $2.c_str());
		// todo
	}
	;

function_declaration_arguments: 
	/* empty */
	| 
	arg ARG_SPLITTER function_declaration_arguments 
	{
		FKLOG("function_declaration_arguments");
		// todo
	}
	| 
	arg
	{
		FKLOG("function_declaration_arguments arg");
		// todo
	}
	;

arg : 
	IDENTIFIER
	{
		FKLOG("arg %s", $1.c_str());
		// todo
	}
	;
	
function_call:
	IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET 
	{
		FKLOG("function_call %s", $1.c_str());
		// todo
	} 
	;
	
function_call_arguments: 
	/* empty */
	| 
	arg_expr ARG_SPLITTER function_call_arguments 
	{
		FKLOG("function_call_arguments");
		// todo
	}
	| 
	arg_expr
	{
		FKLOG("function_call_arguments arg_expr");
		// todo
	}
	;  

arg_expr:
	expr
	{
		FKLOG("arg_expr expr");
		// todo
	}
	|
	variable
	{
		FKLOG("arg_expr variable");
		// todo
	}
	;

/* function declaration end */

block:
	block stmt 
	{
		FKLOG("block");
		// todo
	}
	|
	stmt 
	{
		FKLOG("block stmt");
		// todo
	}
	;
  
stmt:
	while_stmt
	{
		FKLOG("while_stmt");
		// todo
	}
	|
	if_stmt
	{
		FKLOG("if_stmt");
		// todo
	}
	|
	return_stmt
	{
		FKLOG("return_stmt");
		// todo
	}
	|
	assign_stmt
	{
		FKLOG("assign_stmt");
		// todo
	}
	|
	break
	{
		FKLOG("break");
		// todo
	}
	|
	expr
	{
		FKLOG("expr");
		// todo
	}
	;

while_stmt:
	WHILE cmp THEN block END 
	{
		FKLOG("while_stmt cmp block");
		// todo
	}
	|
	WHILE cmp THEN END 
	{
		FKLOG("while_stmt cmp");
		// todo
	}
	;
	
if_stmt:
	IF cmp THEN block else_stmt END
	{
		FKLOG("if_stmt cmp block");
		// todo
	}
	|
	IF cmp THEN else_stmt END
	{
		FKLOG("if_stmt cmp");
		// todo
	}
	;
	
else_stmt:
	/*empty*/
	|
	ELSE block
	{
		FKLOG("else_stmt block");
		// todo
	}
	|
	ELSE
	{
		FKLOG("else_stmt");
		// todo
	}
	;

cmp:
	cmp_value LESS cmp_value
	{
		FKLOG("cmp_value LESS cmp_value");
		// todo
	}
	|
	cmp_value MORE cmp_value
	{
		FKLOG("cmp_value MORE cmp_value");
		// todo
	}
	|
	cmp_value EQUAL cmp_value
	{
		FKLOG("cmp_value EQUAL cmp_value");
		// todo
	}
	|
	cmp_value MORE_OR_EQUAL cmp_value
	{
		FKLOG("cmp_value MORE_OR_EQUAL cmp_value");
		// todo
	}
	|
	cmp_value LESS_OR_EQUAL cmp_value
	{
		FKLOG("cmp_value LESS_OR_EQUAL cmp_value");
		// todo
	}
	|
	cmp_value NOT_EQUAL cmp_value
	{
		FKLOG("cmp_value NOT_EQUAL cmp_value");
		// todo
	}
	;

cmp_value:
	explicit_value
	{
		FKLOG("cmp_value explicit_value");
		// todo
	}
	|
	variable
	{
		FKLOG("cmp_value variable");
		// todo
	}
	|
	expr
	{
		FKLOG("cmp_value expr");
		// todo
	}
	;
	
return_stmt:
	RETURN return_value
	{
		FKLOG("return_stmt");
		// todo
	}
	;
 
return_value:
	explicit_value
	{
		FKLOG("return_value explicit_value");
		// todo
	}
	|
	variable
	{
		FKLOG("return_value variable");
		// todo
	}
	|
	expr
	{
		FKLOG("return_value expr");
		// todo
	}
	;

assign_stmt:
	var ASSIGN assign_value
	{
		FKLOG("assign_stmt");
		// todo
	}
	;
	
assign_value:
	explicit_value
	{
		FKLOG("assign_value explicit_value");
		// todo
	}
	|
	variable
	{
		FKLOG("assign_value variable");
		// todo
	}
	|
	expr
	{
		FKLOG("assign_value expr");
		// todo
	}
	;

var:
	VAR_BEGIN IDENTIFIER
	{
		FKLOG("var %s", $2.c_str());
		// todo
	}
	|
	variable
	{
		FKLOG("var variable");
		// todo
	}
	;

variable:
	IDENTIFIER
	{
		FKLOG("variable %s", $1.c_str());
		// todo
	}
	;

expr:
	OPEN_BRACKET expr CLOSE_BRACKET
	{
		FKLOG("expr expr");
		// todo
	}
	|
	function_call
	{
		FKLOG("expr function_call");
		// todo
	}
	|
	math_expr
	{
		FKLOG("expr math_expr");
		// todo
	}
	;

math_expr:
	OPEN_BRACKET math_expr CLOSE_BRACKET
	{
		FKLOG("math_expr math_expr");
		// todo
	}
	|
	expr_value PLUS expr_value
	{
		FKLOG("math_expr expr_value PLUS expr_value");
		// todo
	}
	|
	expr_value MINUS expr_value
	{
		FKLOG("math_expr expr_value MINUS expr_value");
		// todo
	}
	|
	expr_value MULTIPLY expr_value
	{
		FKLOG("math_expr expr_value MULTIPLY expr_value");
		// todo
	}
	|
	expr_value DIVIDE expr_value
	{
		FKLOG("math_expr expr_value DIVIDE expr_value");
		// todo
	}
	|
	expr_value DIVIDE_MOD expr_value
	{
		FKLOG("math_expr expr_value DIVIDE_MOD expr_value");
		// todo
	}
	;	

expr_value:
	math_expr
	{
		FKLOG("expr_value math_expr");
		// todo
	}
	|
	explicit_value
	{
		FKLOG("expr_value explicit_value");
		// todo
	}
	|
	function_call
	{
		FKLOG("expr_value function_call");
		// todo
	}
	|
	variable
	{
		FKLOG("expr_value variable");
		// todo
	}
	;
	
explicit_value:
	TRUE 
	{
		FKLOG("explicit_value TRUE");
		// todo
	}
	|
	FALSE 
	{
		FKLOG("explicit_value FALSE");
		// todo
	}
	|
	NUMBER 
	{
		FKLOG("explicit_value NUMBER");
		// todo
	}
	|
	STRING_DEFINITION 
	{
		FKLOG("explicit_value STRING_DEFINITION");
		// todo
	}
	;
      
break:
	BREAK 
	{
		FKLOG("break BREAK");
		// todo
	}
	;
 