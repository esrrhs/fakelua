// A Bison parser, made by GNU Bison 3.8.

// Skeleton interface for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


/**
 ** \file parser.h
 ** Define the yy::parser class.
 */

// C++ LALR(1) parser skeleton written by Akim Demaille.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
// "%code requires" blocks.
#line 11 "parser.y"

#include "fakelua.h"
#include "util/common.h"
#include "util/exception.h"
#include "compile/syntax_tree.h"

namespace fakelua {
    class MyFlexer;
}

// https://www.gnu.org/software/bison/manual/html_node/A-Simple-C_002b_002b-Example.html


#line 63 "parser.h"

# include <cassert>
# include <cstdlib> // std::abort
# include <iostream>
# include <stdexcept>
# include <string>
# include <vector>

#if defined __cplusplus
# define YY_CPLUSPLUS __cplusplus
#else
# define YY_CPLUSPLUS 199711L
#endif

// Support move semantics when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_MOVE           std::move
# define YY_MOVE_OR_COPY   move
# define YY_MOVE_REF(Type) Type&&
# define YY_RVREF(Type)    Type&&
# define YY_COPY(Type)     Type
#else
# define YY_MOVE
# define YY_MOVE_OR_COPY   copy
# define YY_MOVE_REF(Type) Type&
# define YY_RVREF(Type)    const Type&
# define YY_COPY(Type)     const Type&
#endif

// Support noexcept when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_NOEXCEPT noexcept
# define YY_NOTHROW
#else
# define YY_NOEXCEPT
# define YY_NOTHROW throw ()
#endif

// Support constexpr when possible.
#if 201703 <= YY_CPLUSPLUS
# define YY_CONSTEXPR constexpr
#else
# define YY_CONSTEXPR
#endif
# include "location.hh"
#include <typeinfo>
#ifndef YY_ASSERT
# include <cassert>
# define YY_ASSERT assert
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

/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

namespace yy {
#line 203 "parser.h"




  /// A Bison parser.
  class parser
  {
  public:
#ifdef YYSTYPE
# ifdef __GNUC__
#  pragma GCC message "bison: do not #define YYSTYPE in C++, use %define api.value.type"
# endif
    typedef YYSTYPE ValueType;
#else
  /// A buffer to store and retrieve objects.
  ///
  /// Sort of a variant, but does not keep track of the nature
  /// of the stored data, since that knowledge is available
  /// via the current parser state.
  class ValueType
  {
  public:
    /// Type of *this.
    typedef ValueType self_type;

    /// Empty construction.
    ValueType () YY_NOEXCEPT
      : yyraw_ ()
      , yytypeid_ (YY_NULLPTR)
    {}

    /// Construct and fill.
    template <typename T>
    ValueType (YY_RVREF (T) t)
      : yytypeid_ (&typeid (T))
    {
      YY_ASSERT (sizeof (T) <= size);
      new (yyas_<T> ()) T (YY_MOVE (t));
    }

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    ValueType (const self_type&) = delete;
    /// Non copyable.
    self_type& operator= (const self_type&) = delete;
#endif

    /// Destruction, allowed only if empty.
    ~ValueType () YY_NOEXCEPT
    {
      YY_ASSERT (!yytypeid_);
    }

# if 201103L <= YY_CPLUSPLUS
    /// Instantiate a \a T in here from \a t.
    template <typename T, typename... U>
    T&
    emplace (U&&... u)
    {
      YY_ASSERT (!yytypeid_);
      YY_ASSERT (sizeof (T) <= size);
      yytypeid_ = & typeid (T);
      return *new (yyas_<T> ()) T (std::forward <U>(u)...);
    }
# else
    /// Instantiate an empty \a T in here.
    template <typename T>
    T&
    emplace ()
    {
      YY_ASSERT (!yytypeid_);
      YY_ASSERT (sizeof (T) <= size);
      yytypeid_ = & typeid (T);
      return *new (yyas_<T> ()) T ();
    }

    /// Instantiate a \a T in here from \a t.
    template <typename T>
    T&
    emplace (const T& t)
    {
      YY_ASSERT (!yytypeid_);
      YY_ASSERT (sizeof (T) <= size);
      yytypeid_ = & typeid (T);
      return *new (yyas_<T> ()) T (t);
    }
# endif

    /// Instantiate an empty \a T in here.
    /// Obsolete, use emplace.
    template <typename T>
    T&
    build ()
    {
      return emplace<T> ();
    }

    /// Instantiate a \a T in here from \a t.
    /// Obsolete, use emplace.
    template <typename T>
    T&
    build (const T& t)
    {
      return emplace<T> (t);
    }

    /// Accessor to a built \a T.
    template <typename T>
    T&
    as () YY_NOEXCEPT
    {
      YY_ASSERT (yytypeid_);
      YY_ASSERT (*yytypeid_ == typeid (T));
      YY_ASSERT (sizeof (T) <= size);
      return *yyas_<T> ();
    }

    /// Const accessor to a built \a T (for %printer).
    template <typename T>
    const T&
    as () const YY_NOEXCEPT
    {
      YY_ASSERT (yytypeid_);
      YY_ASSERT (*yytypeid_ == typeid (T));
      YY_ASSERT (sizeof (T) <= size);
      return *yyas_<T> ();
    }

    /// Swap the content with \a that, of same type.
    ///
    /// Both variants must be built beforehand, because swapping the actual
    /// data requires reading it (with as()), and this is not possible on
    /// unconstructed variants: it would require some dynamic testing, which
    /// should not be the variant's responsibility.
    /// Swapping between built and (possibly) non-built is done with
    /// self_type::move ().
    template <typename T>
    void
    swap (self_type& that) YY_NOEXCEPT
    {
      YY_ASSERT (yytypeid_);
      YY_ASSERT (*yytypeid_ == *that.yytypeid_);
      std::swap (as<T> (), that.as<T> ());
    }

    /// Move the content of \a that to this.
    ///
    /// Destroys \a that.
    template <typename T>
    void
    move (self_type& that)
    {
# if 201103L <= YY_CPLUSPLUS
      emplace<T> (std::move (that.as<T> ()));
# else
      emplace<T> ();
      swap<T> (that);
# endif
      that.destroy<T> ();
    }

# if 201103L <= YY_CPLUSPLUS
    /// Move the content of \a that to this.
    template <typename T>
    void
    move (self_type&& that)
    {
      emplace<T> (std::move (that.as<T> ()));
      that.destroy<T> ();
    }
#endif

    /// Copy the content of \a that to this.
    template <typename T>
    void
    copy (const self_type& that)
    {
      emplace<T> (that.as<T> ());
    }

    /// Destroy the stored \a T.
    template <typename T>
    void
    destroy ()
    {
      as<T> ().~T ();
      yytypeid_ = YY_NULLPTR;
    }

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    ValueType (const self_type&);
    /// Non copyable.
    self_type& operator= (const self_type&);
#endif

