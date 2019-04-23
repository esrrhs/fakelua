/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "bison.y" /* yacc.c:339  */

#include "semantic.h"
#include "types.h"
#include "fake.h"

#define YYPARSE_PARAM   parm
#define YYLEX_PARAM     parm

#define yyerror(loc, param, msg) my_yyerror(msg, param)

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
	

#line 104 "bison.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "bison.tab.h".  */
#ifndef YY_YY_BISON_TAB_H_INCLUDED
# define YY_YY_BISON_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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

/* Copy the second part of user declarations.  */

#line 232 "bison.tab.c" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

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
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  272

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   177,   177,   186,   189,   196,   205,   208,   210,   214,
     224,   227,   229,   233,   244,   248,   257,   268,   271,   273,
     277,   287,   290,   292,   298,   310,   325,   329,   338,   348,
     358,   370,   382,   394,   412,   433,   437,   446,   456,   466,
     475,   485,   491,   497,   503,   509,   515,   521,   527,   533,
     539,   545,   551,   557,   563,   569,   577,   587,   598,   611,
     650,   683,   692,   703,   714,   728,   732,   741,   751,   760,
     772,   776,   784,   794,   800,   810,   820,   830,   840,   850,
     860,   870,   880,   890,   900,   910,   922,   928,   934,   942,
     950,   960,   969,   978,   984,   990,   998,  1008,  1020,  1030,
    1042,  1051,  1060,  1066,  1072,  1080,  1090,  1100,  1110,  1120,
    1130,  1146,  1154,  1162,  1170,  1179,  1187,  1197,  1203,  1209,
    1217,  1223,  1233,  1243,  1253,  1263,  1273,  1285,  1291,  1297,
    1303,  1311,  1320,  1329,  1338,  1347,  1356,  1365,  1374,  1384,
    1397,  1403,  1411,  1422,  1434,  1440,  1448,  1459,  1468,  1477,
    1486,  1496,  1506,  1518,  1526,  1537,  1546
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
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
  "switch_case_define", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
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

#define YYPACT_NINF -211

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-211)))

#define YYTABLE_NINF -131

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
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

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
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

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
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

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
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


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, parm, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, parm); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void * parm)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (parm);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void * parm)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, parm);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, void * parm)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , parm);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, parm); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void * parm)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (parm);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void * parm)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
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
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
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

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc, parm);
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
      if (yytable_value_is_error (yyn))
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
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
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
     '$$ = $1'.

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
#line 186 "bison.y" /* yacc.c:1646  */
    {
	}
#line 1833 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 190 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: package %s", (yyvsp[0].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[0].str).c_str());
	}
#line 1843 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 197 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: package %s", (yyvsp[0].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[0].str).c_str());
	}
#line 1853 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 205 "bison.y" /* yacc.c:1646  */
    {
	}
#line 1860 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 215 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: include %s", (yyvsp[0].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_include((yyvsp[0].str).c_str());
	}
#line 1870 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 224 "bison.y" /* yacc.c:1646  */
    {
	}
#line 1877 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 234 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: struct_define %s", (yyvsp[-2].str).c_str());
		myflexer *l = (myflexer *)parm;
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[-1].syntree));
		l->add_struct_desc((yyvsp[-2].str).c_str(), p);
	}
#line 1888 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 244 "bison.y" /* yacc.c:1646  */
    {
		(yyval.syntree) = 0;
	}
#line 1896 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 249 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER struct_mem_declaration");
		assert((yyvsp[-1].syntree)->gettype() == est_struct_memlist);
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[-1].syntree));
		p->add_arg((yyvsp[0].str));
		(yyval.syntree) = p;
	}
#line 1908 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 258 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER");
		NEWTYPE(p, struct_desc_memlist_node);
		p->add_arg((yyvsp[0].str));
		(yyval.syntree) = p;
	}
#line 1919 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 268 "bison.y" /* yacc.c:1646  */
    {
	}
#line 1926 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 278 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: const_define %s", (yyvsp[-2].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_const_desc((yyvsp[-2].str).c_str(), (yyvsp[0].syntree));
	}
#line 1936 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 287 "bison.y" /* yacc.c:1646  */
    {
	}
#line 1943 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 299 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_declaration <- block %s %d", (yyvsp[-5].str).c_str(), yylloc.first_line);
		NEWTYPE(p, func_desc_node);
		p->funcname = (yyvsp[-5].str);
		p->arglist = dynamic_cast<func_desc_arglist_node*>((yyvsp[-3].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[-1].syntree));
		p->endline = yylloc.first_line;
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	}
#line 1958 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 311 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_declaration <- empty %s %d", (yyvsp[-4].str).c_str(), yylloc.first_line);
		NEWTYPE(p, func_desc_node);
		p->funcname = (yyvsp[-4].str);
		p->arglist = 0;
		p->block = 0;
		p->endline = yylloc.first_line;
		myflexer *l = (myflexer *)parm;
		l->add_func_desc(p);
	}
