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

#define NEWTYPE(p, x) \
	x* p = (x*)(((myflexer *)parm)->malloc(sizeof(x))); \
	new (p) x();				  

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

%type<syntree> break
%type<syntree> function_declaration
%type<syntree> block
%type<syntree> stmt
%type<syntree> body

%type<syntree> expr
%type<syntree> math_expr
%type<syntree> function_call
%type<syntree> explicit_value
%type<syntree> var
%type<syntree> arg

%type<syntree> function_call_arguments
%type<syntree> function_declaration_arguments
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
		FKLOG("function_declaration <- block %s", $2.c_str());
		NEWTYPE(p, func_desc_node);
		p->funcname = $2;
		p->arglist = dynamic_cast<func_desc_arglist_node*>($4);
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	}
	|
	FUNC IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET END
	{
		FKLOG("function_declaration <- empty %s", $2.c_str());
		NEWTYPE(p, func_desc_node);
		p->funcname = $2;
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	}
	;

function_declaration_arguments: 
	/* empty */
	| 
	arg ARG_SPLITTER function_declaration_arguments 
	{
		FKLOG("function_declaration_arguments <- arg function_declaration_arguments");
		assert($3->gettype() == est_arglist);
		func_desc_arglist_node * p = dynamic_cast<func_desc_arglist_node*>($3);
		p->add_arg($1);
		$$ = p;
	}
	| 
	arg
	{
		FKLOG("function_declaration_arguments <- arg");
		NEWTYPE(p, func_desc_arglist_node);
		p->add_arg($1);
		$$ = p;
	}
	;

arg : 
	IDENTIFIER
	{
		FKLOG("arg <- IDENTIFIER %s", $1.c_str());
		NEWTYPE(p, identifier_node);
		p->str = $1;
		$$ = p;
	}
	;
	
function_call:
	IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET 
	{
		FKLOG("function_call <- function_call_arguments %s", $1.c_str());
		// todo
	} 
	;
	
function_call_arguments: 
	/* empty */
	| 
	arg_expr ARG_SPLITTER function_call_arguments 
	{
		FKLOG("function_call_arguments <- arg_expr function_call_arguments");
		// todo
	}
	| 
	arg_expr
	{
		FKLOG("function_call_arguments <- arg_expr");
		// todo
	}
	;  

arg_expr:
	expr
	{
		FKLOG("arg_expr <- expr");
		// todo
	}
	|
	variable
	{
		FKLOG("arg_expr <- variable");
		// todo
	}
	;

/* function declaration end */

block:
	block stmt 
	{
		FKLOG("block <- block stmt");
		// todo
	}
	|
	stmt 
	{
		FKLOG("block <- stmt");
		// todo
	}
	;
  
stmt:
	while_stmt
	{
		FKLOG("stmt <- while_stmt");
		// todo
	}
	|
	if_stmt
	{
		FKLOG("stmt <- if_stmt");
		// todo
	}
	|
	return_stmt
	{
		FKLOG("stmt <- return_stmt");
		// todo
	}
	|
	assign_stmt
	{
		FKLOG("stmt <- assign_stmt");
		// todo
	}
	|
	break
	{
		FKLOG("stmt <- break");
		// todo
	}
	|
	expr
	{
		FKLOG("stmt <- expr");
		// todo
	}
	;

while_stmt:
	WHILE cmp THEN block END 
	{
		FKLOG("while_stmt <- cmp block");
		// todo
	}
	|
	WHILE cmp THEN END 
	{
		FKLOG("while_stmt <- cmp");
		// todo
	}
	;
	
if_stmt:
	IF cmp THEN block else_stmt END
	{
		FKLOG("if_stmt <- cmp block");
		// todo
	}
	|
	IF cmp THEN else_stmt END
	{
		FKLOG("if_stmt <- cmp");
		// todo
	}
	;
	
else_stmt:
	/*empty*/
	|
	ELSE block
	{
		FKLOG("else_stmt <- block");
		// todo
	}
	|
	ELSE
	{
		FKLOG("else_stmt <- empty");
		// todo
	}
	;

