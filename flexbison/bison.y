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
		// todo
	}
	;

function_declaration_arguments: 
	/* empty */
	| 
	arg ARG_SPLITTER function_declaration_arguments 
	{
		// todo
	}
	| 
	arg
	{
		// todo
	}
	;

arg : 
	IDENTIFIER
	{
		// todo
	}
	;
	
function_call:
	IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET 
	{
		// todo
	} 
	;
	
function_call_arguments: 
	/* empty */
	| 
	expr ARG_SPLITTER function_call_arguments 
	{
		// todo
	}
	| 
	expr
	{
		// todo
	}
	;  

/* function declaration end */

block:
	stmt block
	|
	stmt 
	;
  
stmt:
	while_stmt
	{
		// todo
	}
	|
	if_stmt
	{
		// todo
	}
	|
	return_stmt
	{
		// todo
	}
	|
	assign_stmt
	{
		// todo
	}
	|
	break
	{
		// todo
	}
	|
	expr
	{
		// todo
	}
	;

while_stmt:
	WHILE cmp THEN block END 
	;
	
if_stmt:
	IF cmp THEN block else_stmt END
	;
	
else_stmt:
	/*empty*/
	|
	ELSE block
	;

cmp:
	cmp_value LESS cmp_value
	|
	cmp_value MORE cmp_value
	|
	cmp_value EQUAL cmp_value
	|
	cmp_value MORE_OR_EQUAL cmp_value
	|
	cmp_value LESS_OR_EQUAL cmp_value
	|
	cmp_value NOT_EQUAL cmp_value
	;

cmp_value:
	explicit_value
	|
	variable
	|
	expr
	;
	
return_stmt:
	RETURN return_value
	;
 
return_value:
	explicit_value
	|
	variable
	|
	expr
	;

assign_stmt:
	var ASSIGN assign_value
	;
	
assign_value:
	explicit_value
	|
	variable
	|
	expr
	;

var:
	VAR_BEGIN IDENTIFIER
	{
		// todo
	}
	|
	variable
	{
		// todo
	}
	;

variable:
	IDENTIFIER
	;

expr:
	OPEN_BRACKET expr CLOSE_BRACKET
	{
		// todo
	}
	|
	function_call
	|
	math_expr
	;

math_expr:
	OPEN_BRACKET math_expr CLOSE_BRACKET
	{
		// todo
	}
	|
	expr_value PLUS expr_value
	{
		// todo
	}
	|
	expr_value MINUS expr_value
	{
		// todo
	}
	|
	expr_value MULTIPLY expr_value
	{
		// todo
	}
	|
	expr_value DIVIDE expr_value
	{
		// todo
	}
	|
	expr_value DIVIDE_MOD expr_value
	{
		// todo
	}
	;	

expr_value:
	math_expr
	{
		// todo
	}
	|
	explicit_value
	|
	function_call
	|
	variable
	;
	
explicit_value:
	TRUE 
	{
		// todo
	}
	|
	FALSE 
	{
		// todo
	}
	|
	NUMBER 
	{
		// todo
	}
	|
	STRING_DEFINITION 
	{
		// todo
	}
	;
      
break:
	BREAK 
	{
		// todo
	}
	;
 