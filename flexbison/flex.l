%{
#include <cstdlib>
#include <cstring>
#include <string>

#include "y.tab.h"  
extern int yylval;  

#define FLEX_DEBUG(x, ...) printf("FLEX DEBUG %d:", yylineno); printf(x, ##__VA_ARGS__);

%}

%x STR
%x SINGLE_LINE_COMMENT

%%

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

"else" {
	return ELSE;
}


"\""	yy_push_state(STR); yylval.str = String();
<STR>\\n	yylval.str += String("\n");
<STR>\\t	yylval.str += String("\t");
<STR>\\\"	yylval.str += String("\"");
<STR>[^\"] 	yylval.str += String(yytext);
<STR>"\""	yy_pop_state(); return STRING_DEFINITION; 


[a-zA-Z_][a-zA-Z0-9_]* {
	yylval.str = String(yytext);
	return IDENTIFIER;
}

[0-9]+ {
	yylval.str = String(yytext);
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
	return  PLUS;
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
	yylval.str = String(yytext);
	return MORE;
}

"<" {
	yylval.str = String(yytext);
	return LESS;
}

">=" {
	yylval.str = String(yytext);
	return MORE_OR_EQUAL;
}

"<=" {
	yylval.str = String(yytext);
	return LESS_OR_EQUAL;
}

"==" {
	yylval.str = String(yytext);
	return EQUAL;
}

"!=" {
	yylval.str = String(yytext);
	return NOT_EQUAL;
}

"(" {
	return OPEN_BRACKET;
}

")" {
	return CLOSE_BRACKET;
}

"." {
	yylval.str = String(yytext);
	return STRING_CONCETATE;
}

%%



