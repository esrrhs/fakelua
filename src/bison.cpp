
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
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "bison.y"

#include "semantic.h"
#include "types.h"
#include "fake.h"

#define YYPARSE_PARAM   parm
#define YYLEX_PARAM     parm

#define yyerror(msg) my_yyerror(msg, YYPARSE_PARAM)

int yylex(YYSTYPE *lvalp, void * parm)
{
	myflexer *l = (myflexer *)parm;
	int ret = l->yylex(lvalp);
	FKLOG("[bison]: bison get token[%s] str[%s]", fkget_token_name(ret).c_str(), lvalp->str.c_str());
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
	p->lno = ((myflexer *)parm)->lineno(); \
	FKLOG("[bison]: bison new type %s %p line %d", #x, p, p->lno);
	


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
# define YYERROR_VERBOSE 0
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
     RIGHT_POINTER = 319
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 213 "bison.tab.c"

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
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

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
#define YYLAST   1127

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  65
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  142
/* YYNRULES -- Number of states.  */
#define YYNSTATES  247

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64
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
     359,   361,   363,   368,   370,   374,   376,   378,   382,   386,
     390,   394,   398,   402,   404,   406,   408,   410,   412,   414,
     416,   418,   420,   422,   424,   426,   429,   432,   439,   445,
     447,   450,   455
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      66,     0,    -1,    67,    68,    70,    73,    75,    -1,    -1,
      49,    15,    -1,    49,    51,    -1,    -1,    69,    -1,    68,
      69,    -1,    50,    14,    -1,    -1,    71,    -1,    70,    71,
      -1,    53,    15,    72,    13,    -1,    -1,    72,    15,    -1,
      15,    -1,    -1,    74,    -1,    73,    74,    -1,    48,    15,
      24,   107,    -1,    -1,    76,    -1,    75,    76,    -1,     6,
      15,    31,    77,    32,    82,    13,    -1,     6,    15,    31,
      77,    32,    13,    -1,    -1,    77,    19,    78,    -1,    78,
      -1,    15,    -1,    15,    31,    80,    32,    -1,    51,    31,
      80,    32,    -1,   103,    41,    15,    31,    80,    32,    -1,
      -1,    80,    19,    81,    -1,    81,    -1,   106,    -1,    82,
      83,    -1,    83,    -1,    87,    -1,    88,    -1,    94,    -1,
      97,    -1,    98,    -1,   108,    -1,   109,    -1,   104,    -1,
     101,    -1,    85,    -1,    86,    -1,    84,    -1,   110,    -1,
     111,    -1,   112,    -1,    44,    79,    -1,    42,    82,    19,
      92,    19,    82,    11,    82,    13,    -1,    42,    82,    19,
      92,    19,    82,    11,    13,    -1,    42,   102,    24,   100,
      64,    93,    19,   106,    11,    82,    13,    -1,    42,   102,
      24,   100,    64,    93,    19,   106,    11,    13,    -1,     7,
      92,    11,    82,    13,    -1,     7,    92,    11,    13,    -1,
      10,    92,    11,    82,    89,    91,    13,    -1,    10,    92,
      11,    89,    91,    13,    -1,    -1,    89,    90,    -1,    90,
      -1,    63,    92,    11,    82,    -1,    63,    92,    11,    -1,
      -1,    12,    82,    -1,    12,    -1,    31,    92,    32,    -1,
      92,    33,    92,    -1,    92,    34,    92,    -1,    93,    26,
      93,    -1,    93,    25,    93,    -1,    93,    29,    93,    -1,
      93,    27,    93,    -1,    93,    28,    93,    -1,    93,    30,
      93,    -1,     8,    -1,     9,    -1,    54,    93,    -1,    55,
      93,    -1,   107,    -1,   103,    -1,   104,    -1,     4,    95,
      -1,     4,    -1,    95,    19,    96,    -1,    96,    -1,   107,
      -1,   103,    -1,   104,    -1,   102,    24,   100,    -1,   102,
      62,   100,    -1,    99,    24,    79,    -1,    99,    62,    79,
      -1,    99,    19,   102,    -1,   102,    -1,   107,    -1,   103,
      -1,   104,    -1,   103,    36,   100,    -1,   103,    37,   100,
      -1,   103,    38,   100,    -1,   103,    39,   100,    -1,   103,
      40,   100,    -1,   103,    43,    -1,     3,    15,    -1,   103,
      -1,    15,    -1,    15,    46,   106,    47,    -1,    52,    -1,
      31,   104,    32,    -1,    79,    -1,   105,    -1,    31,   105,
      32,    -1,   106,    20,   106,    -1,   106,    21,   106,    -1,
     106,    23,   106,    -1,   106,    22,   106,    -1,   106,    18,
     106,    -1,   105,    -1,   107,    -1,    79,    -1,   103,    -1,
       8,    -1,     9,    -1,    16,    -1,    45,    -1,    14,    -1,
      35,    -1,     5,    -1,    56,    -1,    58,   106,    -1,    57,
     106,    -1,    59,    93,   113,    61,    82,    13,    -1,    59,
      93,   113,    61,    13,    -1,   114,    -1,   113,   114,    -1,
      60,    93,    11,    82,    -1,    60,    93,    11,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   162,   162,   171,   174,   181,   190,   193,   195,   199,
     209,   212,   214,   218,   229,   233,   242,   253,   256,   258,
     262,   272,   275,   277,   283,   294,   308,   312,   321,   331,
     341,   351,   361,   380,   384,   393,   403,   413,   422,   432,
     438,   444,   450,   456,   462,   468,   474,   480,   486,   492,
     498,   504,   510,   516,   524,   534,   545,   558,   570,   584,
     593,   604,   615,   629,   633,   642,   652,   661,   673,   677,
     685,   695,   701,   711,   721,   731,   741,   751,   761,   771,
     781,   791,   801,   811,   823,   829,   835,   843,   851,   861,
     870,   879,   885,   891,   899,   909,   921,   931,   943,   952,
     961,   967,   973,   981,   991,  1001,  1011,  1021,  1031,  1047,
    1055,  1063,  1071,  1080,  1090,  1096,  1102,  1110,  1116,  1126,
    1136,  1146,  1156,  1168,  1174,  1180,  1186,  1194,  1203,  1212,
    1221,  1230,  1239,  1250,  1259,  1268,  1277,  1287,  1297,  1309,
    1317,  1328,  1337
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
  "ELSEIF", "RIGHT_POINTER", "$accept", "program", "package_head",
  "include_head", "include_define", "struct_head", "struct_define",
  "struct_mem_declaration", "const_head", "const_define", "body",
  "function_declaration", "function_declaration_arguments", "arg",
  "function_call", "function_call_arguments", "arg_expr", "block", "stmt",
  "fake_call_stmt", "for_stmt", "for_loop_stmt", "while_stmt", "if_stmt",
  "elseif_stmt_list", "elseif_stmt", "else_stmt", "cmp", "cmp_value",
  "return_stmt", "return_value_list", "return_value", "assign_stmt",
  "multi_assign_stmt", "var_list", "assign_value", "math_assign_stmt",
  "var", "variable", "expr", "math_expr", "expr_value", "explicit_value",
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
     315,   316,   317,   318,   319
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    65,    66,    67,    67,    67,    68,    68,    68,    69,
      70,    70,    70,    71,    72,    72,    72,    73,    73,    73,
      74,    75,    75,    75,    76,    76,    77,    77,    77,    78,
      79,    79,    79,    80,    80,    80,    81,    82,    82,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    84,    85,    85,    86,    86,    87,
      87,    88,    88,    89,    89,    89,    90,    90,    91,    91,
      91,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    93,    93,    93,    94,    94,    95,
      95,    96,    96,    96,    97,    97,    98,    98,    99,    99,
     100,   100,   100,   101,   101,   101,   101,   101,   101,   102,
     102,   103,   103,   103,   104,   104,   104,   105,   105,   105,
     105,   105,   105,   106,   106,   106,   106,   107,   107,   107,
     107,   107,   107,   108,   109,   110,   111,   112,   112,   113,
     113,   114,   114
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
       1,     1,     4,     1,     3,     1,     1,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     6,     5,     1,
       2,     4,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     6,     4,     5,     1,     0,    10,     7,
       9,     0,     8,    17,    11,    14,     0,    12,    21,    18,
      16,     0,     0,     0,    19,     2,    22,    13,    15,     0,
       0,    23,   127,   128,   131,   129,   132,   130,    20,    26,
      29,     0,    28,     0,     0,    27,     0,    88,   133,     0,
       0,    25,   111,     0,     0,     0,     0,   113,   134,     0,
       0,     0,   115,     0,    38,    50,    48,    49,    39,    40,
      41,    42,    43,     0,    47,    99,   126,    46,   116,     0,
     124,    44,    45,    51,    52,    53,   109,    87,    90,    92,
      93,    91,   127,   128,     0,     0,     0,     0,     0,    85,
      86,    84,     0,    33,     0,   126,     0,   116,     0,    99,
      54,     0,    33,     0,   125,   123,   136,   135,     0,    24,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   108,     0,     0,     0,     0,     0,     0,     0,
      86,    82,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    63,     0,    35,    36,     0,   114,   117,     0,
       0,     0,   123,     0,     0,   139,   111,    98,   110,    96,
      97,    94,   101,   102,   100,    95,   103,   104,   105,   106,
     107,     0,   122,   118,   119,   121,   120,    89,    71,    60,
       0,    72,    73,    75,    74,    77,    78,    76,    79,     0,
      63,    68,    65,     0,    30,   112,     0,    94,    31,     0,
       0,   140,    33,    59,     0,    68,    70,    64,     0,    34,
       0,     0,   142,   138,     0,     0,    67,     0,    69,    62,
       0,     0,   141,   137,    32,    66,    61,     0,     0,    56,
       0,     0,    55,     0,    58,     0,    57
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     8,     9,    13,    14,    21,    18,    19,
      25,    26,    41,    42,    62,   153,   154,    63,    64,    65,
      66,    67,    68,    69,   201,   202,   218,    97,    98,    70,
      87,    88,    71,    72,    73,   171,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,   164,   165
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -176
static const yytype_int16 yypact[] =
{
     -33,    -2,    41,   -26,  -176,  -176,  -176,    34,    55,  -176,
    -176,    47,  -176,   -14,  -176,    57,    62,  -176,     5,  -176,
    -176,     2,   105,   117,  -176,    63,  -176,  -176,  -176,    19,
     107,  -176,  -176,  -176,  -176,  -176,  -176,  -176,  -176,   148,
    -176,    54,  -176,   148,   501,  -176,   151,   349,  -176,   259,
     259,  -176,   -17,   349,  1068,    23,   138,  -176,  -176,   504,
     504,   349,   413,   558,  -176,  -176,  -176,  -176,  -176,  -176,
    -176,  -176,  -176,     6,  -176,   -15,   427,  -176,   439,   503,
    -176,  -176,  -176,  -176,  -176,  -176,  -176,   159,  -176,   330,
    -176,   632,    12,    91,   259,   349,   349,    45,   385,   330,
    -176,   632,   115,   504,   504,   143,   147,   367,   576,   -12,
    -176,   143,   504,   504,  -176,  -176,   503,   503,   123,  -176,
    -176,     7,    23,    23,   349,   349,   349,   349,   349,   349,
     349,   170,  -176,   504,   504,   504,   504,   504,   349,   142,
     147,  -176,  -176,   633,   259,   259,   349,   349,   349,   349,
     349,   349,   440,    61,  -176,   503,   237,  -176,  -176,   259,
     349,   108,   161,   349,    83,  -176,   145,  -176,  -176,  -176,
    -176,  -176,   330,  -176,   632,  -176,  -176,  -176,  -176,  -176,
    -176,   164,  -176,   179,    89,    53,   178,  -176,  -176,  -176,
     690,   125,   125,  -176,  -176,  -176,  -176,  -176,  -176,   259,
     440,    -8,  -176,   504,  -176,  -176,    24,   141,  -176,   195,
     747,  -176,   504,  -176,   131,    -8,  1068,  -176,   205,  -176,
    1068,   349,  1068,  -176,   804,   118,  1068,   212,  1068,  -176,
     822,   207,  1068,  -176,  -176,  1068,  -176,   879,   504,  -176,
     936,   243,  -176,   993,  -176,  1050,  -176
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -176,  -176,  -176,  -176,   219,  -176,   215,  -176,  -176,   211,
    -176,   214,  -176,   197,   183,  -106,    38,   -49,   109,  -176,
    -176,  -176,  -176,  -176,    44,  -175,    35,   -31,   -59,  -176,
    -176,   113,  -176,  -176,  -176,    27,  -176,   -51,    86,   279,
     -52,   144,   -29,  -176,  -176,  -176,  -176,  -176,  -176,    88
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -127
static const yytype_int16 yytable[] =
{
      38,   107,   118,   109,   216,   108,   161,   115,   115,   124,
      46,    23,   160,     4,   103,    27,     1,    28,    91,   102,
     101,   101,   166,   -80,     7,   121,   217,    32,    33,   104,
     122,   -80,   101,    34,    16,    35,   141,   142,    52,    11,
     217,     6,   107,   220,   -80,   -80,   -80,   125,    10,     5,
     125,   115,   115,    16,    36,   199,   143,   144,   145,    57,
     115,   162,    15,   139,    37,   101,   101,   101,   123,    23,
     167,   133,    20,    43,    56,    57,   137,    22,   144,   145,
     203,   115,   115,   115,   115,   115,    44,   193,   194,   195,
     196,   197,   198,   204,   190,   174,   174,   174,   174,   174,
     174,   174,   -81,   200,   209,     7,   225,   133,    11,    91,
     -81,   136,   137,   191,   192,   101,   101,   101,   101,   101,
     101,   101,   101,   -81,   -81,   -81,   152,   203,   206,    29,
     101,   174,    30,    89,   101,    99,    99,   203,    39,   105,
     208,   111,   226,   163,   210,   105,   105,    99,   144,   145,
     234,   115,   175,   176,   177,   178,   179,   180,   144,   145,
     115,   224,   231,    40,   144,   145,    86,   228,   214,   112,
     101,   230,   120,   232,   188,   144,   145,   235,   138,   157,
      99,    99,    99,   163,   131,   181,   115,   207,   240,   105,
     105,   104,   101,   158,   245,   212,   133,   133,   105,   105,
     135,   136,   137,   116,   117,   221,   222,   168,   111,   111,
     172,   172,   172,   172,   172,   172,   172,   120,   229,   105,
     105,   105,   105,   105,    89,   236,   238,    12,    17,    24,
      99,    99,    99,    99,    99,    99,    99,    99,   110,    31,
      45,   219,   114,   114,   215,    99,   172,   155,   156,    99,
     227,   187,   211,     0,   243,   133,   155,   134,   135,   136,
     137,   133,     0,   134,   135,   136,   137,    92,    93,     0,
       0,     0,     0,    34,    52,    35,     0,   182,   183,   184,
     185,   186,     0,     0,   205,    99,   114,   114,     0,   105,
      94,     0,     0,     0,    36,   114,   114,     0,   105,   120,
       0,     0,     0,     0,    37,   169,   170,    99,     0,   120,
      56,    57,     0,    95,    96,     0,   114,   114,   114,   114,
     114,     0,     0,     0,   105,     0,    90,     0,   100,   100,
       0,     0,   106,   120,     0,     0,     0,   120,     0,   120,
     100,   120,     0,     0,   120,     0,     0,   155,  -126,   120,
    -126,  -126,  -126,  -126,   120,     0,   155,    32,    33,     0,
       0,     0,     0,    34,    52,    35,     0,     0,     0,     0,
       0,   131,     0,   140,   100,   100,     0,     0,     0,     0,
      53,     0,   241,     0,    36,  -123,   114,  -123,  -123,  -123,
    -123,     0,     0,     0,    37,   114,     0,     0,     0,   158,
      56,    57,     0,   173,   173,   173,   173,   173,   173,   173,
     146,   147,   148,   149,   150,   151,     0,    90,     0,     0,
       0,   114,     0,   100,   100,   100,   100,   100,   100,   100,
     100,  -125,     0,  -125,  -125,  -125,  -125,     0,   100,   173,
       0,     0,   100,    46,    47,    48,  -110,    49,    32,    33,
      50,  -110,     0,     0,    34,    52,    35,  -123,     0,  -123,
    -123,  -123,  -123,   126,   127,   128,   129,   130,   131,     0,
     132,    53,     0,     0,     0,    36,     0,     0,   100,     0,
       0,     0,    54,     0,    55,    37,     0,     0,     0,  -110,
       0,    56,    57,     0,     0,     0,    58,    59,    60,    61,
     100,     0,     0,   199,    46,    47,    48,     0,    49,    32,
      33,    50,    32,    33,    51,    34,    52,    35,    34,    52,
      35,   133,     0,   134,   135,   136,   137,     0,     0,     0,
       0,     0,    53,     0,     0,   113,    36,     0,     0,    36,
       0,     0,     0,    54,     0,    55,    37,     0,     0,    37,
       0,     0,    56,    57,     0,    56,    57,    58,    59,    60,
      61,    46,    47,    48,     0,    49,    32,    33,    50,     0,
       0,   119,    34,    52,    35,     0,     0,     0,     0,    46,
      47,    48,     0,    49,    32,    33,    50,     0,     0,    53,
      34,    52,    35,    36,     0,   159,     0,     0,     0,     0,
      54,     0,    55,    37,     0,     0,     0,    53,     0,    56,
      57,    36,     0,     0,    58,    59,    60,    61,    54,     0,
      55,    37,     0,     0,     0,     0,     0,    56,    57,     0,
       0,     0,    58,    59,    60,    61,    46,    47,    48,     0,
      49,    32,    33,    50,     0,     0,   189,    34,    52,    35,
    -124,     0,  -124,  -124,  -124,  -124,     0,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    54,     0,    55,    37,     0,
       0,     0,     0,     0,    56,    57,     0,     0,     0,    58,
      59,    60,    61,    46,    47,    48,     0,    49,    32,    33,
      50,     0,     0,   213,    34,    52,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,    54,     0,    55,    37,     0,     0,     0,     0,
       0,    56,    57,     0,     0,     0,    58,    59,    60,    61,
      46,    47,    48,     0,    49,    32,    33,    50,     0,     0,
     223,    34,    52,    35,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,    54,
       0,    55,    37,     0,     0,     0,     0,     0,    56,    57,
       0,     0,     0,    58,    59,    60,    61,    46,    47,    48,
       0,    49,    32,    33,    50,     0,     0,   233,    34,    52,
      35,     0,     0,     0,     0,    46,    47,    48,     0,    49,
      32,    33,    50,   237,     0,    53,    34,    52,    35,    36,
       0,     0,     0,     0,     0,     0,    54,     0,    55,    37,
       0,     0,     0,    53,     0,    56,    57,    36,     0,     0,
      58,    59,    60,    61,    54,     0,    55,    37,     0,     0,
       0,     0,     0,    56,    57,     0,     0,     0,    58,    59,
      60,    61,    46,    47,    48,     0,    49,    32,    33,    50,
       0,     0,   239,    34,    52,    35,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,    54,     0,    55,    37,     0,     0,     0,     0,     0,
      56,    57,     0,     0,     0,    58,    59,    60,    61,    46,
      47,    48,     0,    49,    32,    33,    50,     0,     0,   242,
      34,    52,    35,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,    54,     0,
      55,    37,     0,     0,     0,     0,     0,    56,    57,     0,
       0,     0,    58,    59,    60,    61,    46,    47,    48,     0,
      49,    32,    33,    50,     0,     0,   244,    34,    52,    35,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    54,     0,    55,    37,     0,
       0,     0,     0,     0,    56,    57,     0,     0,     0,    58,
      59,    60,    61,    46,    47,    48,     0,    49,    32,    33,
      50,     0,     0,   246,    34,    52,    35,     0,     0,     0,
       0,    46,    47,    48,     0,    49,    32,    33,    50,     0,
       0,    53,    34,    52,    35,    36,     0,     0,     0,     0,
       0,     0,    54,     0,    55,    37,     0,     0,     0,    53,
       0,    56,    57,    36,     0,     0,    58,    59,    60,    61,
      54,     0,    55,    37,     0,     0,     0,     0,     0,    56,
      57,     0,     0,     0,    58,    59,    60,    61
};

static const yytype_int16 yycheck[] =
{
      29,    53,    61,    54,    12,    54,   112,    59,    60,    24,
       3,     6,    24,    15,    31,    13,    49,    15,    47,    50,
      49,    50,    15,    11,    50,    19,   201,     8,     9,    46,
      24,    19,    61,    14,    48,    16,    95,    96,    15,    53,
     215,     0,    94,    19,    32,    33,    34,    62,    14,    51,
      62,   103,   104,    48,    35,    63,    11,    33,    34,    52,
     112,   113,    15,    94,    45,    94,    95,    96,    62,     6,
     121,    18,    15,    19,    51,    52,    23,    15,    33,    34,
      19,   133,   134,   135,   136,   137,    32,   146,   147,   148,
     149,   150,   151,    32,   143,   124,   125,   126,   127,   128,
     129,   130,    11,   152,   163,    50,   212,    18,    53,   138,
      19,    22,    23,   144,   145,   144,   145,   146,   147,   148,
     149,   150,   151,    32,    33,    34,    11,    19,   159,    24,
     159,   160,    15,    47,   163,    49,    50,    19,    31,    53,
      32,    55,    11,    60,    61,    59,    60,    61,    33,    34,
      32,   203,   125,   126,   127,   128,   129,   130,    33,    34,
     212,   210,   221,    15,    33,    34,    15,   216,   199,    31,
     199,   220,    63,   222,    32,    33,    34,   226,    19,    32,
      94,    95,    96,    60,    41,    15,   238,   160,   237,   103,
     104,    46,   221,    32,   243,    31,    18,    18,   112,   113,
      21,    22,    23,    59,    60,    64,    11,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   108,    13,   133,
     134,   135,   136,   137,   138,    13,    19,     8,    13,    18,
     144,   145,   146,   147,   148,   149,   150,   151,    55,    25,
      43,   203,    59,    60,   200,   159,   160,   103,   104,   163,
     215,   138,   164,    -1,    11,    18,   112,    20,    21,    22,
      23,    18,    -1,    20,    21,    22,    23,     8,     9,    -1,
      -1,    -1,    -1,    14,    15,    16,    -1,   133,   134,   135,
     136,   137,    -1,    -1,    47,   199,   103,   104,    -1,   203,
      31,    -1,    -1,    -1,    35,   112,   113,    -1,   212,   190,
      -1,    -1,    -1,    -1,    45,   122,   123,   221,    -1,   200,
      51,    52,    -1,    54,    55,    -1,   133,   134,   135,   136,
     137,    -1,    -1,    -1,   238,    -1,    47,    -1,    49,    50,
      -1,    -1,    53,   224,    -1,    -1,    -1,   228,    -1,   230,
      61,   232,    -1,    -1,   235,    -1,    -1,   203,    18,   240,
      20,    21,    22,    23,   245,    -1,   212,     8,     9,    -1,
      -1,    -1,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    41,    -1,    94,    95,    96,    -1,    -1,    -1,    -1,
      31,    -1,   238,    -1,    35,    18,   203,    20,    21,    22,
      23,    -1,    -1,    -1,    45,   212,    -1,    -1,    -1,    32,
      51,    52,    -1,   124,   125,   126,   127,   128,   129,   130,
      25,    26,    27,    28,    29,    30,    -1,   138,    -1,    -1,
      -1,   238,    -1,   144,   145,   146,   147,   148,   149,   150,
     151,    18,    -1,    20,    21,    22,    23,    -1,   159,   160,
      -1,    -1,   163,     3,     4,     5,    19,     7,     8,     9,
      10,    24,    -1,    -1,    14,    15,    16,    18,    -1,    20,
      21,    22,    23,    36,    37,    38,    39,    40,    41,    -1,
      43,    31,    -1,    -1,    -1,    35,    -1,    -1,   199,    -1,
      -1,    -1,    42,    -1,    44,    45,    -1,    -1,    -1,    62,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
     221,    -1,    -1,    63,     3,     4,     5,    -1,     7,     8,
       9,    10,     8,     9,    13,    14,    15,    16,    14,    15,
      16,    18,    -1,    20,    21,    22,    23,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    31,    35,    -1,    -1,    35,
      -1,    -1,    -1,    42,    -1,    44,    45,    -1,    -1,    45,
      -1,    -1,    51,    52,    -1,    51,    52,    56,    57,    58,
      59,     3,     4,     5,    -1,     7,     8,     9,    10,    -1,
      -1,    13,    14,    15,    16,    -1,    -1,    -1,    -1,     3,
       4,     5,    -1,     7,     8,     9,    10,    -1,    -1,    31,
      14,    15,    16,    35,    -1,    19,    -1,    -1,    -1,    -1,
      42,    -1,    44,    45,    -1,    -1,    -1,    31,    -1,    51,
      52,    35,    -1,    -1,    56,    57,    58,    59,    42,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,
      -1,    -1,    56,    57,    58,    59,     3,     4,     5,    -1,
       7,     8,     9,    10,    -1,    -1,    13,    14,    15,    16,
      18,    -1,    20,    21,    22,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    44,    45,    -1,
      -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,
      57,    58,    59,     3,     4,     5,    -1,     7,     8,     9,
      10,    -1,    -1,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
       3,     4,     5,    -1,     7,     8,     9,    10,    -1,    -1,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    57,    58,    59,     3,     4,     5,
      -1,     7,     8,     9,    10,    -1,    -1,    13,    14,    15,
      16,    -1,    -1,    -1,    -1,     3,     4,     5,    -1,     7,
       8,     9,    10,    11,    -1,    31,    14,    15,    16,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    44,    45,
      -1,    -1,    -1,    31,    -1,    51,    52,    35,    -1,    -1,
      56,    57,    58,    59,    42,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,    57,
      58,    59,     3,     4,     5,    -1,     7,     8,     9,    10,
      -1,    -1,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      51,    52,    -1,    -1,    -1,    56,    57,    58,    59,     3,
       4,     5,    -1,     7,     8,     9,    10,    -1,    -1,    13,
      14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,
      -1,    -1,    56,    57,    58,    59,     3,     4,     5,    -1,
       7,     8,     9,    10,    -1,    -1,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    44,    45,    -1,
      -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,
      57,    58,    59,     3,     4,     5,    -1,     7,     8,     9,
      10,    -1,    -1,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,     3,     4,     5,    -1,     7,     8,     9,    10,    -1,
      -1,    31,    14,    15,    16,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    -1,    44,    45,    -1,    -1,    -1,    31,
      -1,    51,    52,    35,    -1,    -1,    56,    57,    58,    59,
      42,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    51,
      52,    -1,    -1,    -1,    56,    57,    58,    59
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    49,    66,    67,    15,    51,     0,    50,    68,    69,
      14,    53,    69,    70,    71,    15,    48,    71,    73,    74,
      15,    72,    15,     6,    74,    75,    76,    13,    15,    24,
      15,    76,     8,     9,    14,    16,    35,    45,   107,    31,
      15,    77,    78,    19,    32,    78,     3,     4,     5,     7,
      10,    13,    15,    31,    42,    44,    51,    52,    56,    57,
      58,    59,    79,    82,    83,    84,    85,    86,    87,    88,
      94,    97,    98,    99,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    15,    95,    96,   103,
     104,   107,     8,     9,    31,    54,    55,    92,    93,   103,
     104,   107,    92,    31,    46,   103,   104,   105,    82,   102,
      79,   103,    31,    31,    79,   105,   106,   106,    93,    13,
      83,    19,    24,    62,    24,    62,    36,    37,    38,    39,
      40,    41,    43,    18,    20,    21,    22,    23,    19,    92,
     104,    93,    93,    11,    33,    34,    25,    26,    27,    28,
      29,    30,    11,    80,    81,   106,   106,    32,    32,    19,
      24,    80,   105,    60,   113,   114,    15,   102,   103,    79,
      79,   100,   103,   104,   107,   100,   100,   100,   100,   100,
     100,    15,   106,   106,   106,   106,   106,    96,    32,    13,
      82,    92,    92,    93,    93,    93,    93,    93,    93,    63,
      82,    89,    90,    19,    32,    47,    92,   100,    32,    93,
      61,   114,    31,    13,    92,    89,    12,    90,    91,    81,
      19,    64,    11,    13,    82,    80,    11,    91,    82,    13,
      82,    93,    82,    13,    32,    82,    13,    11,    19,    13,
      82,   106,    13,    11,    13,    82,    13
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
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
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
		  Type, Value); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
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
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

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

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

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

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
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

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

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
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:

/* Line 1455 of yacc.c  */
#line 171 "bison.y"
    {
	;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 175 "bison.y"
    {
		FKLOG("[bison]: package %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 182 "bison.y"
    {
		FKLOG("[bison]: package %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 190 "bison.y"
    {
	;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 200 "bison.y"
    {
		FKLOG("[bison]: include %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_include((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 209 "bison.y"
    {
	;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 219 "bison.y"
    {
		FKLOG("[bison]: struct_define %s", (yyvsp[(2) - (4)].str).c_str());
		myflexer *l = (myflexer *)parm;
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[(3) - (4)].syntree));
		l->add_struct_desc((yyvsp[(2) - (4)].str).c_str(), p);
	;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 229 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 234 "bison.y"
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
#line 243 "bison.y"
    {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER");
		NEWTYPE(p, struct_desc_memlist_node);
		p->add_arg((yyvsp[(1) - (1)].str));
		(yyval.syntree) = p;
	;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 253 "bison.y"
    {
	;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 263 "bison.y"
    {
		FKLOG("[bison]: const_define %s", (yyvsp[(2) - (4)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_const_desc((yyvsp[(2) - (4)].str).c_str(), (yyvsp[(4) - (4)].syntree));
	;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 272 "bison.y"
    {
	;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 284 "bison.y"
    {
		FKLOG("[bison]: function_declaration <- block %s", (yyvsp[(2) - (7)].str).c_str());
		NEWTYPE(p, func_desc_node);
		p->funcname = (yyvsp[(2) - (7)].str);
		p->arglist = dynamic_cast<func_desc_arglist_node*>((yyvsp[(4) - (7)].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[(6) - (7)].syntree));
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 295 "bison.y"
    {
		FKLOG("[bison]: function_declaration <- empty %s", (yyvsp[(2) - (6)].str).c_str());
		NEWTYPE(p, func_desc_node);
		p->funcname = (yyvsp[(2) - (6)].str);
		p->arglist = 0;
		p->block = 0;
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 308 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 313 "bison.y"
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
#line 322 "bison.y"
    {
		FKLOG("[bison]: function_declaration_arguments <- arg");
		NEWTYPE(p, func_desc_arglist_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 332 "bison.y"
    {
		FKLOG("[bison]: arg <- IDENTIFIER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, identifier_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 342 "bison.y"
    {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[(1) - (4)].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[(1) - (4)].str);
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[(3) - (4)].syntree));
		p->fakecall = false;
		(yyval.syntree) = p;
	;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 352 "bison.y"
    {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[(1) - (4)].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[(1) - (4)].str);
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[(3) - (4)].syntree));
		p->fakecall = false;
		(yyval.syntree) = p;
	;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 362 "bison.y"
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
		(yyval.syntree) = p;
	;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 380 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 385 "bison.y"
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
#line 394 "bison.y"
    {
		FKLOG("[bison]: function_call_arguments <- arg_expr");
		NEWTYPE(p, function_call_arglist_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 404 "bison.y"
    {
		FKLOG("[bison]: arg_expr <- expr_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 414 "bison.y"
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
#line 423 "bison.y"
    {
		FKLOG("[bison]: block <- stmt");
		NEWTYPE(p, block_node);
		p->add_stmt((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 433 "bison.y"
    {
		FKLOG("[bison]: stmt <- while_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 439 "bison.y"
    {
		FKLOG("[bison]: stmt <- if_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 445 "bison.y"
    {
		FKLOG("[bison]: stmt <- return_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 451 "bison.y"
    {
		FKLOG("[bison]: stmt <- assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 457 "bison.y"
    {
		FKLOG("[bison]: stmt <- multi_assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 463 "bison.y"
    {
		FKLOG("[bison]: stmt <- break");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 469 "bison.y"
    {
		FKLOG("[bison]: stmt <- continue");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 475 "bison.y"
    {
		FKLOG("[bison]: stmt <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 481 "bison.y"
    {
		FKLOG("[bison]: stmt <- math_assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 487 "bison.y"
    {
		FKLOG("[bison]: stmt <- for_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 493 "bison.y"
    {
		FKLOG("[bison]: stmt <- for_loop_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 499 "bison.y"
    {
		FKLOG("[bison]: stmt <- fake_call_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 505 "bison.y"
    {
		FKLOG("[bison]: stmt <- sleep_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 511 "bison.y"
    {
		FKLOG("[bison]: stmt <- yield_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 517 "bison.y"
    {
		FKLOG("[bison]: stmt <- switch_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 525 "bison.y"
    {
		FKLOG("[bison]: fake_call_stmt <- fake function_call");
		function_call_node * p = dynamic_cast<function_call_node*>((yyvsp[(2) - (2)].syntree));
		p->fakecall = true;
		(yyval.syntree) = p;
	;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 535 "bison.y"
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
#line 546 "bison.y"
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
#line 559 "bison.y"
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
#line 571 "bison.y"
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
#line 585 "bison.y"
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
#line 594 "bison.y"
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
#line 605 "bison.y"
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
#line 616 "bison.y"
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
#line 629 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 634 "bison.y"
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
#line 643 "bison.y"
    {
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt");
		NEWTYPE(p, elseif_stmt_list);
		p->add_stmt((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 653 "bison.y"
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
#line 662 "bison.y"
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
#line 673 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 678 "bison.y"
    {
		FKLOG("[bison]: else_stmt <- block");
		NEWTYPE(p, else_stmt);
		p->block = dynamic_cast<block_node*>((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 686 "bison.y"
    {
		FKLOG("[bison]: else_stmt <- empty");
		NEWTYPE(p, else_stmt);
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 696 "bison.y"
    {
		FKLOG("[bison]: cmp <- ( cmp )");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 702 "bison.y"
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
#line 712 "bison.y"
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
#line 722 "bison.y"
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
#line 732 "bison.y"
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
#line 742 "bison.y"
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
#line 752 "bison.y"
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
#line 762 "bison.y"
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
#line 772 "bison.y"
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
#line 782 "bison.y"
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
#line 792 "bison.y"
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
#line 802 "bison.y"
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
#line 812 "bison.y"
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
#line 824 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 830 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 836 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 844 "bison.y"
    {
		FKLOG("[bison]: return_stmt <- RETURN return_value_list");
		NEWTYPE(p, return_stmt);
		p->returnlist = dynamic_cast<return_value_list_node*>((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 852 "bison.y"
    {
		FKLOG("[bison]: return_stmt <- RETURN");
		NEWTYPE(p, return_stmt);
		p->returnlist = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 862 "bison.y"
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
#line 871 "bison.y"
    {
		NEWTYPE(p, return_value_list_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 880 "bison.y"
    {
		FKLOG("[bison]: return_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 886 "bison.y"
    {
		FKLOG("[bison]: return_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 892 "bison.y"
    {
		FKLOG("[bison]: return_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 900 "bison.y"
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
#line 910 "bison.y"
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
#line 922 "bison.y"
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
#line 932 "bison.y"
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
#line 944 "bison.y"
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
#line 953 "bison.y"
    {
		NEWTYPE(p, var_list_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 962 "bison.y"
    {
		FKLOG("[bison]: assign_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 968 "bison.y"
    {
		FKLOG("[bison]: assign_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 974 "bison.y"
    {
		FKLOG("[bison]: assign_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 982 "bison.y"
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
#line 992 "bison.y"
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
#line 1002 "bison.y"
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
#line 1012 "bison.y"
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
#line 1022 "bison.y"
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
#line 1032 "bison.y"
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
#line 1048 "bison.y"
    {
		FKLOG("[bison]: var <- VAR_BEGIN IDENTIFIER %s", (yyvsp[(2) - (2)].str).c_str());
		NEWTYPE(p, var_node);
		p->str = (yyvsp[(2) - (2)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1056 "bison.y"
    {
		FKLOG("[bison]: var <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1064 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1072 "bison.y"
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
#line 1081 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER_POINTER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, struct_pointer_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1091 "bison.y"
    {
		FKLOG("[bison]: expr <- (expr)");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1097 "bison.y"
    {
		FKLOG("[bison]: expr <- function_call");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1103 "bison.y"
    {
		FKLOG("[bison]: expr <- math_expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1111 "bison.y"
    {
		FKLOG("[bison]: math_expr <- (math_expr)");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1117 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "+";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1127 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "-";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1137 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "*";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1147 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "/";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1157 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "%";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1169 "bison.y"
    {
		FKLOG("[bison]: expr_value <- math_expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1175 "bison.y"
    {
		FKLOG("[bison]: expr_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1181 "bison.y"
    {
		FKLOG("[bison]: expr_value <- function_call");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1187 "bison.y"
    {
		FKLOG("[bison]: expr_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1195 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FTRUE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_TRUE;
		(yyval.syntree) = p;
	;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1204 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FFALSE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_FALSE;
		(yyval.syntree) = p;
	;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1213 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- NUMBER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_NUM;
		(yyval.syntree) = p;
	;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1222 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FKUUID %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_UUID;
		(yyval.syntree) = p;
	;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1231 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- STRING_DEFINITION %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_STR;
		(yyval.syntree) = p;
	;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1240 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FKFLOAT %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_FLOAT;
		(yyval.syntree) = p;
	;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1251 "bison.y"
    {
		FKLOG("[bison]: break <- BREAK");
		NEWTYPE(p, break_stmt);
		(yyval.syntree) = p;
	;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1260 "bison.y"
    {
		FKLOG("[bison]: CONTINUE");
		NEWTYPE(p, continue_stmt);
		(yyval.syntree) = p;
	;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1269 "bison.y"
    {
		FKLOG("[bison]: SLEEP");
		NEWTYPE(p, sleep_stmt);
		p->time = (yyvsp[(2) - (2)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1278 "bison.y"
    {
		FKLOG("[bison]: YIELD");
		NEWTYPE(p, yield_stmt);
		p->time = (yyvsp[(2) - (2)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1288 "bison.y"
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[(2) - (6)].syntree);
		p->caselist = (yyvsp[(3) - (6)].syntree);
		p->def = (yyvsp[(5) - (6)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1298 "bison.y"
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[(2) - (5)].syntree);
		p->caselist = (yyvsp[(3) - (5)].syntree);
		p->def = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1310 "bison.y"
    {
		FKLOG("[bison]: switch_case_list <- switch_case_define");
		NEWTYPE(p, switch_caselist_node);
		p->add_case((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1318 "bison.y"
    {
		FKLOG("[bison]: switch_case_list <- switch_case_list switch_case_define");
		assert((yyvsp[(2) - (2)].syntree)->gettype() == est_switch_case_node);
		switch_caselist_node * p = dynamic_cast<switch_caselist_node*>((yyvsp[(1) - (2)].syntree));
		p->add_case((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1329 "bison.y"
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[(2) - (4)].syntree);
		p->block = (yyvsp[(4) - (4)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1338 "bison.y"
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[(2) - (3)].syntree);
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;



/* Line 1455 of yacc.c  */
#line 3419 "bison.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

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
		      yytoken, &yylval);
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


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
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
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



