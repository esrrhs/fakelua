/* A Bison parser, made by GNU Bison 3.8.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30800

/* Bison version string.  */
#define YYBISON_VERSION "3.8"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "bison.y"

#include "semantic.h"
#include "types.h"
#include "fake.h"

#define YYPARSE_PARAM   parm
#define YYLEX_PARAM     parm

#define yyerror(loc, param, msg) my_yyerror(msg, param)

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
	

#line 105 "bison.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "bison.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_VAR_BEGIN = 3,                  /* VAR_BEGIN  */
  YYSYMBOL_RETURN = 4,                     /* RETURN  */
  YYSYMBOL_BREAK = 5,                      /* BREAK  */
  YYSYMBOL_FUNC = 6,                       /* FUNC  */
  YYSYMBOL_WHILE = 7,                      /* WHILE  */
  YYSYMBOL_FTRUE = 8,                      /* FTRUE  */
  YYSYMBOL_FFALSE = 9,                     /* FFALSE  */
  YYSYMBOL_IF = 10,                        /* IF  */
  YYSYMBOL_THEN = 11,                      /* THEN  */
  YYSYMBOL_ELSE = 12,                      /* ELSE  */
  YYSYMBOL_END = 13,                       /* END  */
  YYSYMBOL_STRING_DEFINITION = 14,         /* STRING_DEFINITION  */
  YYSYMBOL_IDENTIFIER = 15,                /* IDENTIFIER  */
  YYSYMBOL_NUMBER = 16,                    /* NUMBER  */
  YYSYMBOL_SINGLE_LINE_COMMENT = 17,       /* SINGLE_LINE_COMMENT  */
  YYSYMBOL_DIVIDE_MOD = 18,                /* DIVIDE_MOD  */
  YYSYMBOL_ARG_SPLITTER = 19,              /* ARG_SPLITTER  */
  YYSYMBOL_PLUS = 20,                      /* PLUS  */
  YYSYMBOL_MINUS = 21,                     /* MINUS  */
  YYSYMBOL_DIVIDE = 22,                    /* DIVIDE  */
  YYSYMBOL_MULTIPLY = 23,                  /* MULTIPLY  */
  YYSYMBOL_ASSIGN = 24,                    /* ASSIGN  */
  YYSYMBOL_MORE = 25,                      /* MORE  */
  YYSYMBOL_LESS = 26,                      /* LESS  */
  YYSYMBOL_MORE_OR_EQUAL = 27,             /* MORE_OR_EQUAL  */
  YYSYMBOL_LESS_OR_EQUAL = 28,             /* LESS_OR_EQUAL  */
  YYSYMBOL_EQUAL = 29,                     /* EQUAL  */
  YYSYMBOL_NOT_EQUAL = 30,                 /* NOT_EQUAL  */
  YYSYMBOL_OPEN_BRACKET = 31,              /* OPEN_BRACKET  */
  YYSYMBOL_CLOSE_BRACKET = 32,             /* CLOSE_BRACKET  */
  YYSYMBOL_AND = 33,                       /* AND  */
  YYSYMBOL_OR = 34,                        /* OR  */
  YYSYMBOL_FKFLOAT = 35,                   /* FKFLOAT  */
  YYSYMBOL_PLUS_ASSIGN = 36,               /* PLUS_ASSIGN  */
  YYSYMBOL_MINUS_ASSIGN = 37,              /* MINUS_ASSIGN  */
  YYSYMBOL_DIVIDE_ASSIGN = 38,             /* DIVIDE_ASSIGN  */
  YYSYMBOL_MULTIPLY_ASSIGN = 39,           /* MULTIPLY_ASSIGN  */
  YYSYMBOL_DIVIDE_MOD_ASSIGN = 40,         /* DIVIDE_MOD_ASSIGN  */
  YYSYMBOL_COLON = 41,                     /* COLON  */
  YYSYMBOL_FOR = 42,                       /* FOR  */
  YYSYMBOL_INC = 43,                       /* INC  */
  YYSYMBOL_FKUUID = 44,                    /* FKUUID  */
  YYSYMBOL_OPEN_SQUARE_BRACKET = 45,       /* OPEN_SQUARE_BRACKET  */
  YYSYMBOL_CLOSE_SQUARE_BRACKET = 46,      /* CLOSE_SQUARE_BRACKET  */
  YYSYMBOL_FCONST = 47,                    /* FCONST  */
  YYSYMBOL_PACKAGE = 48,                   /* PACKAGE  */
  YYSYMBOL_INCLUDE = 49,                   /* INCLUDE  */
  YYSYMBOL_IDENTIFIER_DOT = 50,            /* IDENTIFIER_DOT  */
  YYSYMBOL_IDENTIFIER_POINTER = 51,        /* IDENTIFIER_POINTER  */
  YYSYMBOL_STRUCT = 52,                    /* STRUCT  */
  YYSYMBOL_IS = 53,                        /* IS  */
  YYSYMBOL_NOT = 54,                       /* NOT  */
  YYSYMBOL_CONTINUE = 55,                  /* CONTINUE  */
  YYSYMBOL_SWITCH = 56,                    /* SWITCH  */
  YYSYMBOL_CASE = 57,                      /* CASE  */
  YYSYMBOL_DEFAULT = 58,                   /* DEFAULT  */
  YYSYMBOL_NEW_ASSIGN = 59,                /* NEW_ASSIGN  */
  YYSYMBOL_ELSEIF = 60,                    /* ELSEIF  */
  YYSYMBOL_RIGHT_POINTER = 61,             /* RIGHT_POINTER  */
  YYSYMBOL_STRING_CAT = 62,                /* STRING_CAT  */
  YYSYMBOL_OPEN_BIG_BRACKET = 63,          /* OPEN_BIG_BRACKET  */
  YYSYMBOL_CLOSE_BIG_BRACKET = 64,         /* CLOSE_BIG_BRACKET  */
  YYSYMBOL_FNULL = 65,                     /* FNULL  */
  YYSYMBOL_YYACCEPT = 66,                  /* $accept  */
  YYSYMBOL_program = 67,                   /* program  */
  YYSYMBOL_package_head = 68,              /* package_head  */
  YYSYMBOL_include_head = 69,              /* include_head  */
  YYSYMBOL_include_define = 70,            /* include_define  */
  YYSYMBOL_struct_head = 71,               /* struct_head  */
  YYSYMBOL_struct_define = 72,             /* struct_define  */
  YYSYMBOL_struct_mem_declaration = 73,    /* struct_mem_declaration  */
  YYSYMBOL_const_head = 74,                /* const_head  */
  YYSYMBOL_const_define = 75,              /* const_define  */
  YYSYMBOL_body = 76,                      /* body  */
  YYSYMBOL_function_declaration = 77,      /* function_declaration  */
  YYSYMBOL_function_declaration_arguments = 78, /* function_declaration_arguments  */
  YYSYMBOL_arg = 79,                       /* arg  */
  YYSYMBOL_function_call = 80,             /* function_call  */
  YYSYMBOL_function_call_arguments = 81,   /* function_call_arguments  */
  YYSYMBOL_arg_expr = 82,                  /* arg_expr  */
  YYSYMBOL_block = 83,                     /* block  */
  YYSYMBOL_stmt = 84,                      /* stmt  */
  YYSYMBOL_for_stmt = 85,                  /* for_stmt  */
  YYSYMBOL_for_loop_value = 86,            /* for_loop_value  */
  YYSYMBOL_for_loop_stmt = 87,             /* for_loop_stmt  */
  YYSYMBOL_while_stmt = 88,                /* while_stmt  */
  YYSYMBOL_if_stmt = 89,                   /* if_stmt  */
  YYSYMBOL_elseif_stmt_list = 90,          /* elseif_stmt_list  */
  YYSYMBOL_elseif_stmt = 91,               /* elseif_stmt  */
  YYSYMBOL_else_stmt = 92,                 /* else_stmt  */
  YYSYMBOL_cmp = 93,                       /* cmp  */
  YYSYMBOL_cmp_value = 94,                 /* cmp_value  */
  YYSYMBOL_return_stmt = 95,               /* return_stmt  */
  YYSYMBOL_return_value_list = 96,         /* return_value_list  */
  YYSYMBOL_return_value = 97,              /* return_value  */
  YYSYMBOL_assign_stmt = 98,               /* assign_stmt  */
  YYSYMBOL_multi_assign_stmt = 99,         /* multi_assign_stmt  */
  YYSYMBOL_var_list = 100,                 /* var_list  */
  YYSYMBOL_assign_value = 101,             /* assign_value  */
  YYSYMBOL_math_assign_stmt = 102,         /* math_assign_stmt  */
  YYSYMBOL_var = 103,                      /* var  */
  YYSYMBOL_variable = 104,                 /* variable  */
  YYSYMBOL_expr = 105,                     /* expr  */
  YYSYMBOL_math_expr = 106,                /* math_expr  */
  YYSYMBOL_expr_value = 107,               /* expr_value  */
  YYSYMBOL_explicit_value = 108,           /* explicit_value  */
  YYSYMBOL_const_map_list_value = 109,     /* const_map_list_value  */
  YYSYMBOL_const_map_value = 110,          /* const_map_value  */
  YYSYMBOL_const_array_list_value = 111,   /* const_array_list_value  */
  YYSYMBOL_break = 112,                    /* break  */
  YYSYMBOL_continue = 113,                 /* continue  */
  YYSYMBOL_switch_stmt = 114,              /* switch_stmt  */
  YYSYMBOL_switch_case_list = 115,         /* switch_case_list  */
  YYSYMBOL_switch_case_define = 116        /* switch_case_define  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

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
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
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
#define YYLAST   1433

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  66
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  152
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  267

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   320


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
      65
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   169,   169,   178,   181,   188,   197,   200,   202,   206,
     216,   219,   221,   225,   236,   240,   249,   260,   263,   265,
     269,   279,   282,   284,   290,   302,   317,   321,   330,   340,
     350,   361,   372,   383,   400,   420,   424,   433,   443,   453,
     462,   472,   478,   484,   490,   496,   502,   508,   514,   520,
     526,   532,   538,   546,   557,   570,   576,   584,   598,   614,
     623,   634,   645,   659,   663,   672,   682,   691,   703,   707,
     715,   725,   731,   741,   751,   761,   771,   781,   791,   801,
     811,   821,   831,   841,   853,   859,   865,   873,   881,   891,
     900,   909,   915,   921,   929,   939,   951,   961,   973,   982,
     991,   997,  1003,  1011,  1021,  1031,  1041,  1051,  1061,  1077,
    1085,  1093,  1101,  1110,  1118,  1128,  1134,  1140,  1148,  1154,
    1164,  1174,  1184,  1194,  1204,  1216,  1222,  1228,  1234,  1242,
    1251,  1260,  1269,  1278,  1287,  1296,  1305,  1315,  1328,  1334,
    1342,  1353,  1365,  1371,  1379,  1390,  1399,  1408,  1418,  1430,
    1438,  1449,  1458
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "VAR_BEGIN", "RETURN",
  "BREAK", "FUNC", "WHILE", "FTRUE", "FFALSE", "IF", "THEN", "ELSE", "END",
  "STRING_DEFINITION", "IDENTIFIER", "NUMBER", "SINGLE_LINE_COMMENT",
  "DIVIDE_MOD", "ARG_SPLITTER", "PLUS", "MINUS", "DIVIDE", "MULTIPLY",
  "ASSIGN", "MORE", "LESS", "MORE_OR_EQUAL", "LESS_OR_EQUAL", "EQUAL",
  "NOT_EQUAL", "OPEN_BRACKET", "CLOSE_BRACKET", "AND", "OR", "FKFLOAT",
  "PLUS_ASSIGN", "MINUS_ASSIGN", "DIVIDE_ASSIGN", "MULTIPLY_ASSIGN",
  "DIVIDE_MOD_ASSIGN", "COLON", "FOR", "INC", "FKUUID",
  "OPEN_SQUARE_BRACKET", "CLOSE_SQUARE_BRACKET", "FCONST", "PACKAGE",
  "INCLUDE", "IDENTIFIER_DOT", "IDENTIFIER_POINTER", "STRUCT", "IS", "NOT",
  "CONTINUE", "SWITCH", "CASE", "DEFAULT", "NEW_ASSIGN", "ELSEIF",
  "RIGHT_POINTER", "STRING_CAT", "OPEN_BIG_BRACKET", "CLOSE_BIG_BRACKET",
  "FNULL", "$accept", "program", "package_head", "include_head",
  "include_define", "struct_head", "struct_define",
  "struct_mem_declaration", "const_head", "const_define", "body",
  "function_declaration", "function_declaration_arguments", "arg",
  "function_call", "function_call_arguments", "arg_expr", "block", "stmt",
  "for_stmt", "for_loop_value", "for_loop_stmt", "while_stmt", "if_stmt",
  "elseif_stmt_list", "elseif_stmt", "else_stmt", "cmp", "cmp_value",
  "return_stmt", "return_value_list", "return_value", "assign_stmt",
  "multi_assign_stmt", "var_list", "assign_value", "math_assign_stmt",
  "var", "variable", "expr", "math_expr", "expr_value", "explicit_value",
  "const_map_list_value", "const_map_value", "const_array_list_value",
  "break", "continue", "switch_stmt", "switch_case_list",
  "switch_case_define", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-193)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-129)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -40,    27,    14,   -26,  -193,  -193,  -193,     6,   130,  -193,
    -193,    32,  -193,    98,  -193,    66,   100,  -193,    -1,  -193,
    -193,   114,   112,   153,  -193,   174,  -193,  -193,  -193,  1368,
     159,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  1368,  1368,
    -193,  -193,   169,  -193,   426,   150,  1357,  -193,  -193,    33,
    -193,  -193,  -193,  1368,  -193,  -193,   169,   423,  -193,  -193,
     184,    13,  -193,  1283,  1283,  -193,   -20,    13,  1245,   170,
    -193,  -193,    13,   110,   489,  -193,  -193,  -193,  -193,  -193,
    -193,  -193,  -193,    -6,  -193,     9,   539,  -193,   381,   491,
    -193,  -193,  -193,  -193,  -193,   181,  -193,   240,  -193,   507,
     128,   258,  1283,    13,    13,    83,   289,   240,  -193,   507,
     132,  1307,  1307,   163,   175,   223,   552,    59,  1307,   145,
    1307,   191,  -193,  -193,     4,    57,    57,    13,    13,    13,
      13,    13,    13,    13,   213,  -193,  1307,  1307,  1307,  1307,
    1307,  1307,    13,   161,   175,  -193,  -193,   615,  1283,  1283,
      13,    13,    13,    13,    13,    13,   678,  1307,   103,    52,
    -193,  -193,   491,   303,  -193,  -193,  1283,    13,    73,    13,
      96,  -193,   123,   188,   186,  -193,  -193,  -193,   103,   163,
     103,  -193,   240,  -193,   507,  -193,  -193,  -193,  -193,  -193,
    -193,   201,   185,     8,     8,   185,   185,  -193,  -193,  -193,
    -193,   741,   141,   141,  -193,  -193,  -193,  -193,  -193,  -193,
    1283,   678,   -11,  -193,   216,  1307,  -193,  -193,    76,   189,
      18,   387,  -193,   238,   804,  -193,  -193,  1307,  1307,  -193,
     152,   -11,  1245,  -193,   243,  -193,  1245,  1345,  1245,  -193,
     867,   137,   151,  1245,   244,  1245,  -193,   930,   234,  -193,
    -193,  1245,  -193,  -193,  -193,  1245,  -193,   993,  1345,  -193,
    1056,   261,  -193,  1119,  -193,  1182,  -193
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     6,     4,     5,     1,     0,    10,     7,
       9,     0,     8,    17,    11,    14,     0,    12,    21,    18,
      16,     0,     0,     0,    19,     2,    22,    13,    15,     0,
       0,    23,   129,   130,   133,   131,   134,   132,   142,   138,
     135,    20,    26,   143,     0,     0,     0,   139,    29,     0,
      28,   137,   144,     0,   136,   140,     0,     0,   141,    27,
       0,    88,   145,     0,     0,    25,   111,     0,     0,   114,
     113,   146,     0,   116,     0,    40,    50,    51,    41,    42,
      43,    44,    45,     0,    49,    99,   128,    48,   117,     0,
     126,    46,    47,    52,   109,    87,    90,    92,    93,    91,
     129,   130,     0,     0,     0,     0,     0,    85,    86,    84,
       0,    35,     0,   128,     0,   117,     0,    99,    35,     0,
      35,     0,    24,    39,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   108,     0,     0,     0,     0,
       0,     0,     0,     0,    86,    82,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63,     0,   127,     0,
      37,   125,    38,     0,   115,   118,     0,     0,     0,     0,
       0,   149,     0,     0,   111,   114,    98,   110,    96,     0,
      97,    94,   101,   102,   100,    95,   103,   104,   105,   106,
     107,     0,   123,   119,   120,   122,   121,   124,    89,    71,
      60,     0,    72,    73,    75,    74,    77,    78,    76,    79,
       0,    63,    68,    65,   125,     0,    30,   112,     0,     0,
     101,   100,    31,     0,     0,   150,    32,    35,    35,    59,
       0,    68,    70,    64,     0,    36,     0,     0,   152,   148,
       0,     0,     0,    67,     0,    69,    62,     0,     0,    56,
      55,   151,   147,    33,    34,    66,    61,     0,     0,    54,
       0,     0,    53,     0,    58,     0,    57
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -193,  -193,  -193,  -193,   265,  -193,   262,  -193,  -193,   260,
    -193,   255,  -193,   227,   249,  -116,    67,   -65,    19,  -193,
    -192,  -193,  -193,  -193,    75,  -175,    56,   -52,   226,  -193,
    -193,   146,  -193,  -193,  -193,   352,  -193,   -62,    85,   204,
     -51,   280,   -29,  -193,   247,  -193,  -193,  -193,  -193,  -193,
     119
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     2,     3,     8,     9,    13,    14,    21,    18,    19,
      25,    26,    49,    50,    73,   159,   160,    74,    75,    76,
     219,    77,    78,    79,   212,   213,   234,   105,   106,    80,
      95,    96,    81,    82,    83,   181,    84,    85,    86,    87,
      88,    89,    90,    46,    47,    44,    91,    92,    93,   170,
     171
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      41,   232,   168,   116,   172,    23,   117,    60,     1,    43,
      45,   111,   110,   124,     6,    52,   115,    45,   125,   174,
      10,    32,    33,     7,    58,   112,   136,    34,    66,    35,
     139,   140,    99,   127,   109,   109,  -128,   233,  -128,  -128,
    -128,  -128,     4,   109,    67,   248,    16,    15,    36,   210,
     143,   115,    56,   126,   175,    70,   233,    37,    38,   134,
     161,   161,   176,    69,    70,    57,   261,   161,   128,   161,
     141,   215,    66,   109,   109,   109,    39,     5,    40,   -56,
    -128,    20,   201,   167,   216,   161,   161,   161,   161,   161,
     161,   211,   215,   123,   147,   236,   202,   203,   184,   184,
     184,   184,   184,   184,   184,   222,   214,    69,    70,   148,
     149,   241,   242,    99,   218,    22,   148,   149,   128,   109,
     109,   109,   109,   109,   109,   109,   109,    27,  -127,    28,
    -127,  -127,  -127,  -127,   120,   123,    29,   109,   221,   -80,
     109,   120,   215,   156,   121,    16,    97,   -80,   107,   107,
      11,   121,   113,   169,   224,   226,   215,   107,   230,   240,
     -80,   -80,   -80,   243,   161,   148,   149,   245,    30,   253,
     215,   247,  -127,   251,   148,   149,   161,   161,   255,     7,
      23,   109,    11,   254,    48,   148,   149,   107,   107,   107,
      42,    53,   260,   199,   148,   149,   113,   113,   265,    94,
     142,   118,   169,   113,   134,   113,   173,   164,   250,   177,
     179,   179,   182,   182,   182,   182,   182,   182,   182,   227,
     123,   113,   113,   113,   113,   113,   113,    97,   191,   250,
     123,   112,   228,   107,   107,   107,   107,   107,   107,   107,
     107,  -125,   113,  -125,  -125,  -125,  -125,   141,   165,   238,
     237,   107,   220,   258,   107,   165,   246,   256,  -128,   123,
    -128,  -128,  -128,  -128,   123,    98,   123,   108,   108,   -81,
     123,   114,   263,    12,   123,    17,   108,   -81,    24,   123,
      31,   134,   235,    59,   123,  -125,   231,   244,   198,   225,
     -81,   -81,   -81,    55,     0,   107,     0,     0,   119,     0,
     113,     0,  -128,     0,     0,     0,   144,   108,   108,     0,
       0,     0,   113,   113,   150,   151,   152,   153,   154,   155,
       0,   136,   249,   137,   138,   139,   140,     0,     0,   145,
     146,   183,   183,   183,   183,   183,   183,   183,     0,     0,
       0,     0,     0,   249,     0,     0,    98,     0,     0,   217,
       0,     0,   108,   108,   108,   108,   108,   108,   108,   108,
     158,   158,     0,     0,     0,   141,     0,   158,     0,   158,
     108,   183,     0,   108,   178,   180,   204,   205,   206,   207,
     208,   209,     0,     0,     0,   158,   158,   158,   158,   158,
     158,   162,   163,     0,     0,   223,     0,     0,   162,  -125,
     162,  -125,  -125,  -125,  -125,  -126,   158,  -126,  -126,  -126,
    -126,     0,     0,     0,   108,     0,   192,   193,   194,   195,
     196,   197,     0,     0,     0,     0,    60,    61,    62,     0,
      63,    32,    33,    64,    32,    33,    65,    34,    66,    35,
      34,     0,    35,  -125,     0,     0,     0,     0,   -55,  -126,
       0,     0,     0,     0,    67,     0,     0,     0,    36,     0,
       0,    36,     0,     0,   158,    68,     0,    37,    38,     0,
      37,    38,    51,    69,    70,     0,   158,   158,    71,    72,
     185,   186,   187,   188,   189,   190,    39,     0,    40,    39,
       0,    40,    60,    61,    62,   162,    63,    32,    33,    64,
       0,     0,   122,    34,    66,    35,     0,   162,   162,   136,
       0,   137,   138,   139,   140,     0,     0,     0,     0,     0,
      67,     0,     0,     0,    36,  -126,     0,  -126,  -126,  -126,
    -126,    68,     0,    37,    38,     0,     0,     0,     0,    69,
      70,     0,     0,     0,    71,    72,     0,     0,     0,     0,
       0,     0,    39,   141,    40,    60,    61,    62,  -110,    63,
      32,    33,    64,  -110,     0,     0,    34,    66,    35,  -126,
       0,   166,     0,     0,     0,   129,   130,   131,   132,   133,
     134,     0,   135,    67,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,    68,     0,    37,    38,  -110,     0,
       0,     0,    69,    70,     0,     0,     0,    71,    72,     0,
       0,     0,     0,     0,     0,    39,     0,    40,    60,    61,
      62,     0,    63,    32,    33,    64,     0,     0,   200,    34,
      66,    35,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    67,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,    68,     0,    37,
      38,     0,     0,     0,     0,    69,    70,     0,     0,     0,
      71,    72,     0,     0,     0,     0,     0,     0,    39,     0,
      40,    60,    61,    62,     0,    63,    32,    33,    64,     0,
       0,     0,    34,    66,    35,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    67,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
      68,     0,    37,    38,     0,     0,     0,     0,    69,    70,
       0,     0,     0,    71,    72,     0,     0,     0,   210,     0,
       0,    39,     0,    40,    60,    61,    62,     0,    63,    32,
      33,    64,     0,     0,   229,    34,    66,    35,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    67,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    68,     0,    37,    38,     0,     0,     0,
       0,    69,    70,     0,     0,     0,    71,    72,     0,     0,
       0,     0,     0,     0,    39,     0,    40,    60,    61,    62,
       0,    63,    32,    33,    64,     0,     0,   239,    34,    66,
      35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    67,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,    68,     0,    37,    38,
       0,     0,     0,     0,    69,    70,     0,     0,     0,    71,
      72,     0,     0,     0,     0,     0,     0,    39,     0,    40,
      60,    61,    62,     0,    63,    32,    33,    64,     0,     0,
     252,    34,    66,    35,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    67,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,    68,
       0,    37,    38,     0,     0,     0,     0,    69,    70,     0,
       0,     0,    71,    72,     0,     0,     0,     0,     0,     0,
      39,     0,    40,    60,    61,    62,     0,    63,    32,    33,
      64,   257,     0,     0,    34,    66,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    67,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,    68,     0,    37,    38,     0,     0,     0,     0,
      69,    70,     0,     0,     0,    71,    72,     0,     0,     0,
       0,     0,     0,    39,     0,    40,    60,    61,    62,     0,
      63,    32,    33,    64,     0,     0,   259,    34,    66,    35,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    67,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    68,     0,    37,    38,     0,
       0,     0,     0,    69,    70,     0,     0,     0,    71,    72,
       0,     0,     0,     0,     0,     0,    39,     0,    40,    60,
      61,    62,     0,    63,    32,    33,    64,     0,     0,   262,
      34,    66,    35,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    67,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,    68,     0,
      37,    38,     0,     0,     0,     0,    69,    70,     0,     0,
       0,    71,    72,     0,     0,     0,     0,     0,     0,    39,
       0,    40,    60,    61,    62,     0,    63,    32,    33,    64,
       0,     0,   264,    34,    66,    35,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      67,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,    68,     0,    37,    38,     0,     0,     0,     0,    69,
      70,     0,     0,     0,    71,    72,     0,     0,     0,     0,
       0,     0,    39,     0,    40,    60,    61,    62,     0,    63,
      32,    33,    64,     0,     0,   266,    34,    66,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    67,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,    68,     0,    37,    38,     0,     0,
       0,     0,    69,    70,     0,     0,     0,    71,    72,     0,
       0,     0,     0,     0,     0,    39,     0,    40,    60,    61,
      62,     0,    63,    32,    33,    64,     0,     0,     0,    34,
      66,    35,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    67,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,    68,     0,    37,
      38,   100,   101,     0,     0,    69,    70,    34,    66,    35,
      71,    72,     0,     0,     0,     0,     0,     0,    39,     0,
      40,     0,     0,     0,   102,    32,    33,     0,    36,     0,
       0,    34,    66,    35,     0,     0,     0,    37,    38,     0,
       0,     0,     0,    69,    70,     0,   103,   104,   157,     0,
       0,     0,    36,     0,     0,     0,    39,     0,    40,     0,
       0,    37,    38,    32,    33,     0,     0,    69,    70,    34,
     174,    35,     0,     0,     0,    32,    33,     0,     0,     0,
      39,    34,    40,    35,     0,     0,    32,    33,     0,     0,
      36,     0,    34,     0,    35,     0,     0,     0,     0,    37,
      38,     0,    36,     0,     0,   175,    70,     0,     0,     0,
       0,    37,    38,    36,     0,     0,     0,     0,    39,     0,
      40,     0,    37,    38,     0,     0,     0,     0,     0,     0,
      39,    54,    40,     0,     0,     0,     0,     0,     0,     0,
       0,    39,     0,    40
};

static const yytype_int16 yycheck[] =
{
      29,    12,   118,    68,   120,     6,    68,     3,    48,    38,
      39,    31,    64,    19,     0,    44,    67,    46,    24,    15,
      14,     8,     9,    49,    53,    45,    18,    14,    15,    16,
      22,    23,    61,    24,    63,    64,    18,   212,    20,    21,
      22,    23,    15,    72,    31,   237,    47,    15,    35,    60,
     102,   102,    19,    59,    50,    51,   231,    44,    45,    41,
     111,   112,   124,    50,    51,    32,   258,   118,    59,   120,
      62,    19,    15,   102,   103,   104,    63,    50,    65,    61,
      62,    15,   147,    24,    32,   136,   137,   138,   139,   140,
     141,   156,    19,    74,    11,    19,   148,   149,   127,   128,
     129,   130,   131,   132,   133,    32,   157,    50,    51,    33,
      34,   227,   228,   142,   166,    15,    33,    34,    59,   148,
     149,   150,   151,   152,   153,   154,   155,    13,    18,    15,
      20,    21,    22,    23,    31,   116,    24,   166,   167,    11,
     169,    31,    19,    11,    41,    47,    61,    19,    63,    64,
      52,    41,    67,    57,    58,    32,    19,    72,   210,   224,
      32,    33,    34,    11,   215,    33,    34,   232,    15,    32,
      19,   236,    62,   238,    33,    34,   227,   228,   243,    49,
       6,   210,    52,    32,    15,    33,    34,   102,   103,   104,
      31,    41,   257,    32,    33,    34,   111,   112,   263,    15,
      19,    31,    57,   118,    41,   120,    15,    32,   237,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,    31,
     201,   136,   137,   138,   139,   140,   141,   142,    15,   258,
     211,    45,    31,   148,   149,   150,   151,   152,   153,   154,
     155,    18,   157,    20,    21,    22,    23,    62,    32,    11,
      61,   166,   167,    19,   169,    32,    13,    13,    18,   240,
      20,    21,    22,    23,   245,    61,   247,    63,    64,    11,
     251,    67,    11,     8,   255,    13,    72,    19,    18,   260,
      25,    41,   215,    56,   265,    62,   211,   231,   142,   170,
      32,    33,    34,    46,    -1,   210,    -1,    -1,    72,    -1,
     215,    -1,    62,    -1,    -1,    -1,   102,   103,   104,    -1,
      -1,    -1,   227,   228,    25,    26,    27,    28,    29,    30,
      -1,    18,   237,    20,    21,    22,    23,    -1,    -1,   103,
     104,   127,   128,   129,   130,   131,   132,   133,    -1,    -1,
      -1,    -1,    -1,   258,    -1,    -1,   142,    -1,    -1,    46,
      -1,    -1,   148,   149,   150,   151,   152,   153,   154,   155,
     111,   112,    -1,    -1,    -1,    62,    -1,   118,    -1,   120,
     166,   167,    -1,   169,   125,   126,   150,   151,   152,   153,
     154,   155,    -1,    -1,    -1,   136,   137,   138,   139,   140,
     141,   111,   112,    -1,    -1,   169,    -1,    -1,   118,    18,
     120,    20,    21,    22,    23,    18,   157,    20,    21,    22,
      23,    -1,    -1,    -1,   210,    -1,   136,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,     3,     4,     5,    -1,
       7,     8,     9,    10,     8,     9,    13,    14,    15,    16,
      14,    -1,    16,    62,    -1,    -1,    -1,    -1,    61,    62,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    35,    -1,    -1,   215,    42,    -1,    44,    45,    -1,
      44,    45,    46,    50,    51,    -1,   227,   228,    55,    56,
     128,   129,   130,   131,   132,   133,    63,    -1,    65,    63,
      -1,    65,     3,     4,     5,   215,     7,     8,     9,    10,
      -1,    -1,    13,    14,    15,    16,    -1,   227,   228,    18,
      -1,    20,    21,    22,    23,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    18,    -1,    20,    21,    22,
      23,    42,    -1,    44,    45,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    55,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    62,    65,     3,     4,     5,    19,     7,
       8,     9,    10,    24,    -1,    -1,    14,    15,    16,    62,
      -1,    19,    -1,    -1,    -1,    36,    37,    38,    39,    40,
      41,    -1,    43,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    44,    45,    59,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    65,     3,     4,
       5,    -1,     7,     8,     9,    10,    -1,    -1,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    44,
      45,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    56,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      65,     3,     4,     5,    -1,     7,     8,     9,    10,    -1,
      -1,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    -1,    44,    45,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    55,    56,    -1,    -1,    -1,    60,    -1,
      -1,    63,    -1,    65,     3,     4,     5,    -1,     7,     8,
       9,    10,    -1,    -1,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    44,    45,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    -1,    65,     3,     4,     5,
      -1,     7,     8,     9,    10,    -1,    -1,    13,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    44,    45,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    55,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    65,
       3,     4,     5,    -1,     7,     8,     9,    10,    -1,    -1,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    44,    45,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    55,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    -1,    65,     3,     4,     5,    -1,     7,     8,     9,
      10,    11,    -1,    -1,    14,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    -1,    44,    45,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    65,     3,     4,     5,    -1,
       7,     8,     9,    10,    -1,    -1,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    44,    45,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    55,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    65,     3,
       4,     5,    -1,     7,     8,     9,    10,    -1,    -1,    13,
      14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      44,    45,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    56,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      -1,    65,     3,     4,     5,    -1,     7,     8,     9,    10,
      -1,    -1,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    -1,    44,    45,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    55,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    65,     3,     4,     5,    -1,     7,
       8,     9,    10,    -1,    -1,    13,    14,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    44,    45,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    65,     3,     4,
       5,    -1,     7,     8,     9,    10,    -1,    -1,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    44,
      45,     8,     9,    -1,    -1,    50,    51,    14,    15,    16,
      55,    56,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      65,    -1,    -1,    -1,    31,     8,     9,    -1,    35,    -1,
      -1,    14,    15,    16,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    -1,    50,    51,    -1,    53,    54,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    63,    -1,    65,    -1,
      -1,    44,    45,     8,     9,    -1,    -1,    50,    51,    14,
      15,    16,    -1,    -1,    -1,     8,     9,    -1,    -1,    -1,
      63,    14,    65,    16,    -1,    -1,     8,     9,    -1,    -1,
      35,    -1,    14,    -1,    16,    -1,    -1,    -1,    -1,    44,
      45,    -1,    35,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    44,    45,    35,    -1,    -1,    -1,    -1,    63,    -1,
      65,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    -1,    65
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    48,    67,    68,    15,    50,     0,    49,    69,    70,
      14,    52,    70,    71,    72,    15,    47,    72,    74,    75,
      15,    73,    15,     6,    75,    76,    77,    13,    15,    24,
      15,    77,     8,     9,    14,    16,    35,    44,    45,    63,
      65,   108,    31,   108,   111,   108,   109,   110,    15,    78,
      79,    46,   108,    41,    64,   110,    19,    32,   108,    79,
       3,     4,     5,     7,    10,    13,    15,    31,    42,    50,
      51,    55,    56,    80,    83,    84,    85,    87,    88,    89,
      95,    98,    99,   100,   102,   103,   104,   105,   106,   107,
     108,   112,   113,   114,    15,    96,    97,   104,   105,   108,
       8,     9,    31,    53,    54,    93,    94,   104,   105,   108,
      93,    31,    45,   104,   105,   106,    83,   103,    31,    94,
      31,    41,    13,    84,    19,    24,    59,    24,    59,    36,
      37,    38,    39,    40,    41,    43,    18,    20,    21,    22,
      23,    62,    19,    93,   105,    94,    94,    11,    33,    34,
      25,    26,    27,    28,    29,    30,    11,    31,    80,    81,
      82,   106,   107,   107,    32,    32,    19,    24,    81,    57,
     115,   116,    81,    15,    15,    50,   103,   104,    80,   104,
      80,   101,   104,   105,   108,   101,   101,   101,   101,   101,
     101,    15,   107,   107,   107,   107,   107,   107,    97,    32,
      13,    83,    93,    93,    94,    94,    94,    94,    94,    94,
      60,    83,    90,    91,   106,    19,    32,    46,    93,    86,
     104,   108,    32,    94,    58,   116,    32,    31,    31,    13,
      93,    90,    12,    91,    92,    82,    19,    61,    11,    13,
      83,    81,    81,    11,    92,    83,    13,    83,    86,   104,
     108,    83,    13,    32,    32,    83,    13,    11,    19,    13,
      83,    86,    13,    11,    13,    83,    13
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    66,    67,    68,    68,    68,    69,    69,    69,    70,
      71,    71,    71,    72,    73,    73,    73,    74,    74,    74,
      75,    76,    76,    76,    77,    77,    78,    78,    78,    79,
      80,    80,    80,    80,    80,    81,    81,    81,    82,    83,
      83,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    85,    85,    86,    86,    87,    87,    88,
      88,    89,    89,    90,    90,    90,    91,    91,    92,    92,
      92,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    94,    94,    94,    95,    95,    96,
      96,    97,    97,    97,    98,    98,    99,    99,   100,   100,
     101,   101,   101,   102,   102,   102,   102,   102,   102,   103,
     103,   104,   104,   104,   104,   105,   105,   105,   106,   106,
     106,   106,   106,   106,   106,   107,   107,   107,   107,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   109,   109,
     109,   110,   111,   111,   111,   112,   113,   114,   114,   115,
     115,   116,   116
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     5,     0,     2,     2,     0,     1,     2,     2,
       0,     1,     2,     4,     0,     2,     1,     0,     1,     2,
       4,     0,     1,     2,     7,     6,     0,     3,     1,     1,
       4,     4,     4,     6,     6,     0,     3,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     9,     8,     1,     1,    11,    10,     5,
       4,     7,     6,     0,     2,     1,     4,     3,     0,     2,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     2,     2,     1,     1,     1,     2,     1,     3,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     2,     2,
       1,     1,     4,     1,     1,     3,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     0,     1,
       2,     3,     0,     1,     2,     1,     1,     6,     5,     1,
       2,     4,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

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


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
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

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, parm); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void * parm)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (parm);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void * parm)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, parm);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, void * parm)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), parm);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
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
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
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
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
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
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
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
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void * parm)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (parm);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void * parm)
{
/* Lookahead token kind.  */
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
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc, parm);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 3: /* package_head: %empty  */
#line 178 "bison.y"
        {
	}
