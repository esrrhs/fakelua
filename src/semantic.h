#pragma once

#include "types.h"
#include "syntree.h"

typedef struct _YYSTYPE 
{
	String str;
	syntree_node * syntree;
} YYSTYPE;

extern int yyparse(void * param);
extern int yylex(YYSTYPE *lvalp, void * parm);
extern void yyerror(const char *str);
extern int yylex_destroy();

#define YYSTYPE _YYSTYPE

extern int yy_flex_debug;
extern FILE *yyin, *yyout;

extern int yydebug;

