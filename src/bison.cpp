
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
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
	int ret = l->yylex(lvalp, loc);
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
	


/* Line 189 of yacc.c  */
#line 108 "bison.tab.c"

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
     CLOSE_BIG_BRACKET = 322
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
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


/* Line 264 of yacc.c  */
#line 229 "bison.tab.c"

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
# if YYENABLE_NLS
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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1181

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  68
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  53
/* YYNRULES -- Number of rules.  */
#define YYNRULES  153
/* YYNRULES -- Number of states.  */
#define YYNSTATES  263

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

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
      65,    66,    67
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     9,    10,    13,    16,    17,    19,    22,
      25,    26,    28,    31,    36,    37,    40,    42,    43,    45,
      48,    53,    54,    56,    59,    67,    74,    75,    79,    81,
      83,    88,    93,   100,   101,   105,   107,   109,   112,   114,
     116,   118,   120,   122,   124,   126,   128,   130,   132,   134,
     136,   138,   140,   142,   144,   147,   157,   166,   178,   189,
     195,   200,   208,   215,   216,   219,   221,   226,   230,   231,
     234,   236,   240,   244,   248,   252,   256,   260,   264,   268,
     272,   274,   276,   279,   282,   284,   286,   288,   291,   293,
     297,   299,   301,   303,   305,   309,   313,   317,   321,   325,
     327,   329,   331,   333,   337,   341,   345,   349,   353,   356,
     359,   361,   363,   368,   370,   372,   376,   378,   380,   384,
     388,   392,   396,   400,   404,   408,   410,   412,   414,   416,
     418,   420,   422,   424,   426,   428,   432,   436,   437,   439,
     442,   446,   447,   449,   452,   454,   456,   459,   462,   469,
     475,   477,   480,   485
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      69,     0,    -1,    70,    71,    73,    76,    78,    -1,    -1,
      49,    15,    -1,    49,    51,    -1,    -1,    72,    -1,    71,
      72,    -1,    50,    14,    -1,    -1,    74,    -1,    73,    74,
      -1,    53,    15,    75,    13,    -1,    -1,    75,    15,    -1,
      15,    -1,    -1,    77,    -1,    76,    77,    -1,    48,    15,
      24,   110,    -1,    -1,    79,    -1,    78,    79,    -1,     6,
      15,    31,    80,    32,    85,    13,    -1,     6,    15,    31,
      80,    32,    13,    -1,    -1,    80,    19,    81,    -1,    81,
      -1,    15,    -1,    15,    31,    83,    32,    -1,    51,    31,
      83,    32,    -1,   106,    41,    15,    31,    83,    32,    -1,
      -1,    83,    19,    84,    -1,    84,    -1,   109,    -1,    85,
      86,    -1,    86,    -1,    90,    -1,    91,    -1,    97,    -1,
     100,    -1,   101,    -1,   114,    -1,   115,    -1,   107,    -1,
     104,    -1,    88,    -1,    89,    -1,    87,    -1,   116,    -1,
     117,    -1,   118,    -1,    44,    82,    -1,    42,    85,    19,
      95,    19,    85,    11,    85,    13,    -1,    42,    85,    19,
      95,    19,    85,    11,    13,    -1,    42,   105,    24,   103,
      64,    96,    19,   109,    11,    85,    13,    -1,    42,   105,
      24,   103,    64,    96,    19,   109,    11,    13,    -1,     7,
      95,    11,    85,    13,    -1,     7,    95,    11,    13,    -1,
      10,    95,    11,    85,    92,    94,    13,    -1,    10,    95,
      11,    92,    94,    13,    -1,    -1,    92,    93,    -1,    93,
      -1,    63,    95,    11,    85,    -1,    63,    95,    11,    -1,
      -1,    12,    85,    -1,    12,    -1,    31,    95,    32,    -1,
      95,    33,    95,    -1,    95,    34,    95,    -1,    96,    26,
      96,    -1,    96,    25,    96,    -1,    96,    29,    96,    -1,
      96,    27,    96,    -1,    96,    28,    96,    -1,    96,    30,
      96,    -1,     8,    -1,     9,    -1,    54,    96,    -1,    55,
      96,    -1,   110,    -1,   106,    -1,   107,    -1,     4,    98,
      -1,     4,    -1,    98,    19,    99,    -1,    99,    -1,   110,
      -1,   106,    -1,   107,    -1,   105,    24,   103,    -1,   105,
      62,   103,    -1,   102,    24,    82,    -1,   102,    62,    82,
      -1,   102,    19,   105,    -1,   105,    -1,   110,    -1,   106,
      -1,   107,    -1,   106,    36,   103,    -1,   106,    37,   103,
      -1,   106,    38,   103,    -1,   106,    39,   103,    -1,   106,
      40,   103,    -1,   106,    43,    -1,     3,    15,    -1,   106,
      -1,    15,    -1,    15,    46,   109,    47,    -1,    52,    -1,
      51,    -1,    31,   107,    32,    -1,    82,    -1,   108,    -1,
      31,   108,    32,    -1,   109,    20,   109,    -1,   109,    21,
     109,    -1,   109,    23,   109,    -1,   109,    22,   109,    -1,
     109,    18,   109,    -1,   109,    65,   109,    -1,   108,    -1,
     110,    -1,    82,    -1,   106,    -1,     8,    -1,     9,    -1,
      16,    -1,    45,    -1,    14,    -1,    35,    -1,    66,   111,
      67,    -1,    46,   113,    47,    -1,    -1,   112,    -1,   111,
     112,    -1,   110,    41,   110,    -1,    -1,   110,    -1,   113,
     110,    -1,     5,    -1,    56,    -1,    58,   109,    -1,    57,
     109,    -1,    59,    96,   119,    61,    85,    13,    -1,    59,
      96,   119,    61,    13,    -1,   120,    -1,   119,   120,    -1,
      60,    96,    11,    85,    -1,    60,    96,    11,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   173,   173,   182,   185,   192,   201,   204,   206,   210,
     220,   223,   225,   229,   240,   244,   253,   264,   267,   269,
     273,   283,   286,   288,   294,   306,   321,   325,   334,   344,
     354,   365,   376,   396,   400,   409,   419,   429,   438,   448,
     454,   460,   466,   472,   478,   484,   490,   496,   502,   508,
     514,   520,   526,   532,   540,   550,   561,   574,   586,   600,
     609,   620,   631,   645,   649,   658,   668,   677,   689,   693,
     701,   711,   717,   727,   737,   747,   757,   767,   777,   787,
     797,   807,   817,   827,   839,   845,   851,   859,   867,   877,
     886,   895,   901,   907,   915,   925,   937,   947,   959,   968,
     977,   983,   989,   997,  1007,  1017,  1027,  1037,  1047,  1063,
    1071,  1079,  1087,  1096,  1104,  1114,  1120,  1126,  1134,  1140,
    1150,  1160,  1170,  1180,  1190,  1202,  1208,  1214,  1220,  1228,
    1237,  1246,  1255,  1264,  1273,  1282,  1292,  1305,  1311,  1319,
    1330,  1342,  1348,  1356,  1367,  1376,  1385,  1394,  1404,  1414,
    1426,  1434,  1445,  1454
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
  "CLOSE_BIG_BRACKET", "$accept", "program", "package_head",
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
     315,   316,   317,   318,   319,   320,   321,   322
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    68,    69,    70,    70,    70,    71,    71,    71,    72,
      73,    73,    73,    74,    75,    75,    75,    76,    76,    76,
      77,    78,    78,    78,    79,    79,    80,    80,    80,    81,
      82,    82,    82,    83,    83,    83,    84,    85,    85,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    87,    88,    88,    89,    89,    90,
      90,    91,    91,    92,    92,    92,    93,    93,    94,    94,
      94,    95,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    96,    96,    96,    97,    97,    98,
      98,    99,    99,    99,   100,   100,   101,   101,   102,   102,
     103,   103,   103,   104,   104,   104,   104,   104,   104,   105,
     105,   106,   106,   106,   106,   107,   107,   107,   108,   108,
     108,   108,   108,   108,   108,   109,   109,   109,   109,   110,
     110,   110,   110,   110,   110,   110,   110,   111,   111,   111,
     112,   113,   113,   113,   114,   115,   116,   117,   118,   118,
     119,   119,   120,   120
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     5,     0,     2,     2,     0,     1,     2,     2,
       0,     1,     2,     4,     0,     2,     1,     0,     1,     2,
       4,     0,     1,     2,     7,     6,     0,     3,     1,     1,
       4,     4,     6,     0,     3,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     9,     8,    11,    10,     5,
       4,     7,     6,     0,     2,     1,     4,     3,     0,     2,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     2,     2,     1,     1,     1,     2,     1,     3,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     2,     2,
       1,     1,     4,     1,     1,     3,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     0,     1,     2,
       3,     0,     1,     2,     1,     1,     2,     2,     6,     5,
       1,     2,     4,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     6,     4,     5,     1,     0,    10,     7,
       9,     0,     8,    17,    11,    14,     0,    12,    21,    18,
      16,     0,     0,     0,    19,     2,    22,    13,    15,     0,
       0,    23,   129,   130,   133,   131,   134,   132,   141,   137,
      20,    26,   142,     0,     0,     0,   138,    29,     0,    28,
     136,   143,     0,   135,   139,     0,     0,   140,    27,     0,
      88,   144,     0,     0,    25,   111,     0,     0,     0,   114,
     113,   145,     0,     0,     0,   116,     0,    38,    50,    48,
      49,    39,    40,    41,    42,    43,     0,    47,    99,   128,
      46,   117,     0,   126,    44,    45,    51,    52,    53,   109,
      87,    90,    92,    93,    91,   129,   130,     0,     0,     0,
       0,     0,    85,    86,    84,     0,    33,     0,   128,     0,
     117,     0,    99,    54,     0,    33,     0,   127,   125,   147,
     146,     0,    24,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   108,     0,     0,     0,     0,
       0,     0,     0,     0,    86,    82,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63,     0,    35,    36,
       0,   115,   118,     0,     0,     0,   125,     0,     0,   150,
     111,   114,    98,   110,    96,    97,    94,   101,   102,   100,
      95,   103,   104,   105,   106,   107,     0,   123,   119,   120,
     122,   121,   124,    89,    71,    60,     0,    72,    73,    75,
      74,    77,    78,    76,    79,     0,    63,    68,    65,     0,
      30,   112,     0,    94,    31,     0,     0,   151,    33,    59,
       0,    68,    70,    64,     0,    34,     0,     0,   153,   149,
       0,     0,    67,     0,    69,    62,     0,     0,   152,   148,
      32,    66,    61,     0,     0,    56,     0,     0,    55,     0,
      58,     0,    57
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     8,     9,    13,    14,    21,    18,    19,
      25,    26,    48,    49,    75,   167,   168,    76,    77,    78,
      79,    80,    81,    82,   217,   218,   234,   110,   111,    83,
     100,   101,    84,    85,    86,   186,    87,    88,    89,    90,
      91,    92,    93,    45,    46,    43,    94,    95,    96,    97,
      98,   178,   179
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -212
static const yytype_int16 yypact[] =
{
     -27,    -7,    51,   -26,  -212,  -212,  -212,    41,   187,  -212,
    -212,    62,  -212,    69,  -212,    76,    82,  -212,    12,  -212,
    -212,    70,    75,   112,  -212,   137,  -212,  -212,  -212,    49,
     121,  -212,  -212,  -212,  -212,  -212,  -212,  -212,    49,    49,
    -212,   160,  -212,   324,   173,     3,  -212,  -212,    -4,  -212,
    -212,  -212,    49,  -212,  -212,   160,   417,  -212,  -212,   211,
    1079,  -212,    21,    21,  -212,    95,  1079,  1033,    73,   171,
    -212,  -212,  1088,  1088,  1079,    98,   477,  -212,  -212,  -212,
    -212,  -212,  -212,  -212,  -212,  -212,     2,  -212,   -21,  1119,
    -212,   643,   653,  -212,  -212,  -212,  -212,  -212,  -212,  -212,
     222,  -212,   256,  -212,   703,   292,   296,    21,  1079,  1079,
      48,   629,   256,  -212,   703,   117,  1088,  1088,   206,   212,
     321,   495,   -19,  -212,   206,  1088,  1088,  -212,  -212,   653,
     653,   188,  -212,  -212,    10,    73,    73,  1079,  1079,  1079,
    1079,  1079,  1079,  1079,   237,  -212,  1088,  1088,  1088,  1088,
    1088,  1088,  1079,   151,   212,  -212,  -212,   555,    21,    21,
    1079,  1079,  1079,  1079,  1079,  1079,   573,   191,  -212,   653,
     390,  -212,  -212,    21,  1079,   201,   221,  1079,   190,  -212,
     208,  -212,  -212,  -212,  -212,  -212,  -212,   256,  -212,   703,
    -212,  -212,  -212,  -212,  -212,  -212,   226,   194,   178,    24,
       9,   -11,  -212,  -212,  -212,  -212,   637,     6,     6,  -212,
    -212,  -212,  -212,  -212,  -212,    21,   573,   -10,  -212,  1088,
    -212,  -212,    67,   196,  -212,   252,   697,  -212,  1088,  -212,
     205,   -10,  1033,  -212,   253,  -212,  1033,  1079,  1033,  -212,
     757,   203,  1033,   260,  1033,  -212,   775,   250,  1033,  -212,
    -212,  1033,  -212,   835,  1088,  -212,   895,   423,  -212,   955,
    -212,  1015,  -212
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -212,  -212,  -212,  -212,   267,  -212,   270,  -212,  -212,   280,
    -212,   274,  -212,   246,   145,  -121,    94,   -17,    66,  -212,
    -212,  -212,  -212,  -212,   102,  -211,    92,   161,    -3,  -212,
    -212,   179,  -212,  -212,  -212,   453,  -212,   -66,    30,   242,
     139,    81,   -29,  -212,   291,  -212,  -212,  -212,  -212,  -212,
    -212,  -212,   159
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -129
static const yytype_int16 yytable[] =
{
      40,   122,   232,   137,   175,   174,   233,   146,     4,    42,
      44,    32,    33,    59,    51,    55,    44,    34,    23,    35,
     233,   134,     1,    57,     7,   180,   135,   146,    56,   105,
     106,   104,   150,   114,   114,    34,    65,    35,    36,   158,
     159,   138,   146,   138,     5,   114,   149,   150,    37,    38,
     121,     6,   107,   215,   151,    10,    36,    32,    33,   157,
      16,   181,    70,    34,   136,    35,    37,    38,   182,    39,
      53,   131,    69,    70,   151,   108,   109,    15,   114,   114,
     114,   158,   159,    27,    36,    28,   236,    39,    65,   151,
     102,    20,   112,   112,    37,    38,   118,    22,   124,    29,
     158,   159,   118,   118,   112,   155,   156,   241,   189,   189,
     189,   189,   189,   189,   189,    39,  -127,    16,  -127,  -127,
    -127,  -127,    11,   104,    69,    70,   116,    30,   166,   114,
     114,   114,   114,   114,   114,   114,   114,   112,   112,   112,
     206,   117,   133,    23,   114,   189,   118,   118,   114,   216,
     158,   159,    41,   129,   130,   118,   118,   209,   210,   211,
     212,   213,   214,  -127,   183,   124,   124,   187,   187,   187,
     187,   187,   187,   187,   225,    47,   118,   118,   118,   118,
     118,   118,   102,   204,   158,   159,   114,   133,   112,   112,
     112,   112,   112,   112,   112,   112,   146,   169,   170,   148,
     149,   150,   125,   112,   187,   120,   169,   112,   114,   240,
     219,   128,   128,   123,    52,   244,   242,   127,   127,   246,
     219,   248,   219,   220,   115,   251,    99,   197,   198,   199,
     200,   201,   202,   224,   247,   250,   256,     7,   158,   159,
      11,   152,   261,   151,   171,   112,   120,   144,   177,   118,
     177,   226,   196,   172,   117,   128,   128,   228,   118,   151,
     237,   127,   127,   238,   128,   176,   245,   112,   153,   254,
     127,   127,   133,   252,  -128,    12,  -128,  -128,  -128,  -128,
     184,   185,   133,    17,   118,   128,   128,   128,   128,   128,
     128,   127,   127,   127,   127,   127,   127,   144,    24,    31,
     169,    58,   103,   -80,   113,   113,   133,   -81,   119,   169,
     133,   -80,   133,   235,   133,   -81,   113,   133,   231,   207,
     208,  -128,   133,   243,   -80,   -80,   -80,   133,   -81,   -81,
     -81,   203,    32,    33,   222,   257,    54,   227,    34,  -125,
      35,  -125,  -125,  -125,  -125,     0,     0,     0,     0,   154,
     113,   113,     0,   172,     0,     0,     0,     0,   128,    36,
       0,     0,     0,     0,   127,     0,     0,   128,     0,    37,
      38,    50,     0,   127,     0,     0,   230,     0,     0,   188,
     188,   188,   188,   188,   188,   188,  -125,     0,     0,     0,
      39,     0,     0,   128,   103,     0,     0,     0,     0,   127,
     113,   113,   113,   113,   113,   113,   113,   113,   146,     0,
     147,   148,   149,   150,     0,   113,   188,     0,     0,   113,
      59,    60,    61,     0,    62,    32,    33,    63,     0,     0,
      64,    34,    65,    35,   259,     0,     0,   221,     0,     0,
       0,   146,     0,   147,   148,   149,   150,     0,    66,     0,
       0,     0,    36,     0,     0,   151,     0,   113,     0,    67,
       0,    68,    37,    38,     0,     0,     0,     0,    69,    70,
       0,     0,     0,    71,    72,    73,    74,     0,     0,   113,
      59,    60,    61,    39,    62,    32,    33,    63,   151,     0,
     132,    34,    65,    35,     0,     0,     0,     0,    59,    60,
      61,     0,    62,    32,    33,    63,     0,     0,    66,    34,
      65,    35,    36,     0,   173,     0,     0,     0,     0,    67,
       0,    68,    37,    38,     0,     0,    66,     0,    69,    70,
      36,     0,     0,    71,    72,    73,    74,    67,     0,    68,
      37,    38,     0,    39,     0,     0,    69,    70,     0,     0,
       0,    71,    72,    73,    74,     0,     0,     0,    59,    60,
      61,    39,    62,    32,    33,    63,     0,     0,   205,    34,
      65,    35,     0,     0,     0,     0,    59,    60,    61,     0,
      62,    32,    33,    63,     0,     0,    66,    34,    65,    35,
      36,   190,   191,   192,   193,   194,   195,    67,     0,    68,
      37,    38,     0,     0,    66,     0,    69,    70,    36,     0,
       0,    71,    72,    73,    74,    67,     0,    68,    37,    38,
       0,    39,     0,     0,    69,    70,     0,   223,     0,    71,
      72,    73,    74,     0,     0,     0,   215,     0,     0,    39,
      59,    60,    61,     0,    62,    32,    33,    63,     0,     0,
     229,    34,    65,    35,   160,   161,   162,   163,   164,   165,
       0,  -125,     0,  -125,  -125,  -125,  -125,     0,    66,     0,
       0,   146,    36,   147,   148,   149,   150,     0,     0,    67,
       0,    68,    37,    38,     0,     0,     0,     0,    69,    70,
       0,     0,     0,    71,    72,    73,    74,     0,     0,     0,
      59,    60,    61,    39,    62,    32,    33,    63,  -125,     0,
     239,    34,    65,    35,     0,     0,     0,     0,   151,     0,
       0,  -126,     0,  -126,  -126,  -126,  -126,     0,    66,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,    67,
       0,    68,    37,    38,     0,     0,     0,     0,    69,    70,
       0,     0,     0,    71,    72,    73,    74,     0,     0,     0,
      59,    60,    61,    39,    62,    32,    33,    63,  -126,     0,
     249,    34,    65,    35,     0,     0,     0,     0,    59,    60,
      61,     0,    62,    32,    33,    63,   253,     0,    66,    34,
      65,    35,    36,     0,     0,     0,     0,     0,     0,    67,
       0,    68,    37,    38,     0,     0,    66,     0,    69,    70,
      36,     0,     0,    71,    72,    73,    74,    67,     0,    68,
      37,    38,     0,    39,     0,     0,    69,    70,     0,     0,
       0,    71,    72,    73,    74,     0,     0,     0,    59,    60,
      61,    39,    62,    32,    33,    63,     0,     0,   255,    34,
      65,    35,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    66,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,    67,     0,    68,
      37,    38,     0,     0,     0,     0,    69,    70,     0,     0,
       0,    71,    72,    73,    74,     0,     0,     0,    59,    60,
      61,    39,    62,    32,    33,    63,     0,     0,   258,    34,
      65,    35,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    66,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,    67,     0,    68,
      37,    38,     0,     0,     0,     0,    69,    70,     0,     0,
       0,    71,    72,    73,    74,     0,     0,     0,    59,    60,
      61,    39,    62,    32,    33,    63,     0,     0,   260,    34,
      65,    35,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    66,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,    67,     0,    68,
      37,    38,     0,     0,     0,     0,    69,    70,     0,     0,
       0,    71,    72,    73,    74,     0,     0,     0,    59,    60,
      61,    39,    62,    32,    33,    63,     0,     0,   262,    34,
      65,    35,     0,     0,     0,     0,    59,    60,    61,     0,
      62,    32,    33,    63,     0,     0,    66,    34,    65,    35,
      36,     0,     0,     0,     0,     0,     0,    67,     0,    68,
      37,    38,     0,     0,    66,     0,    69,    70,    36,     0,
       0,    71,    72,    73,    74,    67,     0,    68,    37,    38,
       0,    39,     0,     0,    69,    70,     0,    32,    33,    71,
      72,    73,    74,    34,    65,    35,    32,    33,     0,    39,
       0,     0,    34,    65,    35,     0,     0,     0,     0,     0,
      66,     0,     0,     0,    36,     0,     0,     0,     0,   126,
       0,     0,     0,    36,    37,    38,     0,     0,     0,     0,
      69,    70,     0,    37,    38,     0,     0,     0,  -110,    69,
      70,     0,     0,  -110,     0,    39,     0,     0,     0,     0,
       0,     0,     0,     0,    39,   139,   140,   141,   142,   143,
     144,     0,   145,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  -110
};

static const yytype_int16 yycheck[] =
{
      29,    67,    12,    24,   125,    24,   217,    18,    15,    38,
      39,     8,     9,     3,    43,    19,    45,    14,     6,    16,
     231,    19,    49,    52,    50,    15,    24,    18,    32,     8,
       9,    60,    23,    62,    63,    14,    15,    16,    35,    33,
      34,    62,    18,    62,    51,    74,    22,    23,    45,    46,
      67,     0,    31,    63,    65,    14,    35,     8,     9,    11,
      48,    51,    52,    14,    62,    16,    45,    46,   134,    66,
      67,    74,    51,    52,    65,    54,    55,    15,   107,   108,
     109,    33,    34,    13,    35,    15,    19,    66,    15,    65,
      60,    15,    62,    63,    45,    46,    66,    15,    68,    24,
      33,    34,    72,    73,    74,   108,   109,   228,   137,   138,
     139,   140,   141,   142,   143,    66,    18,    48,    20,    21,
      22,    23,    53,   152,    51,    52,    31,    15,    11,   158,
     159,   160,   161,   162,   163,   164,   165,   107,   108,   109,
     157,    46,    76,     6,   173,   174,   116,   117,   177,   166,
      33,    34,    31,    72,    73,   125,   126,   160,   161,   162,
     163,   164,   165,    65,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   177,    15,   146,   147,   148,   149,
     150,   151,   152,    32,    33,    34,   215,   121,   158,   159,
     160,   161,   162,   163,   164,   165,    18,   116,   117,    21,
      22,    23,    31,   173,   174,    66,   125,   177,   237,   226,
      19,    72,    73,    68,    41,   232,    11,    72,    73,   236,
      19,   238,    19,    32,    63,   242,    15,   146,   147,   148,
     149,   150,   151,    32,   237,    32,   253,    50,    33,    34,
      53,    19,   259,    65,    32,   215,   107,    41,    60,   219,
      60,    61,    15,    32,    46,   116,   117,    31,   228,    65,
      64,   116,   117,    11,   125,   126,    13,   237,   107,    19,
     125,   126,   206,    13,    18,     8,    20,    21,    22,    23,
     135,   136,   216,    13,   254,   146,   147,   148,   149,   150,
     151,   146,   147,   148,   149,   150,   151,    41,    18,    25,
     219,    55,    60,    11,    62,    63,   240,    11,    66,   228,
     244,    19,   246,   219,   248,    19,    74,   251,   216,   158,
     159,    65,   256,   231,    32,    33,    34,   261,    32,    33,
      34,   152,     8,     9,   173,   254,    45,   178,    14,    18,
      16,    20,    21,    22,    23,    -1,    -1,    -1,    -1,   107,
     108,   109,    -1,    32,    -1,    -1,    -1,    -1,   219,    35,
      -1,    -1,    -1,    -1,   219,    -1,    -1,   228,    -1,    45,
      46,    47,    -1,   228,    -1,    -1,   215,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    65,    -1,    -1,    -1,
      66,    -1,    -1,   254,   152,    -1,    -1,    -1,    -1,   254,
     158,   159,   160,   161,   162,   163,   164,   165,    18,    -1,
      20,    21,    22,    23,    -1,   173,   174,    -1,    -1,   177,
       3,     4,     5,    -1,     7,     8,     9,    10,    -1,    -1,
      13,    14,    15,    16,    11,    -1,    -1,    47,    -1,    -1,
      -1,    18,    -1,    20,    21,    22,    23,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    65,    -1,   215,    -1,    42,
      -1,    44,    45,    46,    -1,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    57,    58,    59,    -1,    -1,   237,
       3,     4,     5,    66,     7,     8,     9,    10,    65,    -1,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,     3,     4,
       5,    -1,     7,     8,     9,    10,    -1,    -1,    31,    14,
      15,    16,    35,    -1,    19,    -1,    -1,    -1,    -1,    42,
      -1,    44,    45,    46,    -1,    -1,    31,    -1,    51,    52,
      35,    -1,    -1,    56,    57,    58,    59,    42,    -1,    44,
      45,    46,    -1,    66,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    -1,    -1,    -1,     3,     4,
       5,    66,     7,     8,     9,    10,    -1,    -1,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,     3,     4,     5,    -1,
       7,     8,     9,    10,    -1,    -1,    31,    14,    15,    16,
      35,   138,   139,   140,   141,   142,   143,    42,    -1,    44,
      45,    46,    -1,    -1,    31,    -1,    51,    52,    35,    -1,
      -1,    56,    57,    58,    59,    42,    -1,    44,    45,    46,
      -1,    66,    -1,    -1,    51,    52,    -1,   174,    -1,    56,
      57,    58,    59,    -1,    -1,    -1,    63,    -1,    -1,    66,
       3,     4,     5,    -1,     7,     8,     9,    10,    -1,    -1,
      13,    14,    15,    16,    25,    26,    27,    28,    29,    30,
      -1,    18,    -1,    20,    21,    22,    23,    -1,    31,    -1,
      -1,    18,    35,    20,    21,    22,    23,    -1,    -1,    42,
      -1,    44,    45,    46,    -1,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    57,    58,    59,    -1,    -1,    -1,
       3,     4,     5,    66,     7,     8,     9,    10,    65,    -1,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    18,    -1,    20,    21,    22,    23,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    44,    45,    46,    -1,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    57,    58,    59,    -1,    -1,    -1,
       3,     4,     5,    66,     7,     8,     9,    10,    65,    -1,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,     3,     4,
       5,    -1,     7,     8,     9,    10,    11,    -1,    31,    14,
      15,    16,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    44,    45,    46,    -1,    -1,    31,    -1,    51,    52,
      35,    -1,    -1,    56,    57,    58,    59,    42,    -1,    44,
      45,    46,    -1,    66,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    -1,    -1,    -1,     3,     4,
       5,    66,     7,     8,     9,    10,    -1,    -1,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    44,
      45,    46,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    -1,    -1,    -1,     3,     4,
       5,    66,     7,     8,     9,    10,    -1,    -1,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    44,
      45,    46,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    -1,    -1,    -1,     3,     4,
       5,    66,     7,     8,     9,    10,    -1,    -1,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    44,
      45,    46,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    -1,    -1,    -1,     3,     4,
       5,    66,     7,     8,     9,    10,    -1,    -1,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,     3,     4,     5,    -1,
       7,     8,     9,    10,    -1,    -1,    31,    14,    15,    16,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    44,
      45,    46,    -1,    -1,    31,    -1,    51,    52,    35,    -1,
      -1,    56,    57,    58,    59,    42,    -1,    44,    45,    46,
      -1,    66,    -1,    -1,    51,    52,    -1,     8,     9,    56,
      57,    58,    59,    14,    15,    16,     8,     9,    -1,    66,
      -1,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    45,    46,    -1,    -1,    -1,    -1,
      51,    52,    -1,    45,    46,    -1,    -1,    -1,    19,    51,
      52,    -1,    -1,    24,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    36,    37,    38,    39,    40,
      41,    -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    49,    69,    70,    15,    51,     0,    50,    71,    72,
      14,    53,    72,    73,    74,    15,    48,    74,    76,    77,
      15,    75,    15,     6,    77,    78,    79,    13,    15,    24,
      15,    79,     8,     9,    14,    16,    35,    45,    46,    66,
     110,    31,   110,   113,   110,   111,   112,    15,    80,    81,
      47,   110,    41,    67,   112,    19,    32,   110,    81,     3,
       4,     5,     7,    10,    13,    15,    31,    42,    44,    51,
      52,    56,    57,    58,    59,    82,    85,    86,    87,    88,
      89,    90,    91,    97,   100,   101,   102,   104,   105,   106,
     107,   108,   109,   110,   114,   115,   116,   117,   118,    15,
      98,    99,   106,   107,   110,     8,     9,    31,    54,    55,
      95,    96,   106,   107,   110,    95,    31,    46,   106,   107,
     108,    85,   105,    82,   106,    31,    31,    82,   108,   109,
     109,    96,    13,    86,    19,    24,    62,    24,    62,    36,
      37,    38,    39,    40,    41,    43,    18,    20,    21,    22,
      23,    65,    19,    95,   107,    96,    96,    11,    33,    34,
      25,    26,    27,    28,    29,    30,    11,    83,    84,   109,
     109,    32,    32,    19,    24,    83,   108,    60,   119,   120,
      15,    51,   105,   106,    82,    82,   103,   106,   107,   110,
     103,   103,   103,   103,   103,   103,    15,   109,   109,   109,
     109,   109,   109,    99,    32,    13,    85,    95,    95,    96,
      96,    96,    96,    96,    96,    63,    85,    92,    93,    19,
      32,    47,    95,   103,    32,    96,    61,   120,    31,    13,
      95,    92,    12,    93,    94,    84,    19,    64,    11,    13,
      85,    83,    11,    94,    85,    13,    85,    96,    85,    13,
      32,    85,    13,    11,    19,    13,    85,   109,    13,    11,
      13,    85,    13
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
# if YYLTYPE_IS_TRIVIAL
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      YYFPRINTF (stderr, "\n");
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





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
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

/* Line 1455 of yacc.c  */
#line 182 "bison.y"
    {
	;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 186 "bison.y"
    {
		FKLOG("[bison]: package %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 193 "bison.y"
    {
		FKLOG("[bison]: package %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 201 "bison.y"
    {
	;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 211 "bison.y"
    {
		FKLOG("[bison]: include %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_include((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 220 "bison.y"
    {
	;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 230 "bison.y"
    {
		FKLOG("[bison]: struct_define %s", (yyvsp[(2) - (4)].str).c_str());
		myflexer *l = (myflexer *)parm;
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[(3) - (4)].syntree));
		l->add_struct_desc((yyvsp[(2) - (4)].str).c_str(), p);
	;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 240 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 245 "bison.y"
    {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER struct_mem_declaration");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_struct_memlist);
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[(1) - (2)].syntree));
		p->add_arg((yyvsp[(2) - (2)].str));
		(yyval.syntree) = p;
	;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 254 "bison.y"
    {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER");
		NEWTYPE(p, struct_desc_memlist_node);
		p->add_arg((yyvsp[(1) - (1)].str));
		(yyval.syntree) = p;
	;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 264 "bison.y"
    {
	;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 274 "bison.y"
    {
		FKLOG("[bison]: const_define %s", (yyvsp[(2) - (4)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_const_desc((yyvsp[(2) - (4)].str).c_str(), (yyvsp[(4) - (4)].syntree));
	;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 283 "bison.y"
    {
	;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 295 "bison.y"
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

/* Line 1455 of yacc.c  */
#line 307 "bison.y"
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

/* Line 1455 of yacc.c  */
#line 321 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 326 "bison.y"
    {
		FKLOG("[bison]: function_declaration_arguments <- arg function_declaration_arguments");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_arglist);
		func_desc_arglist_node * p = dynamic_cast<func_desc_arglist_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 335 "bison.y"
    {
		FKLOG("[bison]: function_declaration_arguments <- arg");
		NEWTYPE(p, func_desc_arglist_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 345 "bison.y"
    {
		FKLOG("[bison]: arg <- IDENTIFIER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, identifier_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 355 "bison.y"
    {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[(1) - (4)].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[(1) - (4)].str);
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[(3) - (4)].syntree));
		p->fakecall = false;
		p->classmem_call = false;
		(yyval.syntree) = p;
	;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 366 "bison.y"
    {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[(1) - (4)].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[(1) - (4)].str);
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[(3) - (4)].syntree));
		p->fakecall = false;
		p->classmem_call = false;
		(yyval.syntree) = p;
	;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 377 "bison.y"
    {
		FKLOG("[bison]: function_call <- mem function_call_arguments %s", (yyvsp[(3) - (6)].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[(3) - (6)].str);
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

  case 33:

/* Line 1455 of yacc.c  */
#line 396 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 401 "bison.y"
    {
		FKLOG("[bison]: function_call_arguments <- arg_expr function_call_arguments");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_call_arglist);
		function_call_arglist_node * p = dynamic_cast<function_call_arglist_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 410 "bison.y"
    {
		FKLOG("[bison]: function_call_arguments <- arg_expr");
		NEWTYPE(p, function_call_arglist_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 420 "bison.y"
    {
		FKLOG("[bison]: arg_expr <- expr_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 430 "bison.y"
    {
		FKLOG("[bison]: block <- block stmt");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_block);
		block_node * p = dynamic_cast<block_node*>((yyvsp[(1) - (2)].syntree));
		p->add_stmt((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 439 "bison.y"
    {
		FKLOG("[bison]: block <- stmt");
		NEWTYPE(p, block_node);
		p->add_stmt((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 449 "bison.y"
    {
		FKLOG("[bison]: stmt <- while_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 455 "bison.y"
    {
		FKLOG("[bison]: stmt <- if_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 461 "bison.y"
    {
		FKLOG("[bison]: stmt <- return_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 467 "bison.y"
    {
		FKLOG("[bison]: stmt <- assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 473 "bison.y"
    {
		FKLOG("[bison]: stmt <- multi_assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 479 "bison.y"
    {
		FKLOG("[bison]: stmt <- break");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 485 "bison.y"
    {
		FKLOG("[bison]: stmt <- continue");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 491 "bison.y"
    {
		FKLOG("[bison]: stmt <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 497 "bison.y"
    {
		FKLOG("[bison]: stmt <- math_assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 503 "bison.y"
    {
		FKLOG("[bison]: stmt <- for_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 509 "bison.y"
    {
		FKLOG("[bison]: stmt <- for_loop_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 515 "bison.y"
    {
		FKLOG("[bison]: stmt <- fake_call_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 521 "bison.y"
    {
		FKLOG("[bison]: stmt <- sleep_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 527 "bison.y"
    {
		FKLOG("[bison]: stmt <- yield_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 533 "bison.y"
    {
		FKLOG("[bison]: stmt <- switch_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 541 "bison.y"
    {
		FKLOG("[bison]: fake_call_stmt <- fake function_call");
		function_call_node * p = dynamic_cast<function_call_node*>((yyvsp[(2) - (2)].syntree));
		p->fakecall = true;
		(yyval.syntree) = p;
	;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 551 "bison.y"
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

  case 56:

/* Line 1455 of yacc.c  */
#line 562 "bison.y"
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

  case 57:

/* Line 1455 of yacc.c  */
#line 575 "bison.y"
    {
		FKLOG("[bison]: for_loop_stmt <- block");
		NEWTYPE(p, for_loop_stmt);
		p->var = (yyvsp[(2) - (11)].syntree);
		p->begin = (yyvsp[(4) - (11)].syntree);
		p->end = (yyvsp[(6) - (11)].syntree);
		p->add = (yyvsp[(8) - (11)].syntree);
		p->block = dynamic_cast<block_node*>((yyvsp[(10) - (11)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 587 "bison.y"
    {
		FKLOG("[bison]: for_loop_stmt <- empty");
		NEWTYPE(p, for_loop_stmt);
		p->var = (yyvsp[(2) - (10)].syntree);
		p->begin = (yyvsp[(4) - (10)].syntree);
		p->end = (yyvsp[(6) - (10)].syntree);
		p->add = (yyvsp[(8) - (10)].syntree);
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 601 "bison.y"
    {
		FKLOG("[bison]: while_stmt <- cmp block");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (5)].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[(4) - (5)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 610 "bison.y"
    {
		FKLOG("[bison]: while_stmt <- cmp");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (4)].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 621 "bison.y"
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

  case 62:

/* Line 1455 of yacc.c  */
#line 632 "bison.y"
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

  case 63:

/* Line 1455 of yacc.c  */
#line 645 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 650 "bison.y"
    {
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt_list elseif_stmt");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_elseif_stmt_list);
		elseif_stmt_list * p = dynamic_cast<elseif_stmt_list*>((yyvsp[(1) - (2)].syntree));
		p->add_stmt((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 659 "bison.y"
    {
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt");
		NEWTYPE(p, elseif_stmt_list);
		p->add_stmt((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 669 "bison.y"
    {
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (4)].syntree));
		p->block = (yyvsp[(4) - (4)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 678 "bison.y"
    {
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (3)].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 689 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 694 "bison.y"
    {
		FKLOG("[bison]: else_stmt <- block");
		NEWTYPE(p, else_stmt);
		p->block = dynamic_cast<block_node*>((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 702 "bison.y"
    {
		FKLOG("[bison]: else_stmt <- empty");
		NEWTYPE(p, else_stmt);
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 712 "bison.y"
    {
		FKLOG("[bison]: cmp <- ( cmp )");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 718 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp AND cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "&&";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 728 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp OR cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "||";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 738 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value LESS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 748 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value MORE cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 758 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 768 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value MORE_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 778 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value LESS_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 788 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value NOT_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 798 "bison.y"
    {
		FKLOG("[bison]: cmp <- true");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "true";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 808 "bison.y"
    {
		FKLOG("[bison]: cmp <- false");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "false";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 818 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value IS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "is";
		p->left = (yyvsp[(2) - (2)].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 828 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value NOT cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "not";
		p->left = (yyvsp[(2) - (2)].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 840 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 846 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 852 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 860 "bison.y"
    {
		FKLOG("[bison]: return_stmt <- RETURN return_value_list");
		NEWTYPE(p, return_stmt);
		p->returnlist = dynamic_cast<return_value_list_node*>((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 868 "bison.y"
    {
		FKLOG("[bison]: return_stmt <- RETURN");
		NEWTYPE(p, return_stmt);
		p->returnlist = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 878 "bison.y"
    {
		FKLOG("[bison]: return_value_list <- return_value_list return_value");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_return_value_list);
		return_value_list_node * p = dynamic_cast<return_value_list_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 887 "bison.y"
    {
		NEWTYPE(p, return_value_list_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 896 "bison.y"
    {
		FKLOG("[bison]: return_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 902 "bison.y"
    {
		FKLOG("[bison]: return_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 908 "bison.y"
    {
		FKLOG("[bison]: return_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 916 "bison.y"
    {
		FKLOG("[bison]: assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->value = (yyvsp[(3) - (3)].syntree);
		p->isnew = false;
		(yyval.syntree) = p;
	;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 926 "bison.y"
    {
		FKLOG("[bison]: new assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->value = (yyvsp[(3) - (3)].syntree);
		p->isnew = true;
		(yyval.syntree) = p;
	;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 938 "bison.y"
    {
		FKLOG("[bison]: multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>((yyvsp[(1) - (3)].syntree));
		p->value = (yyvsp[(3) - (3)].syntree);
		p->isnew = false;
		(yyval.syntree) = p;
	;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 948 "bison.y"
    {
		FKLOG("[bison]: new multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>((yyvsp[(1) - (3)].syntree));
		p->value = (yyvsp[(3) - (3)].syntree);
		p->isnew = true;
		(yyval.syntree) = p;
	;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 960 "bison.y"
    {
		FKLOG("[bison]: var_list <- var_list var");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_var_list);
		var_list_node * p = dynamic_cast<var_list_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 969 "bison.y"
    {
		NEWTYPE(p, var_list_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 978 "bison.y"
    {
		FKLOG("[bison]: assign_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 984 "bison.y"
    {
		FKLOG("[bison]: assign_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 990 "bison.y"
    {
		FKLOG("[bison]: assign_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 998 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "+=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1008 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "-=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1018 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "/=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1028 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "*=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1038 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "%=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1048 "bison.y"
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

  case 109:

/* Line 1455 of yacc.c  */
#line 1064 "bison.y"
    {
		FKLOG("[bison]: var <- VAR_BEGIN IDENTIFIER %s", (yyvsp[(2) - (2)].str).c_str());
		NEWTYPE(p, var_node);
		p->str = (yyvsp[(2) - (2)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1072 "bison.y"
    {
		FKLOG("[bison]: var <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1080 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1088 "bison.y"
    {
		FKLOG("[bison]: container_get_node <- IDENTIFIER[expr_value] %s", (yyvsp[(1) - (4)].str).c_str());
		NEWTYPE(p, container_get_node);
		p->container = (yyvsp[(1) - (4)].str);
		p->key = (yyvsp[(3) - (4)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1097 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER_POINTER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, struct_pointer_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1105 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER_DOT %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1115 "bison.y"
    {
		FKLOG("[bison]: expr <- (expr)");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1121 "bison.y"
    {
		FKLOG("[bison]: expr <- function_call");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1127 "bison.y"
    {
		FKLOG("[bison]: expr <- math_expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1135 "bison.y"
    {
		FKLOG("[bison]: math_expr <- (math_expr)");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1141 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "+";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1151 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "-";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1161 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "*";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1171 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "/";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1181 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "%";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1191 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "..";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1203 "bison.y"
    {
		FKLOG("[bison]: expr_value <- math_expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1209 "bison.y"
    {
		FKLOG("[bison]: expr_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1215 "bison.y"
    {
		FKLOG("[bison]: expr_value <- function_call");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1221 "bison.y"
    {
		FKLOG("[bison]: expr_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1229 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FTRUE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_TRUE;
		(yyval.syntree) = p;
	;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1238 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FFALSE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_FALSE;
		(yyval.syntree) = p;
	;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1247 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- NUMBER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_NUM;
		(yyval.syntree) = p;
	;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1256 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FKUUID %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_UUID;
		(yyval.syntree) = p;
	;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1265 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- STRING_DEFINITION %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_STR;
		(yyval.syntree) = p;
	;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1274 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FKFLOAT %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_FLOAT;
		(yyval.syntree) = p;
	;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1283 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- const_map_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_MAP;
		p->v = (yyvsp[(2) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1293 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- const_array_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_ARRAY;
		p->v = (yyvsp[(2) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1305 "bison.y"
    {
		FKLOG("[bison]: const_map_list_value <- null");
		NEWTYPE(p, const_map_list_value_node);
		(yyval.syntree) = p;
	;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1312 "bison.y"
    {
		FKLOG("[bison]: const_map_list_value <- const_map_value");
		NEWTYPE(p, const_map_list_value_node);
		p->add_ele((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1320 "bison.y"
    {
		FKLOG("[bison]: const_map_list_value <- const_map_list_value const_map_value");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_constmaplist);
		const_map_list_value_node * p = dynamic_cast<const_map_list_value_node*>((yyvsp[(1) - (2)].syntree));
		p->add_ele((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1331 "bison.y"
    {
		FKLOG("[bison]: const_map_value <- explicit_value");
		NEWTYPE(p, const_map_value_node);
		p->k = (yyvsp[(1) - (3)].syntree);
		p->v = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1342 "bison.y"
    {
		FKLOG("[bison]: const_array_list_value <- null");
		NEWTYPE(p, const_array_list_value_node);
		(yyval.syntree) = p;
	;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1349 "bison.y"
    {
		FKLOG("[bison]: const_array_list_value <- explicit_value");
		NEWTYPE(p, const_array_list_value_node);
		p->add_ele((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1357 "bison.y"
    {
		FKLOG("[bison]: const_array_list_value <- const_array_list_value explicit_value");
		assert((yyvsp[(1) - (2)].syntree)->gettype() == est_constarraylist);
		const_array_list_value_node * p = dynamic_cast<const_array_list_value_node*>((yyvsp[(1) - (2)].syntree));
		p->add_ele((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1368 "bison.y"
    {
		FKLOG("[bison]: break <- BREAK");
		NEWTYPE(p, break_stmt);
		(yyval.syntree) = p;
	;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1377 "bison.y"
    {
		FKLOG("[bison]: CONTINUE");
		NEWTYPE(p, continue_stmt);
		(yyval.syntree) = p;
	;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1386 "bison.y"
    {
		FKLOG("[bison]: SLEEP");
		NEWTYPE(p, sleep_stmt);
		p->time = (yyvsp[(2) - (2)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1395 "bison.y"
    {
		FKLOG("[bison]: YIELD");
		NEWTYPE(p, yield_stmt);
		p->time = (yyvsp[(2) - (2)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1405 "bison.y"
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[(2) - (6)].syntree);
		p->caselist = (yyvsp[(3) - (6)].syntree);
		p->def = (yyvsp[(5) - (6)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1415 "bison.y"
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[(2) - (5)].syntree);
		p->caselist = (yyvsp[(3) - (5)].syntree);
		p->def = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1427 "bison.y"
    {
		FKLOG("[bison]: switch_case_list <- switch_case_define");
		NEWTYPE(p, switch_caselist_node);
		p->add_case((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1435 "bison.y"
    {
		FKLOG("[bison]: switch_case_list <- switch_case_list switch_case_define");
		assert((yyvsp[(2) - (2)].syntree)->gettype() == est_switch_case_node);
		switch_caselist_node * p = dynamic_cast<switch_caselist_node*>((yyvsp[(1) - (2)].syntree));
		p->add_case((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1446 "bison.y"
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[(2) - (4)].syntree);
		p->block = (yyvsp[(4) - (4)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1455 "bison.y"
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[(2) - (3)].syntree);
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;



/* Line 1455 of yacc.c  */
#line 3646 "bison.tab.c"
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
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
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