#line 2041 "bison.tab.c"
    break;

  case 4: /* package_head: PACKAGE IDENTIFIER  */
#line 182 "bison.y"
        {
		FKLOG("[bison]: package %s", (yyvsp[0].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[0].str).c_str());
	}
#line 2051 "bison.tab.c"
    break;

  case 5: /* package_head: PACKAGE IDENTIFIER_DOT  */
#line 189 "bison.y"
        {
		FKLOG("[bison]: package %s", (yyvsp[0].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->set_package((yyvsp[0].str).c_str());
	}
#line 2061 "bison.tab.c"
    break;

  case 6: /* include_head: %empty  */
#line 197 "bison.y"
        {
	}
#line 2068 "bison.tab.c"
    break;

  case 9: /* include_define: INCLUDE STRING_DEFINITION  */
#line 207 "bison.y"
        {
		FKLOG("[bison]: include %s", (yyvsp[0].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_include((yyvsp[0].str).c_str());
	}
#line 2078 "bison.tab.c"
    break;

  case 10: /* struct_head: %empty  */
#line 216 "bison.y"
        {
	}
#line 2085 "bison.tab.c"
    break;

  case 13: /* struct_define: STRUCT IDENTIFIER struct_mem_declaration END  */
#line 226 "bison.y"
        {
		FKLOG("[bison]: struct_define %s", (yyvsp[-2].str).c_str());
		myflexer *l = (myflexer *)parm;
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[-1].syntree));
		l->add_struct_desc((yyvsp[-2].str).c_str(), p);
	}
#line 2096 "bison.tab.c"
    break;

  case 14: /* struct_mem_declaration: %empty  */
#line 236 "bison.y"
        {
		(yyval.syntree) = 0;
	}
#line 2104 "bison.tab.c"
    break;

  case 15: /* struct_mem_declaration: struct_mem_declaration IDENTIFIER  */
#line 241 "bison.y"
        {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER struct_mem_declaration");
		assert((yyvsp[-1].syntree)->gettype() == est_struct_memlist);
		struct_desc_memlist_node * p = dynamic_cast<struct_desc_memlist_node*>((yyvsp[-1].syntree));
		p->add_arg((yyvsp[0].str));
		(yyval.syntree) = p;
	}
#line 2116 "bison.tab.c"
    break;

  case 16: /* struct_mem_declaration: IDENTIFIER  */
#line 250 "bison.y"
        {
		FKLOG("[bison]: struct_mem_declaration <- IDENTIFIER");
		NEWTYPE(p, struct_desc_memlist_node);
		p->add_arg((yyvsp[0].str));
		(yyval.syntree) = p;
	}
#line 2127 "bison.tab.c"
    break;

  case 17: /* const_head: %empty  */
#line 260 "bison.y"
        {
	}
#line 2134 "bison.tab.c"
    break;

  case 20: /* const_define: FCONST IDENTIFIER ASSIGN explicit_value  */
#line 270 "bison.y"
        {
		FKLOG("[bison]: const_define %s", (yyvsp[-2].str).c_str());
		myflexer *l = (myflexer *)parm;
		l->add_const_desc((yyvsp[-2].str).c_str(), (yyvsp[0].syntree));
	}
#line 2144 "bison.tab.c"
    break;

  case 21: /* body: %empty  */
#line 279 "bison.y"
        {
	}
#line 2151 "bison.tab.c"
    break;

  case 24: /* function_declaration: FUNC IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET block END  */
#line 291 "bison.y"
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
#line 2166 "bison.tab.c"
    break;

  case 25: /* function_declaration: FUNC IDENTIFIER OPEN_BRACKET function_declaration_arguments CLOSE_BRACKET END  */
#line 303 "bison.y"
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
#line 2181 "bison.tab.c"
    break;

  case 26: /* function_declaration_arguments: %empty  */
#line 317 "bison.y"
        {
		(yyval.syntree) = 0;
	}
#line 2189 "bison.tab.c"
    break;

  case 27: /* function_declaration_arguments: function_declaration_arguments ARG_SPLITTER arg  */
#line 322 "bison.y"
        {
		FKLOG("[bison]: function_declaration_arguments <- arg function_declaration_arguments");
		assert((yyvsp[-2].syntree)->gettype() == est_arglist);
		func_desc_arglist_node * p = dynamic_cast<func_desc_arglist_node*>((yyvsp[-2].syntree));
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2201 "bison.tab.c"
    break;

  case 28: /* function_declaration_arguments: arg  */
#line 331 "bison.y"
        {
		FKLOG("[bison]: function_declaration_arguments <- arg");
		NEWTYPE(p, func_desc_arglist_node);
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2212 "bison.tab.c"
    break;

  case 29: /* arg: IDENTIFIER  */
#line 341 "bison.y"
        {
		FKLOG("[bison]: arg <- IDENTIFIER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, identifier_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 2223 "bison.tab.c"
    break;

  case 30: /* function_call: IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET  */
#line 351 "bison.y"
        {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[-3].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[-3].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[-1].syntree));
		p->classmem_call = false;
		(yyval.syntree) = p;
	}
#line 2237 "bison.tab.c"
    break;

  case 31: /* function_call: IDENTIFIER_DOT OPEN_BRACKET function_call_arguments CLOSE_BRACKET  */
#line 362 "bison.y"
        {
		FKLOG("[bison]: function_call <- function_call_arguments %s", (yyvsp[-3].str).c_str());
		NEWTYPE(p, function_call_node);
		p->fuc = (yyvsp[-3].str);
		p->prefunc = 0;
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[-1].syntree));
		p->classmem_call = false;
		(yyval.syntree) = p;
	}
#line 2251 "bison.tab.c"
    break;

  case 32: /* function_call: function_call OPEN_BRACKET function_call_arguments CLOSE_BRACKET  */
#line 373 "bison.y"
        {
		FKLOG("[bison]: function_call <- function_call_arguments");
		NEWTYPE(p, function_call_node);
		p->fuc = "";
		p->prefunc = (yyvsp[-3].syntree);
		p->arglist = dynamic_cast<function_call_arglist_node*>((yyvsp[-1].syntree));
		p->classmem_call = false;
		(yyval.syntree) = p;
	}
#line 2265 "bison.tab.c"
    break;

  case 33: /* function_call: function_call COLON IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET  */
#line 384 "bison.y"
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
		p->classmem_call = true;
		(yyval.syntree) = p;
	}
#line 2285 "bison.tab.c"
    break;

  case 34: /* function_call: variable COLON IDENTIFIER OPEN_BRACKET function_call_arguments CLOSE_BRACKET  */
#line 401 "bison.y"
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
		p->classmem_call = true;
		(yyval.syntree) = p;
	}