    /// Accessor to raw memory as \a T.
    template <typename T>
    T*
    yyas_ () YY_NOEXCEPT
    {
      void *yyp = yyraw_;
      return static_cast<T*> (yyp);
     }

    /// Const accessor to raw memory as \a T.
    template <typename T>
    const T*
    yyas_ () const YY_NOEXCEPT
    {
      const void *yyp = yyraw_;
      return static_cast<const T*> (yyp);
     }

    /// An auxiliary type to compute the largest semantic type.
    union union_type
    {
      // chunk
      // block
      // stmt
      // attnamelist
      // elseifs
      // retstat
      // label
      // funcnamelist
      // funcname
      // varlist
      // var
      // namelist
      // explist
      // exp
      // prefixexp
      // functioncall
      // args
      // functiondef
      // funcbody
      // parlist
      // tableconstructor
      // fieldlist
      // field
      char dummy1[sizeof (fakelua::SyntaxTreeInterfacePtr)];

      // "identifier"
      // "string"
      // "number"
      char dummy2[sizeof (std::string)];
    };

    /// The size of the largest semantic type.
    enum { size = sizeof (union_type) };

    /// A buffer to store semantic values.
    union
    {
      /// Strongest alignment constraints.
      long double yyalign_me_;
      /// A buffer large enough to store any of the semantic values.
      char yyraw_[size];
    };

    /// Whether the content is built: if defined, the name of the stored type.
    const std::type_info *yytypeid_;
  };

#endif
    /// Backward compatibility (Bison 3.8).
    typedef ValueType semantic_type;
    /// Symbol locations.
    typedef location location_type;

    /// Syntax errors thrown from user actions.
    struct SyntaxError : std::runtime_error
    {
      SyntaxError (const location_type& l, const std::string& m)
        : std::runtime_error (m)
        , location (l)
      {}

      SyntaxError (const SyntaxError& s)
        : std::runtime_error (s.what ())
        , location (s.location)
      {}

      ~SyntaxError () YY_NOEXCEPT YY_NOTHROW;

      location_type location;
    };

    /// Token kinds.
    struct token
    {
      enum TokenKindType
      {
        TOK_YYEMPTY = -2,
    TOK_YYEOF = 0,                 // "end of file"
    TOK_YYerror = 1,               // error
    TOK_YYUNDEF = 2,               // "invalid token"
    TOK_ASSIGN = 3,                // "="
    TOK_MINUS = 4,                 // "-"
    TOK_PLUS = 5,                  // "+"
    TOK_STAR = 6,                  // "*"
    TOK_SLASH = 7,                 // "/"
    TOK_LPAREN = 8,                // "("
    TOK_RPAREN = 9,                // ")"
    TOK_LCURLY = 10,               // "{"
    TOK_RCURLY = 11,               // "}"
    TOK_LSQUARE = 12,              // "["
    TOK_RSQUARE = 13,              // "]"
    TOK_AND = 14,                  // "and"
    TOK_BREAK = 15,                // "break"
    TOK_DO = 16,                   // "do"
    TOK_ELSE = 17,                 // "else"
    TOK_ELSEIF = 18,               // "elseif"
    TOK_END = 19,                  // "end"
    TOK_FALSES = 20,               // "false"
    TOK_FOR = 21,                  // "for"
    TOK_FUNCTION = 22,             // "function"
    TOK_IF = 23,                   // "if"
    TOK_IN = 24,                   // "in"
    TOK_LOCAL = 25,                // "local"
    TOK_NIL = 26,                  // "nil"
    TOK_NOT = 27,                  // "not"
    TOK_OR = 28,                   // "or"
    TOK_REPEAT = 29,               // "repeat"
    TOK_RETURN = 30,               // "return"
    TOK_THEN = 31,                 // "then"
    TOK_TRUE = 32,                 // "true"
    TOK_UNTIL = 33,                // "until"
    TOK_WHILE = 34,                // "while"
    TOK_GOTO = 35,                 // "goto"
    TOK_DOUBLE_SLASH = 36,         // "//"
    TOK_CONCAT = 37,               // ".."
    TOK_VAR_PARAMS = 38,           // "..."
    TOK_EQUAL = 39,                // "=="
    TOK_MORE_EQUAL = 40,           // ">="
    TOK_LESS_EQUAL = 41,           // "<="
    TOK_NOT_EQUAL = 42,            // "~="
    TOK_LEFT_SHIFT = 43,           // "<<"
    TOK_RIGHT_SHIFT = 44,          // ">>"
    TOK_GOTO_TAG = 45,             // "::"
    TOK_SEMICOLON = 46,            // ";"
    TOK_COLON = 47,                // ":"
    TOK_COMMA = 48,                // ","
    TOK_DOT = 49,                  // "."
    TOK_POW = 50,                  // "^"
    TOK_MOD = 51,                  // "%"
    TOK_BITAND = 52,               // "&"
    TOK_BITOR = 53,                // "|"
    TOK_BITNOT = 54,               // "~"
    TOK_MORE = 55,                 // ">"
    TOK_LESS = 56,                 // "<"
    TOK_NUMBER_SIGN = 57,          // "#"
    TOK_UNARY = 58,                // UNARY
    TOK_IDENTIFIER = 59,           // "identifier"
    TOK_STRING = 60,               // "string"
    TOK_NUMBER = 61                // "number"
      };
      /// Backward compatibility alias (Bison 3.6).
      typedef TokenKindType yytokentype;
    };

    /// Token kind, as returned by yylex.
    typedef token::yytokentype TokenKindType;

    /// Backward compatibility alias (Bison 3.6).
    typedef TokenKindType token_type;

