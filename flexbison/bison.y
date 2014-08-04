%{
#include "types.h"

void yyerror(const char *str)
{
	fprintf(stderr, "Parse error\tline: %d, %s in %s\n", yylineno, str, yytext);
}

%}

%token FUNCTION VAR_BEGIN  ARRAY_DECLARATION
%token IDENTIFIER NUMBER STRING_DEFINITION 
%token END_INSTRUCTION ARG_SPLITTER 
%token PLUS MINUS DIVIDE MULTIPLY DIVIDE_MOD
%token ASSIGN
%token MORE LESS MORE_OR_EQUAL LESS_OR_EQUAL EQUAL NOT_EQUAL
%token OPEN_BLOCK CLOSE_BLOCK OPEN_BRACKET CLOSE_BRACKET STRING_CONCETATE RETURN INCLUDE REQUIRE
%token OPEN_SQUARE_BRACKET CLOSE_SQUARE_BRACKET
%token IF ELSE
%token TRUE FALSE
%token INC DEC
%token WHILE FOR
%token BREAK
%token UNSET

%right PLUS
%right MINUS
%right DEVIDE
%right MULTIPLY
%right STRING_CONCETATE

%expect 11

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
%type<str> DEC INC
%type<str> ASSIGN

%type<oper> return_value
%type<oper> break
%type<oper> require
%type<oper> include
%type<oper> loop_for
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
%type<oper> unset

%type<expr> value
%type<expr> string_expr
%type<expr> math_hight_expr
%type<expr> math_expr
%type<expr> expresion
%type<expr> function_call
%type<expr> array
%type<expr> unar_op
%type<expr> explicit_value
%type<expr> var

%type<args> function_call_arguments
%type<args> function_declaration_arguments
%%

/* Top level rules */
program: body
  {
   runtime_t::get_instance()->set_main_block((block_t*) $1);
   runtime_t::get_instance()->semantic_analys();
   runtime_t::get_instance()->interpretate();
  }
  ;

body:
  top_level_cmd
  |
  body top_level_cmd {    
      $$ = new block_t($1, $2);
  }
  ;

/* Avalilable statements in source body*/
top_level_cmd: 
  function_declaration
  |
  include END_INSTRUCTION
  |
  require END_INSTRUCTION
  |
  instructions { $$ = $1; }
  ;
 
instructions:
  instructions instruction { $$ = new block_t($1, $2); }
  |
  instruction { }
  ;
  
instruction:
   command END_INSTRUCTION { $$ = $1; }
   |
   condition_statement { $$ = $1; }
   ;
	
command:
  unar_op {
    $$ =  (oper_t*)($1); 
  }
  |
  function_call {
    /* Function call is expr_t and oper_t child */
      $$ =  (oper_t*)($1); 
    }
  | 
  assign_value
  |
  return_value
  |
  unset
  |
  break
  |
  error { yyerrok; yyclearin;  std::cerr <<"Error in command running statement\n"; }
  ;

condition_statement:
  loop_for
  |
  loop_while
  |
  if_stmt
  ;

block: 
  OPEN_BLOCK instructions CLOSE_BLOCK { $$ = new block_t($2); };

function_declaration:
  FUNCTION IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET block
  {
    function_declaration_t* fd = new function_declaration_t($2, $4, $6);
    $$ = fd;
    runtime_t::get_instance()->add_function_declaration(fd);
  }
  |
  error block{
    yyerrok;
    yyclearin;
    std::cerr << "Function declaration error!\n";
  };

function_declaration_arguments: /* empty */
  | 
  var ARG_SPLITTER function_declaration_arguments {
    $$ = std::list<expr_t*>($3);
    $$.push_back($1);
  }
  | 
  var {  
     $$ = std::list<expr_t*>();
     $$.push_back($1);
  }
  ;
  
function_call_arguments: /* empty */
  | 
  value ARG_SPLITTER function_call_arguments {
    $$ = std::list<expr_t*>($3);
    $$.push_back($1);
  }
  | value
  {  
     $$ = std::list<expr_t*>();
     $$.push_back($1);
  }
  ;  