#line 1973 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 325 "bison.y" /* yacc.c:1646  */
    {
		(yyval.syntree) = 0;
	}
#line 1981 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 330 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_declaration_arguments <- arg function_declaration_arguments");
		assert((yyvsp[-2].syntree)->gettype() == est_arglist);
		func_desc_arglist_node * p = dynamic_cast<func_desc_arglist_node*>((yyvsp[-2].syntree));
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 1993 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 339 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_declaration_arguments <- arg");
		NEWTYPE(p, func_desc_arglist_node);
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2004 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 349 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: arg <- IDENTIFIER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, identifier_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 2015 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 359 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[-3].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[-3].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[-1].syntree));
		p->fakecall = false;
		p->classmem_call = false;
		(yyval.syntree) = p;
	}
#line 2030 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 371 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[-3].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[-3].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[-1].syntree));
		p->fakecall = false;
		p->classmem_call = false;
		(yyval.syntree) = p;
	}
#line 2045 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 383 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_call <- function_call_arguments");
		NEWTYPE(p, function_call_node);
		p->fuc = "";
		p->prefunc = (yyvsp[-3].syntree);
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[-1].syntree));
		p->fakecall = false;
		p->classmem_call = false;
		(yyval.syntree) = p;
	}
#line 2060 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 395 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_call <- mem function_call_arguments %s", (yyvsp[-3].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[-3].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[-1].syntree));
		if (p->arglist == 0)
		{
			NEWTYPE(pa, function_call_arglist_node);
			p->arglist = pa;
		}
		p->arglist->add_arg((yyvsp[-5].syntree));
		p->fakecall = false;
		p->classmem_call = true;
		(yyval.syntree) = p;
	}
#line 2081 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 413 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_call <- mem function_call_arguments %s", (yyvsp[-3].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[-3].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[-1].syntree));
		if (p->arglist == 0)
		{
			NEWTYPE(pa, function_call_arglist_node);
			p->arglist = pa;
		}
		p->arglist->add_arg((yyvsp[-5].syntree));
		p->fakecall = false;
		p->classmem_call = true;
		(yyval.syntree) = p;
	}
#line 2102 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 433 "bison.y" /* yacc.c:1646  */
    {
		(yyval.syntree) = 0;
	}
#line 2110 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 438 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_call_arguments <- arg_expr function_call_arguments");
		assert((yyvsp[-2].syntree)->gettype() == est_call_arglist);
		function_call_arglist_node * p = dynamic_cast<function_call_arglist_node*>((yyvsp[-2].syntree));
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2122 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 447 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: function_call_arguments <- arg_expr");
		NEWTYPE(p, function_call_arglist_node);
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2133 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 457 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: arg_expr <- expr_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2142 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 467 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: block <- block stmt");
		assert((yyvsp[-1].syntree)->gettype() == est_block);
		block_node * p = dynamic_cast<block_node*>((yyvsp[-1].syntree));
		p->add_stmt((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2154 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 476 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: block <- stmt");
		NEWTYPE(p, block_node);
		p->add_stmt((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2165 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 486 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- while_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2174 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 492 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- if_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2183 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 498 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- return_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2192 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 504 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- assign_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2201 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 510 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- multi_assign_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2210 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 516 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- break");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2219 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 522 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- continue");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2228 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 528 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2237 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 534 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- math_assign_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2246 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 540 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- for_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2255 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 546 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- for_loop_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2264 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 552 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- fake_call_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2273 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 558 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- sleep_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2282 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 564 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- yield_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2291 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 570 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: stmt <- switch_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2300 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 578 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: fake_call_stmt <- fake function_call");
		function_call_node * p = dynamic_cast<function_call_node*>((yyvsp[0].syntree));
		p->fakecall = true;
		(yyval.syntree) = p;
	}
#line 2311 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 588 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: for_stmt <- block cmp block");
		NEWTYPE(p, for_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-5].syntree));
		p->beginblock = dynamic_cast<block_node*>((yyvsp[-7].syntree));
		p->endblock = dynamic_cast<block_node*>((yyvsp[-3].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[-1].syntree));
		(yyval.syntree) = p;
	}
