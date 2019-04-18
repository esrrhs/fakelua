/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     VAR_BEGIN = 258,
     RETURN = 259,
     BREAK = 260,
     FUNC = 261,
     WHILE = 262,
     FTRUE = 263,
     FFALSE = 264,
     IF = 265,
     THEN = 266,
     ELSE = 267,
     END = 268,
     STRING_DEFINITION = 269,
     IDENTIFIER = 270,
     NUMBER = 271,
     SINGLE_LINE_COMMENT = 272,
     DIVIDE_MOD = 273,
     ARG_SPLITTER = 274,
     PLUS = 275,
     MINUS = 276,
     DIVIDE = 277,
     MULTIPLY = 278,
     ASSIGN = 279,
     MORE = 280,
     LESS = 281,
     MORE_OR_EQUAL = 282,
     LESS_OR_EQUAL = 283,
     EQUAL = 284,
     NOT_EQUAL = 285,
     OPEN_BRACKET = 286,
     CLOSE_BRACKET = 287,
     AND = 288,
     OR = 289,
     FKFLOAT = 290,
     PLUS_ASSIGN = 291,
     MINUS_ASSIGN = 292,
     DIVIDE_ASSIGN = 293,
     MULTIPLY_ASSIGN = 294,
     DIVIDE_MOD_ASSIGN = 295,
     COLON = 296,
     FOR = 297,
     INC = 298,
     FAKE = 299,
     FKUUID = 300,
     OPEN_SQUARE_BRACKET = 301,
     CLOSE_SQUARE_BRACKET = 302,
     FCONST = 303,
     PACKAGE = 304,
     INCLUDE = 305,
     IDENTIFIER_DOT = 306,
     IDENTIFIER_POINTER = 307,
     STRUCT = 308,
     IS = 309,
     NOT = 310,
     CONTINUE = 311,
     YIELD = 312,
     SLEEP = 313,
     SWITCH = 314,
     CASE = 315,
     DEFAULT = 316,
     NEW_ASSIGN = 317,
     ELSEIF = 318,
     RIGHT_POINTER = 319,
     STRING_CAT = 320,
     OPEN_BIG_BRACKET = 321,
     CLOSE_BIG_BRACKET = 322,
     FNULL = 323
   };
#endif
/* Tokens.  */
#define VAR_BEGIN 258
#define RETURN 259
#define BREAK 260
#define FUNC 261
#define WHILE 262
#define FTRUE 263
#define FFALSE 264
#define IF 265
#define THEN 266
#define ELSE 267
#define END 268
#define STRING_DEFINITION 269
#define IDENTIFIER 270
#define NUMBER 271
#define SINGLE_LINE_COMMENT 272
#define DIVIDE_MOD 273
#define ARG_SPLITTER 274
#define PLUS 275
#define MINUS 276
#define DIVIDE 277
#define MULTIPLY 278
#define ASSIGN 279
#define MORE 280
#define LESS 281
#define MORE_OR_EQUAL 282
#define LESS_OR_EQUAL 283
#define EQUAL 284
#define NOT_EQUAL 285
#define OPEN_BRACKET 286
#define CLOSE_BRACKET 287
#define AND 288
#define OR 289
#define FKFLOAT 290
#define PLUS_ASSIGN 291
#define MINUS_ASSIGN 292
#define DIVIDE_ASSIGN 293
#define MULTIPLY_ASSIGN 294
#define DIVIDE_MOD_ASSIGN 295
#define COLON 296
#define FOR 297
#define INC 298
#define FAKE 299
#define FKUUID 300
#define OPEN_SQUARE_BRACKET 301
#define CLOSE_SQUARE_BRACKET 302
#define FCONST 303
#define PACKAGE 304
#define INCLUDE 305
#define IDENTIFIER_DOT 306
#define IDENTIFIER_POINTER 307
#define STRUCT 308
#define IS 309
#define NOT 310
#define CONTINUE 311
#define YIELD 312
#define SLEEP 313
#define SWITCH 314
#define CASE 315
#define DEFAULT 316
#define NEW_ASSIGN 317
#define ELSEIF 318
#define RIGHT_POINTER 319
#define STRING_CAT 320
#define OPEN_BIG_BRACKET 321
#define CLOSE_BIG_BRACKET 322
#define FNULL 323




