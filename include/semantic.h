#pragma once

#include "instructions.h"

extern int yyparse(void);
extern int yylex (void);
extern void yyerror(const char *str);

typedef struct _YYSTYPE 
{
    fscript::String str;
    fscript::operat* oper;
    fscript::expression* expr;
    fscript::ExpressionsList args;
} YYSTYPE;

#define YYSTYPE _YYSTYPE

extern char *yytext;
extern YYSTYPE yylval;

extern int yy_flex_debug;
extern FILE *yyin, *yyout;

extern int yylineno;
extern int yydebug;