value:
  expresion { $$ = $1; }
  ;
/* End of top level rules */

explicit_value:
  TRUE { $$ = new value_t($1); }
  |
  FALSE { $$ = new value_t($1); }
  |
  NUMBER { $$ = new value_t($1); }
  |
  STRING_DEFINITION { $$ = new value_t($1); }
  |
  var 
  |
  function_call
  ;
      
break:
  BREAK { $$ = new break_op_t(); }
  ;
  
unar_op:
  var INC { $$ = new unary_t("++", $1); }
  |
  var DEC { $$ = new unary_t("--", $1); }
  |
  DEC var { $$ = new unary_t("--", $2); }
  |
  INC var { $$ = new unary_t("++", $2); };

array:
  ARRAY_DECLARATION { $$ = new array_init_t(); }
  ;

var:
  VAR_BEGIN IDENTIFIER { $$ = new var_t($2); }
  |
  VAR_BEGIN IDENTIFIER OPEN_SQUARE_BRACKET value CLOSE_SQUARE_BRACKET { 
    $$ = new array_t($2, $4); 
  };
  
unset: 
  UNSET var { $$ = new unset_t($2); };
  
function_call:
  IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET  {
    $$ = new function_call_t($1, $3);
  } ;

assign_value:
  var ASSIGN value { $$ = new assign_t($1, $3); }
  | 
  var ASSIGN array { $$ = new assign_t($1, $3); }
  ;
 
return_value:
  RETURN value { $$ = new return_op_t($2); };

loop_for:
  FOR OPEN_BRACKET command END_INSTRUCTION expresion END_INSTRUCTION command CLOSE_BRACKET block
  {
    $$ = new for_op_t($3, $5, $7, $9);
  };

loop_while:
  WHILE OPEN_BRACKET value CLOSE_BRACKET block {
    $$ = new while_op_t($3, $5); 
  };

if_stmt:
  IF OPEN_BRACKET value CLOSE_BRACKET block else_stmt { $$ = new if_op_t($3, $5, $6); };

else_stmt:
  /* empty */
  { $$ = 0x0; }
  |
  ELSE block { $$ = new block_t($2); };
  
expresion:
  math_expr { $$ = $1; }
  |
  expresion LESS math_expr { $$ = new binary_t("<", $1, $3); }
  |
  expresion MORE math_expr { $$ = new binary_t(">", $1, $3); }
  |
  expresion EQUAL math_expr { $$ = new binary_t("==", $1, $3); }
  |
  expresion MORE_OR_EQUAL math_expr { $$ = new binary_t(">=", $1, $3); }
  |
  expresion LESS_OR_EQUAL math_expr { $$ = new binary_t("<=", $1, $3); }
  |
  expresion NOT_EQUAL math_expr { $$ = new binary_t("!=", $1, $3); }
  |
  OPEN_BRACKET expresion CLOSE_BRACKET { $$ = $2; }
  |
  error  { std::cerr << "Expresion error\n"; }
  ;
 
/* Math operation implementation with priority */
math_expr: 
  math_hight_expr 
  |
  math_expr PLUS math_hight_expr { $$ = new binary_t("+", $1, $3); }
  |
  math_expr MINUS math_hight_expr { $$ = new binary_t("-", $1, $3); }
  ;
  
math_hight_expr:
  string_expr
  |
  math_hight_expr MULTIPLY string_expr { $$ = new binary_t("*", $1, $3); }
  |
  math_hight_expr DIVIDE string_expr { $$ = new binary_t("/", $1, $3); }
  |
  math_hight_expr DIVIDE_MOD string_expr { $$ = new binary_t("%", $1, $3); }
  ;

string_expr:	
  explicit_value
  |
  string_expr STRING_CONCETATE explicit_value { $$ = new binary_t(".", $1, $3); }
  ;
  
include:
  INCLUDE value	{ $$ = new include_t($2); } ;   
  
require:
  REQUIRE value	{ $$ = new require_t($2); };
