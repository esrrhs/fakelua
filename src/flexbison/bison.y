%{
#include "semantic.h"
#include "types.h"
#include "fake.h"

#define YYPARSE_PARAM   parm
#define YYLEX_PARAM     parm

#define yyerror(msg) my_yyerror(msg, YYPARSE_PARAM)

int yylex(YYSTYPE *lvalp, YYLTYPE * loc, void * parm)
{
	myflexer *l = (myflexer *)parm;
#ifdef __APPLE__
	int ret = l->yylexex(lvalp, loc);
#else
	int ret = l->yylex(lvalp, loc);
#endif
	FKLOG("[bison]: bison get token[%s] str[%s] line[%d,%d]", fkget_token_name(ret).c_str(), lvalp->str.c_str(), loc->first_line, loc->last_line);
	return ret;
}

int my_yyerror(const char *s, void * parm)
{
    myflexer *l = (myflexer *)parm;
    l->LexerError(s);
    return 1;
}

#define NEWTYPE(p, x) \
	x* p = (x*)(((myflexer *)parm)->malloc(sizeof(x), #x)); \
	new (p) x(); \
	p->fk = ((myflexer *)parm)->getfake(); \
	p->lno = yylsp->first_line; \
	FKLOG("[bison]: bison new type %s %p line %d %d %d", #x, p, ((myflexer *)parm)->lineno(), yylloc.first_line, yylsp->first_line);
	
%}

%pure_parser

%error_verbose

%locations
%token VAR_BEGIN
%token RETURN
%token BREAK
%token FUNC
%token WHILE
%token FTRUE
%token FFALSE
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
%token AND
%token OR
%token FKFLOAT
%token PLUS_ASSIGN MINUS_ASSIGN DIVIDE_ASSIGN MULTIPLY_ASSIGN DIVIDE_MOD_ASSIGN
%token COLON
%token FOR
%token INC
%token FAKE
%token FKUUID
%token OPEN_SQUARE_BRACKET
%token CLOSE_SQUARE_BRACKET
%token FCONST
%token PACKAGE
%token INCLUDE
%token IDENTIFIER_DOT
%token IDENTIFIER_POINTER
%token STRUCT
%token IS NOT CONTINUE
%token YIELD SLEEP
%token SWITCH CASE DEFAULT
%token NEW_ASSIGN
%token ELSEIF
%token RIGHT_POINTER
%token STRING_CAT
%token OPEN_BIG_BRACKET
%token CLOSE_BIG_BRACKET
%token FNULL

%left PLUS MINUS
%left DIVIDE MULTIPLY DIVIDE_MOD
%left STRING_CAT

%expect 56

%type<str> IDENTIFIER  
%type<str> NUMBER
%type<str> FKUUID
%type<str> STRING_DEFINITION
%type<str> PLUS
%type<str> MINUS
%type<str> DIVIDE
%type<str> MULTIPLY
%type<str> DIVIDE_MOD
%type<str> STRING_CAT
%type<str> MORE LESS MORE_OR_EQUAL LESS_OR_EQUAL EQUAL NOT_EQUAL
%type<str> FTRUE FFALSE
%type<str> ASSIGN
%type<str> AND OR
%type<str> FKFLOAT FNULL
%type<str> PLUS_ASSIGN MINUS_ASSIGN DIVIDE_ASSIGN MULTIPLY_ASSIGN DIVIDE_MOD_ASSIGN
%type<str> IDENTIFIER_DOT  
%type<str> IDENTIFIER_POINTER  
%type<str> IS NOT CONTINUE

%type<syntree> break continue
%type<syntree> function_declaration
%type<syntree> block
%type<syntree> stmt
%type<syntree> while_stmt
%type<syntree> else_stmt
%type<syntree> elseif_stmt
%type<syntree> elseif_stmt_list
%type<syntree> if_stmt
%type<syntree> cmp
%type<syntree> body
%type<syntree> return_stmt
%type<syntree> return_value
%type<syntree> return_value_list
%type<syntree> explicit_value
%type<syntree> variable
%type<syntree> arg_expr
%type<syntree> expr
%type<syntree> math_expr
%type<syntree> function_call
%type<syntree> var
%type<syntree> arg
%type<syntree> function_call_arguments
%type<syntree> function_declaration_arguments
%type<syntree> assign_stmt
%type<syntree> cmp_value
%type<syntree> assign_value
%type<syntree> expr_value
%type<syntree> math_assign_stmt
%type<syntree> for_stmt
%type<syntree> for_loop_stmt
%type<syntree> multi_assign_stmt
%type<syntree> var_list
%type<syntree> fake_call_stmt
%type<syntree> struct_mem_declaration
%type<syntree> sleep
%type<syntree> yield
%type<syntree> switch_stmt
%type<syntree> switch_case_define
%type<syntree> switch_case_list
%type<syntree> const_map_list_value
%type<syntree> const_map_value
%type<syntree> const_array_list_value

%%

/* Top level rules */
program: package_head
	include_head
	struct_head
	const_head 
	body 
	;
	
package_head:
	/* empty */
	{
	}
	|
	PACKAGE IDENTIFIER
	{
		FKLOG("[bison]: package %s", $2.c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package($2.c_str());
	}
	|
	PACKAGE IDENTIFIER_DOT
	{
		FKLOG("[bison]: package %s", $2.c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package($2.c_str());
	}
	
include_head:
	/* empty */
	{
	}
	|
	include_define
	|
	include_head include_define
	;
	
include_define:
	INCLUDE STRING_DEFINITION
	{
		FKLOG("[bison]: include %s", $2.c_str());
		myflexer *l = (myflexer *)parm;
		l->add_include($2.c_str());
	}
	;

struct_head:
	/* empty */
	{
	}
	|
	struct_define
	|
	struct_head struct_define
	;

struct_define:
	STRUCT IDENTIFIER struct_mem_declaration END
	{
		FKLOG("[bison]: struct_define %s", $2.c_str());
		myflexer *l = (myflexer *)parm;
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>($3);
		l->add_struct_desc($2.c_str(), p);
	}
	;
	
struct_mem_declaration: 
	/* empty */
	{
		$$ = 0;
	}
	| 
	struct_mem_declaration IDENTIFIER 
	{
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER struct_mem_declaration");
		assert($1->gettype() == est_struct_memlist);
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>($1);
		p->add_arg($2);
		$$ = p;
	}
	| 
	IDENTIFIER
	{
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER");
		NEWTYPE(p, struct_desc_memlist_node);
		p->add_arg($1);
		$$ = p;
	}
	;

const_head:
	/* empty */
	{
	}
	|
	const_define
	|
	const_head const_define
	;

const_define:
	FCONST IDENTIFIER ASSIGN explicit_value
	{
		FKLOG("[bison]: const_define %s", $2.c_str());
		myflexer *l = (myflexer *)parm;
		l->add_const_desc($2.c_str(), $4);
	}
	;

body:
	/* empty */
	{
	}
	|
	function_declaration
	|
	body function_declaration
	;

/* function declaration begin */

function_declaration:
	FUNC IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET block END
	{
		FKLOG("[bison]: function_declaration <- block %s %d", $2.c_str(), yylloc.first_line);
		NEWTYPE(p, func_desc_node);
		p->funcname = $2;
		p->arglist = dynamic_cast<func_desc_arglist_node*>($4);
		p->block = dynamic_cast<block_node*>($6);
		p->endline = yylloc.first_line;
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	}
	|
	FUNC IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET END
	{
		FKLOG("[bison]: function_declaration <- empty %s %d", $2.c_str(), yylloc.first_line);
		NEWTYPE(p, func_desc_node);
		p->funcname = $2;
		p->arglist = 0;
		p->block = 0;
		p->endline = yylloc.first_line;
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	}
	;

function_declaration_arguments: 
	/* empty */
	{
		$$ = 0;
	}
	| 
	function_declaration_arguments ARG_SPLITTER arg 
	{
		FKLOG("[bison]: function_declaration_arguments <- arg function_declaration_arguments");
		assert($1->gettype() == est_arglist);
		func_desc_arglist_node * p = dynamic_cast<func_desc_arglist_node*>($1);
		p->add_arg($3);
		$$ = p;
	}
	| 
	arg
	{
		FKLOG("[bison]: function_declaration_arguments <- arg");
		NEWTYPE(p, func_desc_arglist_node);
		p->add_arg($1);
		$$ = p;
	}
	;

arg : 
	IDENTIFIER
	{
		FKLOG("[bison]: arg <- IDENTIFIER %s", $1.c_str());
		NEWTYPE(p, identifier_node);
		p->str = $1;
		$$ = p;
	}
	;
	
function_call:
	IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET 
	{
		FKLOG("[bison]: function_call <- function_call_arguments %s", $1.c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = $1;
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>($3);
		p->fakecall = false;
		p->classmem_call = false;
		$$ = p;
	} 
	|
	IDENTIFIER_DOT OPEN_BRACKET function_call_arguments CLOSE_BRACKET 
	{
		FKLOG("[bison]: function_call <- function_call_arguments %s", $1.c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = $1;
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>($3);
		p->fakecall = false;
		p->classmem_call = false;
		$$ = p;
	} 
	|
	function_call OPEN_BRACKET function_call_arguments CLOSE_BRACKET 
	{
		FKLOG("[bison]: function_call <- function_call_arguments");
		NEWTYPE(p, function_call_node);
		p->fuc = "";
		p->prefunc = $1;
		p->arglist = dynamic_cast<function_call_arglist_node*>($3);
		p->fakecall = false;
		p->classmem_call = false;
		$$ = p;
	} 
	|
	function_call COLON IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET 
	{
		FKLOG("[bison]: function_call <- mem function_call_arguments %s", $3.c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = $3;
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>($5);
		if (p->arglist == 0)
		{
			NEWTYPE(pa, function_call_arglist_node);
			p->arglist = pa;
		}
		p->arglist->add_arg($1);
		p->fakecall = false;
		p->classmem_call = true;
		$$ = p;
	}
	|
	variable COLON IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET 
	{
		FKLOG("[bison]: function_call <- mem function_call_arguments %s", $3.c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = $3;
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>($5);
		if (p->arglist == 0)
		{
			NEWTYPE(pa, function_call_arglist_node);
			p->arglist = pa;
		}
		p->arglist->add_arg($1);
		p->fakecall = false;
		p->classmem_call = true;
		$$ = p;
	} 
	;
	
function_call_arguments: 
	/* empty */
	{
		$$ = 0;
	}
	| 
	function_call_arguments ARG_SPLITTER arg_expr
	{
		FKLOG("[bison]: function_call_arguments <- arg_expr function_call_arguments");
		assert($1->gettype() == est_call_arglist);
		function_call_arglist_node * p = dynamic_cast<function_call_arglist_node*>($1);
		p->add_arg($3);
		$$ = p;
	}
	| 
	arg_expr
	{
		FKLOG("[bison]: function_call_arguments <- arg_expr");
		NEWTYPE(p, function_call_arglist_node);
		p->add_arg($1);
		$$ = p;
	}
	;  

arg_expr:
	expr_value
	{
		FKLOG("[bison]: arg_expr <- expr_value");
		$$ = $1;
	}
	;

/* function declaration end */

block:
	block stmt 
	{
		FKLOG("[bison]: block <- block stmt");
		assert($1->gettype() == est_block);
		block_node * p = dynamic_cast<block_node*>($1);
		p->add_stmt($2);
		$$ = p;
	}
	|
	stmt 
	{
		FKLOG("[bison]: block <- stmt");
		NEWTYPE(p, block_node);
		p->add_stmt($1);
		$$ = p;
	}
	;
  
stmt:
	while_stmt
	{
		FKLOG("[bison]: stmt <- while_stmt");
		$$ = $1;
	}
	|
	if_stmt
	{
		FKLOG("[bison]: stmt <- if_stmt");
		$$ = $1;
	}
	|
	return_stmt
	{
		FKLOG("[bison]: stmt <- return_stmt");
		$$ = $1;
	}
	|
	assign_stmt
	{
		FKLOG("[bison]: stmt <- assign_stmt");
		$$ = $1;
	}
	|
	multi_assign_stmt
	{
		FKLOG("[bison]: stmt <- multi_assign_stmt");
		$$ = $1;
	}
	|
	break
	{
		FKLOG("[bison]: stmt <- break");
		$$ = $1;
	}
	|
	continue
	{
		FKLOG("[bison]: stmt <- continue");
		$$ = $1;
	}
	|
	expr
	{
		FKLOG("[bison]: stmt <- expr");
		$$ = $1;
	}
	|
	math_assign_stmt
	{
		FKLOG("[bison]: stmt <- math_assign_stmt");
		$$ = $1;
	}
	|
	for_stmt
	{
		FKLOG("[bison]: stmt <- for_stmt");
		$$ = $1;
	}
	|
	for_loop_stmt
	{
		FKLOG("[bison]: stmt <- for_loop_stmt");
		$$ = $1;
	}
	|
	fake_call_stmt
	{
		FKLOG("[bison]: stmt <- fake_call_stmt");
		$$ = $1;
	}
	|
	sleep
	{
		FKLOG("[bison]: stmt <- sleep_stmt");
		$$ = $1;
	}
	|
	yield
	{
		FKLOG("[bison]: stmt <- yield_stmt");
		$$ = $1;
	}
	|
	switch_stmt
	{
		FKLOG("[bison]: stmt <- switch_stmt");
		$$ = $1;
	}
	;

fake_call_stmt:
	FAKE function_call
	{
		FKLOG("[bison]: fake_call_stmt <- fake function_call");
		function_call_node * p = dynamic_cast<function_call_node*>($2);
		p->fakecall = true;
		$$ = p;
	}
	;
	
for_stmt:
	FOR block ARG_SPLITTER cmp ARG_SPLITTER block THEN block END
	{
		FKLOG("[bison]: for_stmt <- block cmp block");
		NEWTYPE(p, for_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>($4);
		p->beginblock = dynamic_cast<block_node*>($2);
		p->endblock = dynamic_cast<block_node*>($6);
		p->block = dynamic_cast<block_node*>($8);
		$$ = p;
	}
	|
	FOR block ARG_SPLITTER cmp ARG_SPLITTER block THEN END
	{
		FKLOG("[bison]: for_stmt <- block cmp");
		NEWTYPE(p, for_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>($4);
		p->beginblock = dynamic_cast<block_node*>($2);
		p->endblock = dynamic_cast<block_node*>($6);
		p->block = 0;
		$$ = p;
	}
	;

for_loop_stmt:
	FOR var ASSIGN assign_value RIGHT_POINTER cmp_value ARG_SPLITTER expr_value THEN block END
	{
		FKLOG("[bison]: for_loop_stmt <- block");
		NEWTYPE(p, for_stmt);
		
		syntree_node * pi = $2;
		if (pi->gettype() == est_var)
		{
			NEWTYPE(pvar, variable_node);
			pvar->str = (dynamic_cast<var_node*>(pi))->str;
			pi = pvar;
		}
		
		NEWTYPE(pcmp, cmp_stmt);
		pcmp->cmp = "<";
		pcmp->left = pi;
		pcmp->right = $6;
		p->cmp = pcmp;
		
		NEWTYPE(pbeginblockassign, assign_stmt);
		pbeginblockassign->var = $2;
		pbeginblockassign->value = $4;
		pbeginblockassign->isnew = false;
		NEWTYPE(pbeginblock, block_node);
		pbeginblock->add_stmt(pbeginblockassign);
		p->beginblock = pbeginblock;
		
		NEWTYPE(pendblockassign, math_assign_stmt);
		pendblockassign->var = pi;
		pendblockassign->oper = "+=";
		pendblockassign->value = $8;
		NEWTYPE(pendblock, block_node);
		pendblock->add_stmt(pendblockassign);
		p->endblock = pendblock;
		
		p->block = dynamic_cast<block_node*>($10);
		$$ = p;
	}
	|
	FOR var ASSIGN assign_value RIGHT_POINTER cmp_value ARG_SPLITTER expr_value THEN END
	{
		FKLOG("[bison]: for_loop_stmt <- empty");
		NEWTYPE(p, for_stmt);
		
		NEWTYPE(pcmp, cmp_stmt);
		pcmp->cmp = "<";
		pcmp->left = $2;
		pcmp->right = $6;
		p->cmp = pcmp;
		
		NEWTYPE(pbeginblockassign, assign_stmt);
		pbeginblockassign->var = $2;
		pbeginblockassign->value = $4;
		pbeginblockassign->isnew = false;
		NEWTYPE(pbeginblock, block_node);
		pbeginblock->add_stmt(pbeginblockassign);
		p->beginblock = pbeginblock;
		
		NEWTYPE(pendblockassign, math_assign_stmt);
		pendblockassign->var = $2;
		pendblockassign->oper = "+=";
		pendblockassign->value = $8;
		NEWTYPE(pendblock, block_node);
		pendblock->add_stmt(pendblockassign);
		p->endblock = pendblock;
		
		p->block = 0;
		$$ = p;
	}
	;
	
while_stmt:
	WHILE cmp THEN block END 
	{
		FKLOG("[bison]: while_stmt <- cmp block");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>($2);
		p->block = dynamic_cast<block_node*>($4);
		$$ = p;
	}
	|
	WHILE cmp THEN END 
	{
		FKLOG("[bison]: while_stmt <- cmp");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>($2);
		p->block = 0;
		$$ = p;
	}
	;
	
if_stmt:
	IF cmp THEN block elseif_stmt_list else_stmt END
	{
		FKLOG("[bison]: if_stmt <- cmp block");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>($2);
		p->block = dynamic_cast<block_node*>($4);
		p->elseifs = dynamic_cast<elseif_stmt_list*>($5);
		p->elses = dynamic_cast<else_stmt*>($6);
		$$ = p;
	}
	|
	IF cmp THEN elseif_stmt_list else_stmt END
	{
		FKLOG("[bison]: if_stmt <- cmp");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>($2);
		p->block = 0;
		p->elseifs = dynamic_cast<elseif_stmt_list*>($4);
		p->elses = dynamic_cast<else_stmt*>($5);
		$$ = p;
	}
	;
	
elseif_stmt_list:
	/* empty */
	{
		$$ = 0;
	}
	| 
	elseif_stmt_list elseif_stmt
	{
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt_list elseif_stmt");
		assert($1->gettype() == est_elseif_stmt_list);
		elseif_stmt_list * p = dynamic_cast<elseif_stmt_list*>($1);
		p->add_stmt($2);
		$$ = p;
	}
	| 
	elseif_stmt
	{
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt");
		NEWTYPE(p, elseif_stmt_list);
		p->add_stmt($1);
		$$ = p;
	}
	;
	
elseif_stmt:
	ELSEIF cmp THEN block
	{
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>($2);
		p->block = $4;
		$$ = p;
	}
	|
	ELSEIF cmp THEN
	{
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>($2);
		p->block = 0;
		$$ = p;
	}
	;
	
else_stmt:
	/*empty*/
	{
		$$ = 0;
	}
	|
	ELSE block
	{
		FKLOG("[bison]: else_stmt <- block");
		NEWTYPE(p, else_stmt);
		p->block = dynamic_cast<block_node*>($2);
		$$ = p;
	}
	|
	ELSE
	{
		FKLOG("[bison]: else_stmt <- empty");
		NEWTYPE(p, else_stmt);
		p->block = 0;
		$$ = p;
	}
	;

cmp:
	OPEN_BRACKET cmp CLOSE_BRACKET
	{
		FKLOG("[bison]: cmp <- ( cmp )");
		$$ = $2;
	}
	|
	cmp AND cmp
	{
		FKLOG("[bison]: cmp <- cmp AND cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "&&";
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	cmp OR cmp
	{
		FKLOG("[bison]: cmp <- cmp OR cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "||";
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	cmp_value LESS cmp_value
	{
		FKLOG("[bison]: cmp <- cmp_value LESS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = $2;
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	cmp_value MORE cmp_value
	{
		FKLOG("[bison]: cmp <- cmp_value MORE cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = $2;
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	cmp_value EQUAL cmp_value
	{
		FKLOG("[bison]: cmp <- cmp_value EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = $2;
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	cmp_value MORE_OR_EQUAL cmp_value
	{
		FKLOG("[bison]: cmp <- cmp_value MORE_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = $2;
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	cmp_value LESS_OR_EQUAL cmp_value
	{
		FKLOG("[bison]: cmp <- cmp_value LESS_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = $2;
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	cmp_value NOT_EQUAL cmp_value
	{
		FKLOG("[bison]: cmp <- cmp_value NOT_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = $2;
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	FTRUE
	{
		FKLOG("[bison]: cmp <- true");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "true";
		p->left = 0;
		p->right = 0;
		$$ = p;
	}
	|
	FFALSE
	{
		FKLOG("[bison]: cmp <- false");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "false";
		p->left = 0;
		p->right = 0;
		$$ = p;
	}
	|
	IS cmp_value
	{
		FKLOG("[bison]: cmp <- cmp_value IS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "is";
		p->left = $2;
		p->right = 0;
		$$ = p;
	}
	|
	NOT cmp_value
	{
		FKLOG("[bison]: cmp <- cmp_value NOT cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "not";
		p->left = $2;
		p->right = 0;
		$$ = p;
	}
	;

cmp_value:
	explicit_value
	{
		FKLOG("[bison]: cmp_value <- explicit_value");
		$$ = $1;
	}
	|
	variable
	{
		FKLOG("[bison]: cmp_value <- variable");
		$$ = $1;
	}
	|
	expr
	{
		FKLOG("[bison]: cmp_value <- expr");
		$$ = $1;
	}
	;
	
return_stmt:
	RETURN return_value_list
	{
		FKLOG("[bison]: return_stmt <- RETURN return_value_list");
		NEWTYPE(p, return_stmt);
		p->returnlist = dynamic_cast<return_value_list_node*>($2);
		$$ = p;
	}
	|
	RETURN
	{
		FKLOG("[bison]: return_stmt <- RETURN");
		NEWTYPE(p, return_stmt);
		p->returnlist = 0;
		$$ = p;
	}
	;
 
return_value_list:
	return_value_list ARG_SPLITTER return_value
	{
		FKLOG("[bison]: return_value_list <- return_value_list return_value");
		assert($1->gettype() == est_return_value_list);
		return_value_list_node * p = dynamic_cast<return_value_list_node*>($1);
		p->add_arg($3);
		$$ = p;
	}
	|
	return_value
	{
		NEWTYPE(p, return_value_list_node);
		p->add_arg($1);
		$$ = p;
	}
	;
 
return_value:
	explicit_value
	{
		FKLOG("[bison]: return_value <- explicit_value");
		$$ = $1;
	}
	|
	variable
	{
		FKLOG("[bison]: return_value <- variable");
		$$ = $1;
	}
	|
	expr
	{
		FKLOG("[bison]: return_value <- expr");
		$$ = $1;
	}
	;

assign_stmt:
	var ASSIGN assign_value
	{
		FKLOG("[bison]: assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = $1;
		p->value = $3;
		p->isnew = false;
		$$ = p;
	}
	|
	var NEW_ASSIGN assign_value
	{
		FKLOG("[bison]: new assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = $1;
		p->value = $3;
		p->isnew = true;
		$$ = p;
	}
	;

multi_assign_stmt:
	var_list ASSIGN function_call
	{
		FKLOG("[bison]: multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>($1);
		p->value = $3;
		p->isnew = false;
		$$ = p;
	}
	|
	var_list NEW_ASSIGN function_call
	{
		FKLOG("[bison]: new multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>($1);
		p->value = $3;
		p->isnew = true;
		$$ = p;
	}
	;
	
var_list:
	var_list ARG_SPLITTER var
	{
		FKLOG("[bison]: var_list <- var_list var");
		assert($1->gettype() == est_var_list);
		var_list_node * p = dynamic_cast<var_list_node*>($1);
		p->add_arg($3);
		$$ = p;
	}
	|
	var
	{
		NEWTYPE(p, var_list_node);
		p->add_arg($1);
		$$ = p;
	}
	;
	
assign_value:
	explicit_value
	{
		FKLOG("[bison]: assign_value <- explicit_value");
		$$ = $1;
	}
	|
	variable
	{
		FKLOG("[bison]: assign_value <- variable");
		$$ = $1;
	}
	|
	expr
	{
		FKLOG("[bison]: assign_value <- expr");
		$$ = $1;
	}
	;
	
math_assign_stmt :
	variable PLUS_ASSIGN assign_value
	{
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = $1;
		p->oper = "+=";
		p->value = $3;
		$$ = p;
	}
	|
	variable MINUS_ASSIGN assign_value
	{
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = $1;
		p->oper = "-=";
		p->value = $3;
		$$ = p;
	}
	|
	variable DIVIDE_ASSIGN assign_value
	{
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = $1;
		p->oper = "/=";
		p->value = $3;
		$$ = p;
	}
	|
	variable MULTIPLY_ASSIGN assign_value
	{
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = $1;
		p->oper = "*=";
		p->value = $3;
		$$ = p;
	}
	|
	variable DIVIDE_MOD_ASSIGN assign_value
	{
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = $1;
		p->oper = "%=";
		p->value = $3;
		$$ = p;
	}
	|
	variable INC
	{
		FKLOG("[bison]: math_assign_stmt <- variable INC");
		NEWTYPE(pp, explicit_value_node);
		pp->str = "1";
		pp->type = explicit_value_node::EVT_NUM;
		
		NEWTYPE(p, math_assign_stmt);
		p->var = $1;
		p->oper = "+=";
		p->value = pp;
		$$ = p;
	}
	;
	
var:
	VAR_BEGIN IDENTIFIER
	{
		FKLOG("[bison]: var <- VAR_BEGIN IDENTIFIER %s", $2.c_str());
		NEWTYPE(p, var_node);
		p->str = $2;
		$$ = p;
	}
	|
	variable
	{
		FKLOG("[bison]: var <- variable");
		$$ = $1;
	}
	;

variable:
	IDENTIFIER
	{
		FKLOG("[bison]: variable <- IDENTIFIER %s", $1.c_str());
		NEWTYPE(p, variable_node);
		p->str = $1;
		$$ = p;
	}
	|
	IDENTIFIER OPEN_SQUARE_BRACKET expr_value CLOSE_SQUARE_BRACKET
	{
		FKLOG("[bison]: container_get_node <- IDENTIFIER[expr_value] %s", $1.c_str());
		NEWTYPE(p, container_get_node);
		p->container = $1;
		p->key = $3;
		$$ = p;
	}
	|
	IDENTIFIER_POINTER
	{
		FKLOG("[bison]: variable <- IDENTIFIER_POINTER %s", $1.c_str());
		NEWTYPE(p, struct_pointer_node);
		p->str = $1;
		$$ = p;
	}
	|
	IDENTIFIER_DOT
	{
		FKLOG("[bison]: variable <- IDENTIFIER_DOT %s", $1.c_str());
		NEWTYPE(p, variable_node);
		p->str = $1;
		$$ = p;
	}
	;

expr:
	OPEN_BRACKET expr CLOSE_BRACKET
	{
		FKLOG("[bison]: expr <- (expr)");
		$$ = $2;
	}
	|
	function_call
	{
		FKLOG("[bison]: expr <- function_call");
		$$ = $1;
	}
	|
	math_expr
	{
		FKLOG("[bison]: expr <- math_expr");
		$$ = $1;
	}
	;

math_expr:
	OPEN_BRACKET math_expr CLOSE_BRACKET
	{
		FKLOG("[bison]: math_expr <- (math_expr)");
		$$ = $2;
	}
	|
	expr_value PLUS expr_value
	{
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", $2.c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "+";
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	expr_value MINUS expr_value
	{
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", $2.c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "-";
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	expr_value MULTIPLY expr_value
	{
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", $2.c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "*";
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	expr_value DIVIDE expr_value
	{
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", $2.c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "/";
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	expr_value DIVIDE_MOD expr_value
	{
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", $2.c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "%";
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	|
	expr_value STRING_CAT expr_value
	{
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", $2.c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "..";
		p->left = $1;
		p->right = $3;
		$$ = p;
	}
	;	

expr_value:
	math_expr
	{
		FKLOG("[bison]: expr_value <- math_expr");
		$$ = $1;
	}
	|
	explicit_value
	{
		FKLOG("[bison]: expr_value <- explicit_value");
		$$ = $1;
	}
	|
	function_call
	{
		FKLOG("[bison]: expr_value <- function_call");
		$$ = $1;
	}
	|
	variable
	{
		FKLOG("[bison]: expr_value <- variable");
		$$ = $1;
	}
	;
	
explicit_value:
	FTRUE 
	{
		FKLOG("[bison]: explicit_value <- FTRUE");
		NEWTYPE(p, explicit_value_node);
		p->str = $1;
		p->type = explicit_value_node::EVT_TRUE;
		$$ = p;
	}
	|
	FFALSE 
	{
		FKLOG("[bison]: explicit_value <- FFALSE");
		NEWTYPE(p, explicit_value_node);
		p->str = $1;
		p->type = explicit_value_node::EVT_FALSE;
		$$ = p;
	}
	|
	NUMBER 
	{
		FKLOG("[bison]: explicit_value <- NUMBER %s", $1.c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = $1;
		p->type = explicit_value_node::EVT_NUM;
		$$ = p;
	}
	|
	FKUUID
	{
		FKLOG("[bison]: explicit_value <- FKUUID %s", $1.c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = $1;
		p->type = explicit_value_node::EVT_UUID;
		$$ = p;
	}
	|
	STRING_DEFINITION 
	{
		FKLOG("[bison]: explicit_value <- STRING_DEFINITION %s", $1.c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = $1;
		p->type = explicit_value_node::EVT_STR;
		$$ = p;
	}
	|
	FKFLOAT
	{
		FKLOG("[bison]: explicit_value <- FKFLOAT %s", $1.c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = $1;
		p->type = explicit_value_node::EVT_FLOAT;
		$$ = p;
	}
	|
	FNULL
	{
		FKLOG("[bison]: explicit_value <- FNULL %s", $1.c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = $1;
		p->type = explicit_value_node::EVT_NULL;
		$$ = p;
	}
	|
	OPEN_BIG_BRACKET const_map_list_value CLOSE_BIG_BRACKET
	{
		FKLOG("[bison]: explicit_value <- const_map_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_MAP;
		p->v = $2;
		$$ = p;
	}
	|
	OPEN_SQUARE_BRACKET const_array_list_value CLOSE_SQUARE_BRACKET
	{
		FKLOG("[bison]: explicit_value <- const_array_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_ARRAY;
		p->v = $2;
		$$ = p;
	}
	;
      
const_map_list_value:
	/* empty */
	{
		FKLOG("[bison]: const_map_list_value <- null");
		NEWTYPE(p, const_map_list_value_node);
		$$ = p;
	}
	|
	const_map_value
	{
		FKLOG("[bison]: const_map_list_value <- const_map_value");
		NEWTYPE(p, const_map_list_value_node);
		p->add_ele($1);
		$$ = p;
	}
	|
	const_map_list_value const_map_value
	{
		FKLOG("[bison]: const_map_list_value <- const_map_list_value const_map_value");
		assert($1->gettype() == est_constmaplist);
		const_map_list_value_node * p = dynamic_cast<const_map_list_value_node*>($1);
		p->add_ele($2);
		$$ = p;
	}
	;
	
const_map_value:
	explicit_value COLON explicit_value
	{
		FKLOG("[bison]: const_map_value <- explicit_value");
		NEWTYPE(p, const_map_value_node);
		p->k = $1;
		p->v = $3;
		$$ = p;
	}
	;

const_array_list_value:
	/* empty */
	{
		FKLOG("[bison]: const_array_list_value <- null");
		NEWTYPE(p, const_array_list_value_node);
		$$ = p;
	}
	|
	explicit_value
	{
		FKLOG("[bison]: const_array_list_value <- explicit_value");
		NEWTYPE(p, const_array_list_value_node);
		p->add_ele($1);
		$$ = p;
	}
	|
	const_array_list_value explicit_value
	{
		FKLOG("[bison]: const_array_list_value <- const_array_list_value explicit_value");
		assert($1->gettype() == est_constarraylist);
		const_array_list_value_node * p = dynamic_cast<const_array_list_value_node*>($1);
		p->add_ele($2);
		$$ = p;
	}
	;
	
break:
	BREAK 
	{
		FKLOG("[bison]: break <- BREAK");
		NEWTYPE(p, break_stmt);
		$$ = p;
	}
	;
	
continue:
	CONTINUE 
	{
		FKLOG("[bison]: CONTINUE");
		NEWTYPE(p, continue_stmt);
		$$ = p;
	}
	;

sleep:
	SLEEP expr_value 
	{
		FKLOG("[bison]: SLEEP");
		NEWTYPE(p, sleep_stmt);
		p->time = $2;
		$$ = p;
	}
	
yield:
	YIELD expr_value
	{
		FKLOG("[bison]: YIELD");
		NEWTYPE(p, yield_stmt);
		p->time = $2;
		$$ = p;
	}
	;
	
switch_stmt:
	SWITCH cmp_value switch_case_list DEFAULT block END
	{
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = $2;
		p->caselist = $3;
		p->def = $5;
		$$ = p;
	}
	|
	SWITCH cmp_value switch_case_list DEFAULT END
	{
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = $2;
		p->caselist = $3;
		p->def = 0;
		$$ = p;
	}
	;
	
switch_case_list:
	switch_case_define
	{
		FKLOG("[bison]: switch_case_list <- switch_case_define");
		NEWTYPE(p, switch_caselist_node);
		p->add_case($1);
		$$ = p;
	}
	|
	switch_case_list switch_case_define
	{
		FKLOG("[bison]: switch_case_list <- switch_case_list switch_case_define");
		assert($2->gettype() == est_switch_case_node);
		switch_caselist_node * p = dynamic_cast<switch_caselist_node*>($1);
		p->add_case($2);
		$$ = p;
	}
	;

switch_case_define:
	CASE cmp_value THEN block
	{
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = $2;
		p->block = $4;
		$$ = p;
	}
	|
	CASE cmp_value THEN
	{
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = $2;
		p->block = 0;
		$$ = p;
	}
	;
	