    /// Symbol kinds.
    struct SymbolKind
    {
      enum SymbolKindType
      {
        YYNTOKENS = 62, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "end of file"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_ASSIGN = 3,                            // "="
        S_MINUS = 4,                             // "-"
        S_PLUS = 5,                              // "+"
        S_STAR = 6,                              // "*"
        S_SLASH = 7,                             // "/"
        S_LPAREN = 8,                            // "("
        S_RPAREN = 9,                            // ")"
        S_LCURLY = 10,                           // "{"
        S_RCURLY = 11,                           // "}"
        S_LSQUARE = 12,                          // "["
        S_RSQUARE = 13,                          // "]"
        S_AND = 14,                              // "and"
        S_BREAK = 15,                            // "break"
        S_DO = 16,                               // "do"
        S_ELSE = 17,                             // "else"
        S_ELSEIF = 18,                           // "elseif"
        S_END = 19,                              // "end"
        S_FALSES = 20,                           // "false"
        S_FOR = 21,                              // "for"
        S_FUNCTION = 22,                         // "function"
        S_IF = 23,                               // "if"
        S_IN = 24,                               // "in"
        S_LOCAL = 25,                            // "local"
        S_NIL = 26,                              // "nil"
        S_NOT = 27,                              // "not"
        S_OR = 28,                               // "or"
        S_REPEAT = 29,                           // "repeat"
        S_RETURN = 30,                           // "return"
        S_THEN = 31,                             // "then"
        S_TRUE = 32,                             // "true"
        S_UNTIL = 33,                            // "until"
        S_WHILE = 34,                            // "while"
        S_GOTO = 35,                             // "goto"
        S_DOUBLE_SLASH = 36,                     // "//"
        S_CONCAT = 37,                           // ".."
        S_VAR_PARAMS = 38,                       // "..."
        S_EQUAL = 39,                            // "=="
        S_MORE_EQUAL = 40,                       // ">="
        S_LESS_EQUAL = 41,                       // "<="
        S_NOT_EQUAL = 42,                        // "~="
        S_LEFT_SHIFT = 43,                       // "<<"
        S_RIGHT_SHIFT = 44,                      // ">>"
        S_GOTO_TAG = 45,                         // "::"
        S_SEMICOLON = 46,                        // ";"
        S_COLON = 47,                            // ":"
        S_COMMA = 48,                            // ","
        S_DOT = 49,                              // "."
        S_POW = 50,                              // "^"
        S_MOD = 51,                              // "%"
        S_BITAND = 52,                           // "&"
        S_BITOR = 53,                            // "|"
        S_BITNOT = 54,                           // "~"
        S_MORE = 55,                             // ">"
        S_LESS = 56,                             // "<"
        S_NUMBER_SIGN = 57,                      // "#"
        S_UNARY = 58,                            // UNARY
        S_IDENTIFIER = 59,                       // "identifier"
        S_STRING = 60,                           // "string"
        S_NUMBER = 61,                           // "number"
        S_YYACCEPT = 62,                         // $accept
        S_chunk = 63,                            // chunk
        S_block = 64,                            // block
        S_stmt = 65,                             // stmt
        S_attnamelist = 66,                      // attnamelist
        S_elseifs = 67,                          // elseifs
        S_retstat = 68,                          // retstat
        S_label = 69,                            // label
        S_funcnamelist = 70,                     // funcnamelist
        S_funcname = 71,                         // funcname
        S_varlist = 72,                          // varlist
        S_var = 73,                              // var
        S_namelist = 74,                         // namelist
        S_explist = 75,                          // explist
        S_exp = 76,                              // exp
        S_prefixexp = 77,                        // prefixexp
        S_functioncall = 78,                     // functioncall
        S_args = 79,                             // args
        S_functiondef = 80,                      // functiondef
        S_funcbody = 81,                         // funcbody
        S_parlist = 82,                          // parlist
        S_tableconstructor = 83,                 // tableconstructor
        S_fieldlist = 84,                        // fieldlist
        S_field = 85,                            // field
        S_fieldsep = 86                          // fieldsep
      };
    };

    /// (Internal) symbol kind.
    typedef SymbolKind::SymbolKindType SymbolKindType;

    /// The number of tokens.
    static const SymbolKindType YYNTOKENS = SymbolKind::YYNTOKENS;

    /// A complete symbol.
    ///
    /// Expects its Base type to provide access to the symbol kind
    /// via kind ().
    ///
    /// Provide access to semantic value and location.
    template <typename Base>
    struct BasicSymbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      BasicSymbol ()
        : value ()
        , location ()
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      BasicSymbol (BasicSymbol&& that)
        : Base (std::move (that))
        , value ()
        , location (std::move (that.location))
      {
        switch (this->kind ())
    {
      case SymbolKind::S_chunk: // chunk
      case SymbolKind::S_block: // block
      case SymbolKind::S_stmt: // stmt
      case SymbolKind::S_attnamelist: // attnamelist
      case SymbolKind::S_elseifs: // elseifs
      case SymbolKind::S_retstat: // retstat
      case SymbolKind::S_label: // label
      case SymbolKind::S_funcnamelist: // funcnamelist
      case SymbolKind::S_funcname: // funcname
      case SymbolKind::S_varlist: // varlist
      case SymbolKind::S_var: // var
      case SymbolKind::S_namelist: // namelist
      case SymbolKind::S_explist: // explist
      case SymbolKind::S_exp: // exp
      case SymbolKind::S_prefixexp: // prefixexp
      case SymbolKind::S_functioncall: // functioncall
      case SymbolKind::S_args: // args
      case SymbolKind::S_functiondef: // functiondef
      case SymbolKind::S_funcbody: // funcbody
      case SymbolKind::S_parlist: // parlist
      case SymbolKind::S_tableconstructor: // tableconstructor
      case SymbolKind::S_fieldlist: // fieldlist
      case SymbolKind::S_field: // field
        value.move< fakelua::SyntaxTreeInterfacePtr > (std::move (that.value));
        break;

      case SymbolKind::S_IDENTIFIER: // "identifier"
      case SymbolKind::S_STRING: // "string"
      case SymbolKind::S_NUMBER: // "number"
        value.move< std::string > (std::move (that.value));
        break;

      default:
        break;
    }

      }
#endif

      /// Copy constructor.
      BasicSymbol (const BasicSymbol& that);