#line 2305 "bison.tab.c"
    break;

  case 35: /* function_call_arguments: %empty  */
#line 420 "bison.y"
        {
		(yyval.syntree) = 0;
	}
#line 2313 "bison.tab.c"
    break;

  case 36: /* function_call_arguments: function_call_arguments ARG_SPLITTER arg_expr  */
#line 425 "bison.y"
        {
		FKLOG("[bison]: function_call_arguments <- arg_expr function_call_arguments");
		assert((yyvsp[-2].syntree)->gettype() == est_call_arglist);
		function_call_arglist_node * p = dynamic_cast<function_call_arglist_node*>((yyvsp[-2].syntree));
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2325 "bison.tab.c"
    break;

  case 37: /* function_call_arguments: arg_expr  */
#line 434 "bison.y"
        {
		FKLOG("[bison]: function_call_arguments <- arg_expr");
		NEWTYPE(p, function_call_arglist_node);
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2336 "bison.tab.c"
    break;

  case 38: /* arg_expr: expr_value  */
#line 444 "bison.y"
        {
		FKLOG("[bison]: arg_expr <- expr_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2345 "bison.tab.c"
    break;

  case 39: /* block: block stmt  */
#line 454 "bison.y"
        {
		FKLOG("[bison]: block <- block stmt");
		assert((yyvsp[-1].syntree)->gettype() == est_block);
		block_node * p = dynamic_cast<block_node*>((yyvsp[-1].syntree));
		p->add_stmt((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2357 "bison.tab.c"
    break;

  case 40: /* block: stmt  */
#line 463 "bison.y"
        {
		FKLOG("[bison]: block <- stmt");
		NEWTYPE(p, block_node);
		p->add_stmt((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2368 "bison.tab.c"
    break;

  case 41: /* stmt: while_stmt  */
#line 473 "bison.y"
        {
		FKLOG("[bison]: stmt <- while_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2377 "bison.tab.c"
    break;

  case 42: /* stmt: if_stmt  */
#line 479 "bison.y"
        {
		FKLOG("[bison]: stmt <- if_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2386 "bison.tab.c"
    break;

  case 43: /* stmt: return_stmt  */
#line 485 "bison.y"
        {
		FKLOG("[bison]: stmt <- return_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2395 "bison.tab.c"
    break;

  case 44: /* stmt: assign_stmt  */
#line 491 "bison.y"
        {
		FKLOG("[bison]: stmt <- assign_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2404 "bison.tab.c"
    break;

  case 45: /* stmt: multi_assign_stmt  */
#line 497 "bison.y"
        {
		FKLOG("[bison]: stmt <- multi_assign_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2413 "bison.tab.c"
    break;

  case 46: /* stmt: break  */
#line 503 "bison.y"
        {
		FKLOG("[bison]: stmt <- break");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2422 "bison.tab.c"
    break;

  case 47: /* stmt: continue  */
#line 509 "bison.y"
        {
		FKLOG("[bison]: stmt <- continue");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2431 "bison.tab.c"
    break;

  case 48: /* stmt: expr  */
#line 515 "bison.y"
        {
		FKLOG("[bison]: stmt <- expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2440 "bison.tab.c"
    break;

  case 49: /* stmt: math_assign_stmt  */
#line 521 "bison.y"
        {
		FKLOG("[bison]: stmt <- math_assign_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2449 "bison.tab.c"
    break;

  case 50: /* stmt: for_stmt  */
#line 527 "bison.y"
        {
		FKLOG("[bison]: stmt <- for_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2458 "bison.tab.c"
    break;

  case 51: /* stmt: for_loop_stmt  */
#line 533 "bison.y"
        {
		FKLOG("[bison]: stmt <- for_loop_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2467 "bison.tab.c"
    break;

  case 52: /* stmt: switch_stmt  */
#line 539 "bison.y"
        {
		FKLOG("[bison]: stmt <- switch_stmt");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2476 "bison.tab.c"
    break;

  case 53: /* for_stmt: FOR block ARG_SPLITTER cmp ARG_SPLITTER block THEN block END  */
#line 547 "bison.y"
        {
		FKLOG("[bison]: for_stmt <- block cmp block");
		NEWTYPE(p, for_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-5].syntree));
		p->beginblock = dynamic_cast<block_node*>((yyvsp[-7].syntree));
		p->endblock = dynamic_cast<block_node*>((yyvsp[-3].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[-1].syntree));
		(yyval.syntree) = p;
	}
#line 2490 "bison.tab.c"
    break;

  case 54: /* for_stmt: FOR block ARG_SPLITTER cmp ARG_SPLITTER block THEN END  */
#line 558 "bison.y"
        {
		FKLOG("[bison]: for_stmt <- block cmp");
		NEWTYPE(p, for_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-4].syntree));
		p->beginblock = dynamic_cast<block_node*>((yyvsp[-6].syntree));
		p->endblock = dynamic_cast<block_node*>((yyvsp[-2].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 2504 "bison.tab.c"
    break;

  case 55: /* for_loop_value: explicit_value  */
#line 571 "bison.y"
        {
		FKLOG("[bison]: for_loop_value <- explicit_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2513 "bison.tab.c"
    break;

  case 56: /* for_loop_value: variable  */
#line 577 "bison.y"
        {
		FKLOG("[bison]: for_loop_value <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2522 "bison.tab.c"
    break;

  case 57: /* for_loop_stmt: FOR var ASSIGN for_loop_value RIGHT_POINTER for_loop_value ARG_SPLITTER for_loop_value THEN block END  */
#line 585 "bison.y"
        {
		FKLOG("[bison]: for_loop_stmt <- block");
		NEWTYPE(p, for_loop_stmt);
		
		p->iter = (yyvsp[-9].syntree);
		p->begin = (yyvsp[-7].syntree);
		p->end = (yyvsp[-5].syntree);
		p->step = (yyvsp[-3].syntree);
		p->block = dynamic_cast<block_node*>((yyvsp[-1].syntree));

		(yyval.syntree) = p;
	}
#line 2539 "bison.tab.c"
    break;

  case 58: /* for_loop_stmt: FOR var ASSIGN for_loop_value RIGHT_POINTER for_loop_value ARG_SPLITTER for_loop_value THEN END  */
#line 599 "bison.y"
        {
		FKLOG("[bison]: for_loop_stmt <- empty");
		NEWTYPE(p, for_loop_stmt);
		
		p->iter = (yyvsp[-8].syntree);
		p->begin = (yyvsp[-6].syntree);
		p->end = (yyvsp[-4].syntree);
		p->step = (yyvsp[-2].syntree);
		p->block = 0;

		(yyval.syntree) = p;
	}
#line 2556 "bison.tab.c"
    break;

  case 59: /* while_stmt: WHILE cmp THEN block END  */
#line 615 "bison.y"
        {
		FKLOG("[bison]: while_stmt <- cmp block");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-3].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[-1].syntree));
		(yyval.syntree) = p;
	}
#line 2568 "bison.tab.c"
    break;

  case 60: /* while_stmt: WHILE cmp THEN END  */
#line 624 "bison.y"
        {
		FKLOG("[bison]: while_stmt <- cmp");
		NEWTYPE(p, while_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-2].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 2580 "bison.tab.c"
    break;

  case 61: /* if_stmt: IF cmp THEN block elseif_stmt_list else_stmt END  */
#line 635 "bison.y"
        {
		FKLOG("[bison]: if_stmt <- cmp block");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-5].syntree));
		p->block = dynamic_cast<block_node*>((yyvsp[-3].syntree));
		p->elseifs = dynamic_cast<elseif_stmt_list*>((yyvsp[-2].syntree));
		p->elses = dynamic_cast<else_stmt*>((yyvsp[-1].syntree));
		(yyval.syntree) = p;
	}
#line 2594 "bison.tab.c"
    break;

  case 62: /* if_stmt: IF cmp THEN elseif_stmt_list else_stmt END  */
#line 646 "bison.y"
        {
		FKLOG("[bison]: if_stmt <- cmp");
		NEWTYPE(p, if_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-4].syntree));
		p->block = 0;
		p->elseifs = dynamic_cast<elseif_stmt_list*>((yyvsp[-2].syntree));
		p->elses = dynamic_cast<else_stmt*>((yyvsp[-1].syntree));
		(yyval.syntree) = p;
	}
#line 2608 "bison.tab.c"
    break;

  case 63: /* elseif_stmt_list: %empty  */
#line 659 "bison.y"
        {
		(yyval.syntree) = 0;
	}
#line 2616 "bison.tab.c"
    break;

  case 64: /* elseif_stmt_list: elseif_stmt_list elseif_stmt  */
#line 664 "bison.y"
        {
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt_list elseif_stmt");
		assert((yyvsp[-1].syntree)->gettype() == est_elseif_stmt_list);
		elseif_stmt_list * p = dynamic_cast<elseif_stmt_list*>((yyvsp[-1].syntree));
		p->add_stmt((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2628 "bison.tab.c"
    break;

  case 65: /* elseif_stmt_list: elseif_stmt  */
#line 673 "bison.y"
        {
		FKLOG("[bison]: elseif_stmt_list <- elseif_stmt");
		NEWTYPE(p, elseif_stmt_list);
		p->add_stmt((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2639 "bison.tab.c"
    break;

  case 66: /* elseif_stmt: ELSEIF cmp THEN block  */
#line 683 "bison.y"
        {
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-2].syntree));
		p->block = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2651 "bison.tab.c"
    break;

  case 67: /* elseif_stmt: ELSEIF cmp THEN  */
#line 692 "bison.y"
        {
		FKLOG("[bison]: elseif_stmt <- ELSEIF cmp THEN block");
		NEWTYPE(p, elseif_stmt);
		p->cmp = dynamic_cast<cmp_stmt*>((yyvsp[-1].syntree));
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 2663 "bison.tab.c"
    break;

  case 68: /* else_stmt: %empty  */
#line 703 "bison.y"
        {
		(yyval.syntree) = 0;
	}
#line 2671 "bison.tab.c"
    break;

  case 69: /* else_stmt: ELSE block  */
#line 708 "bison.y"
        {
		FKLOG("[bison]: else_stmt <- block");
		NEWTYPE(p, else_stmt);
		p->block = dynamic_cast<block_node*>((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2682 "bison.tab.c"
    break;

  case 70: /* else_stmt: ELSE  */
#line 716 "bison.y"
        {
		FKLOG("[bison]: else_stmt <- empty");
		NEWTYPE(p, else_stmt);
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 2693 "bison.tab.c"
    break;

  case 71: /* cmp: OPEN_BRACKET cmp CLOSE_BRACKET  */
#line 726 "bison.y"
        {
		FKLOG("[bison]: cmp <- ( cmp )");
		(yyval.syntree) = (yyvsp[-1].syntree);
	}
#line 2702 "bison.tab.c"
    break;

  case 72: /* cmp: cmp AND cmp  */
#line 732 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp AND cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "&&";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2715 "bison.tab.c"
    break;

  case 73: /* cmp: cmp OR cmp  */
#line 742 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp OR cmp");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "||";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2728 "bison.tab.c"
    break;

  case 74: /* cmp: cmp_value LESS cmp_value  */
#line 752 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp_value LESS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2741 "bison.tab.c"
    break;

  case 75: /* cmp: cmp_value MORE cmp_value  */
#line 762 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp_value MORE cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2754 "bison.tab.c"
    break;

  case 76: /* cmp: cmp_value EQUAL cmp_value  */
#line 772 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp_value EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2767 "bison.tab.c"
    break;

  case 77: /* cmp: cmp_value MORE_OR_EQUAL cmp_value  */
#line 782 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp_value MORE_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2780 "bison.tab.c"
    break;

  case 78: /* cmp: cmp_value LESS_OR_EQUAL cmp_value  */
#line 792 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp_value LESS_OR_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2793 "bison.tab.c"
    break;

  case 79: /* cmp: cmp_value NOT_EQUAL cmp_value  */
#line 802 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp_value NOT_EQUAL cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = (yyvsp[-1].str);
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 2806 "bison.tab.c"
    break;

  case 80: /* cmp: FTRUE  */
#line 812 "bison.y"
        {
		FKLOG("[bison]: cmp <- true");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "true";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	}
#line 2819 "bison.tab.c"
    break;

  case 81: /* cmp: FFALSE  */
#line 822 "bison.y"
        {
		FKLOG("[bison]: cmp <- false");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "false";
		p->left = 0;
		p->right = 0;
		(yyval.syntree) = p;
	}
#line 2832 "bison.tab.c"
    break;

  case 82: /* cmp: IS cmp_value  */
#line 832 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp_value IS cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "is";
		p->left = (yyvsp[0].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	}
#line 2845 "bison.tab.c"
    break;

  case 83: /* cmp: NOT cmp_value  */
#line 842 "bison.y"
        {
		FKLOG("[bison]: cmp <- cmp_value NOT cmp_value");
		NEWTYPE(p, cmp_stmt);
		p->cmp = "not";
		p->left = (yyvsp[0].syntree);
		p->right = 0;
		(yyval.syntree) = p;
	}
#line 2858 "bison.tab.c"
    break;

  case 84: /* cmp_value: explicit_value  */
#line 854 "bison.y"
        {
		FKLOG("[bison]: cmp_value <- explicit_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2867 "bison.tab.c"
    break;

  case 85: /* cmp_value: variable  */
#line 860 "bison.y"
        {
		FKLOG("[bison]: cmp_value <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2876 "bison.tab.c"
    break;

  case 86: /* cmp_value: expr  */
#line 866 "bison.y"
        {
		FKLOG("[bison]: cmp_value <- expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2885 "bison.tab.c"
    break;

  case 87: /* return_stmt: RETURN return_value_list  */
#line 874 "bison.y"
        {
		FKLOG("[bison]: return_stmt <- RETURN return_value_list");
		NEWTYPE(p, return_stmt);
		p->returnlist = dynamic_cast<return_value_list_node*>((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2896 "bison.tab.c"
    break;

  case 88: /* return_stmt: RETURN  */
#line 882 "bison.y"
        {
		FKLOG("[bison]: return_stmt <- RETURN");
		NEWTYPE(p, return_stmt);
		p->returnlist = 0;
		(yyval.syntree) = p;
	}
#line 2907 "bison.tab.c"
    break;

  case 89: /* return_value_list: return_value_list ARG_SPLITTER return_value  */
#line 892 "bison.y"
        {
		FKLOG("[bison]: return_value_list <- return_value_list return_value");
		assert((yyvsp[-2].syntree)->gettype() == est_return_value_list);
		return_value_list_node * p = dynamic_cast<return_value_list_node*>((yyvsp[-2].syntree));
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2919 "bison.tab.c"
    break;

  case 90: /* return_value_list: return_value  */
#line 901 "bison.y"
        {
		NEWTYPE(p, return_value_list_node);
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 2929 "bison.tab.c"
    break;

  case 91: /* return_value: explicit_value  */
#line 910 "bison.y"
        {
		FKLOG("[bison]: return_value <- explicit_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2938 "bison.tab.c"
    break;

  case 92: /* return_value: variable  */
#line 916 "bison.y"
        {
		FKLOG("[bison]: return_value <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2947 "bison.tab.c"
    break;

  case 93: /* return_value: expr  */
#line 922 "bison.y"
        {
		FKLOG("[bison]: return_value <- expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 2956 "bison.tab.c"
    break;

  case 94: /* assign_stmt: var ASSIGN assign_value  */
#line 930 "bison.y"
        {
		FKLOG("[bison]: assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->value = (yyvsp[0].syntree);
		p->isnew = false;
		(yyval.syntree) = p;
	}
#line 2969 "bison.tab.c"
    break;

  case 95: /* assign_stmt: var NEW_ASSIGN assign_value  */
#line 940 "bison.y"
        {
		FKLOG("[bison]: new assign_stmt <- var assign_value");
		NEWTYPE(p, assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->value = (yyvsp[0].syntree);
		p->isnew = true;
		(yyval.syntree) = p;
	}
#line 2982 "bison.tab.c"
    break;

  case 96: /* multi_assign_stmt: var_list ASSIGN function_call  */
#line 952 "bison.y"
        {
		FKLOG("[bison]: multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>((yyvsp[-2].syntree));
		p->value = (yyvsp[0].syntree);
		p->isnew = false;
		(yyval.syntree) = p;
	}
#line 2995 "bison.tab.c"
    break;

  case 97: /* multi_assign_stmt: var_list NEW_ASSIGN function_call  */
#line 962 "bison.y"
        {
		FKLOG("[bison]: new multi_assign_stmt <- var_list function_call");
		NEWTYPE(p, multi_assign_stmt);
		p->varlist = dynamic_cast<var_list_node*>((yyvsp[-2].syntree));
		p->value = (yyvsp[0].syntree);
		p->isnew = true;
		(yyval.syntree) = p;
	}
#line 3008 "bison.tab.c"
    break;

  case 98: /* var_list: var_list ARG_SPLITTER var  */
#line 974 "bison.y"
        {
		FKLOG("[bison]: var_list <- var_list var");
		assert((yyvsp[-2].syntree)->gettype() == est_var_list);
		var_list_node * p = dynamic_cast<var_list_node*>((yyvsp[-2].syntree));
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3020 "bison.tab.c"
    break;

  case 99: /* var_list: var  */
#line 983 "bison.y"
        {
		NEWTYPE(p, var_list_node);
		p->add_arg((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3030 "bison.tab.c"
    break;

  case 100: /* assign_value: explicit_value  */
#line 992 "bison.y"
        {
		FKLOG("[bison]: assign_value <- explicit_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3039 "bison.tab.c"
    break;

  case 101: /* assign_value: variable  */
#line 998 "bison.y"
        {
		FKLOG("[bison]: assign_value <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3048 "bison.tab.c"
    break;

  case 102: /* assign_value: expr  */
#line 1004 "bison.y"
        {
		FKLOG("[bison]: assign_value <- expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3057 "bison.tab.c"
    break;

  case 103: /* math_assign_stmt: variable PLUS_ASSIGN assign_value  */
#line 1012 "bison.y"
        {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "+=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3070 "bison.tab.c"
    break;

  case 104: /* math_assign_stmt: variable MINUS_ASSIGN assign_value  */
#line 1022 "bison.y"
        {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "-=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3083 "bison.tab.c"
    break;

  case 105: /* math_assign_stmt: variable DIVIDE_ASSIGN assign_value  */
#line 1032 "bison.y"
        {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "/=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3096 "bison.tab.c"
    break;

  case 106: /* math_assign_stmt: variable MULTIPLY_ASSIGN assign_value  */
#line 1042 "bison.y"
        {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "*=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3109 "bison.tab.c"
    break;

  case 107: /* math_assign_stmt: variable DIVIDE_MOD_ASSIGN assign_value  */
#line 1052 "bison.y"
        {
		FKLOG("[bison]: math_assign_stmt <- variable assign_value");
		NEWTYPE(p, math_assign_stmt);
		p->var = (yyvsp[-2].syntree);
		p->oper = "%=";
		p->value = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3122 "bison.tab.c"
    break;

  case 108: /* math_assign_stmt: variable INC  */
#line 1062 "bison.y"
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
#line 3139 "bison.tab.c"
    break;

  case 109: /* var: VAR_BEGIN IDENTIFIER  */
#line 1078 "bison.y"
        {
		FKLOG("[bison]: var <- VAR_BEGIN IDENTIFIER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, var_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 3150 "bison.tab.c"
    break;

  case 110: /* var: variable  */
#line 1086 "bison.y"
        {
		FKLOG("[bison]: var <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3159 "bison.tab.c"
    break;

  case 111: /* variable: IDENTIFIER  */
#line 1094 "bison.y"
        {
		FKLOG("[bison]: variable <- IDENTIFIER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 3170 "bison.tab.c"
    break;

  case 112: /* variable: IDENTIFIER OPEN_SQUARE_BRACKET expr_value CLOSE_SQUARE_BRACKET  */
#line 1102 "bison.y"
        {
		FKLOG("[bison]: container_get_node <- IDENTIFIER[expr_value] %s", (yyvsp[-3].str).c_str());
		NEWTYPE(p, container_get_node);
		p->container = (yyvsp[-3].str);
		p->key = (yyvsp[-1].syntree);
		(yyval.syntree) = p;
	}
#line 3182 "bison.tab.c"
    break;

  case 113: /* variable: IDENTIFIER_POINTER  */
#line 1111 "bison.y"
        {
		FKLOG("[bison]: variable <- IDENTIFIER_POINTER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, struct_pointer_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 3193 "bison.tab.c"
    break;

  case 114: /* variable: IDENTIFIER_DOT  */
#line 1119 "bison.y"
        {
		FKLOG("[bison]: variable <- IDENTIFIER_DOT %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, variable_node);
		p->str = (yyvsp[0].str);
		(yyval.syntree) = p;
	}
#line 3204 "bison.tab.c"
    break;

  case 115: /* expr: OPEN_BRACKET expr CLOSE_BRACKET  */
#line 1129 "bison.y"
        {
		FKLOG("[bison]: expr <- (expr)");
		(yyval.syntree) = (yyvsp[-1].syntree);
	}
#line 3213 "bison.tab.c"
    break;

  case 116: /* expr: function_call  */
#line 1135 "bison.y"
        {
		FKLOG("[bison]: expr <- function_call");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3222 "bison.tab.c"
    break;

  case 117: /* expr: math_expr  */
#line 1141 "bison.y"
        {
		FKLOG("[bison]: expr <- math_expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3231 "bison.tab.c"
    break;

  case 118: /* math_expr: OPEN_BRACKET math_expr CLOSE_BRACKET  */
#line 1149 "bison.y"
        {
		FKLOG("[bison]: math_expr <- (math_expr)");
		(yyval.syntree) = (yyvsp[-1].syntree);
	}
#line 3240 "bison.tab.c"
    break;

  case 119: /* math_expr: expr_value PLUS expr_value  */
#line 1155 "bison.y"
        {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "+";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3253 "bison.tab.c"
    break;

  case 120: /* math_expr: expr_value MINUS expr_value  */
#line 1165 "bison.y"
        {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "-";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3266 "bison.tab.c"
    break;

  case 121: /* math_expr: expr_value MULTIPLY expr_value  */
#line 1175 "bison.y"
        {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "*";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3279 "bison.tab.c"
    break;

  case 122: /* math_expr: expr_value DIVIDE expr_value  */
#line 1185 "bison.y"
        {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "/";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3292 "bison.tab.c"
    break;

  case 123: /* math_expr: expr_value DIVIDE_MOD expr_value  */
#line 1195 "bison.y"
        {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "%";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3305 "bison.tab.c"
    break;

  case 124: /* math_expr: expr_value STRING_CAT expr_value  */
#line 1205 "bison.y"
        {
		FKLOG("[bison]: math_expr <- expr_value %s expr_value", (yyvsp[-1].str).c_str());
		NEWTYPE(p, math_expr_node);
		p->oper = "..";
		p->left = (yyvsp[-2].syntree);
		p->right = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3318 "bison.tab.c"
    break;

  case 125: /* expr_value: math_expr  */
#line 1217 "bison.y"
        {
		FKLOG("[bison]: expr_value <- math_expr");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3327 "bison.tab.c"
    break;

  case 126: /* expr_value: explicit_value  */
#line 1223 "bison.y"
        {
		FKLOG("[bison]: expr_value <- explicit_value");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3336 "bison.tab.c"
    break;

  case 127: /* expr_value: function_call  */
#line 1229 "bison.y"
        {
		FKLOG("[bison]: expr_value <- function_call");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3345 "bison.tab.c"
    break;

  case 128: /* expr_value: variable  */
#line 1235 "bison.y"
        {
		FKLOG("[bison]: expr_value <- variable");
		(yyval.syntree) = (yyvsp[0].syntree);
	}
#line 3354 "bison.tab.c"
    break;

  case 129: /* explicit_value: FTRUE  */
#line 1243 "bison.y"
        {
		FKLOG("[bison]: explicit_value <- FTRUE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_TRUE;
		(yyval.syntree) = p;
	}
#line 3366 "bison.tab.c"
    break;

  case 130: /* explicit_value: FFALSE  */
#line 1252 "bison.y"
        {
		FKLOG("[bison]: explicit_value <- FFALSE");
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_FALSE;
		(yyval.syntree) = p;
	}
#line 3378 "bison.tab.c"
    break;

  case 131: /* explicit_value: NUMBER  */
#line 1261 "bison.y"
        {
		FKLOG("[bison]: explicit_value <- NUMBER %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_NUM;
		(yyval.syntree) = p;
	}
#line 3390 "bison.tab.c"
    break;

  case 132: /* explicit_value: FKUUID  */
#line 1270 "bison.y"
        {
		FKLOG("[bison]: explicit_value <- FKUUID %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_UUID;
		(yyval.syntree) = p;
	}
#line 3402 "bison.tab.c"
    break;

  case 133: /* explicit_value: STRING_DEFINITION  */
#line 1279 "bison.y"
        {
		FKLOG("[bison]: explicit_value <- STRING_DEFINITION %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_STR;
		(yyval.syntree) = p;
	}
#line 3414 "bison.tab.c"
    break;

  case 134: /* explicit_value: FKFLOAT  */
#line 1288 "bison.y"
        {
		FKLOG("[bison]: explicit_value <- FKFLOAT %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_FLOAT;
		(yyval.syntree) = p;
	}
#line 3426 "bison.tab.c"
    break;

  case 135: /* explicit_value: FNULL  */
#line 1297 "bison.y"
        {
		FKLOG("[bison]: explicit_value <- FNULL %s", (yyvsp[0].str).c_str());
		NEWTYPE(p, explicit_value_node);
		p->str = (yyvsp[0].str);
		p->type = explicit_value_node::EVT_NULL;
		(yyval.syntree) = p;
	}
#line 3438 "bison.tab.c"
    break;

  case 136: /* explicit_value: OPEN_BIG_BRACKET const_map_list_value CLOSE_BIG_BRACKET  */
#line 1306 "bison.y"
        {
		FKLOG("[bison]: explicit_value <- const_map_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_MAP;
		p->v = (yyvsp[-1].syntree);
		(yyval.syntree) = p;
	}
#line 3451 "bison.tab.c"
    break;

  case 137: /* explicit_value: OPEN_SQUARE_BRACKET const_array_list_value CLOSE_SQUARE_BRACKET  */
#line 1316 "bison.y"
        {
		FKLOG("[bison]: explicit_value <- const_array_list_value");
		NEWTYPE(p, explicit_value_node);
		p->str = "";
		p->type = explicit_value_node::EVT_ARRAY;
		p->v = (yyvsp[-1].syntree);
		(yyval.syntree) = p;
	}
#line 3464 "bison.tab.c"
    break;

  case 138: /* const_map_list_value: %empty  */
#line 1328 "bison.y"
        {
		FKLOG("[bison]: const_map_list_value <- null");
		NEWTYPE(p, const_map_list_value_node);
		(yyval.syntree) = p;
	}
#line 3474 "bison.tab.c"
    break;

  case 139: /* const_map_list_value: const_map_value  */
#line 1335 "bison.y"
        {
		FKLOG("[bison]: const_map_list_value <- const_map_value");
		NEWTYPE(p, const_map_list_value_node);
		p->add_ele((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3485 "bison.tab.c"
    break;

  case 140: /* const_map_list_value: const_map_list_value const_map_value  */
#line 1343 "bison.y"
        {
		FKLOG("[bison]: const_map_list_value <- const_map_list_value const_map_value");
		assert((yyvsp[-1].syntree)->gettype() == est_constmaplist);
		const_map_list_value_node * p = dynamic_cast<const_map_list_value_node*>((yyvsp[-1].syntree));
		p->add_ele((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3497 "bison.tab.c"
    break;

  case 141: /* const_map_value: explicit_value COLON explicit_value  */
#line 1354 "bison.y"
        {
		FKLOG("[bison]: const_map_value <- explicit_value");
		NEWTYPE(p, const_map_value_node);
		p->k = (yyvsp[-2].syntree);
		p->v = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3509 "bison.tab.c"
    break;

  case 142: /* const_array_list_value: %empty  */
#line 1365 "bison.y"
        {
		FKLOG("[bison]: const_array_list_value <- null");
		NEWTYPE(p, const_array_list_value_node);
		(yyval.syntree) = p;
	}
#line 3519 "bison.tab.c"
    break;

  case 143: /* const_array_list_value: explicit_value  */
#line 1372 "bison.y"
        {
		FKLOG("[bison]: const_array_list_value <- explicit_value");
		NEWTYPE(p, const_array_list_value_node);
		p->add_ele((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3530 "bison.tab.c"
    break;

  case 144: /* const_array_list_value: const_array_list_value explicit_value  */
#line 1380 "bison.y"
        {
		FKLOG("[bison]: const_array_list_value <- const_array_list_value explicit_value");
		assert((yyvsp[-1].syntree)->gettype() == est_constarraylist);
		const_array_list_value_node * p = dynamic_cast<const_array_list_value_node*>((yyvsp[-1].syntree));
		p->add_ele((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3542 "bison.tab.c"
    break;

  case 145: /* break: BREAK  */
#line 1391 "bison.y"
        {
		FKLOG("[bison]: break <- BREAK");
		NEWTYPE(p, break_stmt);
		(yyval.syntree) = p;
	}
#line 3552 "bison.tab.c"
    break;

  case 146: /* continue: CONTINUE  */
#line 1400 "bison.y"
        {
		FKLOG("[bison]: CONTINUE");
		NEWTYPE(p, continue_stmt);
		(yyval.syntree) = p;
	}
#line 3562 "bison.tab.c"
    break;

  case 147: /* switch_stmt: SWITCH cmp_value switch_case_list DEFAULT block END  */
#line 1409 "bison.y"
        {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[-4].syntree);
		p->caselist = (yyvsp[-3].syntree);
		p->def = (yyvsp[-1].syntree);
		(yyval.syntree) = p;
	}
#line 3575 "bison.tab.c"
    break;

  case 148: /* switch_stmt: SWITCH cmp_value switch_case_list DEFAULT END  */
#line 1419 "bison.y"
        {
		FKLOG("[bison]: switch_stmt");
		NEWTYPE(p, switch_stmt);
		p->cmp = (yyvsp[-3].syntree);
		p->caselist = (yyvsp[-2].syntree);
		p->def = 0;
		(yyval.syntree) = p;
	}
#line 3588 "bison.tab.c"
    break;

  case 149: /* switch_case_list: switch_case_define  */
#line 1431 "bison.y"
        {
		FKLOG("[bison]: switch_case_list <- switch_case_define");
		NEWTYPE(p, switch_caselist_node);
		p->add_case((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3599 "bison.tab.c"
    break;

  case 150: /* switch_case_list: switch_case_list switch_case_define  */
#line 1439 "bison.y"
        {
		FKLOG("[bison]: switch_case_list <- switch_case_list switch_case_define");
		assert((yyvsp[0].syntree)->gettype() == est_switch_case_node);
		switch_caselist_node * p = dynamic_cast<switch_caselist_node*>((yyvsp[-1].syntree));
		p->add_case((yyvsp[0].syntree));
		(yyval.syntree) = p;
	}
#line 3611 "bison.tab.c"
    break;

  case 151: /* switch_case_define: CASE cmp_value THEN block  */
#line 1450 "bison.y"
        {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[-2].syntree);
		p->block = (yyvsp[0].syntree);
		(yyval.syntree) = p;
	}
#line 3623 "bison.tab.c"
    break;

  case 152: /* switch_case_define: CASE cmp_value THEN  */
#line 1459 "bison.y"
        {
		FKLOG("[bison]: switch_case_define");
		NEWTYPE(p, switch_case_node);
		p->cmp = (yyvsp[-1].syntree);
		p->block = 0;
		(yyval.syntree) = p;
	}
#line 3635 "bison.tab.c"
    break;


#line 3639 "bison.tab.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, parm, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, parm);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, parm, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, parm);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