#line 2325 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 599 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: for_stmt <- block cmp");
		NEWTYPE(p, for_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-4].syntree));
		p->beginblock = dynamic_cast<block_node*>((yyvsp[-6].syntree));
		p->endblock = dynamic_cast<block_node*>((yyvsp[-2].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 2339 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 612 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: for_loop_stmt <- block");
		NEWTYPE(p, for_stmt);
		
		syntree_node * pi = (yyvsp[-9].syntree);
		if (pi->gettype() == est_var)
		{
			NEWTYPE(pvar, variable_node);
			pvar->str = (dynamic_cast<var_node*>(pi))->str;
			pi = pvar;
		}
		
		NEWTYPE(pcmp, cmp_stmt);
		pcmp->cmp = "<";
		pcmp->left = pi;
		pcmp->right = (yyvsp[-5].syntree);
		p->cmp = pcmp;
		
		NEWTYPE(pbeginblockassign, assign_stmt);
		pbeginblockassign->var = (yyvsp[-9].syntree);
		pbeginblockassign->value = (yyvsp[-7].syntree);
		pbeginblockassign->isnew = false;
		NEWTYPE(pbeginblock, block_node);
		pbeginblock->add_stmt(pbeginblockassign);
		p->beginblock = pbeginblock;
		
		NEWTYPE(pendblockassign, math_assign_stmt);
		pendblockassign->var = pi;
		pendblockassign->oper = "+=";
		pendblockassign->value = (yyvsp[-3].syntree);
		NEWTYPE(pendblock, block_node);
		pendblock->add_stmt(pendblockassign);
		p->endblock = pendblock;
		
		p->block = dynamic_cast<block_node*>((yyvsp[-1].syntree));
		(yyval.syntree) = p;
	}
#line 2381 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 651 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: for_loop_stmt <- empty");
		NEWTYPE(p, for_stmt);
		
		NEWTYPE(pcmp, cmp_stmt);
		pcmp->cmp = "<";
		pcmp->left = (yyvsp[-8].syntree);
		pcmp->right = (yyvsp[-4].syntree);
		p->cmp = pcmp;
		
		NEWTYPE(pbeginblockassign, assign_stmt);
		pbeginblockassign->var = (yyvsp[-8].syntree);
		pbeginblockassign->value = (yyvsp[-6].syntree);
		pbeginblockassign->isnew = false;
		NEWTYPE(pbeginblock, block_node);
		pbeginblock->add_stmt(pbeginblockassign);
		p->beginblock = pbeginblock;
		
		NEWTYPE(pendblockassign, math_assign_stmt);
		pendblockassign->var = (yyvsp[-8].syntree);
		pendblockassign->oper = "+=";
		pendblockassign->value = (yyvsp[-2].syntree);
		NEWTYPE(pendblock, block_node);
		pendblock->add_stmt(pendblockassign);
		p->endblock = pendblock;
		
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 2415 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 684 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: while_stmt <- cmp block");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-3].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[-1].syntree));
		(yyval.syntree) = p;
	}
#line 2427 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 693 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: while_stmt <- cmp");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-2].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 2439 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 704 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: if_stmt <- cmp block");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-5].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[-3].syntree));
		p->elseifs = dynamic_cast<elseif_stmt_list*>((yyvsp[-2].syntree));
		p->elses = dynamic_cast<else_stmt*>((yyvsp[-1].syntree));
		(yyval.syntree) = p;
	}
#line 2453 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 715 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: if_stmt <- cmp");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-4].syntree));
		p->block = 0;
		p->elseifs = dynamic_cast<elseif_stmt_list*>((yyvsp[-2].syntree));
		p->elses = dynamic_cast<else_stmt*>((yyvsp[-1].syntree));
		(yyval.syntree) = p;
	}
#line 2467 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 728 "bison.y" /* yacc.c:1646  */
    {
		(yyval.syntree) = 0;
	}
#line 2475 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 733 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt_list elseif_stmt");
		assert((yyvsp[-1].syntree)->gettype() == est_elseif_stmt_list);
		elseif_stmt_list * p = dynamic_cast<elseif_stmt_list*>((yyvsp[-1].syntree));
		p->add_stmt((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2487 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 742 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt");
		NEWTYPE(p, elseif_stmt_list);
		p->add_stmt((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2498 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 752 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-2].syntree));
		p->block = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2510 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 761 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-1].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 2522 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 772 "bison.y" /* yacc.c:1646  */
    {
		(yyval.syntree) = 0;
	}