      /// Constructors for typed symbols.
#if 201103L <= YY_CPLUSPLUS
      BasicSymbol (typename Base::kind_type t, location_type&& l)
        : Base (t)
        , location (std::move (l))
      {}
#else
      BasicSymbol (typename Base::kind_type t, const location_type& l)
        : Base (t)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      BasicSymbol (typename Base::kind_type t, fakelua::SyntaxTreeInterfacePtr&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      BasicSymbol (typename Base::kind_type t, const fakelua::SyntaxTreeInterfacePtr& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      BasicSymbol (typename Base::kind_type t, std::string&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      BasicSymbol (typename Base::kind_type t, const std::string& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

      /// Destroy the symbol.
      ~BasicSymbol ()
      {
        clear ();
      }

      /// Destroy contents, and record that is empty.
      void clear () YY_NOEXCEPT
      {
        // User destructor.
        SymbolKindType yykind = this->kind ();
        BasicSymbol<Base>& yysym = *this;
        (void) yysym;
        switch (yykind)
        {
       default:
          break;
        }

        // Value type destructor.
switch (yykind)
    {
      case SymbolKind::S_chunk: // chunk
      case SymbolKind::S_block: // block
      case SymbolKind::S_stmt: // stmt
      case SymbolKind::S_attnamelist: // attnamelist
      case SymbolKind::S_elseifs: // elseifs
      case SymbolKind::S_retstat: // retstat
      case SymbolKind::S_label: // label
      case SymbolKind::S_funcnamelist: // funcnamelist
      case SymbolKind::S_funcname: // funcname
      case SymbolKind::S_varlist: // varlist
      case SymbolKind::S_var: // var
      case SymbolKind::S_namelist: // namelist
      case SymbolKind::S_explist: // explist
      case SymbolKind::S_exp: // exp
      case SymbolKind::S_prefixexp: // prefixexp
      case SymbolKind::S_functioncall: // functioncall
      case SymbolKind::S_args: // args
      case SymbolKind::S_functiondef: // functiondef
      case SymbolKind::S_funcbody: // funcbody
      case SymbolKind::S_parlist: // parlist
      case SymbolKind::S_tableconstructor: // tableconstructor
      case SymbolKind::S_fieldlist: // fieldlist
      case SymbolKind::S_field: // field
        value.template destroy< fakelua::SyntaxTreeInterfacePtr > ();
        break;

      case SymbolKind::S_IDENTIFIER: // "identifier"
      case SymbolKind::S_STRING: // "string"
      case SymbolKind::S_NUMBER: // "number"
        value.template destroy< std::string > ();
        break;

      default:
        break;
    }

        Base::clear ();
      }

      /// The user-facing name of this symbol.
      const char *name () const YY_NOEXCEPT
      {
        return parser::SymbolName (this->kind ());
      }

      /// Backward compatibility (Bison 3.6).
      SymbolKindType TypeGet () const YY_NOEXCEPT;

      /// Whether empty.
      bool empty () const YY_NOEXCEPT;

      /// Destructive move, \a s is emptied into this.
      void move (BasicSymbol& s);

      /// The semantic value.
      ValueType value;

      /// The location.
      location_type location;

    private:
#if YY_CPLUSPLUS < 201103L
      /// Assignment operator.
      BasicSymbol& operator= (const BasicSymbol& that);
#endif
    };

    /// Type access provider for token (enum) based symbols.
    struct ByKind
    {
      /// Default constructor.
      ByKind ();

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      ByKind (ByKind&& that);
#endif

      /// Copy constructor.
      ByKind (const ByKind& that);

      /// The symbol kind as needed by the constructor.
      typedef TokenKindType kind_type;

      /// Constructor from (external) token numbers.
      ByKind (kind_type t);

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (ByKind& that);

      /// The (internal) type number (corresponding to \a type).
      /// \a empty when empty.
      SymbolKindType kind () const YY_NOEXCEPT;

      /// Backward compatibility (Bison 3.6).
      SymbolKindType TypeGet () const YY_NOEXCEPT;

      /// The symbol kind.
      /// \a S_YYEMPTY when empty.
      SymbolKindType kind_;
    };

    /// Backward compatibility for a private implementation detail (Bison 3.6).
    typedef ByKind by_type;

    /// "External" symbols: returned by the scanner.
    struct SymbolType : BasicSymbol<ByKind>
    {
      /// Superclass.
      typedef BasicSymbol<ByKind> super_type;

      /// Empty symbol.
      SymbolType () {}

      /// Constructor for valueless symbols, and symbols from each type.
#if 201103L <= YY_CPLUSPLUS
      SymbolType (int tok, location_type l)
        : super_type (token_type (tok), std::move (l))
#else
      SymbolType (int tok, const location_type& l)
        : super_type (token_type (tok), l)
#endif
      {
#if !defined _MSC_VER || defined __clang__
        YY_ASSERT (tok == token::TOK_YYEOF
                   || (token::TOK_YYerror <= tok && tok <= token::TOK_UNARY));
#endif
      }
#if 201103L <= YY_CPLUSPLUS
      SymbolType (int tok, std::string v, location_type l)
        : super_type (token_type (tok), std::move (v), std::move (l))
#else
      SymbolType (int tok, const std::string& v, const location_type& l)
        : super_type (token_type (tok), v, l)
#endif
      {
#if !defined _MSC_VER || defined __clang__
        YY_ASSERT ((token::TOK_IDENTIFIER <= tok && tok <= token::TOK_NUMBER));
#endif
      }
    };

    /// Build a parser object.
    parser (fakelua::MyFlexer* l_yyarg);
    virtual ~parser ();

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    parser (const parser&) = delete;
    /// Non copyable.
    parser& operator= (const parser&) = delete;
#endif

    /// Parse.  An alias for parse ().
    /// \returns  0 iff parsing succeeded.
    int operator() ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if YYDEBUG
    /// The current debugging stream.
    std::ostream& DebugStream () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging stream.
    void SetDebugStream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type DebugLevel () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging level.
    void SetDebugLevel (debug_level_type l);
#endif

    /// Report a syntax error.
    /// \param loc    where the syntax error is found.
    /// \param msg    a description of the syntax error.
    virtual void error (const location_type& loc, const std::string& msg);

    /// Report a syntax error.
    void error (const SyntaxError& err);

    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static const char *SymbolName (SymbolKindType yysymbol);

    // Implementation of make_symbol for each token kind.
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_YYEOF (location_type l)
      {
        return SymbolType (token::TOK_YYEOF, std::move (l));
      }
#else
      static
      SymbolType
      make_YYEOF (const location_type& l)
      {
        return SymbolType (token::TOK_YYEOF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_YYerror (location_type l)
      {
        return SymbolType (token::TOK_YYerror, std::move (l));
      }
#else
      static
      SymbolType
      make_YYerror (const location_type& l)
      {
        return SymbolType (token::TOK_YYerror, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_YYUNDEF (location_type l)
      {
        return SymbolType (token::TOK_YYUNDEF, std::move (l));
      }
#else
      static
      SymbolType
      make_YYUNDEF (const location_type& l)
      {
        return SymbolType (token::TOK_YYUNDEF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_ASSIGN (location_type l)
      {
        return SymbolType (token::TOK_ASSIGN, std::move (l));
      }
#else
      static
      SymbolType
      make_ASSIGN (const location_type& l)
      {
        return SymbolType (token::TOK_ASSIGN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_MINUS (location_type l)
      {
        return SymbolType (token::TOK_MINUS, std::move (l));
      }
#else
      static
      SymbolType
      make_MINUS (const location_type& l)
      {
        return SymbolType (token::TOK_MINUS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_PLUS (location_type l)
      {
        return SymbolType (token::TOK_PLUS, std::move (l));
      }
#else
      static
      SymbolType
      make_PLUS (const location_type& l)
      {
        return SymbolType (token::TOK_PLUS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_STAR (location_type l)
      {
        return SymbolType (token::TOK_STAR, std::move (l));
      }
#else
      static
      SymbolType
      make_STAR (const location_type& l)
      {
        return SymbolType (token::TOK_STAR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_SLASH (location_type l)
      {
        return SymbolType (token::TOK_SLASH, std::move (l));
      }
#else
      static
      SymbolType
      make_SLASH (const location_type& l)
      {
        return SymbolType (token::TOK_SLASH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_LPAREN (location_type l)
      {
        return SymbolType (token::TOK_LPAREN, std::move (l));
      }
#else
      static
      SymbolType
      make_LPAREN (const location_type& l)
      {
        return SymbolType (token::TOK_LPAREN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_RPAREN (location_type l)
      {
        return SymbolType (token::TOK_RPAREN, std::move (l));
      }
#else
      static
      SymbolType
      make_RPAREN (const location_type& l)
      {
        return SymbolType (token::TOK_RPAREN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_LCURLY (location_type l)
      {
        return SymbolType (token::TOK_LCURLY, std::move (l));
      }
#else
      static
      SymbolType
      make_LCURLY (const location_type& l)
      {
        return SymbolType (token::TOK_LCURLY, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_RCURLY (location_type l)
      {
        return SymbolType (token::TOK_RCURLY, std::move (l));
      }
#else
      static
      SymbolType
      make_RCURLY (const location_type& l)
      {
        return SymbolType (token::TOK_RCURLY, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_LSQUARE (location_type l)
      {
        return SymbolType (token::TOK_LSQUARE, std::move (l));
      }
#else
      static
      SymbolType
      make_LSQUARE (const location_type& l)
      {
        return SymbolType (token::TOK_LSQUARE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_RSQUARE (location_type l)
      {
        return SymbolType (token::TOK_RSQUARE, std::move (l));
      }
#else
      static
      SymbolType
      make_RSQUARE (const location_type& l)
      {
        return SymbolType (token::TOK_RSQUARE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_AND (location_type l)
      {
        return SymbolType (token::TOK_AND, std::move (l));
      }
#else
      static
      SymbolType
      make_AND (const location_type& l)
      {
        return SymbolType (token::TOK_AND, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_BREAK (location_type l)
      {
        return SymbolType (token::TOK_BREAK, std::move (l));
      }
#else
      static
      SymbolType
      make_BREAK (const location_type& l)
      {
        return SymbolType (token::TOK_BREAK, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_DO (location_type l)
      {
        return SymbolType (token::TOK_DO, std::move (l));
      }
#else
      static
      SymbolType
      make_DO (const location_type& l)
      {
        return SymbolType (token::TOK_DO, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_ELSE (location_type l)
      {
        return SymbolType (token::TOK_ELSE, std::move (l));
      }
#else
      static
      SymbolType
      make_ELSE (const location_type& l)
      {
        return SymbolType (token::TOK_ELSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_ELSEIF (location_type l)
      {
        return SymbolType (token::TOK_ELSEIF, std::move (l));
      }
#else
      static
      SymbolType
      make_ELSEIF (const location_type& l)
      {
        return SymbolType (token::TOK_ELSEIF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_END (location_type l)
      {
        return SymbolType (token::TOK_END, std::move (l));
      }
#else
      static
      SymbolType
      make_END (const location_type& l)
      {
        return SymbolType (token::TOK_END, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_FALSES (location_type l)
      {
        return SymbolType (token::TOK_FALSES, std::move (l));
      }
#else
      static
      SymbolType
      make_FALSES (const location_type& l)
      {
        return SymbolType (token::TOK_FALSES, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_FOR (location_type l)
      {
        return SymbolType (token::TOK_FOR, std::move (l));
      }
#else
      static
      SymbolType
      make_FOR (const location_type& l)
      {
        return SymbolType (token::TOK_FOR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_FUNCTION (location_type l)
      {
        return SymbolType (token::TOK_FUNCTION, std::move (l));
      }
#else
      static
      SymbolType
      make_FUNCTION (const location_type& l)
      {
        return SymbolType (token::TOK_FUNCTION, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_IF (location_type l)
      {
        return SymbolType (token::TOK_IF, std::move (l));
      }
#else
      static
      SymbolType
      make_IF (const location_type& l)
      {
        return SymbolType (token::TOK_IF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_IN (location_type l)
      {
        return SymbolType (token::TOK_IN, std::move (l));
      }
#else
      static
      SymbolType
      make_IN (const location_type& l)
      {
        return SymbolType (token::TOK_IN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_LOCAL (location_type l)
      {
        return SymbolType (token::TOK_LOCAL, std::move (l));
      }
#else
      static
      SymbolType
      make_LOCAL (const location_type& l)
      {
        return SymbolType (token::TOK_LOCAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_NIL (location_type l)
      {
        return SymbolType (token::TOK_NIL, std::move (l));
      }
#else
      static
      SymbolType
      make_NIL (const location_type& l)
      {
        return SymbolType (token::TOK_NIL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_NOT (location_type l)
      {
        return SymbolType (token::TOK_NOT, std::move (l));
      }
#else
      static
      SymbolType
      make_NOT (const location_type& l)
      {
        return SymbolType (token::TOK_NOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_OR (location_type l)
      {
        return SymbolType (token::TOK_OR, std::move (l));
      }
#else
      static
      SymbolType
      make_OR (const location_type& l)
      {
        return SymbolType (token::TOK_OR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_REPEAT (location_type l)
      {
        return SymbolType (token::TOK_REPEAT, std::move (l));
      }
#else
      static
      SymbolType
      make_REPEAT (const location_type& l)
      {
        return SymbolType (token::TOK_REPEAT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_RETURN (location_type l)
      {
        return SymbolType (token::TOK_RETURN, std::move (l));
      }
#else
      static
      SymbolType
      make_RETURN (const location_type& l)
      {
        return SymbolType (token::TOK_RETURN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_THEN (location_type l)
      {
        return SymbolType (token::TOK_THEN, std::move (l));
      }
#else
      static
      SymbolType
      make_THEN (const location_type& l)
      {
        return SymbolType (token::TOK_THEN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_TRUE (location_type l)
      {
        return SymbolType (token::TOK_TRUE, std::move (l));
      }
#else
      static
      SymbolType
      make_TRUE (const location_type& l)
      {
        return SymbolType (token::TOK_TRUE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_UNTIL (location_type l)
      {
        return SymbolType (token::TOK_UNTIL, std::move (l));
      }
#else
      static
      SymbolType
      make_UNTIL (const location_type& l)
      {
        return SymbolType (token::TOK_UNTIL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_WHILE (location_type l)
      {
        return SymbolType (token::TOK_WHILE, std::move (l));
      }
#else
      static
      SymbolType
      make_WHILE (const location_type& l)
      {
        return SymbolType (token::TOK_WHILE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_GOTO (location_type l)
      {
        return SymbolType (token::TOK_GOTO, std::move (l));
      }
#else
      static
      SymbolType
      make_GOTO (const location_type& l)
      {
        return SymbolType (token::TOK_GOTO, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_DOUBLE_SLASH (location_type l)
      {
        return SymbolType (token::TOK_DOUBLE_SLASH, std::move (l));
      }
#else
      static
      SymbolType
      make_DOUBLE_SLASH (const location_type& l)
      {
        return SymbolType (token::TOK_DOUBLE_SLASH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_CONCAT (location_type l)
      {
        return SymbolType (token::TOK_CONCAT, std::move (l));
      }
#else
      static
      SymbolType
      make_CONCAT (const location_type& l)
      {
        return SymbolType (token::TOK_CONCAT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_VAR_PARAMS (location_type l)
      {
        return SymbolType (token::TOK_VAR_PARAMS, std::move (l));
      }
#else
      static
      SymbolType
      make_VAR_PARAMS (const location_type& l)
      {
        return SymbolType (token::TOK_VAR_PARAMS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_EQUAL (location_type l)
      {
        return SymbolType (token::TOK_EQUAL, std::move (l));
      }
#else
      static
      SymbolType
      make_EQUAL (const location_type& l)
      {
        return SymbolType (token::TOK_EQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_MORE_EQUAL (location_type l)
      {
        return SymbolType (token::TOK_MORE_EQUAL, std::move (l));
      }
#else
      static
      SymbolType
      make_MORE_EQUAL (const location_type& l)
      {
        return SymbolType (token::TOK_MORE_EQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_LESS_EQUAL (location_type l)
      {
        return SymbolType (token::TOK_LESS_EQUAL, std::move (l));
      }
#else
      static
      SymbolType
      make_LESS_EQUAL (const location_type& l)
      {
        return SymbolType (token::TOK_LESS_EQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_NOT_EQUAL (location_type l)
      {
        return SymbolType (token::TOK_NOT_EQUAL, std::move (l));
      }
#else
      static
      SymbolType
      make_NOT_EQUAL (const location_type& l)
      {
        return SymbolType (token::TOK_NOT_EQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_LEFT_SHIFT (location_type l)
      {
        return SymbolType (token::TOK_LEFT_SHIFT, std::move (l));
      }
#else
      static
      SymbolType
      make_LEFT_SHIFT (const location_type& l)
      {
        return SymbolType (token::TOK_LEFT_SHIFT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_RIGHT_SHIFT (location_type l)
      {
        return SymbolType (token::TOK_RIGHT_SHIFT, std::move (l));
      }
#else
      static
      SymbolType
      make_RIGHT_SHIFT (const location_type& l)
      {
        return SymbolType (token::TOK_RIGHT_SHIFT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_GOTO_TAG (location_type l)
      {
        return SymbolType (token::TOK_GOTO_TAG, std::move (l));
      }
#else
      static
      SymbolType
      make_GOTO_TAG (const location_type& l)
      {
        return SymbolType (token::TOK_GOTO_TAG, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_SEMICOLON (location_type l)
      {
        return SymbolType (token::TOK_SEMICOLON, std::move (l));
      }
#else
      static
      SymbolType
      make_SEMICOLON (const location_type& l)
      {
        return SymbolType (token::TOK_SEMICOLON, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_COLON (location_type l)
      {
        return SymbolType (token::TOK_COLON, std::move (l));
      }
#else
      static
      SymbolType
      make_COLON (const location_type& l)
      {
        return SymbolType (token::TOK_COLON, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_COMMA (location_type l)
      {
        return SymbolType (token::TOK_COMMA, std::move (l));
      }
#else
      static
      SymbolType
      make_COMMA (const location_type& l)
      {
        return SymbolType (token::TOK_COMMA, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_DOT (location_type l)
      {
        return SymbolType (token::TOK_DOT, std::move (l));
      }
#else
      static
      SymbolType
      make_DOT (const location_type& l)
      {
        return SymbolType (token::TOK_DOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_POW (location_type l)
      {
        return SymbolType (token::TOK_POW, std::move (l));
      }
#else
      static
      SymbolType
      make_POW (const location_type& l)
      {
        return SymbolType (token::TOK_POW, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_MOD (location_type l)
      {
        return SymbolType (token::TOK_MOD, std::move (l));
      }
#else
      static
      SymbolType
      make_MOD (const location_type& l)
      {
        return SymbolType (token::TOK_MOD, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_BITAND (location_type l)
      {
        return SymbolType (token::TOK_BITAND, std::move (l));
      }
#else
      static
      SymbolType
      make_BITAND (const location_type& l)
      {
        return SymbolType (token::TOK_BITAND, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_BITOR (location_type l)
      {
        return SymbolType (token::TOK_BITOR, std::move (l));
      }
#else
      static
      SymbolType
      make_BITOR (const location_type& l)
      {
        return SymbolType (token::TOK_BITOR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_BITNOT (location_type l)
      {
        return SymbolType (token::TOK_BITNOT, std::move (l));
      }
#else
      static
      SymbolType
      make_BITNOT (const location_type& l)
      {
        return SymbolType (token::TOK_BITNOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_MORE (location_type l)
      {
        return SymbolType (token::TOK_MORE, std::move (l));
      }
#else
      static
      SymbolType
      make_MORE (const location_type& l)
      {
        return SymbolType (token::TOK_MORE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_LESS (location_type l)
      {
        return SymbolType (token::TOK_LESS, std::move (l));
      }
#else
      static
      SymbolType
      make_LESS (const location_type& l)
      {
        return SymbolType (token::TOK_LESS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_NUMBER_SIGN (location_type l)
      {
        return SymbolType (token::TOK_NUMBER_SIGN, std::move (l));
      }
#else
      static
      SymbolType
      make_NUMBER_SIGN (const location_type& l)
      {
        return SymbolType (token::TOK_NUMBER_SIGN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_UNARY (location_type l)
      {
        return SymbolType (token::TOK_UNARY, std::move (l));
      }
#else
      static
      SymbolType
      make_UNARY (const location_type& l)
      {
        return SymbolType (token::TOK_UNARY, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_IDENTIFIER (std::string v, location_type l)
      {
        return SymbolType (token::TOK_IDENTIFIER, std::move (v), std::move (l));
      }
#else
      static
      SymbolType
      make_IDENTIFIER (const std::string& v, const location_type& l)
      {
        return SymbolType (token::TOK_IDENTIFIER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_STRING (std::string v, location_type l)
      {
        return SymbolType (token::TOK_STRING, std::move (v), std::move (l));
      }
#else
      static
      SymbolType
      make_STRING (const std::string& v, const location_type& l)
      {
        return SymbolType (token::TOK_STRING, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      SymbolType
      make_NUMBER (std::string v, location_type l)
      {
        return SymbolType (token::TOK_NUMBER, std::move (v), std::move (l));
      }
#else
      static
      SymbolType
      make_NUMBER (const std::string& v, const location_type& l)
      {
        return SymbolType (token::TOK_NUMBER, v, l);
      }
#endif


    class context
    {
    public:
      context (const parser& yyparser, const SymbolType& yyla);
      const SymbolType& lookahead () const YY_NOEXCEPT { return yyla_; }
      SymbolKindType token () const YY_NOEXCEPT { return yyla_.kind (); }
      const location_type& location () const YY_NOEXCEPT { return yyla_.location; }

      /// Put in YYARG at most YYARGN of the expected tokens, and return the
      /// number of tokens stored in YYARG.  If YYARG is null, return the
      /// number of expected tokens (guaranteed to be less than YYNTOKENS).
      int ExpectedTokens (SymbolKindType yyarg[], int yyargn) const;

    private:
      const parser& yyparser_;
      const SymbolType& yyla_;
    };

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    parser (const parser&);
    /// Non copyable.
    parser& operator= (const parser&);
#endif

    /// Check the lookahead yytoken.
    /// \returns  true iff the token will be eventually shifted.
    bool yy_lac_check_ (SymbolKindType yytoken) const;
    /// Establish the initial context if no initial context currently exists.
    /// \returns  true iff the token will be eventually shifted.
    bool yy_lac_establish_ (SymbolKindType yytoken);
    /// Discard any previous initial lookahead context because of event.
    /// \param event  the event which caused the lookahead to be discarded.
    ///               Only used for debbuging output.
    void yy_lac_discard_ (const char* event);

    /// Stored state numbers (used for stacks).
    typedef unsigned char state_type;

    /// The arguments of the error message.
    int yy_syntax_error_arguments_ (const context& yyctx,
                                    SymbolKindType yyarg[], int yyargn) const;

    /// Generate an error message.
    /// \param yyctx     the context in which the error occurred.
    virtual std::string yysyntax_error_ (const context& yyctx) const;
    /// Compute post-reduction state.
    /// \param yystate   the current state
    /// \param yysym     the nonterminal to push on the stack
    static state_type yy_lr_goto_state_ (state_type yystate, int yysym);

    /// Whether the given \c yypact_ value indicates a defaulted state.
    /// \param yyvalue   the value to check
    static bool yy_pact_value_is_default_ (int yyvalue);

    /// Whether the given \c yytable_ value indicates a syntax error.
    /// \param yyvalue   the value to check
    static bool yy_table_value_is_error_ (int yyvalue);

    static const signed char yypact_ninf_;
    static const signed char yytable_ninf_;

    /// Convert a scanner token kind \a t to a symbol kind.
    /// In theory \a t should be a TokenKindType, but character literals
    /// are valid, yet not members of the token_type enum.
    static SymbolKindType yytranslate_ (int t);



    // Tables.
    // YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
    // STATE-NUM.
    static const short yypact_[];

    // YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
    // Performed when YYTABLE does not specify something else to do.  Zero
    // means the default is an error.
    static const signed char yydefact_[];

    // YYPGOTO[NTERM-NUM].
    static const short yypgoto_[];

    // YYDEFGOTO[NTERM-NUM].
    static const unsigned char yydefgoto_[];

    // YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
    // positive, shift that token.  If negative, reduce the rule whose
    // number is the opposite.  If YYTABLE_NINF, syntax error.
    static const short yytable_[];

    static const short yycheck_[];

    // YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
    // state STATE-NUM.
    static const signed char yystos_[];

    // YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.
    static const signed char yyr1_[];

    // YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.
    static const signed char yyr2_[];


#if YYDEBUG
    // YYRLINE[YYN] -- Source line where rule number YYN was defined.
    static const short yyrline_[];
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r) const;
    /// Print the state stack on the debug stream.
    virtual void yy_stack_print_ () const;

    /// Debugging level.
    int yydebug_;
    /// Debug stream.
    std::ostream* yycdebug_;

    /// \brief Display a symbol kind, value and location.
    /// \param yyo    The output stream.
    /// \param yysym  The symbol.
    template <typename Base>
    void yy_print_ (std::ostream& yyo, const BasicSymbol<Base>& yysym) const;
#endif

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param yysym     The symbol.
    template <typename Base>
    void yy_destroy_ (const char* yymsg, BasicSymbol<Base>& yysym) const;

  private:
    /// Type access provider for state based symbols.
    struct ByState
    {
      /// Default constructor.
      ByState () YY_NOEXCEPT;

      /// The symbol kind as needed by the constructor.
      typedef state_type kind_type;

      /// Constructor.
      ByState (kind_type s) YY_NOEXCEPT;

      /// Copy constructor.
      ByState (const ByState& that) YY_NOEXCEPT;

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (ByState& that);

      /// The symbol kind (corresponding to \a state).
      /// \a SymbolKind::S_YYEMPTY when empty.
      SymbolKindType kind () const YY_NOEXCEPT;

      /// The state number used to denote an empty symbol.
      /// We use the initial state, as it does not have a value.
      enum { empty_state = 0 };

      /// The state.
      /// \a empty when empty.
      state_type state;
    };

    /// "Internal" symbol: element of the stack.
    struct StackSymbolType : BasicSymbol<ByState>
    {
      /// Superclass.
      typedef BasicSymbol<ByState> super_type;
      /// Construct an empty symbol.
      StackSymbolType ();
      /// Move or copy construction.
      StackSymbolType (YY_RVREF (StackSymbolType) that);
      /// Steal the contents from \a sym to build this.
      StackSymbolType (state_type s, YY_MOVE_REF (SymbolType) sym);
#if YY_CPLUSPLUS < 201103L
      /// Assignment, needed by push_back by some old implementations.
      /// Moves the contents of that.
      StackSymbolType& operator= (StackSymbolType& that);

      /// Assignment, needed by push_back by other implementations.
      /// Needed by some other old implementations.
      StackSymbolType& operator= (const StackSymbolType& that);
#endif
    };

    /// A stack with random access from its top.
    template <typename T, typename S = std::vector<T> >
    class stack
    {
    public:
      // Hide our reversed order.
      typedef typename S::iterator iterator;
      typedef typename S::const_iterator const_iterator;
      typedef typename S::size_type size_type;
      typedef typename std::ptrdiff_t IndexType;

      stack (size_type n = 200)
        : seq_ (n)
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Non copyable.
      stack (const stack&) = delete;
      /// Non copyable.
      stack& operator= (const stack&) = delete;
#endif

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      const T&
      operator[] (IndexType i) const
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      T&
      operator[] (IndexType i)
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Steal the contents of \a t.
      ///
      /// Close to move-semantics.
      void
      push (YY_MOVE_REF (T) t)
      {
        seq_.push_back (T ());
        operator[] (0).move (t);
      }

      /// Pop elements from the stack.
      void
      pop (std::ptrdiff_t n = 1) YY_NOEXCEPT
      {
        for (; 0 < n; --n)
          seq_.pop_back ();
      }

      /// Pop all elements from the stack.
      void
      clear () YY_NOEXCEPT
      {
        seq_.clear ();
      }

      /// Number of elements on the stack.
      IndexType
      size () const YY_NOEXCEPT
      {
        return IndexType (seq_.size ());
      }

      /// Iterator on top of the stack (going downwards).
      const_iterator
      begin () const YY_NOEXCEPT
      {
        return seq_.begin ();
      }

      /// Bottom of the stack.
      const_iterator
      end () const YY_NOEXCEPT
      {
        return seq_.end ();
      }

      /// Present a slice of the top of a stack.
      class slice
      {
      public:
        slice (const stack& stack, IndexType range)
          : stack_ (stack)
          , range_ (range)
        {}

        const T&
        operator[] (IndexType i) const
        {
          return stack_[range_ - i];
        }

      private:
        const stack& stack_;
        IndexType range_;
      };

    private:
#if YY_CPLUSPLUS < 201103L
      /// Non copyable.
      stack (const stack&);
      /// Non copyable.
      stack& operator= (const stack&);
#endif
      /// The wrapped container.
      S seq_;
    };


    /// Stack type.
    typedef stack<StackSymbolType> stack_type;

    /// The stack.
    stack_type yystack_;
    /// The stack for LAC.
    /// Logically, the yy_lac_stack's lifetime is confined to the function
    /// yy_lac_check_. We just store it as a member of this class to hold
    /// on to the memory and to avoid frequent reallocations.
    /// Since yy_lac_check_ is const, this member must be mutable.
    mutable std::vector<state_type> yylac_stack_;
    /// Whether an initial LAC context was established.
    bool yy_lac_established_;


    /// Push a new state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param sym  the symbol
    /// \warning the contents of \a s.value is stolen.
    void yypush_ (const char* m, YY_MOVE_REF (StackSymbolType) sym);

    /// Push a new look ahead token on the state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the state
    /// \param sym  the symbol (for its value and location).
    /// \warning the contents of \a sym.value is stolen.
    void yypush_ (const char* m, state_type s, YY_MOVE_REF (SymbolType) sym);

    /// Pop \a n symbols from the stack.
    void yypop_ (int n = 1);

    /// Constants.
    enum
    {
      yylast_ = 1466,     ///< Last index in yytable_.
      yynnts_ = 25,  ///< Number of nonterminal symbols.
      yyfinal_ = 58 ///< Termination state number.
    };


    // User arguments.
    fakelua::MyFlexer* l;

  };

  inline
  parser::SymbolKindType
  parser::yytranslate_ (int t)
  {
    return static_cast<SymbolKindType> (t);
  }

  // BasicSymbol.
  template <typename Base>
  parser::BasicSymbol<Base>::BasicSymbol (const BasicSymbol& that)
    : Base (that)
    , value ()
    , location (that.location)
  {
    switch (this->kind ())
    {
      case SymbolKind::S_chunk: // chunk
      case SymbolKind::S_block: // block
      case SymbolKind::S_stmt: // stmt
      case SymbolKind::S_attnamelist: // attnamelist
      case SymbolKind::S_elseifs: // elseifs
      case SymbolKind::S_retstat: // retstat
      case SymbolKind::S_label: // label
      case SymbolKind::S_funcnamelist: // funcnamelist
      case SymbolKind::S_funcname: // funcname
      case SymbolKind::S_varlist: // varlist
      case SymbolKind::S_var: // var
      case SymbolKind::S_namelist: // namelist
      case SymbolKind::S_explist: // explist
      case SymbolKind::S_exp: // exp
      case SymbolKind::S_prefixexp: // prefixexp
      case SymbolKind::S_functioncall: // functioncall
      case SymbolKind::S_args: // args
      case SymbolKind::S_functiondef: // functiondef
      case SymbolKind::S_funcbody: // funcbody
      case SymbolKind::S_parlist: // parlist
      case SymbolKind::S_tableconstructor: // tableconstructor
      case SymbolKind::S_fieldlist: // fieldlist
      case SymbolKind::S_field: // field
        value.copy< fakelua::SyntaxTreeInterfacePtr > (YY_MOVE (that.value));
        break;

      case SymbolKind::S_IDENTIFIER: // "identifier"
      case SymbolKind::S_STRING: // "string"
      case SymbolKind::S_NUMBER: // "number"
        value.copy< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

  }



  template <typename Base>
  parser::SymbolKindType
  parser::BasicSymbol<Base>::TypeGet () const YY_NOEXCEPT
  {
    return this->kind ();
  }

  template <typename Base>
  bool
  parser::BasicSymbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == SymbolKind::S_YYEMPTY;
  }

  template <typename Base>
  void
  parser::BasicSymbol<Base>::move (BasicSymbol& s)
  {
    super_type::move (s);
    switch (this->kind ())
    {
      case SymbolKind::S_chunk: // chunk
      case SymbolKind::S_block: // block
      case SymbolKind::S_stmt: // stmt
      case SymbolKind::S_attnamelist: // attnamelist
      case SymbolKind::S_elseifs: // elseifs
      case SymbolKind::S_retstat: // retstat
      case SymbolKind::S_label: // label
      case SymbolKind::S_funcnamelist: // funcnamelist
      case SymbolKind::S_funcname: // funcname
      case SymbolKind::S_varlist: // varlist
      case SymbolKind::S_var: // var
      case SymbolKind::S_namelist: // namelist
      case SymbolKind::S_explist: // explist
      case SymbolKind::S_exp: // exp
      case SymbolKind::S_prefixexp: // prefixexp
      case SymbolKind::S_functioncall: // functioncall
      case SymbolKind::S_args: // args
      case SymbolKind::S_functiondef: // functiondef
      case SymbolKind::S_funcbody: // funcbody
      case SymbolKind::S_parlist: // parlist
      case SymbolKind::S_tableconstructor: // tableconstructor
      case SymbolKind::S_fieldlist: // fieldlist
      case SymbolKind::S_field: // field
        value.move< fakelua::SyntaxTreeInterfacePtr > (YY_MOVE (s.value));
        break;

      case SymbolKind::S_IDENTIFIER: // "identifier"
      case SymbolKind::S_STRING: // "string"
      case SymbolKind::S_NUMBER: // "number"
        value.move< std::string > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

    location = YY_MOVE (s.location);
  }

  // ByKind.
  inline
  parser::ByKind::ByKind ()
    : kind_ (SymbolKind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  inline
  parser::ByKind::ByKind (ByKind&& that)
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  inline
  parser::ByKind::ByKind (const ByKind& that)
    : kind_ (that.kind_)
  {}

  inline
  parser::ByKind::ByKind (TokenKindType t)
    : kind_ (yytranslate_ (t))
  {}

  inline
  void
  parser::ByKind::clear () YY_NOEXCEPT
  {
    kind_ = SymbolKind::S_YYEMPTY;
  }

  inline
  void
  parser::ByKind::move (ByKind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  inline
  parser::SymbolKindType
  parser::ByKind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }

  inline
  parser::SymbolKindType
  parser::ByKind::TypeGet () const YY_NOEXCEPT
  {
    return this->kind ();
  }

} // yy
#line 2454 "parser.h"




#endif // !YY_YY_PARSER_H_INCLUDED
