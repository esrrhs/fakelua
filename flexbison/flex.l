%{
#include <cstdlib>
#include <cstring>
#include <string>

#include "semantic.h"
#include "bison.h"  
#include "types.h"
#include "myflexer.h"

#define FLEX_DEBUG(x, ...) printf("FLEX DEBUG %d:", yylineno); printf(x, ##__VA_ARGS__);

#define YY_DECL int myflexer::yylex(YYSTYPE * lvalp)

%}

%option c++
%option stack
%x STR
%x SINGLE_LINE_COMMENT

%%

"var"	{
	return VAR_BEGIN;
}

"return"  {
	return RETURN;
}

"break" {
    return BREAK;
}

"func" {
	return FUNC;
}

"while" {
	return WHILE;
}

"true" {
  lvalp->str = yytext;
  return TRUE;
}

"false" {
  lvalp->str = yytext;
  return FALSE;
}

"if" {
	return IF;
}

"then" {
	return THEN;
}

"else" {
	return ELSE;
}

"end" {
	return END;
}


"\""	yy_push_state(STR); lvalp->str = String();
<STR>\\n	lvalp->str += String("\n");
<STR>\\t	lvalp->str += String("\t");
<STR>\\\"	lvalp->str += String("\"");
<STR>[^\"] 	lvalp->str += String(yytext);
<STR>"\""	yy_pop_state(); return STRING_DEFINITION; 


[a-zA-Z_][a-zA-Z0-9_]* {
	lvalp->str = String(yytext);
	return IDENTIFIER;
}

[0-9]+ {
	lvalp->str = String(yytext);
	return NUMBER;
}


"--"	yy_push_state(SINGLE_LINE_COMMENT);
<SINGLE_LINE_COMMENT>{
	[^\n]*	;
	"\n"	{
		yy_pop_state(); 
	}
}

"%" {
  return DIVIDE_MOD;
}

"," {
	return ARG_SPLITTER;
}

"+" {
	return PLUS;
}

"-" {
	return MINUS;
}

"/" {
	return DIVIDE;
}

"*" {
	return MULTIPLY;
}

"=" {
	return ASSIGN;
}

">" {
	lvalp->str = String(yytext);
	return MORE;
}

"<" {
	lvalp->str = String(yytext);
	return LESS;
}

">=" {
	lvalp->str = String(yytext);
	return MORE_OR_EQUAL;
}

"<=" {
	lvalp->str = String(yytext);
	return LESS_OR_EQUAL;
}

"==" {
	lvalp->str = String(yytext);
	return EQUAL;
}

"!=" {
	lvalp->str = String(yytext);
	return NOT_EQUAL;
}

"(" {
	return OPEN_BRACKET;
}

")" {
	return CLOSE_BRACKET;
}

"." {
	lvalp->str = String(yytext);
	return STRING_CONCETATE;
}

%%
int yyFlexLexer::yywrap()
{
	return 1;
}
int yyFlexLexer::yylex()
{
	return 0;
}