#line 2530 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 777 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: else_stmt <- block");
		NEWTYPE(p, else_stmt);
		p->block = dynamic_cast<block_node*>((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2541 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 785 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: else_stmt <- empty");
		NEWTYPE(p, else_stmt);
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 2552 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 795 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- ( cmp )");
		(yyval.syntree) = (yyvsp[-1].syntree);
	}
#line 2561 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 801 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp AND cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "&&";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2574 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 811 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp OR cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "||";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2587 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 821 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp_value LESS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2600 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 831 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp_value MORE cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2613 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 841 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp_value EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2626 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 851 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp_value MORE_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2639 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 861 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp_value LESS_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2652 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 871 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp_value NOT_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2665 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 881 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- true");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "true";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	}
#line 2678 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 891 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- false");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "false";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	}
#line 2691 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 901 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp_value IS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "is";
		p->left = (yyvsp[0].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	}
#line 2704 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 911 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp <- cmp_value NOT cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "not";
		p->left = (yyvsp[0].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	}
#line 2717 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 923 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp_value <- explicit_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2726 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 929 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp_value <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2735 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 935 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: cmp_value <- expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2744 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 943 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: return_stmt <- RETURN return_value_list");
		NEWTYPE(p, return_stmt);
		p->returnlist = dynamic_cast<return_value_list_node*>((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2755 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 951 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: return_stmt <- RETURN");
		NEWTYPE(p, return_stmt);
		p->returnlist = 0;
		(yyval.syntree) = p;
	}
#line 2766 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 961 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: return_value_list <- return_value_list return_value");
		assert((yyvsp[-2].syntree)->gettype() == est_return_value_list);
		return_value_list_node * p = dynamic_cast<return_value_list_node*>((yyvsp[-2].syntree));
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2778 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 970 "bison.y" /* yacc.c:1646  */
    {
		NEWTYPE(p, return_value_list_node);
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2788 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 979 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: return_value <- explicit_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2797 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 985 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: return_value <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2806 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 991 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: return_value <- expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2815 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 999 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->value = (yyvsp[0].syntree);
		p->isnew = false;
		(yyval.syntree) = p;
	}
#line 2828 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1009 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: new assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->value = (yyvsp[0].syntree);
		p->isnew = true;
		(yyval.syntree) = p;
	}
#line 2841 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1021 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>((yyvsp[-2].syntree));
		p->value = (yyvsp[0].syntree);
		p->isnew = false;
		(yyval.syntree) = p;
	}
#line 2854 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 1031 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: new multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>((yyvsp[-2].syntree));
		p->value = (yyvsp[0].syntree);
		p->isnew = true;
		(yyval.syntree) = p;
	}
#line 2867 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 1043 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: var_list <- var_list var");
		assert((yyvsp[-2].syntree)->gettype() == est_var_list);
		var_list_node * p = dynamic_cast<var_list_node*>((yyvsp[-2].syntree));
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2879 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1052 "bison.y" /* yacc.c:1646  */
    {
		NEWTYPE(p, var_list_node);
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2889 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1061 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: assign_value <- explicit_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2898 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1067 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: assign_value <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2907 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1073 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: assign_value <- expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2916 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1081 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "+=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2929 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1091 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "-=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2942 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1101 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "/=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2955 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1111 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "*=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2968 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1121 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "%=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2981 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1131 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_assign_stmt <- variable INC");
		NEWTYPE(pp, explicit_value_node);
		pp->str = "1";
		pp->type = explicit_value_node::EVT_NUM;
		
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-1].syntree);
		p->oper = "+=";
		p->value = pp;
		(yyval.syntree) = p;
	}
#line 2998 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1147 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: var <- VAR_BEGIN IDENTIFIER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, var_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 3009 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1155 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: var <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3018 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1163 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: variable <- IDENTIFIER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 3029 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1171 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: container_get_node <- IDENTIFIER[expr_value] %s", (yyvsp[-3].str).c_str());
		NEWTYPE(p, container_get_node);
		p->container = (yyvsp[-3].str);
		p->key = (yyvsp[-1].syntree);
		(yyval.syntree) = p;
	}
#line 3041 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1180 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: variable <- IDENTIFIER_POINTER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, struct_pointer_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 3052 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1188 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: variable <- IDENTIFIER_DOT %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 3063 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1198 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: expr <- (expr)");
		(yyval.syntree) = (yyvsp[-1].syntree);
	}
#line 3072 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1204 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: expr <- function_call");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3081 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1210 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: expr <- math_expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3090 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1218 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_expr <- (math_expr)");
		(yyval.syntree) = (yyvsp[-1].syntree);
	}
