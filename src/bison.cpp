
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
     TRUE = 263,
     FALSE = 264,
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
     DEFAULT = 316
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 210 "bison.tab.c"

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
#define YYLAST   860

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  62
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  47
/* YYNRULES -- Number of rules.  */
#define YYNRULES  132
/* YYNRULES -- Number of states.  */
#define YYNSTATES  223

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   316

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
      55,    56,    57,    58,    59,    60,    61
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
     136,   138,   140,   142,   145,   155,   164,   170,   175,   182,
     188,   189,   192,   194,   198,   202,   206,   210,   214,   218,
     222,   226,   230,   232,   234,   237,   240,   242,   244,   246,
     249,   251,   255,   257,   259,   261,   263,   267,   271,   275,
     277,   279,   281,   283,   287,   291,   295,   299,   303,   306,
     309,   311,   313,   318,   320,   324,   326,   328,   332,   336,
     340,   344,   348,   352,   354,   356,   358,   360,   362,   364,
     366,   368,   370,   372,   374,   376,   379,   382,   389,   395,
     397,   400,   405
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      63,     0,    -1,    64,    65,    67,    70,    72,    -1,    -1,
      49,    15,    -1,    49,    51,    -1,    -1,    66,    -1,    65,
      66,    -1,    50,    14,    -1,    -1,    68,    -1,    67,    68,
      -1,    53,    15,    69,    13,    -1,    -1,    69,    15,    -1,
      15,    -1,    -1,    71,    -1,    70,    71,    -1,    48,    15,
      24,   101,    -1,    -1,    73,    -1,    72,    73,    -1,     6,
      15,    31,    74,    32,    79,    13,    -1,     6,    15,    31,
      74,    32,    13,    -1,    -1,    74,    19,    75,    -1,    75,
      -1,    15,    -1,    15,    31,    77,    32,    -1,    51,    31,
      77,    32,    -1,    97,    41,    15,    31,    77,    32,    -1,
      -1,    77,    19,    78,    -1,    78,    -1,   100,    -1,    79,
      80,    -1,    80,    -1,    83,    -1,    84,    -1,    88,    -1,
      91,    -1,    92,    -1,   102,    -1,   103,    -1,    98,    -1,
      95,    -1,    82,    -1,    81,    -1,   104,    -1,   105,    -1,
     106,    -1,    44,    76,    -1,    42,    79,    19,    86,    19,
      79,    11,    79,    13,    -1,    42,    79,    19,    86,    19,
      79,    11,    13,    -1,     7,    86,    11,    79,    13,    -1,
       7,    86,    11,    13,    -1,    10,    86,    11,    79,    85,
      13,    -1,    10,    86,    11,    85,    13,    -1,    -1,    12,
      79,    -1,    12,    -1,    31,    86,    32,    -1,    86,    33,
      86,    -1,    86,    34,    86,    -1,    87,    26,    87,    -1,
      87,    25,    87,    -1,    87,    29,    87,    -1,    87,    27,
      87,    -1,    87,    28,    87,    -1,    87,    30,    87,    -1,
       8,    -1,     9,    -1,    54,    87,    -1,    55,    87,    -1,
     101,    -1,    97,    -1,    98,    -1,     4,    89,    -1,     4,
      -1,    89,    19,    90,    -1,    90,    -1,   101,    -1,    97,
      -1,    98,    -1,    96,    24,    94,    -1,    93,    24,    76,
      -1,    93,    19,    96,    -1,    96,    -1,   101,    -1,    97,
      -1,    98,    -1,    97,    36,    94,    -1,    97,    37,    94,
      -1,    97,    38,    94,    -1,    97,    39,    94,    -1,    97,
      40,    94,    -1,    97,    43,    -1,     3,    15,    -1,    97,
      -1,    15,    -1,    15,    46,   100,    47,    -1,    52,    -1,
      31,    98,    32,    -1,    76,    -1,    99,    -1,    31,    99,
      32,    -1,   100,    20,   100,    -1,   100,    21,   100,    -1,
     100,    23,   100,    -1,   100,    22,   100,    -1,   100,    18,
     100,    -1,    99,    -1,   101,    -1,    76,    -1,    97,    -1,
       8,    -1,     9,    -1,    16,    -1,    45,    -1,    14,    -1,
      35,    -1,     5,    -1,    56,    -1,    58,   100,    -1,    57,
     100,    -1,    59,    87,   107,    61,    79,    13,    -1,    59,
      87,   107,    61,    13,    -1,   108,    -1,   107,   108,    -1,
      60,    87,    11,    79,    -1,    60,    87,    11,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   157,   157,   166,   169,   176,   185,   188,   190,   194,
     204,   207,   209,   213,   224,   228,   237,   248,   251,   253,
     257,   267,   270,   272,   278,   289,   303,   307,   316,   326,
     336,   346,   356,   375,   379,   388,   398,   408,   417,   427,
     433,   439,   445,   451,   457,   463,   469,   475,   481,   487,
     493,   499,   505,   513,   523,   534,   547,   556,   567,   577,
     590,   594,   602,   612,   618,   628,   638,   648,   658,   668,
     678,   688,   698,   708,   718,   728,   740,   746,   752,   760,
     768,   778,   787,   796,   802,   808,   816,   827,   838,   847,
     856,   862,   868,   876,   886,   896,   906,   916,   926,   942,
     950,   958,   966,   975,   985,   991,   997,  1005,  1011,  1021,
    1031,  1041,  1051,  1063,  1069,  1075,  1081,  1089,  1098,  1107,
    1116,  1125,  1134,  1145,  1154,  1163,  1172,  1182,  1192,  1204,
    1212,  1223,  1232
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "VAR_BEGIN", "RETURN", "BREAK", "FUNC",
  "WHILE", "TRUE", "FALSE", "IF", "THEN", "ELSE", "END",
  "STRING_DEFINITION", "IDENTIFIER", "NUMBER", "SINGLE_LINE_COMMENT",
  "DIVIDE_MOD", "ARG_SPLITTER", "PLUS", "MINUS", "DIVIDE", "MULTIPLY",
  "ASSIGN", "MORE", "LESS", "MORE_OR_EQUAL", "LESS_OR_EQUAL", "EQUAL",
  "NOT_EQUAL", "OPEN_BRACKET", "CLOSE_BRACKET", "AND", "OR", "FKFLOAT",
  "PLUS_ASSIGN", "MINUS_ASSIGN", "DIVIDE_ASSIGN", "MULTIPLY_ASSIGN",
  "DIVIDE_MOD_ASSIGN", "COLON", "FOR", "INC", "FAKE", "FKUUID",
  "OPEN_SQUARE_BRACKET", "CLOSE_SQUARE_BRACKET", "FCONST", "PACKAGE",
  "INCLUDE", "IDENTIFIER_DOT", "IDENTIFIER_POINTER", "STRUCT", "IS", "NOT",
  "CONTINUE", "YIELD", "SLEEP", "SWITCH", "CASE", "DEFAULT", "$accept",
  "program", "package_head", "include_head", "include_define",
  "struct_head", "struct_define", "struct_mem_declaration", "const_head",
  "const_define", "body", "function_declaration",
  "function_declaration_arguments", "arg", "function_call",
  "function_call_arguments", "arg_expr", "block", "stmt", "fake_call_stmt",
  "for_stmt", "while_stmt", "if_stmt", "else_stmt", "cmp", "cmp_value",
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
     315,   316
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    62,    63,    64,    64,    64,    65,    65,    65,    66,
      67,    67,    67,    68,    69,    69,    69,    70,    70,    70,
      71,    72,    72,    72,    73,    73,    74,    74,    74,    75,
      76,    76,    76,    77,    77,    77,    78,    79,    79,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    81,    82,    82,    83,    83,    84,    84,
      85,    85,    85,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    87,    87,    87,    88,
      88,    89,    89,    90,    90,    90,    91,    92,    93,    93,
      94,    94,    94,    95,    95,    95,    95,    95,    95,    96,
      96,    97,    97,    97,    98,    98,    98,    99,    99,    99,
      99,    99,    99,   100,   100,   100,   100,   101,   101,   101,
     101,   101,   101,   102,   103,   104,   105,   106,   106,   107,
     107,   108,   108
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     5,     0,     2,     2,     0,     1,     2,     2,
       0,     1,     2,     4,     0,     2,     1,     0,     1,     2,
       4,     0,     1,     2,     7,     6,     0,     3,     1,     1,
       4,     4,     6,     0,     3,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     9,     8,     5,     4,     6,     5,
       0,     2,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     2,     2,     1,     1,     1,     2,
       1,     3,     1,     1,     1,     1,     3,     3,     3,     1,
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
       0,    23,   117,   118,   121,   119,   122,   120,    20,    26,
      29,     0,    28,     0,     0,    27,     0,    80,   123,     0,
       0,    25,   101,     0,     0,     0,     0,   103,   124,     0,
       0,     0,   105,     0,    38,    49,    48,    39,    40,    41,
      42,    43,     0,    47,    89,   116,    46,   106,     0,   114,
      44,    45,    50,    51,    52,    99,    79,    82,    84,    85,
      83,   117,   118,     0,     0,     0,     0,     0,    77,    78,
      76,     0,    33,     0,   116,     0,   106,     0,    53,     0,
      33,     0,   115,   113,   126,   125,     0,    24,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    98,     0,
       0,     0,     0,     0,     0,     0,    78,    74,    75,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    60,     0,
      35,    36,     0,   104,   107,     0,     0,   113,     0,     0,
     129,   101,    88,   100,    87,    86,    91,    92,    90,    93,
      94,    95,    96,    97,     0,   112,   108,   109,   111,   110,
      81,    63,    57,     0,    64,    65,    67,    66,    69,    70,
      68,    71,    62,    60,     0,     0,    30,   102,     0,    31,
       0,     0,   130,    33,    56,    61,     0,    59,    34,     0,
     132,   128,     0,     0,    58,     0,   131,   127,    32,     0,
      55,     0,    54
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     8,     9,    13,    14,    21,    18,    19,
      25,    26,    41,    42,    62,   149,   150,    63,    64,    65,
      66,    67,    68,   194,    96,    97,    69,    86,    87,    70,
      71,    72,   165,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,   159,   160
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -109
static const yytype_int16 yypact[] =
{
     -36,   -12,    24,    25,  -109,  -109,  -109,    41,    -4,  -109,
    -109,    69,  -109,   -34,  -109,    78,    96,  -109,    -1,  -109,
    -109,    59,    93,   112,  -109,   140,  -109,  -109,  -109,    26,
     120,  -109,  -109,  -109,  -109,  -109,  -109,  -109,  -109,   143,
    -109,    11,  -109,   143,   219,  -109,   156,    13,  -109,   805,
     805,  -109,    61,    13,   766,    18,   145,  -109,  -109,   318,
     318,    13,   501,   352,  -109,  -109,  -109,  -109,  -109,  -109,
    -109,  -109,   101,  -109,   155,   489,  -109,   558,   564,  -109,
    -109,  -109,  -109,  -109,  -109,  -109,   164,  -109,   288,  -109,
     575,     4,   107,   805,    13,    13,    -2,   211,   288,  -109,
     575,   111,   318,   318,   144,   160,   518,   370,  -109,   144,
     318,   318,  -109,  -109,   564,   564,   157,  -109,  -109,     5,
      18,    13,    13,    13,    13,    13,    13,   189,  -109,   318,
     318,   318,   318,   318,    13,    76,   160,  -109,  -109,   427,
     805,   805,    13,    13,    13,    13,    13,    13,   445,    22,
    -109,   564,   168,  -109,  -109,   805,    34,   184,    13,    88,
    -109,   172,  -109,  -109,  -109,  -109,   288,  -109,   575,  -109,
    -109,  -109,  -109,  -109,   190,  -109,   377,   159,   119,   202,
    -109,  -109,  -109,   502,   135,   135,  -109,  -109,  -109,  -109,
    -109,  -109,   766,   445,   217,   318,  -109,  -109,    -8,  -109,
     220,   559,  -109,   318,  -109,   766,   229,  -109,  -109,   766,
     766,  -109,   616,   138,  -109,   634,   766,  -109,  -109,   691,
    -109,   748,  -109
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -109,  -109,  -109,  -109,   236,  -109,   234,  -109,  -109,   230,
    -109,   231,  -109,   212,    64,  -108,    63,   -54,   -62,  -109,
    -109,  -109,  -109,    73,    23,    67,  -109,  -109,   126,  -109,
    -109,  -109,   171,  -109,   149,   -43,   196,     3,    70,   158,
    -109,  -109,  -109,  -109,  -109,  -109,   103
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -117
static const yytype_int16 yytable[] =
{
     107,   118,   156,     4,    88,    23,    98,    98,    46,   139,
     104,   209,   109,     1,    16,   -72,   104,   104,    98,    11,
     161,    32,    33,   -72,     6,   140,   141,    34,    52,    35,
      43,   140,   141,    52,    32,    33,   -72,   -72,   -72,     5,
      34,   195,    35,    44,    53,   118,     7,    16,    36,    11,
      98,    98,    98,   195,   196,    10,   106,    57,    37,   104,
     104,    36,   113,   113,    56,    57,   199,   104,   104,    56,
      57,    37,    27,   101,    28,     7,   163,   109,   166,   166,
     166,   166,   166,   166,    15,   183,   104,   104,   104,   104,
     104,    88,   102,    20,   193,   213,   106,    98,    98,    98,
      98,    98,    98,    98,    98,   113,   113,   103,   181,   140,
     141,    22,    98,   113,   157,    98,   135,    29,   -73,   108,
     119,   118,   148,   112,   112,   120,   -73,    30,   116,   114,
     115,   118,   113,   113,   113,   113,   113,   129,   205,   -73,
     -73,   -73,   133,   118,   140,   141,    23,   212,   158,   201,
     118,    39,   104,   118,   118,   215,   216,   195,    40,   118,
     104,   137,   138,   184,   185,   221,   112,   112,   140,   141,
     218,    85,   151,   152,   112,   112,   110,   129,   198,   121,
     151,   132,   133,   134,   164,   127,   129,    38,   130,   131,
     132,   133,   153,   112,   112,   112,   112,   112,   113,   175,
     176,   177,   178,   179,   174,    90,   113,   100,   100,   186,
     187,   188,   189,   190,   191,   197,   154,   158,   103,   100,
     129,   203,    46,    47,    48,   200,    49,    32,    33,    50,
     207,   210,    51,    34,    52,    35,   142,   143,   144,   145,
     146,   147,   214,    89,    12,    99,    99,    17,    24,   105,
      53,   100,   100,   100,    36,    45,    31,    99,   208,   112,
     180,    54,   202,    55,    37,   151,   206,   112,   162,     0,
      56,    57,     0,   151,     0,    58,    59,    60,    61,   168,
     168,   168,   168,   168,   168,     0,     0,     0,     0,   136,
      99,    99,    90,   169,   170,   171,   172,   173,   100,   100,
     100,   100,   100,   100,   100,   100,  -116,     0,  -116,  -116,
    -116,  -116,     0,   100,     0,     0,   100,   167,   167,   167,
     167,   167,   167,     0,     0,     0,    32,    33,     0,   127,
      89,     0,    34,    52,    35,     0,    99,    99,    99,    99,
      99,    99,    99,    99,     0,     0,     0,     0,     0,   111,
       0,    99,     0,    36,    99,    46,    47,    48,     0,    49,
      32,    33,    50,    37,     0,   117,    34,    52,    35,    56,
      57,     0,     0,    46,    47,    48,     0,    49,    32,    33,
      50,     0,     0,    53,    34,    52,    35,    36,     0,   155,
       0,     0,     0,     0,    54,   129,    55,    37,   131,   132,
     133,    53,     0,    56,    57,    36,     0,     0,    58,    59,
      60,    61,    54,     0,    55,    37,     0,     0,     0,     0,
       0,    56,    57,     0,     0,     0,    58,    59,    60,    61,
      46,    47,    48,     0,    49,    32,    33,    50,     0,     0,
     182,    34,    52,    35,     0,     0,     0,     0,    46,    47,
      48,     0,    49,    32,    33,    50,     0,   192,    53,    34,
      52,    35,    36,     0,     0,     0,     0,     0,     0,    54,
       0,    55,    37,     0,     0,     0,    53,     0,    56,    57,
      36,     0,     0,    58,    59,    60,    61,    54,     0,    55,
      37,     0,     0,     0,     0,     0,    56,    57,     0,     0,
       0,    58,    59,    60,    61,    46,    47,    48,  -100,    49,
      32,    33,    50,  -100,     0,   204,    34,    52,    35,  -115,
       0,  -115,  -115,  -115,  -115,   122,   123,   124,   125,   126,
     127,     0,   128,    53,     0,     0,  -113,    36,  -113,  -113,
    -113,  -113,     0,     0,    54,     0,    55,    37,     0,     0,
     154,     0,     0,    56,    57,     0,     0,     0,    58,    59,
      60,    61,    46,    47,    48,     0,    49,    32,    33,    50,
       0,     0,   211,    34,    52,    35,  -113,     0,  -113,  -113,
    -113,  -113,   129,     0,   130,   131,   132,   133,     0,     0,
      53,     0,     0,  -114,    36,  -114,  -114,  -114,  -114,     0,
       0,    54,     0,    55,    37,     0,     0,     0,     0,     0,
      56,    57,     0,     0,     0,    58,    59,    60,    61,    46,
      47,    48,     0,    49,    32,    33,    50,     0,     0,   217,
      34,    52,    35,     0,     0,     0,     0,    46,    47,    48,
       0,    49,    32,    33,    50,   219,     0,    53,    34,    52,
      35,    36,     0,     0,     0,     0,     0,     0,    54,     0,
      55,    37,     0,     0,     0,    53,     0,    56,    57,    36,
       0,     0,    58,    59,    60,    61,    54,     0,    55,    37,
       0,     0,     0,     0,     0,    56,    57,     0,     0,     0,
      58,    59,    60,    61,    46,    47,    48,     0,    49,    32,
      33,    50,     0,     0,   220,    34,    52,    35,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    54,     0,    55,    37,     0,     0,     0,
       0,     0,    56,    57,     0,     0,     0,    58,    59,    60,
      61,    46,    47,    48,     0,    49,    32,    33,    50,     0,
       0,   222,    34,    52,    35,     0,     0,     0,     0,    46,
      47,    48,     0,    49,    32,    33,    50,     0,     0,    53,
      34,    52,    35,    36,     0,     0,     0,     0,     0,     0,
      54,     0,    55,    37,     0,     0,     0,    53,     0,    56,
      57,    36,     0,     0,    58,    59,    60,    61,    54,     0,
      55,    37,     0,    91,    92,     0,     0,    56,    57,    34,
      52,    35,    58,    59,    60,    61,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    93,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,    56,    57,     0,    94,
      95
};

static const yytype_int16 yycheck[] =
{
      54,    63,   110,    15,    47,     6,    49,    50,     3,    11,
      53,    19,    55,    49,    48,    11,    59,    60,    61,    53,
      15,     8,     9,    19,     0,    33,    34,    14,    15,    16,
      19,    33,    34,    15,     8,     9,    32,    33,    34,    51,
      14,    19,    16,    32,    31,   107,    50,    48,    35,    53,
      93,    94,    95,    19,    32,    14,    53,    52,    45,   102,
     103,    35,    59,    60,    51,    52,    32,   110,   111,    51,
      52,    45,    13,    50,    15,    50,   119,   120,   121,   122,
     123,   124,   125,   126,    15,   139,   129,   130,   131,   132,
     133,   134,    31,    15,   148,   203,    93,   140,   141,   142,
     143,   144,   145,   146,   147,   102,   103,    46,    32,    33,
      34,    15,   155,   110,   111,   158,    93,    24,    11,    55,
      19,   183,    11,    59,    60,    24,    19,    15,    61,    59,
      60,   193,   129,   130,   131,   132,   133,    18,   192,    32,
      33,    34,    23,   205,    33,    34,     6,   201,    60,    61,
     212,    31,   195,   215,   216,   209,   210,    19,    15,   221,
     203,    94,    95,   140,   141,   219,   102,   103,    33,    34,
      32,    15,   102,   103,   110,   111,    31,    18,   155,    24,
     110,    22,    23,    19,   120,    41,    18,    29,    20,    21,
      22,    23,    32,   129,   130,   131,   132,   133,   195,   129,
     130,   131,   132,   133,    15,    47,   203,    49,    50,   142,
     143,   144,   145,   146,   147,    47,    32,    60,    46,    61,
      18,    31,     3,     4,     5,   158,     7,     8,     9,    10,
      13,    11,    13,    14,    15,    16,    25,    26,    27,    28,
      29,    30,    13,    47,     8,    49,    50,    13,    18,    53,
      31,    93,    94,    95,    35,    43,    25,    61,   195,   195,
     134,    42,   159,    44,    45,   195,   193,   203,   119,    -1,
      51,    52,    -1,   203,    -1,    56,    57,    58,    59,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,    93,
      94,    95,   134,   122,   123,   124,   125,   126,   140,   141,
     142,   143,   144,   145,   146,   147,    18,    -1,    20,    21,
      22,    23,    -1,   155,    -1,    -1,   158,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,     8,     9,    -1,    41,
     134,    -1,    14,    15,    16,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   155,    -1,    35,   158,     3,     4,     5,    -1,     7,
       8,     9,    10,    45,    -1,    13,    14,    15,    16,    51,
      52,    -1,    -1,     3,     4,     5,    -1,     7,     8,     9,
      10,    -1,    -1,    31,    14,    15,    16,    35,    -1,    19,
      -1,    -1,    -1,    -1,    42,    18,    44,    45,    21,    22,
      23,    31,    -1,    51,    52,    35,    -1,    -1,    56,    57,
      58,    59,    42,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
       3,     4,     5,    -1,     7,     8,     9,    10,    -1,    -1,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,     3,     4,
       5,    -1,     7,     8,     9,    10,    -1,    12,    31,    14,
      15,    16,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    44,    45,    -1,    -1,    -1,    31,    -1,    51,    52,
      35,    -1,    -1,    56,    57,    58,    59,    42,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,     3,     4,     5,    19,     7,
       8,     9,    10,    24,    -1,    13,    14,    15,    16,    18,
      -1,    20,    21,    22,    23,    36,    37,    38,    39,    40,
      41,    -1,    43,    31,    -1,    -1,    18,    35,    20,    21,
      22,    23,    -1,    -1,    42,    -1,    44,    45,    -1,    -1,
      32,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,    57,
      58,    59,     3,     4,     5,    -1,     7,     8,     9,    10,
      -1,    -1,    13,    14,    15,    16,    18,    -1,    20,    21,
      22,    23,    18,    -1,    20,    21,    22,    23,    -1,    -1,
      31,    -1,    -1,    18,    35,    20,    21,    22,    23,    -1,
      -1,    42,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      51,    52,    -1,    -1,    -1,    56,    57,    58,    59,     3,
       4,     5,    -1,     7,     8,     9,    10,    -1,    -1,    13,
      14,    15,    16,    -1,    -1,    -1,    -1,     3,     4,     5,
      -1,     7,     8,     9,    10,    11,    -1,    31,    14,    15,
      16,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      44,    45,    -1,    -1,    -1,    31,    -1,    51,    52,    35,
      -1,    -1,    56,    57,    58,    59,    42,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,
      56,    57,    58,    59,     3,     4,     5,    -1,     7,     8,
       9,    10,    -1,    -1,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,
      59,     3,     4,     5,    -1,     7,     8,     9,    10,    -1,
      -1,    13,    14,    15,    16,    -1,    -1,    -1,    -1,     3,
       4,     5,    -1,     7,     8,     9,    10,    -1,    -1,    31,
      14,    15,    16,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    -1,    44,    45,    -1,    -1,    -1,    31,    -1,    51,
      52,    35,    -1,    -1,    56,    57,    58,    59,    42,    -1,
      44,    45,    -1,     8,     9,    -1,    -1,    51,    52,    14,
      15,    16,    56,    57,    58,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    54,
      55
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    49,    63,    64,    15,    51,     0,    50,    65,    66,
      14,    53,    66,    67,    68,    15,    48,    68,    70,    71,
      15,    69,    15,     6,    71,    72,    73,    13,    15,    24,
      15,    73,     8,     9,    14,    16,    35,    45,   101,    31,
      15,    74,    75,    19,    32,    75,     3,     4,     5,     7,
      10,    13,    15,    31,    42,    44,    51,    52,    56,    57,
      58,    59,    76,    79,    80,    81,    82,    83,    84,    88,
      91,    92,    93,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    15,    89,    90,    97,    98,
     101,     8,     9,    31,    54,    55,    86,    87,    97,    98,
     101,    86,    31,    46,    97,    98,    99,    79,    76,    97,
      31,    31,    76,    99,   100,   100,    87,    13,    80,    19,
      24,    24,    36,    37,    38,    39,    40,    41,    43,    18,
      20,    21,    22,    23,    19,    86,    98,    87,    87,    11,
      33,    34,    25,    26,    27,    28,    29,    30,    11,    77,
      78,   100,   100,    32,    32,    19,    77,    99,    60,   107,
     108,    15,    96,    97,    76,    94,    97,    98,   101,    94,
      94,    94,    94,    94,    15,   100,   100,   100,   100,   100,
      90,    32,    13,    79,    86,    86,    87,    87,    87,    87,
      87,    87,    12,    79,    85,    19,    32,    47,    86,    32,
      87,    61,   108,    31,    13,    79,    85,    13,    78,    19,
      11,    13,    79,    77,    13,    79,    79,    13,    32,    11,
      13,    79,    13
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
#line 166 "bison.y"
    {
	;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 170 "bison.y"
    {
		FKLOG("[bison]: package %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 177 "bison.y"
    {
		FKLOG("[bison]: package %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 185 "bison.y"
    {
	;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 195 "bison.y"
    {
		FKLOG("[bison]: include %s", (yyvsp[(2) - (2)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_include((yyvsp[(2) - (2)].str).c_str());
	;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 204 "bison.y"
    {
	;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 214 "bison.y"
    {
		FKLOG("[bison]: struct_define %s", (yyvsp[(2) - (4)].str).c_str());
		myflexer *l = (myflexer *)parm;
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[(3) - (4)].syntree));
		l->add_struct_desc((yyvsp[(2) - (4)].str).c_str(), p);
	;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 224 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 229 "bison.y"
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
#line 238 "bison.y"
    {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER");
		NEWTYPE(p, struct_desc_memlist_node);
		p->add_arg((yyvsp[(1) - (1)].str));
		(yyval.syntree) = p;
	;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 248 "bison.y"
    {
	;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 258 "bison.y"
    {
		FKLOG("[bison]: const_define %s", (yyvsp[(2) - (4)].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_const_desc((yyvsp[(2) - (4)].str).c_str(), (yyvsp[(4) - (4)].syntree));
	;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 267 "bison.y"
    {
	;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 279 "bison.y"
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
#line 290 "bison.y"
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
#line 303 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 308 "bison.y"
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
#line 317 "bison.y"
    {
		FKLOG("[bison]: function_declaration_arguments <- arg");
		NEWTYPE(p, func_desc_arglist_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 327 "bison.y"
    {
		FKLOG("[bison]: arg <- IDENTIFIER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, identifier_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 337 "bison.y"
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
#line 347 "bison.y"
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
#line 357 "bison.y"
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
#line 375 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 380 "bison.y"
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
#line 389 "bison.y"
    {
		FKLOG("[bison]: function_call_arguments <- arg_expr");
		NEWTYPE(p, function_call_arglist_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 399 "bison.y"
    {
		FKLOG("[bison]: arg_expr <- expr_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 409 "bison.y"
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
#line 418 "bison.y"
    {
		FKLOG("[bison]: block <- stmt");
		NEWTYPE(p, block_node);
		p->add_stmt((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 428 "bison.y"
    {
		FKLOG("[bison]: stmt <- while_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 434 "bison.y"
    {
		FKLOG("[bison]: stmt <- if_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 440 "bison.y"
    {
		FKLOG("[bison]: stmt <- return_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 446 "bison.y"
    {
		FKLOG("[bison]: stmt <- assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 452 "bison.y"
    {
		FKLOG("[bison]: stmt <- multi_assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 458 "bison.y"
    {
		FKLOG("[bison]: stmt <- break");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 464 "bison.y"
    {
		FKLOG("[bison]: stmt <- continue");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 470 "bison.y"
    {
		FKLOG("[bison]: stmt <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 476 "bison.y"
    {
		FKLOG("[bison]: stmt <- math_assign_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 482 "bison.y"
    {
		FKLOG("[bison]: stmt <- for_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 488 "bison.y"
    {
		FKLOG("[bison]: stmt <- fake_call_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 494 "bison.y"
    {
		FKLOG("[bison]: stmt <- sleep_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 500 "bison.y"
    {
		FKLOG("[bison]: stmt <- yield_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 506 "bison.y"
    {
		FKLOG("[bison]: stmt <- switch_stmt");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 514 "bison.y"
    {
		FKLOG("[bison]: fake_call_stmt <- fake function_call");
		function_call_node * p = dynamic_cast<function_call_node*>((yyvsp[(2) - (2)].syntree));
		p->fakecall = true;
		(yyval.syntree) = p;
	;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 524 "bison.y"
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

  case 55:

/* Line 1455 of yacc.c  */
#line 535 "bison.y"
    {
		FKLOG("[bison]: for_stmt <- block cmp block");
		NEWTYPE(p, for_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(4) - (8)].syntree));
		p->beginblock = dynamic_cast<block_node*>((yyvsp[(2) - (8)].syntree));
		p->endblock = dynamic_cast<block_node*>((yyvsp[(6) - (8)].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 548 "bison.y"
    {
		FKLOG("[bison]: while_stmt <- cmp block");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (5)].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[(4) - (5)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 557 "bison.y"
    {
		FKLOG("[bison]: while_stmt <- cmp");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (4)].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 568 "bison.y"
    {
		FKLOG("[bison]: if_stmt <- cmp block");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (6)].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[(4) - (6)].syntree));
		p->elses = dynamic_cast<else_stmt*>((yyvsp[(5) - (6)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 578 "bison.y"
    {
		FKLOG("[bison]: if_stmt <- cmp");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[(2) - (5)].syntree));
		p->block = 0;
		p->elses = dynamic_cast<else_stmt*>((yyvsp[(4) - (5)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 590 "bison.y"
    {
		(yyval.syntree) = 0;
	;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 595 "bison.y"
    {
		FKLOG("[bison]: else_stmt <- block");
		NEWTYPE(p, else_stmt);
		p->block = dynamic_cast<block_node*>((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 603 "bison.y"
    {
		FKLOG("[bison]: else_stmt <- empty");
		NEWTYPE(p, else_stmt);
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 613 "bison.y"
    {
		FKLOG("[bison]: cmp <- ( cmp )");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 619 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp AND cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "&&";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 629 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp OR cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "||";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 639 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value LESS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 649 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value MORE cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 659 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 669 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value MORE_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 679 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value LESS_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 689 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value NOT_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[(2) - (3)].str);
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 699 "bison.y"
    {
		FKLOG("[bison]: cmp <- true");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "true";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 709 "bison.y"
    {
		FKLOG("[bison]: cmp <- false");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "false";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 719 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value IS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "is";
		p->left = (yyvsp[(2) - (2)].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 729 "bison.y"
    {
		FKLOG("[bison]: cmp <- cmp_value NOT cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "not";
		p->left = (yyvsp[(2) - (2)].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 741 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 747 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 753 "bison.y"
    {
		FKLOG("[bison]: cmp_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 761 "bison.y"
    {
		FKLOG("[bison]: return_stmt <- RETURN return_value_list");
		NEWTYPE(p, return_stmt);
		p->returnlist = dynamic_cast<return_value_list_node*>((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 769 "bison.y"
    {
		FKLOG("[bison]: return_stmt <- RETURN");
		NEWTYPE(p, return_stmt);
		p->returnlist = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 779 "bison.y"
    {
		FKLOG("[bison]: return_value_list <- return_value_list return_value");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_return_value_list);
		return_value_list_node * p = dynamic_cast<return_value_list_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 788 "bison.y"
    {
		NEWTYPE(p, return_value_list_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 797 "bison.y"
    {
		FKLOG("[bison]: return_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 803 "bison.y"
    {
		FKLOG("[bison]: return_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 809 "bison.y"
    {
		FKLOG("[bison]: return_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 817 "bison.y"
    {
		FKLOG("[bison]: assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 828 "bison.y"
    {
		FKLOG("[bison]: multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>((yyvsp[(1) - (3)].syntree));
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 839 "bison.y"
    {
		FKLOG("[bison]: var_list <- var_list var");
		assert((yyvsp[(1) - (3)].syntree)->gettype() == est_var_list);
		var_list_node * p = dynamic_cast<var_list_node*>((yyvsp[(1) - (3)].syntree));
		p->add_arg((yyvsp[(3) - (3)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 848 "bison.y"
    {
		NEWTYPE(p, var_list_node);
		p->add_arg((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 857 "bison.y"
    {
		FKLOG("[bison]: assign_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 863 "bison.y"
    {
		FKLOG("[bison]: assign_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 869 "bison.y"
    {
		FKLOG("[bison]: assign_value <- expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 877 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "+=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 887 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "-=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 897 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "/=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 907 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "*=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 917 "bison.y"
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[(1) - (3)].syntree);
		p->oper = "%=";
		p->value = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 927 "bison.y"
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

  case 99:

/* Line 1455 of yacc.c  */
#line 943 "bison.y"
    {
		FKLOG("[bison]: var <- VAR_BEGIN IDENTIFIER %s", (yyvsp[(2) - (2)].str).c_str());
		NEWTYPE(p, var_node);
		p->str = (yyvsp[(2) - (2)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 951 "bison.y"
    {
		FKLOG("[bison]: var <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 959 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 967 "bison.y"
    {
		FKLOG("[bison]: container_get_node <- IDENTIFIER[expr_value] %s", (yyvsp[(1) - (4)].str).c_str());
		NEWTYPE(p, container_get_node);
		p->container = (yyvsp[(1) - (4)].str);
		p->key = (yyvsp[(3) - (4)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 976 "bison.y"
    {
		FKLOG("[bison]: variable <- IDENTIFIER_POINTER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, struct_pointer_node);
		p->str = (yyvsp[(1) - (1)].str);
		(yyval.syntree) = p;
	;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 986 "bison.y"
    {
		FKLOG("[bison]: expr <- (expr)");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 992 "bison.y"
    {
		FKLOG("[bison]: expr <- function_call");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 998 "bison.y"
    {
		FKLOG("[bison]: expr <- math_expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1006 "bison.y"
    {
		FKLOG("[bison]: math_expr <- (math_expr)");
		(yyval.syntree) = (yyvsp[(2) - (3)].syntree);
	;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1012 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "+";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1022 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "-";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1032 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "*";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1042 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "/";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1052 "bison.y"
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[(2) - (3)].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "%";
		p->left = (yyvsp[(1) - (3)].syntree);
		p->right = (yyvsp[(3) - (3)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1064 "bison.y"
    {
		FKLOG("[bison]: expr_value <- math_expr");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1070 "bison.y"
    {
		FKLOG("[bison]: expr_value <- explicit_value");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1076 "bison.y"
    {
		FKLOG("[bison]: expr_value <- function_call");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1082 "bison.y"
    {
		FKLOG("[bison]: expr_value <- variable");
		(yyval.syntree) = (yyvsp[(1) - (1)].syntree);
	;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1090 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- TRUE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_TRUE;
		(yyval.syntree) = p;
	;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1099 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FALSE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_FALSE;
		(yyval.syntree) = p;
	;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1108 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- NUMBER %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_NUM;
		(yyval.syntree) = p;
	;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1117 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FKUUID %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_UUID;
		(yyval.syntree) = p;
	;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1126 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- STRING_DEFINITION %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_STR;
		(yyval.syntree) = p;
	;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1135 "bison.y"
    {
		FKLOG("[bison]: explicit_value <- FKFLOAT %s", (yyvsp[(1) - (1)].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[(1) - (1)].str);
		p->type = explicit_value_node::EVT_FLOAT;
		(yyval.syntree) = p;
	;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1146 "bison.y"
    {
		FKLOG("[bison]: break <- BREAK");
		NEWTYPE(p, break_stmt);
		(yyval.syntree) = p;
	;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1155 "bison.y"
    {
		FKLOG("[bison]: CONTINUE");
		NEWTYPE(p, continue_stmt);
		(yyval.syntree) = p;
	;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1164 "bison.y"
    {
		FKLOG("[bison]: SLEEP");
		NEWTYPE(p, sleep_stmt);
		p->time = (yyvsp[(2) - (2)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1173 "bison.y"
    {
		FKLOG("[bison]: YIELD");
		NEWTYPE(p, yield_stmt);
		p->time = (yyvsp[(2) - (2)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1183 "bison.y"
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[(2) - (6)].syntree);
		p->caselist = (yyvsp[(3) - (6)].syntree);
		p->def = (yyvsp[(5) - (6)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1193 "bison.y"
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[(2) - (5)].syntree);
		p->caselist = (yyvsp[(3) - (5)].syntree);
		p->def = 0;
		(yyval.syntree) = p;
	;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1205 "bison.y"
    {
		FKLOG("[bison]: switch_case_list <- switch_case_define");
		NEWTYPE(p, switch_caselist_node);
		p->add_case((yyvsp[(1) - (1)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1213 "bison.y"
    {
		FKLOG("[bison]: switch_case_list <- switch_case_list switch_case_define");
		assert((yyvsp[(2) - (2)].syntree)->gettype() == est_switch_case_node);
		switch_caselist_node * p = dynamic_cast<switch_caselist_node*>((yyvsp[(1) - (2)].syntree));
		p->add_case((yyvsp[(2) - (2)].syntree));
		(yyval.syntree) = p;
	;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1224 "bison.y"
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[(2) - (4)].syntree);
		p->block = (yyvsp[(4) - (4)].syntree);
		(yyval.syntree) = p;
	;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1233 "bison.y"
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[(2) - (3)].syntree);
		p->block = 0;
		(yyval.syntree) = p;
	;}
    break;



/* Line 1455 of yacc.c  */
#line 3214 "bison.tab.c"
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



