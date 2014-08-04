%{
#include "semantic.h"
#include "types.h"

void yyerror(const char *str)
{
	fprintf(stderr, "Parse error\tline: %d, %s in %s\n", yylineno, str, yytext);
}

%}

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
		fscript::runtime::get_instance()->set_main_block((fscript::block*) $1);
		fscript::runtime::get_instance()->semantic_analys();
		fscript::runtime::get_instance()->interpretate();
	}
	;

body:
	top_level_cmd
	|
	body top_level_cmd 
	{    
		$$ = new fscript::block($1, $2);
	}
	;

/* Avalilable statements in source body*/
top_level_cmd: 
	function_declaration
	;
 
instructions:
	instructions instruction 
	{ 
		$$ = new fscript::block($1, $2);
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
		$$ = (fscript::operat*)($1); 
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
		$$ = new fscript::block($1); 
	};

function_declaration:
	FUNC IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET block END
	{
		fscript::function_declaration* fd = new fscript::function_declaration($2, $4, $6);
		$$ = fd;
		fscript::runtime::get_instance()->add_function_declaration(fd);
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
		$$ = fscript::ExpressionsList($3);
		$$.push_back($1);
	}
	| 
	var {  
		$$ = fscript::ExpressionsList();
		$$.push_back($1);
	}
	;
  
function_call_arguments: /* empty */
	| 
	value ARG_SPLITTER function_call_arguments 
	{
		$$ = fscript::ExpressionsList($3);
		$$.push_back($1);
	}
	| 
	value
	{  
		$$ = fscript::ExpressionsList();
		$$.push_back($1);
	}
	;  

value:
	expresion { $$ = $1; }
	;
	
/* End of top level rules */

explicit_value:
  TRUE { $$ = new fscript::value($1); }
  |
  FALSE { $$ = new fscript::value($1); }
  |
  NUMBER { $$ = new fscript::value($1); }
  |
  STRING_DEFINITION { $$ = new fscript::value($1); }
  |
  var 
  |
  function_call
  ;
      
break:
	BREAK { $$ = new fscript::break_op(); }
	;
  
var:
  VAR_BEGIN IDENTIFIER { $$ = new fscript::var($2); }

function_call:
	IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET  
	{
		$$ = new fscript::function_call($1, $3);
	} 
	;

assign_value:
	var ASSIGN value { $$ = new fscript::assign($1, $3); }
	;
 
return_value:
	RETURN value { $$ = new fscript::return_op($2); };

loop_while:
	WHILE value THEN block END {
		$$ = new fscript::while_op($2, $4); 
	};

if_stmt:
	IF value THEN block else_stmt { $$ = new fscript::if_op($2, $4, $5); };

else_stmt:
	/* empty */
	{ $$ = 0x0; }
	|
	ELSE block { $$ = new fscript::block($2); };
  
expresion:
	math_expr { $$ = $1; }
	|
	expresion LESS math_expr { $$ = new fscript::binary("<", $1, $3); }
	|
	expresion MORE math_expr { $$ = new fscript::binary(">", $1, $3); }
	|
	expresion EQUAL math_expr { $$ = new fscript::binary("==", $1, $3); }
	|
	expresion MORE_OR_EQUAL math_expr { $$ = new fscript::binary(">=", $1, $3); }
	|
	expresion LESS_OR_EQUAL math_expr { $$ = new fscript::binary("<=", $1, $3); }
	|
	expresion NOT_EQUAL math_expr { $$ = new fscript::binary("!=", $1, $3); }
	|
	error  { std::cerr << "Expresion error\n"; }
	;
 
/* Math operation implementation with priority */
math_expr: 
	math_hight_expr 
	|
	math_expr PLUS math_hight_expr { $$ = new fscript::binary("+", $1, $3); }
	|
	math_expr MINUS math_hight_expr { $$ = new fscript::binary("-", $1, $3); }
	;
  
math_hight_expr:
	string_expr
	|
	math_hight_expr MULTIPLY string_expr { $$ = new fscript::binary("*", $1, $3); }
	|
	math_hight_expr DIVIDE string_expr { $$ = new fscript::binary("/", $1, $3); }
	|
	math_hight_expr DIVIDE_MOD string_expr { $$ = new fscript::binary("%", $1, $3); }
	;

string_expr:	
	explicit_value
	|
	string_expr STRING_CONCETATE explicit_value { $$ = new fscript::binary(".", $1, $3); }
	;
  