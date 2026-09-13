%{
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>
#include "compile/bison/parser.h"

#ifdef yyFlexLexer
#undef yyFlexLexer
#endif
#include "compile/my_flexer.h"

// https://westes.github.io/flex/manual/Cxx.html

// Give Flex the prototype of yylex we want ...
# define YY_DECL \
    yy::parser::symbol_type fakelua::MyFlexer::MyYylex()

%}

%{

#define FLEX_VERSION (YY_FLEX_MAJOR_VERSION * 100 + YY_FLEX_MINOR_VERSION)

%}

%option c++ noyywrap nounput noinput batch debug yylineno
%x SINGLE_LINE_COMMENT_STATE
%x MULTI_LINE_COMMENT_STATE

%{

%}

/* 短字符串里不能出现裸换行（\n 和 \r 都算），与 Lua 的 "unfinished string" 一致。
   需要跨行的内容用 [[ ]] 长字符串或转义写法。 */
string1  \"((?:\\\"|[^\"\n\r])*)\"
string2  \'((\\'|[^'\n\r])*)\'
string3  \[\[([^\]]|(\][^\]]))*\]\]
string   {string1}|{string2}|{string3}
name     [_a-zA-Z][_a-zA-Z0-9]*
/* 字面量不带正号：Lua 没有一元加，+1 应该报 unexpected symbol near '+'。
   去掉 + 后它会退回成 PLUS token，语法里没有对应产生式，于是和 Lua 一样被拒。 */
num1     -?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?
num2     -?0[xX][0-9a-fA-F]+(\.[0-9a-fA-F]+)?([pP][+-]?[0-9]+)?
number   {num1}|{num2}

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  yy::location& loc = location_;
  // Code run each time yylex is called.
  loc.step ();
%}

    /* 长注释按 Lua 的长括号层级处理：--[[ 配 ]]，--[==[ 配 ]==]，层级不匹配的
       ]] 只是普通内容。传统的 --]] 收尾依然有效，因为其中的 ]] 就是 0 级闭括号。 */
"--"\[=*\[        { long_comment_level_ = static_cast<int>(yyleng) - 4; yy_push_state(MULTI_LINE_COMMENT_STATE); }
<MULTI_LINE_COMMENT_STATE>\]=*\]   { if (static_cast<int>(yyleng) - 2 == long_comment_level_) yy_pop_state(); }
<MULTI_LINE_COMMENT_STATE>\n   { loc.lines(yyleng); loc.step(); }
<MULTI_LINE_COMMENT_STATE>.    { }
<MULTI_LINE_COMMENT_STATE><<EOF>>   { throw yy::parser::syntax_error(loc, "unfinished long comment"); }

"--"            { yy_push_state(SINGLE_LINE_COMMENT_STATE); }
<SINGLE_LINE_COMMENT_STATE>\n   { loc.lines(yyleng); loc.step(); yy_pop_state(); }
    /* Lua 的行注释遇到 \r 也结束，只认 \n 会把 \r 后面的真实代码一起吞掉。 */
<SINGLE_LINE_COMMENT_STATE>\r   { yy_pop_state(); }
<SINGLE_LINE_COMMENT_STATE>.    { }

[ \t]+          loc.step ();
\n+             loc.lines(yyleng); loc.step();
\r+             {}

"%"             return yy::parser::make_MOD(loc);
"&"             return yy::parser::make_BITAND(loc);
"|"             return yy::parser::make_BITOR(loc);
"~"             return yy::parser::make_BITNOT(loc);
">"             return yy::parser::make_MORE(loc);
"<"             return yy::parser::make_LESS(loc);
"#"             return yy::parser::make_NUMBER_SIGN(loc);
"^"             return yy::parser::make_POW(loc);
";"             return yy::parser::make_SEMICOLON(loc);
":"             return yy::parser::make_COLON(loc);
","             return yy::parser::make_COMMA(loc);
"."             return yy::parser::make_DOT(loc);
".."            return yy::parser::make_CONCAT(loc);
"..."           return yy::parser::make_VAR_PARAMS(loc);
"=="            return yy::parser::make_EQUAL(loc);
">="            return yy::parser::make_MORE_EQUAL(loc);
"<="            return yy::parser::make_LESS_EQUAL(loc);
"~="            return yy::parser::make_NOT_EQUAL(loc);
"<<"            return yy::parser::make_LEFT_SHIFT(loc);
">>"            return yy::parser::make_RIGHT_SHIFT(loc);
"::"            return yy::parser::make_GOTO_TAG(loc);
"="             return yy::parser::make_ASSIGN(loc);
"-"             return yy::parser::make_MINUS(loc);
"+"             return yy::parser::make_PLUS(loc);
"*"             return yy::parser::make_STAR(loc);
"/"             return yy::parser::make_SLASH(loc);
"("             return yy::parser::make_LPAREN(loc);
")"             return yy::parser::make_RPAREN(loc);
"{"             return yy::parser::make_LCURLY(loc);
"}"             return yy::parser::make_RCURLY(loc);
"["             return yy::parser::make_LSQUARE(loc);
"]"             return yy::parser::make_RSQUARE(loc);
"and"           return yy::parser::make_AND(loc);
"break"         return yy::parser::make_BREAK(loc);
"continue"      return yy::parser::make_CONTINUE(loc);
"do"            return yy::parser::make_DO(loc);
"else"          return yy::parser::make_ELSE(loc);
"elseif"        return yy::parser::make_ELSEIF(loc);
"end"           return yy::parser::make_END(loc);
"false"         return yy::parser::make_FALSES(loc);
"for"           return yy::parser::make_FOR(loc);
"function"      return yy::parser::make_FUNCTION(loc);
"goto"          return yy::parser::make_GOTO(loc);
"if"            return yy::parser::make_IF(loc);
"in"            return yy::parser::make_IN(loc);
"local"         return yy::parser::make_LOCAL(loc);
"nil"           return yy::parser::make_NIL(loc);
"not"           return yy::parser::make_NOT(loc);
"or"            return yy::parser::make_OR(loc);
"repeat"        return yy::parser::make_REPEAT(loc);
"return"        return yy::parser::make_RETURN(loc);
"then"          return yy::parser::make_THEN(loc);
"true"          return yy::parser::make_TRUE(loc);
"until"         return yy::parser::make_UNTIL(loc);
"while"         return yy::parser::make_WHILE(loc);
"//"            return yy::parser::make_DOUBLE_SLASH(loc);

{number}        return yy::parser::make_NUMBER(yytext, loc);

    /* Lua 读数字时会把紧跟其后的字母、数字和点一起吞掉再做校验，所以 3and、1local、
       5..2 都是 "malformed number"。这条规则必须排在 {number} 之后：等长时 flex 取
       靠前的规则，于是 0x1f、1e2 这类合法写法仍走 {number}。 */
{number}[0-9a-zA-Z_.]   { throw yy::parser::syntax_error(loc, "malformed number near '" + std::string(yytext) + "'"); }

{name}          return yy::parser::make_IDENTIFIER(yytext, loc);

{string}        return yy::parser::make_STRING(yytext, loc);

.               { throw yy::parser::syntax_error(loc, "invalid character: [" + std::string(yytext) + "]"); }

<<EOF>>         return yy::parser::symbol_type(static_cast<yy::parser::token::yytokentype>(0), loc);

%%
