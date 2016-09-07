%{
#include <cstdlib>
#include <cstring>
#include <string>

#include "semantic.h"
#include "bison.h"  
#include "types.h"
#include "myflexer.h"

#define FLEX_DEBUG(x, ...) printf("FLEX DEBUG %d:", yylineno); printf(x, ##__VA_ARGS__);

#define YY_DECL int myflexer::yylex(YYSTYPE * lvalp, YYLTYPE * loc)

#define YY_USER_ACTION loc->first_line = loc->last_line = yylineno;
	
%}

%option c++
%option batch
%option stack
%option yylineno
%x STR
%x SINGLE_LINE_COMMENT

%%

"--"	yy_push_state(SINGLE_LINE_COMMENT);
<SINGLE_LINE_COMMENT>{
	[^\n]*	;
	"\n"	{
		yy_pop_state(); 
	}
}

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

"fake" {
	return FAKE;
}

"while" {
	return WHILE;
}

"for" {
	return FOR;
}

"true" {
  lvalp->str = yytext;
  return FTRUE;
}

"false" {
  lvalp->str = yytext;
  return FFALSE;
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

"elseif" {
	return ELSEIF;
}

"end" {
	return END;
}

"const" {
	return FCONST;
}

"package" {
	return PACKAGE;
}

"include" {
	return INCLUDE;
}

"struct" {
	return STRUCT;
}

"and" {
	return AND;
}

"or" {
	return OR;
}

"is" {
	return IS;
}

"not" {
	return NOT;
}

"continue" {
	return CONTINUE;
}

"yield" {
	return YIELD;
}

"sleep" {
	return SLEEP;
}

"switch" {
	return SWITCH;
}

"case" {
	return CASE;
}

"default" {
	return DEFAULT;
}

"null" {
	return FNULL;
}

"\""	yy_push_state(STR); lvalp->str = String();
<STR>\\n	lvalp->str += String("\n");
<STR>\\t	lvalp->str += String("\t");
<STR>\\s	lvalp->str += String(" ");
<STR>\\r	lvalp->str += String("\r");
<STR>\\\"	lvalp->str += String("\"");
<STR>[^\"] 	lvalp->str += String(yytext);
<STR>"\""	yy_pop_state(); return STRING_DEFINITION; 

[a-zA-Z_][a-zA-Z0-9_]* {
	lvalp->str = String(yytext);
	return IDENTIFIER;
}

[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)+ {
	lvalp->str = String(yytext);
	return IDENTIFIER_DOT;
}

[a-zA-Z_][a-zA-Z0-9_]*(\-\>[a-zA-Z_][a-zA-Z0-9_]*)+ {
	lvalp->str = String(yytext);
	return IDENTIFIER_POINTER;
}

[0-9]+u {
	lvalp->str = String(yytext);
	return FKUUID;
}

-?[0-9]+ {
	lvalp->str = String(yytext);
	return NUMBER;
}

-?[0-9]+\.[0-9]+([Ee]-?[0-9]+)? {
	lvalp->str = String(yytext);
	return FKFLOAT;
}

"%" {
  return DIVIDE_MOD;
}

"," {
	return ARG_SPLITTER;
}

"->" {
	return RIGHT_POINTER;
}

"++" {
	return INC;
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

":=" {
	return NEW_ASSIGN;
}

"+=" {
	return PLUS_ASSIGN;
}

"-=" {
	return MINUS_ASSIGN;
}

"/=" {
	return DIVIDE_ASSIGN;
}

"*=" {
	return MULTIPLY_ASSIGN;
}

"%=" {
  return DIVIDE_MOD_ASSIGN;
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

":" {
	return COLON;
}

"[" {
	return OPEN_SQUARE_BRACKET;
}

"]" {
	return CLOSE_SQUARE_BRACKET;
}

"{" {
	return OPEN_BIG_BRACKET;
}

"}" {
	return CLOSE_BIG_BRACKET;
}

".." {
	return STRING_CAT;
}

\n {
}

. {
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