cmp:
	cmp_value LESS cmp_value
	{
		FKLOG("cmp <- cmp_value LESS cmp_value");
		// todo
	}
	|
	cmp_value MORE cmp_value
	{
		FKLOG("cmp <- cmp_value MORE cmp_value");
		// todo
	}
	|
	cmp_value EQUAL cmp_value
	{
		FKLOG("cmp <- cmp_value EQUAL cmp_value");
		// todo
	}
	|
	cmp_value MORE_OR_EQUAL cmp_value
	{
		FKLOG("cmp <- cmp_value MORE_OR_EQUAL cmp_value");
		// todo
	}
	|
	cmp_value LESS_OR_EQUAL cmp_value
	{
		FKLOG("cmp <- cmp_value LESS_OR_EQUAL cmp_value");
		// todo
	}
	|
	cmp_value NOT_EQUAL cmp_value
	{
		FKLOG("cmp <- cmp_value NOT_EQUAL cmp_value");
		// todo
	}
	;

cmp_value:
	explicit_value
	{
		FKLOG("cmp_value <- explicit_value");
		// todo
	}
	|
	variable
	{
		FKLOG("cmp_value <- variable");
		// todo
	}
	|
	expr
	{
		FKLOG("cmp_value <- expr");
		// todo
	}
	;
	
return_stmt:
	RETURN return_value
	{
		FKLOG("return_stmt <- RETURN return_value");
		// todo
	}
	;
 
return_value:
	explicit_value
	{
		FKLOG("return_value <- explicit_value");
		// todo
	}
	|
	variable
	{
		FKLOG("return_value <- variable");
		// todo
	}
	|
	expr
	{
		FKLOG("return_value <- expr");
		// todo
	}
	;

assign_stmt:
	var ASSIGN assign_value
	{
		FKLOG("assign_stmt <- var assign_value");
		// todo
	}
	;
	
assign_value:
	explicit_value
	{
		FKLOG("assign_value <- explicit_value");
		// todo
	}
	|
	variable
	{
		FKLOG("assign_value <- variable");
		// todo
	}
	|
	expr
	{
		FKLOG("assign_value <- expr");
		// todo
	}
	;

var:
	VAR_BEGIN IDENTIFIER
	{
		FKLOG("var <- VAR_BEGIN IDENTIFIER %s", $2.c_str());
		// todo
	}
	|
	variable
	{
		FKLOG("var <- variable");
		// todo
	}
	;

variable:
	IDENTIFIER
	{
		FKLOG("variable <- IDENTIFIER %s", $1.c_str());
		// todo
	}
	;

expr:
	OPEN_BRACKET expr CLOSE_BRACKET
	{
		FKLOG("expr <- (expr)");
		// todo
	}
	|
	function_call
	{
		FKLOG("expr <- function_call");
		// todo
	}
	|
	math_expr
	{
		FKLOG("expr <- math_expr");
		// todo
	}
	;

math_expr:
	OPEN_BRACKET math_expr CLOSE_BRACKET
	{
		FKLOG("math_expr <- (math_expr)");
		// todo
	}
	|
	expr_value PLUS expr_value
	{
		FKLOG("math_expr <- expr_value + expr_value");
		// todo
	}
	|
	expr_value MINUS expr_value
	{
		FKLOG("math_expr <- expr_value - expr_value");
		// todo
	}
	|
	expr_value MULTIPLY expr_value
	{
		FKLOG("math_expr <- expr_value * expr_value");
		// todo
	}
	|
	expr_value DIVIDE expr_value
	{
		FKLOG("math_expr <- expr_value / expr_value");
		// todo
	}
	|
	expr_value DIVIDE_MOD expr_value
	{
		FKLOG("math_expr <- expr_value % expr_value");
		// todo
	}
	;	

expr_value:
	math_expr
	{
		FKLOG("expr_value <- math_expr");
		// todo
	}
	|
	explicit_value
	{
		FKLOG("expr_value <- explicit_value");
		// todo
	}
	|
	function_call
	{
		FKLOG("expr_value <- function_call");
		// todo
	}
	|
	variable
	{
		FKLOG("expr_value <- variable");
		// todo
	}
	;
	
explicit_value:
	TRUE 
	{
		FKLOG("explicit_value <- TRUE");
		// todo
	}
	|
	FALSE 
	{
		FKLOG("explicit_value <- FALSE");
		// todo
	}
	|
	NUMBER 
	{
		FKLOG("explicit_value <- NUMBER %s", $1.c_str());
		// todo
	}
	|
	STRING_DEFINITION 
	{
		FKLOG("explicit_value <- STRING_DEFINITION %s", $1.c_str());
		// todo
	}
	;
      
break:
	BREAK 
	{
		FKLOG("break <- BREAK");
		// todo
	}
	;
 