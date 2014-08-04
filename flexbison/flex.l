%{
#include <cstdlib>
#include <cstring>
#include <string>

#include "semantic.h"
#include "bison.h"  
#include "types.h"

#define FLEX_DEBUG(x, ...) printf("FLEX DEBUG %d:", yylineno); printf(x, ##__VA_ARGS__);

%}

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
  yylval.str = yytext;
  return TRUE;
}

"false" {
  yylval.str = yytext;
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


"\""	yy_push_state(STR); yylval.str = fscript::String();
<STR>\\n	yylval.str += fscript::String("\n");
<STR>\\t	yylval.str += fscript::String("\t");
<STR>\\\"	yylval.str += fscript::String("\"");
<STR>[^\"] 	yylval.str += fscript::String(yytext);
<STR>"\""	yy_pop_state(); return STRING_DEFINITION; 


[a-zA-Z_][a-zA-Z0-9_]* {
	yylval.str = fscript::String(yytext);
	return IDENTIFIER;
}

[0-9]+ {
	yylval.str = fscript::String(yytext);
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
	yylval.str = fscript::String(yytext);
	return MORE;
}

"<" {
	yylval.str = fscript::String(yytext);
	return LESS;
}

">=" {
	yylval.str = fscript::String(yytext);
	return MORE_OR_EQUAL;
}

"<=" {
	yylval.str = fscript::String(yytext);
	return LESS_OR_EQUAL;
}

"==" {
	yylval.str = fscript::String(yytext);
	return EQUAL;
}

"!=" {
	yylval.str = fscript::String(yytext);
	return NOT_EQUAL;
}

"(" {
	return OPEN_BRACKET;
}

")" {
	return CLOSE_BRACKET;
}

"." {
	yylval.str = fscript::String(yytext);
	return STRING_CONCETATE;
}

%%
void cheat_complier()
{
	yyunput(0, 0);
	yy_top_state();
}

