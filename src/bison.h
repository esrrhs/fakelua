/* A Bison parser, made by GNU Bison 3.8.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_BISON_TAB_H_INCLUDED
# define YY_YY_BISON_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    VAR_BEGIN = 258,               /* VAR_BEGIN  */
    RETURN = 259,                  /* RETURN  */
    BREAK = 260,                   /* BREAK  */
    FUNC = 261,                    /* FUNC  */
    WHILE = 262,                   /* WHILE  */
    FTRUE = 263,                   /* FTRUE  */
    FFALSE = 264,                  /* FFALSE  */
    IF = 265,                      /* IF  */
    THEN = 266,                    /* THEN  */
    ELSE = 267,                    /* ELSE  */
    END = 268,                     /* END  */
    STRING_DEFINITION = 269,       /* STRING_DEFINITION  */
    IDENTIFIER = 270,              /* IDENTIFIER  */
    NUMBER = 271,                  /* NUMBER  */
    SINGLE_LINE_COMMENT = 272,     /* SINGLE_LINE_COMMENT  */
    DIVIDE_MOD = 273,              /* DIVIDE_MOD  */
    ARG_SPLITTER = 274,            /* ARG_SPLITTER  */
    PLUS = 275,                    /* PLUS  */
    MINUS = 276,                   /* MINUS  */
    DIVIDE = 277,                  /* DIVIDE  */
    MULTIPLY = 278,                /* MULTIPLY  */
    ASSIGN = 279,                  /* ASSIGN  */
    MORE = 280,                    /* MORE  */
    LESS = 281,                    /* LESS  */
    MORE_OR_EQUAL = 282,           /* MORE_OR_EQUAL  */
    LESS_OR_EQUAL = 283,           /* LESS_OR_EQUAL  */
    EQUAL = 284,                   /* EQUAL  */
    NOT_EQUAL = 285,               /* NOT_EQUAL  */
    OPEN_BRACKET = 286,            /* OPEN_BRACKET  */
    CLOSE_BRACKET = 287,           /* CLOSE_BRACKET  */
    AND = 288,                     /* AND  */
    OR = 289,                      /* OR  */
    FKFLOAT = 290,                 /* FKFLOAT  */
    PLUS_ASSIGN = 291,             /* PLUS_ASSIGN  */
    MINUS_ASSIGN = 292,            /* MINUS_ASSIGN  */
    DIVIDE_ASSIGN = 293,           /* DIVIDE_ASSIGN  */
    MULTIPLY_ASSIGN = 294,         /* MULTIPLY_ASSIGN  */
    DIVIDE_MOD_ASSIGN = 295,       /* DIVIDE_MOD_ASSIGN  */
    COLON = 296,                   /* COLON  */
    FOR = 297,                     /* FOR  */
    INC = 298,                     /* INC  */
    FKUUID = 299,                  /* FKUUID  */
    OPEN_SQUARE_BRACKET = 300,     /* OPEN_SQUARE_BRACKET  */
    CLOSE_SQUARE_BRACKET = 301,    /* CLOSE_SQUARE_BRACKET  */
    FCONST = 302,                  /* FCONST  */
    PACKAGE = 303,                 /* PACKAGE  */
    INCLUDE = 304,                 /* INCLUDE  */
    IDENTIFIER_DOT = 305,          /* IDENTIFIER_DOT  */
    IDENTIFIER_POINTER = 306,      /* IDENTIFIER_POINTER  */
    STRUCT = 307,                  /* STRUCT  */
    IS = 308,                      /* IS  */
    NOT = 309,                     /* NOT  */
    CONTINUE = 310,                /* CONTINUE  */
    SWITCH = 311,                  /* SWITCH  */
    CASE = 312,                    /* CASE  */
    DEFAULT = 313,                 /* DEFAULT  */
    NEW_ASSIGN = 314,              /* NEW_ASSIGN  */
    ELSEIF = 315,                  /* ELSEIF  */
    RIGHT_POINTER = 316,           /* RIGHT_POINTER  */
    STRING_CAT = 317,              /* STRING_CAT  */
    OPEN_BIG_BRACKET = 318,        /* OPEN_BIG_BRACKET  */
    CLOSE_BIG_BRACKET = 319,       /* CLOSE_BIG_BRACKET  */
    FNULL = 320                    /* FNULL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




int yyparse (void * parm);


#endif /* !YY_YY_BISON_TAB_H_INCLUDED  */