#line 3099 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1224 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "+";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3112 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1234 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "-";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3125 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1244 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "*";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3138 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1254 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "/";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3151 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1264 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "%";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3164 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1274 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "..";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3177 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1286 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: expr_value <- math_expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3186 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1292 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: expr_value <- explicit_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3195 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1298 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: expr_value <- function_call");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3204 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1304 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: expr_value <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3213 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1312 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: explicit_value <- FTRUE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_TRUE;
		(yyval.syntree) = p;
	}
#line 3225 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1321 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: explicit_value <- FFALSE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_FALSE;
		(yyval.syntree) = p;
	}
#line 3237 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1330 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: explicit_value <- NUMBER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_NUM;
		(yyval.syntree) = p;
	}
#line 3249 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1339 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: explicit_value <- FKUUID %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_UUID;
		(yyval.syntree) = p;
	}
#line 3261 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1348 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: explicit_value <- STRING_DEFINITION %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_STR;
		(yyval.syntree) = p;
	}
#line 3273 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1357 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: explicit_value <- FKFLOAT %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_FLOAT;
		(yyval.syntree) = p;
	}
#line 3285 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1366 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: explicit_value <- FNULL %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_NULL;
		(yyval.syntree) = p;
	}
#line 3297 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1375 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: explicit_value <- const_map_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_MAP;
		p->v = (yyvsp[-1].syntree);
		(yyval.syntree) = p;
	}
#line 3310 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1385 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: explicit_value <- const_array_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_ARRAY;
		p->v = (yyvsp[-1].syntree);
		(yyval.syntree) = p;
	}
#line 3323 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1397 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: const_map_list_value <- null");
		NEWTYPE(p, const_map_list_value_node);
		(yyval.syntree) = p;
	}
#line 3333 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1404 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: const_map_list_value <- const_map_value");
		NEWTYPE(p, const_map_list_value_node);
		p->add_ele((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3344 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1412 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: const_map_list_value <- const_map_list_value const_map_value");
		assert((yyvsp[-1].syntree)->gettype() == est_constmaplist);
		const_map_list_value_node * p = dynamic_cast<const_map_list_value_node*>((yyvsp[-1].syntree));
		p->add_ele((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3356 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1423 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: const_map_value <- explicit_value");
		NEWTYPE(p, const_map_value_node);
		p->k = (yyvsp[-2].syntree);
		p->v = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3368 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1434 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: const_array_list_value <- null");
		NEWTYPE(p, const_array_list_value_node);
		(yyval.syntree) = p;
	}
#line 3378 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1441 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: const_array_list_value <- explicit_value");
		NEWTYPE(p, const_array_list_value_node);
		p->add_ele((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3389 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1449 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: const_array_list_value <- const_array_list_value explicit_value");
		assert((yyvsp[-1].syntree)->gettype() == est_constarraylist);
		const_array_list_value_node * p = dynamic_cast<const_array_list_value_node*>((yyvsp[-1].syntree));
		p->add_ele((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3401 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1460 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: break <- BREAK");
		NEWTYPE(p, break_stmt);
		(yyval.syntree) = p;
	}
#line 3411 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1469 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: CONTINUE");
		NEWTYPE(p, continue_stmt);
		(yyval.syntree) = p;
	}
#line 3421 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1478 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: SLEEP");
		NEWTYPE(p, sleep_stmt);
		p->time = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3432 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1487 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: YIELD");
		NEWTYPE(p, yield_stmt);
		p->time = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3443 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1497 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[-4].syntree);
		p->caselist = (yyvsp[-3].syntree);
		p->def = (yyvsp[-1].syntree);
		(yyval.syntree) = p;
	}
#line 3456 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1507 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[-3].syntree);
		p->caselist = (yyvsp[-2].syntree);
		p->def = 0;
		(yyval.syntree) = p;
	}
#line 3469 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1519 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: switch_case_list <- switch_case_define");
		NEWTYPE(p, switch_caselist_node);
		p->add_case((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3480 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1527 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: switch_case_list <- switch_case_list switch_case_define");
		assert((yyvsp[0].syntree)->gettype() == est_switch_case_node);
		switch_caselist_node * p = dynamic_cast<switch_caselist_node*>((yyvsp[-1].syntree));
		p->add_case((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3492 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1538 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[-2].syntree);
		p->block = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3504 "bison.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1547 "bison.y" /* yacc.c:1646  */
    {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[-1].syntree);
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 3516 "bison.tab.c" /* yacc.c:1646  */
    break;


#line 3520 "bison.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, parm, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, parm, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc, parm);
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

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, parm);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, parm, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, parm);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, parm);
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
  return yyresult;
}
