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
	return l->yylex(lvalp);
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
%token STRING_CONCETATE

%right PLUS
%right MINUS
%right DIVIDE
%right MULTIPLY
%right STRING_CONCETATE

%expect 8

%type<str> IDENTIFIER  
%type<str> NUMBER
%type<str> STRING_DEFINITION
%type<str> STRING_CONCETATE
%type<str> PLUS
%type<str> MINUS
%type<str> DIVIDE
%type<str> MULTIPLY
%type<str> DIVIDE_MOD
%type<str> MORE LESS MORE_OR_EQUAL LESS_OR_EQUAL EQUAL NOT_EQUAL
%type<str> TRUE FALSE
%type<str> ASSIGN

%type<oper> return_value
%type<oper> break
%type<oper> loop_while
%type<oper> if_stmt
%type<oper> else_stmt
%type<oper> function_declaration
%type<oper> block
%type<oper> condition_statement
%type<oper> command
%type<oper> instructions
%type<oper> instruction
%type<oper> top_level_cmd
%type<oper> body
%type<oper> assign_value

%type<expr> value
%type<expr> string_expr
%type<expr> math_hight_expr
%type<expr> math_expr
%type<expr> expresion
%type<expr> function_call
%type<expr> explicit_value
%type<expr> var

%type<args> function_call_arguments
%type<args> function_declaration_arguments
%%

/* Top level rules */
program: body
	{
		fuck * pfuck = (fuck *)(YYPARSE_PARAM);
		pfuck->set_main_block((block*) $1);
		pfuck->semantic_analys();
		pfuck->interpretate();
	}
	;

body:
	top_level_cmd
	|
	body top_level_cmd 
	{    
		$$ = new block($1, $2);
	}
	;

/* Avalilable statements in source body*/
top_level_cmd: 
	function_declaration
	;
 
instructions:
	instructions instruction 
	{ 
		$$ = new block($1, $2);
	}
	|
	instruction { }
	;
  
instruction:
	command { $$ = $1; }
	|
	condition_statement { $$ = $1; }
	;
	
command:
	function_call 
	{
		/* Function call is expr_t and operat child */
		$$ = (operat*)($1); 
	}
	| 
	assign_value
	|
	return_value
	|
	break
	|
	error { yyerrok; yyclearin;  std::cerr <<"Error in command running statement\n"; }
	;

condition_statement:
	loop_while
	|
	if_stmt
	;

block: 
	instructions 
	{ 
		$$ = new block($1); 
	};

function_declaration:
	FUNC IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET block END
	{
		fuck * pfuck = (fuck *)(YYPARSE_PARAM);
		function_declaration* fd = new function_declaration($2, $4, $6);
		$$ = fd;
		pfuck->add_function_declaration(fd);
	}
	|
	error block
	{
		yyerrok;
		yyclearin;
		std::cerr << "Function declaration error!\n";
	};

function_declaration_arguments: /* empty */
	| 
	var ARG_SPLITTER function_declaration_arguments 
	{
		$$ = ExpressionsList($3);
		$$.push_back($1);
	}
	| 
	var {  
		$$ = ExpressionsList();
		$$.push_back($1);
	}
	;
  
function_call_arguments: /* empty */
	| 
	value ARG_SPLITTER function_call_arguments 
	{
		$$ = ExpressionsList($3);
		$$.push_back($1);
	}
	| 
	value
	{  
		$$ = ExpressionsList();
		$$.push_back($1);
	}
	;  

value:
	expresion { $$ = $1; }
	;
	
/* End of top level rules */

explicit_value:
  TRUE { $$ = new value($1); }
  |
  FALSE { $$ = new value($1); }
  |
  NUMBER { $$ = new value($1); }
  |
  STRING_DEFINITION { $$ = new value($1); }
  |
  var 
  |
  function_call
  ;
      
break:
	BREAK { $$ = new break_op(); }
	;
  
var:
  VAR_BEGIN IDENTIFIER { $$ = new var($2); }

function_call:
	IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET  
	{
		$$ = new function_call($1, $3);
	} 
	;

assign_value:
	var ASSIGN value { $$ = new assign($1, $3); }
	;
 
return_value:
	RETURN value { $$ = new return_op($2); };

loop_while:
	WHILE value THEN block END {
		$$ = new while_op($2, $4); 
	};

if_stmt:
	IF value THEN block else_stmt { $$ = new if_op($2, $4, $5); };

else_stmt:
	/* empty */
	{ $$ = 0x0; }
	|
	ELSE block { $$ = new block($2); };
  
expresion:
	math_expr { $$ = $1; }
	|
	expresion LESS math_expr { $$ = new binary("<", $1, $3); }
	|
	expresion MORE math_expr { $$ = new binary(">", $1, $3); }
	|
	expresion EQUAL math_expr { $$ = new binary("==", $1, $3); }
	|
	expresion MORE_OR_EQUAL math_expr { $$ = new binary(">=", $1, $3); }
	|
	expresion LESS_OR_EQUAL math_expr { $$ = new binary("<=", $1, $3); }
	|
	expresion NOT_EQUAL math_expr { $$ = new binary("!=", $1, $3); }
	|
	error  { std::cerr << "Expresion error\n"; }
	;
 
/* Math operation implementation with priority */
math_expr: 
	math_hight_expr 
	|
	math_expr PLUS math_hight_expr { $$ = new binary("+", $1, $3); }
	|
	math_expr MINUS math_hight_expr { $$ = new binary("-", $1, $3); }
	;
  
math_hight_expr:
	string_expr
	|
	math_hight_expr MULTIPLY string_expr { $$ = new binary("*", $1, $3); }
	|
	math_hight_expr DIVIDE string_expr { $$ = new binary("/", $1, $3); }
	|
	math_hight_expr DIVIDE_MOD string_expr { $$ = new binary("%", $1, $3); }
	;

string_expr:	
	explicit_value
	|
	string_expr STRING_CONCETATE explicit_value { $$ = new binary(".", $1, $3); }
	;
  