/* Copy the first part of user declarations.  */
#line 1 "bison.y"

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
	


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 292 "bison.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1300

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  69
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  53
/* YYNRULES -- Number of rules.  */
#define YYNRULES  156
/* YYNRULES -- Number of states.  */
#define YYNSTATES  272

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     9,    10,    13,    16,    17,    19,    22,
      25,    26,    28,    31,    36,    37,    40,    42,    43,    45,
      48,    53,    54,    56,    59,    67,    74,    75,    79,    81,
      83,    88,    93,    98,   105,   112,   113,   117,   119,   121,
     124,   126,   128,   130,   132,   134,   136,   138,   140,   142,
     144,   146,   148,   150,   152,   154,   156,   159,   169,   178,
     190,   201,   207,   212,   220,   227,   228,   231,   233,   238,
     242,   243,   246,   248,   252,   256,   260,   264,   268,   272,
     276,   280,   284,   286,   288,   291,   294,   296,   298,   300,
     303,   305,   309,   311,   313,   315,   317,   321,   325,   329,
     333,   337,   339,   341,   343,   345,   349,   353,   357,   361,
     365,   368,   371,   373,   375,   380,   382,   384,   388,   390,
     392,   396,   400,   404,   408,   412,   416,   420,   422,   424,
     426,   428,   430,   432,   434,   436,   438,   440,   442,   446,
     450,   451,   453,   456,   460,   461,   463,   466,   468,   470,
     473,   476,   483,   489,   491,   494,   499
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      70,     0,    -1,    71,    72,    74,    77,    79,    -1,    -1,
      49,    15,    -1,    49,    51,    -1,    -1,    73,    -1,    72,
      73,    -1,    50,    14,    -1,    -1,    75,    -1,    74,    75,
      -1,    53,    15,    76,    13,    -1,    -1,    76,    15,    -1,
      15,    -1,    -1,    78,    -1,    77,    78,    -1,    48,    15,
      24,   111,    -1,    -1,    80,    -1,    79,    80,    -1,     6,
      15,    31,    81,    32,    86,    13,    -1,     6,    15,    31,
      81,    32,    13,    -1,    -1,    81,    19,    82,    -1,    82,
      -1,    15,    -1,    15,    31,    84,    32,    -1,    51,    31,
      84,    32,    -1,    83,    31,    84,    32,    -1,    83,    41,
      15,    31,    84,    32,    -1,   107,    41,    15,    31,    84,
      32,    -1,    -1,    84,    19,    85,    -1,    85,    -1,   110,
      -1,    86,    87,    -1,    87,    -1,    91,    -1,    92,    -1,
      98,    -1,   101,    -1,   102,    -1,   115,    -1,   116,    -1,
     108,    -1,   105,    -1,    89,    -1,    90,    -1,    88,    -1,
     117,    -1,   118,    -1,   119,    -1,    44,    83,    -1,    42,
      86,    19,    96,    19,    86,    11,    86,    13,    -1,    42,
      86,    19,    96,    19,    86,    11,    13,    -1,    42,   106,
      24,   104,    64,    97,    19,   110,    11,    86,    13,    -1,
      42,   106,    24,   104,    64,    97,    19,   110,    11,    13,
      -1,     7,    96,    11,    86,    13,    -1,     7,    96,    11,
      13,    -1,    10,    96,    11,    86,    93,    95,    13,    -1,
      10,    96,    11,    93,    95,    13,    -1,    -1,    93,    94,
      -1,    94,    -1,    63,    96,    11,    86,    -1,    63,    96,
      11,    -1,    -1,    12,    86,    -1,    12,    -1,    31,    96,
      32,    -1,    96,    33,    96,    -1,    96,    34,    96,    -1,
      97,    26,    97,    -1,    97,    25,    97,    -1,    97,    29,
      97,    -1,    97,    27,    97,    -1,    97,    28,    97,    -1,
      97,    30,    97,    -1,     8,    -1,     9,    -1,    54,    97,
      -1,    55,    97,    -1,   111,    -1,   107,    -1,   108,    -1,
       4,    99,    -1,     4,    -1,    99,    19,   100,    -1,   100,
      -1,   111,    -1,   107,    -1,   108,    -1,   106,    24,   104,
      -1,   106,    62,   104,    -1,   103,    24,    83,    -1,   103,
      62,    83,    -1,   103,    19,   106,    -1,   106,    -1,   111,
      -1,   107,    -1,   108,    -1,   107,    36,   104,    -1,   107,
      37,   104,    -1,   107,    38,   104,    -1,   107,    39,   104,
      -1,   107,    40,   104,    -1,   107,    43,    -1,     3,    15,
      -1,   107,    -1,    15,    -1,    15,    46,   110,    47,    -1,
      52,    -1,    51,    -1,    31,   108,    32,    -1,    83,    -1,
     109,    -1,    31,   109,    32,    -1,   110,    20,   110,    -1,
     110,    21,   110,    -1,   110,    23,   110,    -1,   110,    22,
     110,    -1,   110,    18,   110,    -1,   110,    65,   110,    -1,
     109,    -1,   111,    -1,    83,    -1,   107,    -1,     8,    -1,
       9,    -1,    16,    -1,    45,    -1,    14,    -1,    35,    -1,
      68,    -1,    66,   112,    67,    -1,    46,   114,    47,    -1,
      -1,   113,    -1,   112,   113,    -1,   111,    41,   111,    -1,
      -1,   111,    -1,   114,   111,    -1,     5,    -1,    56,    -1,
      58,   110,    -1,    57,   110,    -1,    59,    97,   120,    61,
      86,    13,    -1,    59,    97,   120,    61,    13,    -1,   121,
      -1,   120,   121,    -1,    60,    97,    11,    86,    -1,    60,
      97,    11,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   175,   175,   184,   187,   194,   203,   206,   208,   212,
     222,   225,   227,   231,   242,   246,   255,   266,   269,   271,
     275,   285,   288,   290,   296,   308,   323,   327,   336,   346,
     356,   368,   380,   392,   410,   431,   435,   444,   454,   464,
     473,   483,   489,   495,   501,   507,   513,   519,   525,   531,
     537,   543,   549,   555,   561,   567,   575,   585,   596,   609,
     648,   681,   690,   701,   712,   726,   730,   739,   749,   758,
     770,   774,   782,   792,   798,   808,   818,   828,   838,   848,
     858,   868,   878,   888,   898,   908,   920,   926,   932,   940,
     948,   958,   967,   976,   982,   988,   996,  1006,  1018,  1028,
    1040,  1049,  1058,  1064,  1070,  1078,  1088,  1098,  1108,  1118,
    1128,  1144,  1152,  1160,  1168,  1177,  1185,  1195,  1201,  1207,
    1215,  1221,  1231,  1241,  1251,  1261,  1271,  1283,  1289,  1295,
    1301,  1309,  1318,  1327,  1336,  1345,  1354,  1363,  1372,  1382,
    1395,  1401,  1409,  1420,  1432,  1438,  1446,  1457,  1466,  1475,
    1484,  1494,  1504,  1516,  1524,  1535,  1544
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "VAR_BEGIN", "RETURN", "BREAK", "FUNC",
  "WHILE", "FTRUE", "FFALSE", "IF", "THEN", "ELSE", "END",
  "STRING_DEFINITION", "IDENTIFIER", "NUMBER", "SINGLE_LINE_COMMENT",
  "DIVIDE_MOD", "ARG_SPLITTER", "PLUS", "MINUS", "DIVIDE", "MULTIPLY",
  "ASSIGN", "MORE", "LESS", "MORE_OR_EQUAL", "LESS_OR_EQUAL", "EQUAL",
  "NOT_EQUAL", "OPEN_BRACKET", "CLOSE_BRACKET", "AND", "OR", "FKFLOAT",
  "PLUS_ASSIGN", "MINUS_ASSIGN", "DIVIDE_ASSIGN", "MULTIPLY_ASSIGN",
  "DIVIDE_MOD_ASSIGN", "COLON", "FOR", "INC", "FAKE", "FKUUID",
  "OPEN_SQUARE_BRACKET", "CLOSE_SQUARE_BRACKET", "FCONST", "PACKAGE",
  "INCLUDE", "IDENTIFIER_DOT", "IDENTIFIER_POINTER", "STRUCT", "IS", "NOT",
  "CONTINUE", "YIELD", "SLEEP", "SWITCH", "CASE", "DEFAULT", "NEW_ASSIGN",
  "ELSEIF", "RIGHT_POINTER", "STRING_CAT", "OPEN_BIG_BRACKET",
  "CLOSE_BIG_BRACKET", "FNULL", "$accept", "program", "package_head",
  "include_head", "include_define", "struct_head", "struct_define",
  "struct_mem_declaration", "const_head", "const_define", "body",
  "function_declaration", "function_declaration_arguments", "arg",
  "function_call", "function_call_arguments", "arg_expr", "block", "stmt",
  "fake_call_stmt", "for_stmt", "for_loop_stmt", "while_stmt", "if_stmt",
  "elseif_stmt_list", "elseif_stmt", "else_stmt", "cmp", "cmp_value",
  "return_stmt", "return_value_list", "return_value", "assign_stmt",
  "multi_assign_stmt", "var_list", "assign_value", "math_assign_stmt",
  "var", "variable", "expr", "math_expr", "expr_value", "explicit_value",
  "const_map_list_value", "const_map_value", "const_array_list_value",
  "break", "continue", "sleep", "yield", "switch_stmt", "switch_case_list",
  "switch_case_define", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    69,    70,    71,    71,    71,    72,    72,    72,    73,
      74,    74,    74,    75,    76,    76,    76,    77,    77,    77,
      78,    79,    79,    79,    80,    80,    81,    81,    81,    82,
      83,    83,    83,    83,    83,    84,    84,    84,    85,    86,
      86,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    88,    89,    89,    90,
      90,    91,    91,    92,    92,    93,    93,    93,    94,    94,
      95,    95,    95,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    97,    97,    97,    98,
      98,    99,    99,   100,   100,   100,   101,   101,   102,   102,
     103,   103,   104,   104,   104,   105,   105,   105,   105,   105,
     105,   106,   106,   107,   107,   107,   107,   108,   108,   108,
     109,   109,   109,   109,   109,   109,   109,   110,   110,   110,
     110,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     112,   112,   112,   113,   114,   114,   114,   115,   116,   117,
     118,   119,   119,   120,   120,   121,   121
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     5,     0,     2,     2,     0,     1,     2,     2,
       0,     1,     2,     4,     0,     2,     1,     0,     1,     2,
       4,     0,     1,     2,     7,     6,     0,     3,     1,     1,
       4,     4,     4,     6,     6,     0,     3,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     9,     8,    11,
      10,     5,     4,     7,     6,     0,     2,     1,     4,     3,
       0,     2,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     2,     2,     1,     1,     1,     2,
       1,     3,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       2,     2,     1,     1,     4,     1,     1,     3,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       0,     1,     2,     3,     0,     1,     2,     1,     1,     2,
       2,     6,     5,     1,     2,     4,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     6,     4,     5,     1,     0,    10,     7,
       9,     0,     8,    17,    11,    14,     0,    12,    21,    18,
      16,     0,     0,     0,    19,     2,    22,    13,    15,     0,
       0,    23,   131,   132,   135,   133,   136,   134,   144,   140,
     137,    20,    26,   145,     0,     0,     0,   141,    29,     0,
      28,   139,   146,     0,   138,   142,     0,     0,   143,    27,
       0,    90,   147,     0,     0,    25,   113,     0,     0,     0,
     116,   115,   148,     0,     0,     0,   118,     0,    40,    52,
      50,    51,    41,    42,    43,    44,    45,     0,    49,   101,
     130,    48,   119,     0,   128,    46,    47,    53,    54,    55,
     111,    89,    92,    94,    95,    93,   131,   132,     0,     0,
       0,     0,     0,    87,    88,    86,     0,    35,     0,   130,
       0,   119,     0,   101,    56,     0,    35,     0,   129,   127,
     150,   149,     0,    35,     0,    24,    39,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   110,     0,
       0,     0,     0,     0,     0,     0,     0,    88,    84,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    65,
       0,    37,    38,     0,   117,   120,     0,     0,     0,   127,
       0,     0,   153,     0,     0,   113,   116,   100,   112,    98,
      99,    96,   103,   104,   102,    97,   105,   106,   107,   108,
     109,     0,   125,   121,   122,   124,   123,   126,    91,    73,
      62,     0,    74,    75,    77,    76,    79,    80,    78,    81,
       0,    65,    70,    67,     0,    30,   114,     0,    96,    31,
       0,     0,   154,    32,    35,    35,    61,     0,    70,    72,
      66,     0,    36,     0,     0,   156,   152,     0,     0,     0,
      69,     0,    71,    64,     0,     0,   155,   151,    33,    34,
      68,    63,     0,     0,    58,     0,     0,    57,     0,    60,
       0,    59
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     8,     9,    13,    14,    21,    18,    19,
      25,    26,    49,    50,    76,   170,   171,    77,    78,    79,
      80,    81,    82,    83,   222,   223,   241,   111,   112,    84,
     101,   102,    85,    86,    87,   191,    88,    89,    90,    91,
      92,    93,    94,    46,    47,    44,    95,    96,    97,    98,
      99,   181,   182
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -211
static const yytype_int16 yypact[] =
{
       9,   -11,    66,    42,  -211,  -211,  -211,    86,    41,  -211,
    -211,   105,  -211,    36,  -211,   139,   164,  -211,    -1,  -211,
    -211,   140,   165,   175,  -211,   186,  -211,  -211,  -211,   392,
     176,  -211,  -211,  -211,  -211,  -211,  -211,  -211,   392,   392,
    -211,  -211,   195,  -211,   197,   178,   345,  -211,  -211,    -6,
    -211,  -211,  -211,   392,  -211,  -211,   195,   511,  -211,  -211,
     201,  1223,  -211,    22,    22,  -211,    14,  1223,  1177,    18,
     189,  -211,  -211,  1232,  1232,  1223,    21,   577,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,    40,  -211,     1,
     498,  -211,   101,   344,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,   198,  -211,   216,  -211,   445,   169,   277,    22,  1223,
    1223,    64,   296,   216,  -211,   445,    97,  1232,  1232,   180,
     191,   295,   595,    25,   115,   180,  1232,  1232,   115,  -211,
     344,   344,   167,  1232,   214,  -211,  -211,     4,    18,    18,
    1223,  1223,  1223,  1223,  1223,  1223,  1223,   215,  -211,  1232,
    1232,  1232,  1232,  1232,  1232,  1223,    95,   191,  -211,  -211,
     661,    22,    22,  1223,  1223,  1223,  1223,  1223,  1223,   679,
      -5,  -211,   344,   399,  -211,  -211,    22,  1223,    -3,   199,
    1223,   100,  -211,    29,   204,   194,  -211,  -211,  -211,   115,
     115,  -211,   216,  -211,   445,  -211,  -211,  -211,  -211,  -211,
    -211,   210,   182,    60,    60,   182,   182,  -211,  -211,  -211,
    -211,   745,   130,   130,  -211,  -211,  -211,  -211,  -211,  -211,
      22,   679,    -9,  -211,  1232,  -211,  -211,   227,   184,  -211,
     242,   811,  -211,  -211,  1232,  1232,  -211,   134,    -9,  1177,
    -211,   245,  -211,  1177,  1223,  1177,  -211,   877,    53,   125,
    1177,   249,  1177,  -211,   895,   260,  1177,  -211,  -211,  -211,
    1177,  -211,   961,  1232,  -211,  1027,     0,  -211,  1093,  -211,
    1159,  -211
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -211,  -211,  -211,  -211,   256,  -211,   269,  -211,  -211,   262,
    -211,   258,  -211,   229,   181,  -125,    62,   -17,   127,  -211,
    -211,  -211,  -211,  -211,    68,  -210,    49,   -58,   267,  -211,
    -211,   135,  -211,  -211,  -211,   129,  -211,   -66,    32,   332,
     151,   219,   -29,  -211,   248,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,   110
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -131
static const yytype_int16 yytable[] =
{
      41,   178,   123,   239,     4,    23,   116,    60,   183,    43,
      45,   268,   240,    56,   224,    52,   224,    45,   149,   185,
     150,   151,   152,   153,    58,   140,    57,   225,   240,   229,
     106,   107,   105,    66,   115,   115,    34,    66,    35,  -129,
       5,  -129,  -129,  -129,  -129,   117,   115,    16,   224,   177,
     156,   122,   133,   108,   220,   186,    71,    36,     1,   137,
     118,   233,   134,   141,   138,   154,     6,    37,    38,    70,
      71,   187,   224,    70,    71,   160,   109,   110,   149,   115,
     115,   115,   152,   153,    16,   258,  -129,   141,    39,    11,
      40,     7,     7,   103,    11,   113,   113,   161,   162,   119,
      10,   125,   139,   212,   213,   119,   119,   113,   169,   248,
     249,   194,   194,   194,   194,   194,   194,   194,   227,  -127,
      15,  -127,  -127,  -127,  -127,   154,   105,   209,   161,   162,
     161,   162,   115,   115,   115,   115,   115,   115,   115,   115,
     113,   113,   113,   211,   224,   250,   133,   115,   194,   119,
     119,   115,   221,    27,    20,    28,   134,   259,   119,   119,
     180,   231,   237,   161,   162,   119,  -127,   161,   162,   188,
     125,   125,   192,   192,   192,   192,   192,   192,   192,    22,
     -82,   119,   119,   119,   119,   119,   119,   103,   -82,    29,
      30,   115,    23,   113,   113,   113,   113,   113,   113,   113,
     113,   -82,   -82,   -82,   136,    32,    33,    42,   113,   192,
      48,    34,   113,    35,   247,   115,   100,   155,   121,    53,
     126,   147,   252,   174,   129,   129,   254,   180,   256,   184,
     201,   175,    36,   260,  -130,   234,  -130,  -130,  -130,  -130,
     118,   235,    37,    38,    51,   265,   243,   154,   244,   136,
     124,   270,   113,   245,   128,   128,   119,   147,   253,   121,
     161,   162,   261,    39,    12,    40,   119,   119,   129,   129,
     195,   196,   197,   198,   199,   200,   113,   129,   179,   263,
      24,  -130,    17,    31,   129,    59,   242,   251,   -83,   238,
     208,   232,   130,   131,    55,   119,   -83,     0,   128,   128,
     129,   129,   129,   129,   129,   129,   228,   128,   128,   -83,
     -83,   -83,     0,  -127,   128,  -127,  -127,  -127,  -127,   189,
     190,   163,   164,   165,   166,   167,   168,   175,     0,     0,
     128,   128,   128,   128,   128,   128,   172,   173,   136,     0,
       0,     0,   132,     0,     0,   172,     0,     0,   136,     0,
       0,     0,   172,    32,    33,     0,     0,     0,     0,    34,
    -127,    35,   149,     0,   150,   151,   152,   153,   202,   203,
     204,   205,   206,   207,   136,   129,   158,   159,     0,   136,
      36,   136,     0,   136,     0,   129,   129,   136,     0,     0,
      37,    38,   136,   104,     0,   114,   114,   136,     0,   120,
      32,    33,     0,     0,     0,   128,    34,   114,    35,   154,
       0,    39,    54,    40,   129,   128,   128,   149,     0,   150,
     151,   152,   153,     0,     0,     0,     0,    36,     0,     0,
     214,   215,   216,   217,   218,   219,     0,    37,    38,     0,
     157,   114,   114,   172,   128,     0,   226,   230,     0,     0,
       0,     0,     0,   172,   172,     0,     0,     0,    39,     0,
      40,     0,     0,  -128,   154,  -128,  -128,  -128,  -128,     0,
       0,     0,   193,   193,   193,   193,   193,   193,   193,     0,
       0,     0,   266,     0,     0,     0,     0,   104,     0,     0,
       0,     0,     0,   114,   114,   114,   114,   114,   114,   114,
     114,     0,     0,     0,     0,     0,     0,     0,   114,   193,
    -128,   255,   114,     0,    60,    61,    62,  -112,    63,    32,
      33,    64,  -112,     0,    65,    34,    66,    35,     0,     0,
       0,     0,     0,     0,   142,   143,   144,   145,   146,   147,
       0,   148,    67,     0,     0,     0,    36,     0,     0,     0,
       0,     0,   114,    68,     0,    69,    37,    38,     0,     0,
    -112,     0,    70,    71,     0,     0,     0,    72,    73,    74,
      75,     0,     0,     0,     0,     0,   114,    39,     0,    40,
      60,    61,    62,     0,    63,    32,    33,    64,     0,     0,
     135,    34,    66,    35,     0,     0,     0,     0,    60,    61,
      62,     0,    63,    32,    33,    64,     0,     0,    67,    34,
      66,    35,    36,     0,   176,     0,     0,     0,     0,    68,
       0,    69,    37,    38,     0,     0,    67,     0,    70,    71,
      36,     0,     0,    72,    73,    74,    75,    68,     0,    69,
      37,    38,     0,    39,     0,    40,    70,    71,     0,     0,
       0,    72,    73,    74,    75,     0,     0,     0,     0,     0,
       0,    39,     0,    40,    60,    61,    62,     0,    63,    32,
      33,    64,     0,     0,   210,    34,    66,    35,     0,     0,
       0,     0,    60,    61,    62,     0,    63,    32,    33,    64,
       0,     0,    67,    34,    66,    35,    36,     0,     0,     0,
       0,     0,     0,    68,     0,    69,    37,    38,     0,     0,
      67,     0,    70,    71,    36,     0,     0,    72,    73,    74,
      75,    68,     0,    69,    37,    38,     0,    39,     0,    40,
      70,    71,     0,     0,     0,    72,    73,    74,    75,     0,
       0,     0,   220,     0,     0,    39,     0,    40,    60,    61,
      62,     0,    63,    32,    33,    64,     0,     0,   236,    34,
      66,    35,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    67,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,    68,     0,    69,
      37,    38,     0,     0,     0,     0,    70,    71,     0,     0,
       0,    72,    73,    74,    75,     0,     0,     0,     0,     0,
       0,    39,     0,    40,    60,    61,    62,     0,    63,    32,
      33,    64,     0,     0,   246,    34,    66,    35,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    67,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    68,     0,    69,    37,    38,     0,     0,
       0,     0,    70,    71,     0,     0,     0,    72,    73,    74,
      75,     0,     0,     0,     0,     0,     0,    39,     0,    40,
      60,    61,    62,     0,    63,    32,    33,    64,     0,     0,
     257,    34,    66,    35,     0,     0,     0,     0,    60,    61,
      62,     0,    63,    32,    33,    64,   262,     0,    67,    34,
      66,    35,    36,     0,     0,     0,     0,     0,     0,    68,
       0,    69,    37,    38,     0,     0,    67,     0,    70,    71,
      36,     0,     0,    72,    73,    74,    75,    68,     0,    69,
      37,    38,     0,    39,     0,    40,    70,    71,     0,     0,
       0,    72,    73,    74,    75,     0,     0,     0,     0,     0,
       0,    39,     0,    40,    60,    61,    62,     0,    63,    32,
      33,    64,     0,     0,   264,    34,    66,    35,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    67,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    68,     0,    69,    37,    38,     0,     0,
       0,     0,    70,    71,     0,     0,     0,    72,    73,    74,
      75,     0,     0,     0,     0,     0,     0,    39,     0,    40,
      60,    61,    62,     0,    63,    32,    33,    64,     0,     0,
     267,    34,    66,    35,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    67,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,    68,
       0,    69,    37,    38,     0,     0,     0,     0,    70,    71,
       0,     0,     0,    72,    73,    74,    75,     0,     0,     0,
       0,     0,     0,    39,     0,    40,    60,    61,    62,     0,
      63,    32,    33,    64,     0,     0,   269,    34,    66,    35,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    67,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    68,     0,    69,    37,    38,
       0,     0,     0,     0,    70,    71,     0,     0,     0,    72,
      73,    74,    75,     0,     0,     0,     0,     0,     0,    39,
       0,    40,    60,    61,    62,     0,    63,    32,    33,    64,
       0,     0,   271,    34,    66,    35,     0,     0,     0,     0,
      60,    61,    62,     0,    63,    32,    33,    64,     0,     0,
      67,    34,    66,    35,    36,     0,     0,     0,     0,     0,
       0,    68,     0,    69,    37,    38,     0,     0,    67,     0,
      70,    71,    36,     0,     0,    72,    73,    74,    75,    68,
       0,    69,    37,    38,     0,    39,     0,    40,    70,    71,
       0,    32,    33,    72,    73,    74,    75,    34,    66,    35,
      32,    33,     0,    39,     0,    40,    34,    66,    35,     0,
       0,     0,     0,     0,    67,     0,     0,     0,    36,     0,
       0,     0,     0,   127,     0,     0,     0,    36,    37,    38,
       0,     0,     0,     0,    70,    71,     0,    37,    38,     0,
       0,     0,     0,    70,    71,     0,     0,     0,     0,    39,
       0,    40,     0,     0,     0,     0,     0,     0,    39,     0,
      40
};

static const yytype_int16 yycheck[] =
{
      29,   126,    68,    12,    15,     6,    64,     3,   133,    38,
      39,    11,   222,    19,    19,    44,    19,    46,    18,    15,
      20,    21,    22,    23,    53,    24,    32,    32,   238,    32,
       8,     9,    61,    15,    63,    64,    14,    15,    16,    18,
      51,    20,    21,    22,    23,    31,    75,    48,    19,    24,
     108,    68,    31,    31,    63,    51,    52,    35,    49,    19,
      46,    32,    41,    62,    24,    65,     0,    45,    46,    51,
      52,   137,    19,    51,    52,    11,    54,    55,    18,   108,
     109,   110,    22,    23,    48,    32,    65,    62,    66,    53,
      68,    50,    50,    61,    53,    63,    64,    33,    34,    67,
      14,    69,    62,   161,   162,    73,    74,    75,    11,   234,
     235,   140,   141,   142,   143,   144,   145,   146,   176,    18,
      15,    20,    21,    22,    23,    65,   155,    32,    33,    34,
      33,    34,   161,   162,   163,   164,   165,   166,   167,   168,
     108,   109,   110,   160,    19,    11,    31,   176,   177,   117,
     118,   180,   169,    13,    15,    15,    41,    32,   126,   127,
      60,    61,   220,    33,    34,   133,    65,    33,    34,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,    15,
      11,   149,   150,   151,   152,   153,   154,   155,    19,    24,
      15,   220,     6,   161,   162,   163,   164,   165,   166,   167,
     168,    32,    33,    34,    77,     8,     9,    31,   176,   177,
      15,    14,   180,    16,   231,   244,    15,    19,    67,    41,
      31,    41,   239,    32,    73,    74,   243,    60,   245,    15,
      15,    32,    35,   250,    18,    31,    20,    21,    22,    23,
      46,    31,    45,    46,    47,   262,    19,    65,    64,   122,
      69,   268,   220,    11,    73,    74,   224,    41,    13,   108,
      33,    34,    13,    66,     8,    68,   234,   235,   117,   118,
     141,   142,   143,   144,   145,   146,   244,   126,   127,    19,
      18,    65,    13,    25,   133,    56,   224,   238,    11,   221,
     155,   181,    73,    74,    46,   263,    19,    -1,   117,   118,
     149,   150,   151,   152,   153,   154,   177,   126,   127,    32,
      33,    34,    -1,    18,   133,    20,    21,    22,    23,   138,
     139,    25,    26,    27,    28,    29,    30,    32,    -1,    -1,
     149,   150,   151,   152,   153,   154,   117,   118,   211,    -1,
      -1,    -1,    75,    -1,    -1,   126,    -1,    -1,   221,    -1,
      -1,    -1,   133,     8,     9,    -1,    -1,    -1,    -1,    14,
      65,    16,    18,    -1,    20,    21,    22,    23,   149,   150,
     151,   152,   153,   154,   247,   224,   109,   110,    -1,   252,
      35,   254,    -1,   256,    -1,   234,   235,   260,    -1,    -1,
      45,    46,   265,    61,    -1,    63,    64,   270,    -1,    67,
       8,     9,    -1,    -1,    -1,   224,    14,    75,    16,    65,
      -1,    66,    67,    68,   263,   234,   235,    18,    -1,    20,
      21,    22,    23,    -1,    -1,    -1,    -1,    35,    -1,    -1,
     163,   164,   165,   166,   167,   168,    -1,    45,    46,    -1,
     108,   109,   110,   224,   263,    -1,    47,   180,    -1,    -1,
      -1,    -1,    -1,   234,   235,    -1,    -1,    -1,    66,    -1,
      68,    -1,    -1,    18,    65,    20,    21,    22,    23,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,   263,    -1,    -1,    -1,    -1,   155,    -1,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,   166,   167,
     168,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,
      65,   244,   180,    -1,     3,     4,     5,    19,     7,     8,
       9,    10,    24,    -1,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    37,    38,    39,    40,    41,
      -1,    43,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,   220,    42,    -1,    44,    45,    46,    -1,    -1,
      62,    -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,
      59,    -1,    -1,    -1,    -1,    -1,   244,    66,    -1,    68,
       3,     4,     5,    -1,     7,     8,     9,    10,    -1,    -1,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,     3,     4,
       5,    -1,     7,     8,     9,    10,    -1,    -1,    31,    14,
      15,    16,    35,    -1,    19,    -1,    -1,    -1,    -1,    42,
      -1,    44,    45,    46,    -1,    -1,    31,    -1,    51,    52,
      35,    -1,    -1,    56,    57,    58,    59,    42,    -1,    44,
      45,    46,    -1,    66,    -1,    68,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    68,     3,     4,     5,    -1,     7,     8,
       9,    10,    -1,    -1,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,     3,     4,     5,    -1,     7,     8,     9,    10,
      -1,    -1,    31,    14,    15,    16,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    44,    45,    46,    -1,    -1,
      31,    -1,    51,    52,    35,    -1,    -1,    56,    57,    58,
      59,    42,    -1,    44,    45,    46,    -1,    66,    -1,    68,
      51,    52,    -1,    -1,    -1,    56,    57,    58,    59,    -1,
      -1,    -1,    63,    -1,    -1,    66,    -1,    68,     3,     4,
       5,    -1,     7,     8,     9,    10,    -1,    -1,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    44,
      45,    46,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    68,     3,     4,     5,    -1,     7,     8,
       9,    10,    -1,    -1,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    44,    45,    46,    -1,    -1,
      -1,    -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    68,
       3,     4,     5,    -1,     7,     8,     9,    10,    -1,    -1,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,     3,     4,
       5,    -1,     7,     8,     9,    10,    11,    -1,    31,    14,
      15,    16,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    44,    45,    46,    -1,    -1,    31,    -1,    51,    52,
      35,    -1,    -1,    56,    57,    58,    59,    42,    -1,    44,
      45,    46,    -1,    66,    -1,    68,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    68,     3,     4,     5,    -1,     7,     8,
       9,    10,    -1,    -1,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    44,    45,    46,    -1,    -1,
      -1,    -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    68,
       3,     4,     5,    -1,     7,     8,     9,    10,    -1,    -1,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    44,    45,    46,    -1,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    57,    58,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    68,     3,     4,     5,    -1,
       7,     8,     9,    10,    -1,    -1,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    44,    45,    46,
      -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,
      57,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    68,     3,     4,     5,    -1,     7,     8,     9,    10,
      -1,    -1,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
       3,     4,     5,    -1,     7,     8,     9,    10,    -1,    -1,
      31,    14,    15,    16,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    -1,    44,    45,    46,    -1,    -1,    31,    -1,
      51,    52,    35,    -1,    -1,    56,    57,    58,    59,    42,
      -1,    44,    45,    46,    -1,    66,    -1,    68,    51,    52,
      -1,     8,     9,    56,    57,    58,    59,    14,    15,    16,
       8,     9,    -1,    66,    -1,    68,    14,    15,    16,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    45,    46,
      -1,    -1,    -1,    -1,    51,    52,    -1,    45,    46,    -1,
      -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    -1,    66,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      68
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    49,    70,    71,    15,    51,     0,    50,    72,    73,
      14,    53,    73,    74,    75,    15,    48,    75,    77,    78,
      15,    76,    15,     6,    78,    79,    80,    13,    15,    24,
      15,    80,     8,     9,    14,    16,    35,    45,    46,    66,
      68,   111,    31,   111,   114,   111,   112,   113,    15,    81,
      82,    47,   111,    41,    67,   113,    19,    32,   111,    82,
       3,     4,     5,     7,    10,    13,    15,    31,    42,    44,
      51,    52,    56,    57,    58,    59,    83,    86,    87,    88,
      89,    90,    91,    92,    98,   101,   102,   103,   105,   106,
     107,   108,   109,   110,   111,   115,   116,   117,   118,   119,
      15,    99,   100,   107,   108,   111,     8,     9,    31,    54,
      55,    96,    97,   107,   108,   111,    96,    31,    46,   107,
     108,   109,    86,   106,    83,   107,    31,    31,    83,   109,
     110,   110,    97,    31,    41,    13,    87,    19,    24,    62,
      24,    62,    36,    37,    38,    39,    40,    41,    43,    18,
      20,    21,    22,    23,    65,    19,    96,   108,    97,    97,
      11,    33,    34,    25,    26,    27,    28,    29,    30,    11,
      84,    85,   110,   110,    32,    32,    19,    24,    84,   109,
      60,   120,   121,    84,    15,    15,    51,   106,   107,    83,
      83,   104,   107,   108,   111,   104,   104,   104,   104,   104,
     104,    15,   110,   110,   110,   110,   110,   110,   100,    32,
      13,    86,    96,    96,    97,    97,    97,    97,    97,    97,
      63,    86,    93,    94,    19,    32,    47,    96,   104,    32,
      97,    61,   121,    32,    31,    31,    13,    96,    93,    12,
      94,    95,    85,    19,    64,    11,    13,    86,    84,    84,
      11,    95,    86,    13,    86,    97,    86,    13,    32,    32,
      86,    13,    11,    19,    13,    86,   110,    13,    11,    13,
      86,    13
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 184 "bison.y"
    {
	;}
    break;

  case 4:
#line 188 "bison.y"
    {
		FKLOG("[bison]: package %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 5:
#line 195 "bison.y"
    {
		FKLOG("[bison]: package %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 6:
#line 203 "bison.y"
    {
	;}
    break;

  case 9:
#line 213 "bison.y"
    {
		FKLOG("[bison]: include %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_include((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 10:
#line 222 "bison.y"
    {
	;}
    break;

  case 13:
#line 232 "bison.y"
    {
		FKLOG("[bison]: struct_define %s", (yyvsp[(2) - (4)].str).c_str());
		myflexer *l = (myflexer *)parm;
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[(3) - (4)].syntree));
		l->add_struct_desc((yyvsp[(2) - (4)].str).c_str(), p);
	;}
    break;

  case 14:
#line 242 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 15:
#line 247 "bison.y"
    {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER struct_mem_declaration");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_struct_memlist);
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[(1) - (2)].syntree));
		p->add_arg((yyvsp[(2) - (2)].str));
		(yyval.syntree) = p;
	;}
    break;

  case 16:
#line 256 "bison.y"
    {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER");
		NEWTYPE(p, struct_desc_memlist_node);
		p->add_arg((yyvsp[(1) - (1)].str));
		(yyval.syntree) = p;
	;}
    break;

  case 17:
#line 266 "bison.y"
    {
	;}
    break;

  case 20:
#line 276 "bison.y"
    {
		FKLOG("[bison]: const_define %s", (yyvsp[(2) - (4)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_const_desc((yyvsp[(2) - (4)].str).c_str(), (yyvsp[(4) - (4)].syntree));
	;}
    break;

  case 21:
#line 285 "bison.y"
    {
	;}
    break;

  case 24:
#line 297 "bison.y"
    {
		FKLOG("[bison]: function_declaration <- block %s %d", (yyvsp[(2) - (7)].str).c_str(), yylloc.first_line);
		NEWTYPE(p, func_desc_node);
		p->funcname = (yyvsp[(2) - (7)].str);
		p->arglist = dynamic_cast<func_desc_arglist_node*>((yyvsp[(4) - (7)].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[(6) - (7)].syntree));
		p->endline = yylloc.first_line;
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	;}
    break;

  case 25:
#line 309 "bison.y"
    {
		FKLOG("[bison]: function_declaration <- empty %s %d", (yyvsp[(2) - (6)].str).c_str(), yylloc.first_line);
		NEWTYPE(p, func_desc_node);
		p->funcname = (yyvsp[(2) - (6)].str);
		p->arglist = 0;
		p->block = 0;
		p->endline = yylloc.first_line;
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	;}
    break;

  case 26:
#line 323 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 27:
#line 328 "bison.y"
    {
		FKLOG("[bison]: function_declaration_arguments <- arg function_declaration_arguments");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_arglist);
		func_desc_arglist_node * p = dynamic_cast<func_desc_arglist_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 28:
#line 337 "bison.y"
    {
		FKLOG("[bison]: function_declaration_arguments <- arg");
		NEWTYPE(p, func_desc_arglist_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 29:
#line 347 "bison.y"
    {
		FKLOG("[bison]: arg <- IDENTIFIER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, identifier_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 30:
#line 357 "bison.y"
    {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[(1) - (4)].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[(1) - (4)].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[(3) - (4)].syntree));
		p->fakecall = false;
		p->classmem_call = false;
		(yyval.syntree) = p;
	;}
    break;

  case 31:
#line 369 "bison.y"
    {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[(1) - (4)].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[(1) - (4)].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[(3) - (4)].syntree));
		p->fakecall = false;
		p->classmem_call = false;
		(yyval.syntree) = p;
	;}
    break;

  case 32:
#line 381 "bison.y"
    {
		FKLOG("[bison]: function_call <- function_call_arguments");
		NEWTYPE(p, function_call_node);
		p->fuc = "";
		p->prefunc = (yyvsp[(1) - (4)].syntree);
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[(3) - (4)].syntree));
		p->fakecall = false;
		p->classmem_call = false;
		(yyval.syntree) = p;
	;}
    break;

  case 33:
#line 393 "bison.y"
    {
		FKLOG("[bison]: function_call <- mem function_call_arguments %s", (yyvsp[(3) - (6)].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[(3) - (6)].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[(5) - (6)].syntree));
		if (p->arglist == 0)
		{
			NEWTYPE(pa, function_call_arglist_node);
			p->arglist = pa;
		}
		p->arglist->add_arg((yyvsp[(1) - (6)].syntree));
		p->fakecall = false;
		p->classmem_call = true;
		(yyval.syntree) = p;
	;}
    break;

  case 34:
#line 411 "bison.y"
    {
		FKLOG("[bison]: function_call <- mem function_call_arguments %s", (yyvsp[(3) - (6)].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[(3) - (6)].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[(5) - (6)].syntree));
		if (p->arglist == 0)
		{
			NEWTYPE(pa, function_call_arglist_node);
			p->arglist = pa;
		}
		p->arglist->add_arg((yyvsp[(1) - (6)].syntree));
		p->fakecall = false;
		p->classmem_call = true;
		(yyval.syntree) = p;
	;}
    break;

  case 35:
#line 431 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 36:
#line 436 "bison.y"
    {
		FKLOG("[bison]: function_call_arguments <- arg_expr function_call_arguments");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_call_arglist);
		function_call_arglist_node * p = dynamic_cast<function_call_arglist_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 37:
#line 445 "bison.y"
    {
		FKLOG("[bison]: function_call_arguments <- arg_expr");
		NEWTYPE(p, function_call_arglist_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 38:
#line 455 "bison.y"
    {
		FKLOG("[bison]: arg_expr <- expr_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 39:
#line 465 "bison.y"
    {
		FKLOG("[bison]: block <- block stmt");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_block);
		block_node * p = dynamic_cast<block_node*>((yyvsp[(1) - (2)].syntree));
		p->add_stmt((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 40:
#line 474 "bison.y"
    {
		FKLOG("[bison]: block <- stmt");
		NEWTYPE(p, block_node);
		p->add_stmt((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 41:
#line 484 "bison.y"
    {
		FKLOG("[bison]: stmt <- while_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 42:
#line 490 "bison.y"
    {
		FKLOG("[bison]: stmt <- if_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 43:
#line 496 "bison.y"
    {
		FKLOG("[bison]: stmt <- return_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 44:
#line 502 "bison.y"
    {
		FKLOG("[bison]: stmt <- assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 45:
#line 508 "bison.y"
    {
		FKLOG("[bison]: stmt <- multi_assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 46:
#line 514 "bison.y"
    {
		FKLOG("[bison]: stmt <- break");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 47:
#line 520 "bison.y"
    {
		FKLOG("[bison]: stmt <- continue");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 48:
#line 526 "bison.y"
    {
		FKLOG("[bison]: stmt <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 49:
#line 532 "bison.y"
    {
		FKLOG("[bison]: stmt <- math_assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 50:
#line 538 "bison.y"
    {
		FKLOG("[bison]: stmt <- for_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 51:
#line 544 "bison.y"
    {
		FKLOG("[bison]: stmt <- for_loop_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 52:
#line 550 "bison.y"
    {
		FKLOG("[bison]: stmt <- fake_call_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 53:
#line 556 "bison.y"
    {
		FKLOG("[bison]: stmt <- sleep_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 54:
#line 562 "bison.y"
    {
		FKLOG("[bison]: stmt <- yield_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 55:
#line 568 "bison.y"
    {
		FKLOG("[bison]: stmt <- switch_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 56:
#line 576 "bison.y"
    {
		FKLOG("[bison]: fake_call_stmt <- fake function_call");
		function_call_node * p = dynamic_cast<function_call_node*>((yyvsp[(2) - (2)].syntree));
		p->fakecall = true;
		(yyval.syntree) = p;
	;}
    break;

  case 57:
#line 586 "bison.y"
    {
		FKLOG("[bison]: for_stmt <- block cmp block");
		NEWTYPE(p, for_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(4) - (9)].syntree));
		p->beginblock = dynamic_cast<block_node*>((yyvsp[(2) - (9)].syntree));
		p->endblock = dynamic_cast<block_node*>((yyvsp[(6) - (9)].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[(8) - (9)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 58:
#line 597 "bison.y"
    {
		FKLOG("[bison]: for_stmt <- block cmp");
		NEWTYPE(p, for_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(4) - (8)].syntree));
		p->beginblock = dynamic_cast<block_node*>((yyvsp[(2) - (8)].syntree));
		p->endblock = dynamic_cast<block_node*>((yyvsp[(6) - (8)].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 59:
#line 610 "bison.y"
    {
		FKLOG("[bison]: for_loop_stmt <- block");
		NEWTYPE(p, for_stmt);
		
		syntree_node * pi = (yyvsp[(2) - (11)].syntree);
		if (pi->gettype() == est_var)
		{
			NEWTYPE(pvar, variable_node);
			pvar->str = (dynamic_cast<var_node*>(pi))->str;
			pi = pvar;
		}
		
		NEWTYPE(pcmp, cmp_stmt);
		pcmp->cmp = "<";
		pcmp->left = pi;
		pcmp->right = (yyvsp[(6) - (11)].syntree);
		p->cmp = pcmp;
		
		NEWTYPE(pbeginblockassign, assign_stmt);
		pbeginblockassign->var = (yyvsp[(2) - (11)].syntree);
		pbeginblockassign->value = (yyvsp[(4) - (11)].syntree);
		pbeginblockassign->isnew = false;
		NEWTYPE(pbeginblock, block_node);
		pbeginblock->add_stmt(pbeginblockassign);
		p->beginblock = pbeginblock;
		
		NEWTYPE(pendblockassign, math_assign_stmt);
		pendblockassign->var = pi;
		pendblockassign->oper = "+=";
		pendblockassign->value = (yyvsp[(8) - (11)].syntree);
		NEWTYPE(pendblock, block_node);
		pendblock->add_stmt(pendblockassign);
		p->endblock = pendblock;
		
		p->block = dynamic_cast<block_node*>((yyvsp[(10) - (11)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 60:
#line 649 "bison.y"
    {
		FKLOG("[bison]: for_loop_stmt <- empty");
		NEWTYPE(p, for_stmt);
		
		NEWTYPE(pcmp, cmp_stmt);
		pcmp->cmp = "<";
		pcmp->left = (yyvsp[(2) - (10)].syntree);
		pcmp->right = (yyvsp[(6) - (10)].syntree);
		p->cmp = pcmp;
		
		NEWTYPE(pbeginblockassign, assign_stmt);
		pbeginblockassign->var = (yyvsp[(2) - (10)].syntree);
		pbeginblockassign->value = (yyvsp[(4) - (10)].syntree);
		pbeginblockassign->isnew = false;
		NEWTYPE(pbeginblock, block_node);
		pbeginblock->add_stmt(pbeginblockassign);
		p->beginblock = pbeginblock;
		
		NEWTYPE(pendblockassign, math_assign_stmt);
		pendblockassign->var = (yyvsp[(2) - (10)].syntree);
		pendblockassign->oper = "+=";
		pendblockassign->value = (yyvsp[(8) - (10)].syntree);
		NEWTYPE(pendblock, block_node);
		pendblock->add_stmt(pendblockassign);
		p->endblock = pendblock;
		
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 61:
#line 682 "bison.y"
    {
		FKLOG("[bison]: while_stmt <- cmp block");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (5)].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[(4) - (5)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 62:
#line 691 "bison.y"
    {
		FKLOG("[bison]: while_stmt <- cmp");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (4)].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 63:
#line 702 "bison.y"
    {
		FKLOG("[bison]: if_stmt <- cmp block");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (7)].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[(4) - (7)].syntree));
		p->elseifs = dynamic_cast<elseif_stmt_list*>((yyvsp[(5) - (7)].syntree));
		p->elses = dynamic_cast<else_stmt*>((yyvsp[(6) - (7)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 64:
#line 713 "bison.y"
    {
		FKLOG("[bison]: if_stmt <- cmp");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (6)].syntree));
		p->block = 0;
		p->elseifs = dynamic_cast<elseif_stmt_list*>((yyvsp[(4) - (6)].syntree));
		p->elses = dynamic_cast<else_stmt*>((yyvsp[(5) - (6)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 65:
#line 726 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 66:
#line 731 "bison.y"
    {
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt_list elseif_stmt");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_elseif_stmt_list);
		elseif_stmt_list * p = dynamic_cast<elseif_stmt_list*>((yyvsp[(1) - (2)].syntree));
		p->add_stmt((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 67:
#line 740 "bison.y"
    {
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt");
		NEWTYPE(p, elseif_stmt_list);
		p->add_stmt((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 68:
#line 750 "bison.y"
    {
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (4)].syntree));
		p->block = (yyvsp[(4) - (4)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 69:
#line 759 "bison.y"
    {
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (3)].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 70:
#line 770 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 71:
#line 775 "bison.y"
    {
		FKLOG("[bison]: else_stmt <- block");
		NEWTYPE(p, else_stmt);
		p->block = dynamic_cast<block_node*>((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 72:
#line 783 "bison.y"
    {
		FKLOG("[bison]: else_stmt <- empty");
		NEWTYPE(p, else_stmt);
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 73:
#line 793 "bison.y"
    {
		FKLOG("[bison]: cmp <- ( cmp )");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 74:
#line 799 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp AND cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "&&";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 75:
#line 809 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp OR cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "||";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 76:
#line 819 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value LESS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 77:
#line 829 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value MORE cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 78:
#line 839 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 79:
#line 849 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value MORE_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 80:
#line 859 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value LESS_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 81:
#line 869 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value NOT_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 82:
#line 879 "bison.y"
    {
		FKLOG("[bison]: cmp <- true");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "true";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 83:
#line 889 "bison.y"
    {
		FKLOG("[bison]: cmp <- false");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "false";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 84:
#line 899 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value IS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "is";
		p->left = (yyvsp[(2) - (2)].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 85:
#line 909 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value NOT cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "not";
		p->left = (yyvsp[(2) - (2)].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 86:
#line 921 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 87:
#line 927 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 88:
#line 933 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 89:
#line 941 "bison.y"
    {
		FKLOG("[bison]: return_stmt <- RETURN return_value_list");
		NEWTYPE(p, return_stmt);
		p->returnlist = dynamic_cast<return_value_list_node*>((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 90:
#line 949 "bison.y"
    {
		FKLOG("[bison]: return_stmt <- RETURN");
		NEWTYPE(p, return_stmt);
		p->returnlist = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 91:
#line 959 "bison.y"
    {
		FKLOG("[bison]: return_value_list <- return_value_list return_value");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_return_value_list);
		return_value_list_node * p = dynamic_cast<return_value_list_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 92:
#line 968 "bison.y"
    {
		NEWTYPE(p, return_value_list_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 93:
#line 977 "bison.y"
    {
		FKLOG("[bison]: return_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 94:
#line 983 "bison.y"
    {
		FKLOG("[bison]: return_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 95:
#line 989 "bison.y"
    {
		FKLOG("[bison]: return_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 96:
#line 997 "bison.y"
    {
		FKLOG("[bison]: assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->value = (yyvsp[(3) - (3)].syntree);
		p->isnew = false;
		(yyval.syntree) = p;
	;}
    break;

  case 97:
#line 1007 "bison.y"
    {
		FKLOG("[bison]: new assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->value = (yyvsp[(3) - (3)].syntree);
		p->isnew = true;
		(yyval.syntree) = p;
	;}
    break;

  case 98:
#line 1019 "bison.y"
    {
		FKLOG("[bison]: multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>((yyvsp[(1) - (3)].syntree));
		p->value = (yyvsp[(3) - (3)].syntree);
		p->isnew = false;
		(yyval.syntree) = p;
	;}
    break;

  case 99:
#line 1029 "bison.y"
    {
		FKLOG("[bison]: new multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>((yyvsp[(1) - (3)].syntree));
		p->value = (yyvsp[(3) - (3)].syntree);
		p->isnew = true;
		(yyval.syntree) = p;
	;}
    break;

  case 100:
#line 1041 "bison.y"
    {
		FKLOG("[bison]: var_list <- var_list var");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_var_list);
		var_list_node * p = dynamic_cast<var_list_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 101:
#line 1050 "bison.y"
    {
		NEWTYPE(p, var_list_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 102:
#line 1059 "bison.y"
    {
		FKLOG("[bison]: assign_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 103:
#line 1065 "bison.y"
    {
		FKLOG("[bison]: assign_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 104:
#line 1071 "bison.y"
    {
		FKLOG("[bison]: assign_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 105:
#line 1079 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "+=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 106:
#line 1089 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "-=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 107:
#line 1099 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "/=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 108:
#line 1109 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "*=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 109:
#line 1119 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "%=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 110:
#line 1129 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable INC");
		NEWTYPE(pp, explicit_value_node);
		pp->str = "1";
		pp->type = explicit_value_node::EVT_NUM;
		
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (2)].syntree);
		p->oper = "+=";
		p->value = pp;
		(yyval.syntree) = p;
	;}
    break;

  case 111:
#line 1145 "bison.y"
    {
		FKLOG("[bison]: var <- VAR_BEGIN IDENTIFIER %s", (yyvsp[(2) - (2)].str).c_str());
		NEWTYPE(p, var_node);
		p->str = (yyvsp[(2) - (2)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 112:
#line 1153 "bison.y"
    {
		FKLOG("[bison]: var <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 113:
#line 1161 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 114:
#line 1169 "bison.y"
    {
		FKLOG("[bison]: container_get_node <- IDENTIFIER[expr_value] %s", (yyvsp[(1) - (4)].str).c_str());
		NEWTYPE(p, container_get_node);
		p->container = (yyvsp[(1) - (4)].str);
		p->key = (yyvsp[(3) - (4)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 115:
#line 1178 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER_POINTER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, struct_pointer_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 116:
#line 1186 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER_DOT %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 117:
#line 1196 "bison.y"
    {
		FKLOG("[bison]: expr <- (expr)");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 118:
#line 1202 "bison.y"
    {
		FKLOG("[bison]: expr <- function_call");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 119:
#line 1208 "bison.y"
    {
		FKLOG("[bison]: expr <- math_expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 120:
#line 1216 "bison.y"
    {
		FKLOG("[bison]: math_expr <- (math_expr)");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 121:
#line 1222 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "+";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 122:
#line 1232 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "-";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 123:
#line 1242 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "*";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 124:
#line 1252 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "/";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 125:
#line 1262 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "%";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 126:
#line 1272 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "..";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 127:
#line 1284 "bison.y"
    {
		FKLOG("[bison]: expr_value <- math_expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 128:
#line 1290 "bison.y"
    {
		FKLOG("[bison]: expr_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 129:
#line 1296 "bison.y"
    {
		FKLOG("[bison]: expr_value <- function_call");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 130:
#line 1302 "bison.y"
    {
		FKLOG("[bison]: expr_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 131:
#line 1310 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FTRUE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_TRUE;
		(yyval.syntree) = p;
	;}
    break;

  case 132:
#line 1319 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FFALSE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_FALSE;
		(yyval.syntree) = p;
	;}
    break;

  case 133:
#line 1328 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- NUMBER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_NUM;
		(yyval.syntree) = p;
	;}
    break;

  case 134:
#line 1337 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FKUUID %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_UUID;
		(yyval.syntree) = p;
	;}
    break;

  case 135:
#line 1346 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- STRING_DEFINITION %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_STR;
		(yyval.syntree) = p;
	;}
    break;

  case 136:
#line 1355 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FKFLOAT %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_FLOAT;
		(yyval.syntree) = p;
	;}
    break;

  case 137:
#line 1364 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FNULL %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_NULL;
		(yyval.syntree) = p;
	;}
    break;

  case 138:
#line 1373 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- const_map_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_MAP;
		p->v = (yyvsp[(2) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 139:
#line 1383 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- const_array_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_ARRAY;
		p->v = (yyvsp[(2) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 140:
#line 1395 "bison.y"
    {
		FKLOG("[bison]: const_map_list_value <- null");
		NEWTYPE(p, const_map_list_value_node);
		(yyval.syntree) = p;
	;}
    break;

  case 141:
#line 1402 "bison.y"
    {
		FKLOG("[bison]: const_map_list_value <- const_map_value");
		NEWTYPE(p, const_map_list_value_node);
		p->add_ele((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 142:
#line 1410 "bison.y"
    {
		FKLOG("[bison]: const_map_list_value <- const_map_list_value const_map_value");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_constmaplist);
		const_map_list_value_node * p = dynamic_cast<const_map_list_value_node*>((yyvsp[(1) - (2)].syntree));
		p->add_ele((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 143:
#line 1421 "bison.y"
    {
		FKLOG("[bison]: const_map_value <- explicit_value");
		NEWTYPE(p, const_map_value_node);
		p->k = (yyvsp[(1) - (3)].syntree);
		p->v = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 144:
#line 1432 "bison.y"
    {
		FKLOG("[bison]: const_array_list_value <- null");
		NEWTYPE(p, const_array_list_value_node);
		(yyval.syntree) = p;
	;}
    break;

  case 145:
#line 1439 "bison.y"
    {
		FKLOG("[bison]: const_array_list_value <- explicit_value");
		NEWTYPE(p, const_array_list_value_node);
		p->add_ele((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 146:
#line 1447 "bison.y"
    {
		FKLOG("[bison]: const_array_list_value <- const_array_list_value explicit_value");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_constarraylist);
		const_array_list_value_node * p = dynamic_cast<const_array_list_value_node*>((yyvsp[(1) - (2)].syntree));
		p->add_ele((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 147:
#line 1458 "bison.y"
    {
		FKLOG("[bison]: break <- BREAK");
		NEWTYPE(p, break_stmt);
		(yyval.syntree) = p;
	;}
    break;

  case 148:
#line 1467 "bison.y"
    {
		FKLOG("[bison]: CONTINUE");
		NEWTYPE(p, continue_stmt);
		(yyval.syntree) = p;
	;}
    break;

  case 149:
#line 1476 "bison.y"
    {
		FKLOG("[bison]: SLEEP");
		NEWTYPE(p, sleep_stmt);
		p->time = (yyvsp[(2) - (2)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 150:
#line 1485 "bison.y"
    {
		FKLOG("[bison]: YIELD");
		NEWTYPE(p, yield_stmt);
		p->time = (yyvsp[(2) - (2)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 151:
#line 1495 "bison.y"
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[(2) - (6)].syntree);
		p->caselist = (yyvsp[(3) - (6)].syntree);
		p->def = (yyvsp[(5) - (6)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 152:
#line 1505 "bison.y"
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[(2) - (5)].syntree);
		p->caselist = (yyvsp[(3) - (5)].syntree);
		p->def = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 153:
#line 1517 "bison.y"
    {
		FKLOG("[bison]: switch_case_list <- switch_case_define");
		NEWTYPE(p, switch_caselist_node);
		p->add_case((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 154:
#line 1525 "bison.y"
    {
		FKLOG("[bison]: switch_case_list <- switch_case_list switch_case_define");
		assert((yyvsp[(2) - (2)].syntree)->gettype() == est_switch_case_node);
		switch_caselist_node * p = dynamic_cast<switch_caselist_node*>((yyvsp[(1) - (2)].syntree));
		p->add_case((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 155:
#line 1536 "bison.y"
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[(2) - (4)].syntree);
		p->block = (yyvsp[(4) - (4)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 156:
#line 1545 "bison.y"
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[(2) - (3)].syntree);
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;


/* Line 1267 of yacc.c.  */
#line 3536 "bison.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



