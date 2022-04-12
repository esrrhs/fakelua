%{
#include <cstdlib>
#include <cstring>
#include <string>

#include "semantic.h"
#include "bison.h"  
#include "types.h"
#include "myflexer.h"

#define FLEX_DEBUG(x, ...) printf("FLEX DEBUG %d:", yylineno); printf(x, ##__VA_ARGS__);
#define FLEX_DEBUG(x, ...) printf("FLEX DEBUG %d:", yylineno); printf(x, ##__VA_ARGS__);

#define YY_DECL int myflexer::yylex(YYSTYPE * lvalp, YYLTYPE * loc)

#define YY_USER_ACTION loc->first_line = loc->last_line = yylineno;

%}

w              [ \t\v\a]+
o              [ \t\v\a]*
name           [_a-zA-Z][_a-zA-Z0-9]*
n              [0-9]+
exp            [Ee][+-]?{n}
number         ({n}|{n}[.]{n}){exp}?

%option c++
%option batch
%option stack
%option yylineno
%x STR
%x SINGLE_LINE_COMMENT_STATE

%%

"--"	yy_push_state(SINGLE_LINE_COMMENT_STATE);
<SINGLE_LINE_COMMENT_STATE>{
	[^\n]*	;
	"\n"	{
		yy_pop_state(); 
	}
}

"local"	{
	return LOCAL;
}

"return"  {
	return RETURN;
}

"break" {
    return BREAK;
}

"function" {
	return FUNC;
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

"and" {
	return AND;
}

"or" {
	return OR;
}

"not" {
	return NOT;
}

"nil" {
	return FNULL;
}

"do" {
	return DO;
}

"in" {
	return IN;
}

"repeat" {
	return REPEAT;
}

"until" {
	return UNTIL;
}

"\""	yy_push_state(STR); lvalp->str = String();
<STR>\\n	lvalp->str += String("\n");
<STR>\\t	lvalp->str += String("\t");
<STR>\\s	lvalp->str += String(" ");
<STR>\\r	lvalp->str += String("\r");
<STR>\\\"	lvalp->str += String("\"");
<STR>[^\"] 	lvalp->str += String(yytext);
<STR>"\""	yy_pop_state(); return STRING_DEFINITION; 

{name} {
	lvalp->str = String(yytext);
	return IDENTIFIER;
}

{number} {
	lvalp->str = String(yytext);
	return NUMBER;
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

"//" {
	return FLOOR_DIVIDE;
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

">>" {
	return BIT_RIGHT_SHIFT;
}

"<<" {
	return BIT_LEFT_SHIFT;
}

"^" {
	return EXPONENTIATION;
}

">=" {
	lvalp->str = String(yytext);
	return MORE_OR_EQUAL;
}

"<=" {
	lvalp->str = String(yytext);
	return LESS_OR_EQUAL;
}

">" {
	lvalp->str = String(yytext);
	return MORE;
}

"<" {
	lvalp->str = String(yytext);
	return LESS;
}

"==" {
	lvalp->str = String(yytext);
	return EQUAL;
}

"~=" {
	lvalp->str = String(yytext);
	return NOT_EQUAL;
}

"(" {
	return OPEN_BRACKET;
}

")" {
	return CLOSE_BRACKET;
}

"::" {
	return DOUBLE_COLON;
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

"&" {
	return BIT_AND;
}

"~" {
	return BIT_XOR;
}

"|" {
	return BIT_OR;
}

"..." {
	return DOTS;
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
