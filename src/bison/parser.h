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

#include "glog/logging.h"
#include "util/common.h"
#include "compile/syntax_tree.h"

namespace fakelua {
    class myflexer;
}

// https://www.gnu.org/software/bison/manual/html_node/A-Simple-C_002b_002b-Example.html


#line 62 "parser.h"

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
#line 202 "parser.h"




  /// A Bison parser.
  class parser
  {
  public:
#ifdef YYSTYPE
# ifdef __GNUC__
#  pragma GCC message "bison: do not #define YYSTYPE in C++, use %define api.value.type"
# endif
    typedef YYSTYPE value_type;
#else
  /// A buffer to store and retrieve objects.
  ///
  /// Sort of a variant, but does not keep track of the nature
  /// of the stored data, since that knowledge is available
  /// via the current parser state.
  class value_type
  {
  public:
    /// Type of *this.
    typedef value_type self_type;

    /// Empty construction.
    value_type () YY_NOEXCEPT
      : yyraw_ ()
      , yytypeid_ (YY_NULLPTR)
    {}

    /// Construct and fill.
    template <typename T>
    value_type (YY_RVREF (T) t)
      : yytypeid_ (&typeid (T))
    {
      YY_ASSERT (sizeof (T) <= size);
      new (yyas_<T> ()) T (YY_MOVE (t));
    }

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    value_type (const self_type&) = delete;
    /// Non copyable.
    self_type& operator= (const self_type&) = delete;
#endif

    /// Destruction, allowed only if empty.
    ~value_type () YY_NOEXCEPT
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
    value_type (const self_type&);
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
      // "number"
      char dummy1[sizeof (double)];

      // chunk
      // block
      // stmt
      // retstat
      // label
      // varlist
      // var
      // explist
      char dummy2[sizeof (fakelua::syntax_tree_interface_ptr)];

      // "identifier"
      // "string"
      char dummy3[sizeof (std::string)];
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
    typedef value_type semantic_type;
    /// Symbol locations.
    typedef location location_type;

    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (const location_type& l, const std::string& m)
        : std::runtime_error (m)
        , location (l)
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())
        , location (s.location)
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;

      location_type location;
    };

    /// Token kinds.
    struct token
    {
      enum token_kind_type
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
    TOK_FALSE = 20,                // "false"
    TOK_FOR = 21,                  // "for"
    TOK_FUNCTION = 22,             // "function"
    TOK_goto = 23,                 // "goto"
    TOK_IF = 24,                   // "if"
    TOK_IN = 25,                   // "in"
    TOK_LOCAL = 26,                // "local"
    TOK_NIL = 27,                  // "nil"
    TOK_NOT = 28,                  // "not"
    TOK_OR = 29,                   // "or"
    TOK_REPEAT = 30,               // "repeat"
    TOK_RETURN = 31,               // "return"
    TOK_THEN = 32,                 // "then"
    TOK_TRUE = 33,                 // "true"
    TOK_UNTIL = 34,                // "until"
    TOK_WHILE = 35,                // "while"
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
    TOK_XOR = 50,                  // "^"
    TOK_MOD = 51,                  // "%"
    TOK_BITAND = 52,               // "&"
    TOK_BITOR = 53,                // "|"
    TOK_BITNOT = 54,               // "~"
    TOK_MORE = 55,                 // ">"
    TOK_LESS = 56,                 // "<"
    TOK_NUMBER_SIGN = 57,          // "#"
    TOK_IDENTIFIER = 58,           // "identifier"
    TOK_STRING = 59,               // "string"
    TOK_NUMBER = 60                // "number"
      };
      /// Backward compatibility alias (Bison 3.6).
      typedef token_kind_type yytokentype;
    };

    /// Token kind, as returned by yylex.
    typedef token::yytokentype token_kind_type;

    /// Backward compatibility alias (Bison 3.6).
    typedef token_kind_type token_type;

    /// Symbol kinds.
    struct symbol_kind
    {
      enum symbol_kind_type
      {
        YYNTOKENS = 61, ///< Number of tokens.
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
        S_FALSE = 20,                            // "false"
        S_FOR = 21,                              // "for"
        S_FUNCTION = 22,                         // "function"
        S_goto = 23,                             // "goto"
        S_IF = 24,                               // "if"
        S_IN = 25,                               // "in"
        S_LOCAL = 26,                            // "local"
        S_NIL = 27,                              // "nil"
        S_NOT = 28,                              // "not"
        S_OR = 29,                               // "or"
        S_REPEAT = 30,                           // "repeat"
        S_RETURN = 31,                           // "return"
        S_THEN = 32,                             // "then"
        S_TRUE = 33,                             // "true"
        S_UNTIL = 34,                            // "until"
        S_WHILE = 35,                            // "while"
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
        S_XOR = 50,                              // "^"
        S_MOD = 51,                              // "%"
        S_BITAND = 52,                           // "&"
        S_BITOR = 53,                            // "|"
        S_BITNOT = 54,                           // "~"
        S_MORE = 55,                             // ">"
        S_LESS = 56,                             // "<"
        S_NUMBER_SIGN = 57,                      // "#"
        S_IDENTIFIER = 58,                       // "identifier"
        S_STRING = 59,                           // "string"
        S_NUMBER = 60,                           // "number"
        S_YYACCEPT = 61,                         // $accept
        S_chunk = 62,                            // chunk
        S_block = 63,                            // block
        S_stmt = 64,                             // stmt
        S_retstat = 65,                          // retstat
        S_label = 66,                            // label
        S_varlist = 67,                          // varlist
        S_var = 68,                              // var
        S_namelist = 69,                         // namelist
        S_explist = 70,                          // explist
        S_exp = 71,                              // exp
        S_prefixexp = 72,                        // prefixexp
        S_functioncall = 73,                     // functioncall
        S_args = 74,                             // args
        S_functiondef = 75,                      // functiondef
        S_funcbody = 76,                         // funcbody
        S_parlist = 77,                          // parlist
        S_tableconstructor = 78,                 // tableconstructor
        S_fieldlist = 79,                        // fieldlist
        S_field = 80,                            // field
        S_fieldsep = 81,                         // fieldsep
        S_binop = 82,                            // binop
        S_unop = 83                              // unop
      };
    };

    /// (Internal) symbol kind.
    typedef symbol_kind::symbol_kind_type symbol_kind_type;

    /// The number of tokens.
    static const symbol_kind_type YYNTOKENS = symbol_kind::YYNTOKENS;

    /// A complete symbol.
    ///
    /// Expects its Base type to provide access to the symbol kind
    /// via kind ().
    ///
    /// Provide access to semantic value and location.
    template <typename Base>
    struct basic_symbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      basic_symbol ()
        : value ()
        , location ()
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value ()
        , location (std::move (that.location))
      {
        switch (this->kind ())
    {
      case symbol_kind::S_NUMBER: // "number"
        value.move< double > (std::move (that.value));
        break;

      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_retstat: // retstat
      case symbol_kind::S_label: // label
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_explist: // explist
        value.move< fakelua::syntax_tree_interface_ptr > (std::move (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
        value.move< std::string > (std::move (that.value));
        break;

      default:
        break;
    }

      }
#endif

      /// Copy constructor.
      basic_symbol (const basic_symbol& that);

      /// Constructors for typed symbols.
#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, location_type&& l)
        : Base (t)
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const location_type& l)
        : Base (t)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, double&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const double& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, fakelua::syntax_tree_interface_ptr&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const fakelua::syntax_tree_interface_ptr& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::string&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::string& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

      /// Destroy the symbol.
      ~basic_symbol ()
      {
        clear ();
      }

      /// Destroy contents, and record that is empty.
      void clear () YY_NOEXCEPT
      {
        // User destructor.
        symbol_kind_type yykind = this->kind ();
        basic_symbol<Base>& yysym = *this;
        (void) yysym;
        switch (yykind)
        {
       default:
          break;
        }

        // Value type destructor.
switch (yykind)
    {
      case symbol_kind::S_NUMBER: // "number"
        value.template destroy< double > ();
        break;

      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_retstat: // retstat
      case symbol_kind::S_label: // label
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_explist: // explist
        value.template destroy< fakelua::syntax_tree_interface_ptr > ();
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
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
        return parser::symbol_name (this->kind ());
      }

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// Whether empty.
      bool empty () const YY_NOEXCEPT;

      /// Destructive move, \a s is emptied into this.
      void move (basic_symbol& s);

      /// The semantic value.
      value_type value;

      /// The location.
      location_type location;

    private:
#if YY_CPLUSPLUS < 201103L
      /// Assignment operator.
      basic_symbol& operator= (const basic_symbol& that);
#endif
    };

    /// Type access provider for token (enum) based symbols.
    struct by_kind
    {
      /// Default constructor.
      by_kind ();

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      by_kind (by_kind&& that);
#endif

      /// Copy constructor.
      by_kind (const by_kind& that);

      /// The symbol kind as needed by the constructor.
      typedef token_kind_type kind_type;

      /// Constructor from (external) token numbers.
      by_kind (kind_type t);

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_kind& that);

      /// The (internal) type number (corresponding to \a type).
      /// \a empty when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// The symbol kind.
      /// \a S_YYEMPTY when empty.
      symbol_kind_type kind_;
    };

    /// Backward compatibility for a private implementation detail (Bison 3.6).
    typedef by_kind by_type;

    /// "External" symbols: returned by the scanner.
    struct symbol_type : basic_symbol<by_kind>
    {
      /// Superclass.
      typedef basic_symbol<by_kind> super_type;

      /// Empty symbol.
      symbol_type () {}

      /// Constructor for valueless symbols, and symbols from each type.
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, location_type l)
        : super_type (token_type (tok), std::move (l))
#else
      symbol_type (int tok, const location_type& l)
        : super_type (token_type (tok), l)
#endif
      {
#if !defined _MSC_VER || defined __clang__
        YY_ASSERT (tok == token::TOK_YYEOF
                   || (token::TOK_YYerror <= tok && tok <= token::TOK_NUMBER_SIGN));
#endif
      }
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, double v, location_type l)
        : super_type (token_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const double& v, const location_type& l)
        : super_type (token_type (tok), v, l)
#endif
      {
#if !defined _MSC_VER || defined __clang__
        YY_ASSERT (tok == token::TOK_NUMBER);
#endif
      }
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, std::string v, location_type l)
        : super_type (token_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const std::string& v, const location_type& l)
        : super_type (token_type (tok), v, l)
#endif
      {
#if !defined _MSC_VER || defined __clang__
        YY_ASSERT ((token::TOK_IDENTIFIER <= tok && tok <= token::TOK_STRING));
#endif
      }
    };

    /// Build a parser object.
    parser (fakelua::myflexer* l_yyarg);
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
    std::ostream& debug_stream () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.
    /// \param loc    where the syntax error is found.
    /// \param msg    a description of the syntax error.
    virtual void error (const location_type& loc, const std::string& msg);

    /// Report a syntax error.
    void error (const syntax_error& err);

    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static const char *symbol_name (symbol_kind_type yysymbol);

    // Implementation of make_symbol for each token kind.
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYEOF (location_type l)
      {
        return symbol_type (token::TOK_YYEOF, std::move (l));
      }
#else
      static
      symbol_type
      make_YYEOF (const location_type& l)
      {
        return symbol_type (token::TOK_YYEOF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYerror (location_type l)
      {
        return symbol_type (token::TOK_YYerror, std::move (l));
      }
#else
      static
      symbol_type
      make_YYerror (const location_type& l)
      {
        return symbol_type (token::TOK_YYerror, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYUNDEF (location_type l)
      {
        return symbol_type (token::TOK_YYUNDEF, std::move (l));
      }
#else
      static
      symbol_type
      make_YYUNDEF (const location_type& l)
      {
        return symbol_type (token::TOK_YYUNDEF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ASSIGN (location_type l)
      {
        return symbol_type (token::TOK_ASSIGN, std::move (l));
      }
#else
      static
      symbol_type
      make_ASSIGN (const location_type& l)
      {
        return symbol_type (token::TOK_ASSIGN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MINUS (location_type l)
      {
        return symbol_type (token::TOK_MINUS, std::move (l));
      }
#else
      static
      symbol_type
      make_MINUS (const location_type& l)
      {
        return symbol_type (token::TOK_MINUS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PLUS (location_type l)
      {
        return symbol_type (token::TOK_PLUS, std::move (l));
      }
#else
      static
      symbol_type
      make_PLUS (const location_type& l)
      {
        return symbol_type (token::TOK_PLUS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STAR (location_type l)
      {
        return symbol_type (token::TOK_STAR, std::move (l));
      }
#else
      static
      symbol_type
      make_STAR (const location_type& l)
      {
        return symbol_type (token::TOK_STAR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SLASH (location_type l)
      {
        return symbol_type (token::TOK_SLASH, std::move (l));
      }
#else
      static
      symbol_type
      make_SLASH (const location_type& l)
      {
        return symbol_type (token::TOK_SLASH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LPAREN (location_type l)
      {
        return symbol_type (token::TOK_LPAREN, std::move (l));
      }
#else
      static
      symbol_type
      make_LPAREN (const location_type& l)
      {
        return symbol_type (token::TOK_LPAREN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RPAREN (location_type l)
      {
        return symbol_type (token::TOK_RPAREN, std::move (l));
      }
#else
      static
      symbol_type
      make_RPAREN (const location_type& l)
      {
        return symbol_type (token::TOK_RPAREN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LCURLY (location_type l)
      {
        return symbol_type (token::TOK_LCURLY, std::move (l));
      }
#else
      static
      symbol_type
      make_LCURLY (const location_type& l)
      {
        return symbol_type (token::TOK_LCURLY, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RCURLY (location_type l)
      {
        return symbol_type (token::TOK_RCURLY, std::move (l));
      }
#else
      static
      symbol_type
      make_RCURLY (const location_type& l)
      {
        return symbol_type (token::TOK_RCURLY, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LSQUARE (location_type l)
      {
        return symbol_type (token::TOK_LSQUARE, std::move (l));
      }
#else
      static
      symbol_type
      make_LSQUARE (const location_type& l)
      {
        return symbol_type (token::TOK_LSQUARE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RSQUARE (location_type l)
      {
        return symbol_type (token::TOK_RSQUARE, std::move (l));
      }
#else
      static
      symbol_type
      make_RSQUARE (const location_type& l)
      {
        return symbol_type (token::TOK_RSQUARE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_AND (location_type l)
      {
        return symbol_type (token::TOK_AND, std::move (l));
      }
#else
      static
      symbol_type
      make_AND (const location_type& l)
      {
        return symbol_type (token::TOK_AND, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_BREAK (location_type l)
      {
        return symbol_type (token::TOK_BREAK, std::move (l));
      }
#else
      static
      symbol_type
      make_BREAK (const location_type& l)
      {
        return symbol_type (token::TOK_BREAK, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DO (location_type l)
      {
        return symbol_type (token::TOK_DO, std::move (l));
      }
#else
      static
      symbol_type
      make_DO (const location_type& l)
      {
        return symbol_type (token::TOK_DO, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ELSE (location_type l)
      {
        return symbol_type (token::TOK_ELSE, std::move (l));
      }
#else
      static
      symbol_type
      make_ELSE (const location_type& l)
      {
        return symbol_type (token::TOK_ELSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ELSEIF (location_type l)
      {
        return symbol_type (token::TOK_ELSEIF, std::move (l));
      }
#else
      static
      symbol_type
      make_ELSEIF (const location_type& l)
      {
        return symbol_type (token::TOK_ELSEIF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_END (location_type l)
      {
        return symbol_type (token::TOK_END, std::move (l));
      }
#else
      static
      symbol_type
      make_END (const location_type& l)
      {
        return symbol_type (token::TOK_END, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_FALSE (location_type l)
      {
        return symbol_type (token::TOK_FALSE, std::move (l));
      }
#else
      static
      symbol_type
      make_FALSE (const location_type& l)
      {
        return symbol_type (token::TOK_FALSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_FOR (location_type l)
      {
        return symbol_type (token::TOK_FOR, std::move (l));
      }
#else
      static
      symbol_type
      make_FOR (const location_type& l)
      {
        return symbol_type (token::TOK_FOR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_FUNCTION (location_type l)
      {
        return symbol_type (token::TOK_FUNCTION, std::move (l));
      }
#else
      static
      symbol_type
      make_FUNCTION (const location_type& l)
      {
        return symbol_type (token::TOK_FUNCTION, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_goto (location_type l)
      {
        return symbol_type (token::TOK_goto, std::move (l));
      }
#else
      static
      symbol_type
      make_goto (const location_type& l)
      {
        return symbol_type (token::TOK_goto, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IF (location_type l)
      {
        return symbol_type (token::TOK_IF, std::move (l));
      }
#else
      static
      symbol_type
      make_IF (const location_type& l)
      {
        return symbol_type (token::TOK_IF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IN (location_type l)
      {
        return symbol_type (token::TOK_IN, std::move (l));
      }
#else
      static
      symbol_type
      make_IN (const location_type& l)
      {
        return symbol_type (token::TOK_IN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LOCAL (location_type l)
      {
        return symbol_type (token::TOK_LOCAL, std::move (l));
      }
#else
      static
      symbol_type
      make_LOCAL (const location_type& l)
      {
        return symbol_type (token::TOK_LOCAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NIL (location_type l)
      {
        return symbol_type (token::TOK_NIL, std::move (l));
      }
#else
      static
      symbol_type
      make_NIL (const location_type& l)
      {
        return symbol_type (token::TOK_NIL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NOT (location_type l)
      {
        return symbol_type (token::TOK_NOT, std::move (l));
      }
#else
      static
      symbol_type
      make_NOT (const location_type& l)
      {
        return symbol_type (token::TOK_NOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_OR (location_type l)
      {
        return symbol_type (token::TOK_OR, std::move (l));
      }
#else
      static
      symbol_type
      make_OR (const location_type& l)
      {
        return symbol_type (token::TOK_OR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_REPEAT (location_type l)
      {
        return symbol_type (token::TOK_REPEAT, std::move (l));
      }
#else
      static
      symbol_type
      make_REPEAT (const location_type& l)
      {
        return symbol_type (token::TOK_REPEAT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RETURN (location_type l)
      {
        return symbol_type (token::TOK_RETURN, std::move (l));
      }
#else
      static
      symbol_type
      make_RETURN (const location_type& l)
      {
        return symbol_type (token::TOK_RETURN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_THEN (location_type l)
      {
        return symbol_type (token::TOK_THEN, std::move (l));
      }
#else
      static
      symbol_type
      make_THEN (const location_type& l)
      {
        return symbol_type (token::TOK_THEN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_TRUE (location_type l)
      {
        return symbol_type (token::TOK_TRUE, std::move (l));
      }
#else
      static
      symbol_type
      make_TRUE (const location_type& l)
      {
        return symbol_type (token::TOK_TRUE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_UNTIL (location_type l)
      {
        return symbol_type (token::TOK_UNTIL, std::move (l));
      }
#else
      static
      symbol_type
      make_UNTIL (const location_type& l)
      {
        return symbol_type (token::TOK_UNTIL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_WHILE (location_type l)
      {
        return symbol_type (token::TOK_WHILE, std::move (l));
      }
#else
      static
      symbol_type
      make_WHILE (const location_type& l)
      {
        return symbol_type (token::TOK_WHILE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DOUBLE_SLASH (location_type l)
      {
        return symbol_type (token::TOK_DOUBLE_SLASH, std::move (l));
      }
#else
      static
      symbol_type
      make_DOUBLE_SLASH (const location_type& l)
      {
        return symbol_type (token::TOK_DOUBLE_SLASH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_CONCAT (location_type l)
      {
        return symbol_type (token::TOK_CONCAT, std::move (l));
      }
#else
      static
      symbol_type
      make_CONCAT (const location_type& l)
      {
        return symbol_type (token::TOK_CONCAT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_VAR_PARAMS (location_type l)
      {
        return symbol_type (token::TOK_VAR_PARAMS, std::move (l));
      }
#else
      static
      symbol_type
      make_VAR_PARAMS (const location_type& l)
      {
        return symbol_type (token::TOK_VAR_PARAMS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_EQUAL (location_type l)
      {
        return symbol_type (token::TOK_EQUAL, std::move (l));
      }
#else
      static
      symbol_type
      make_EQUAL (const location_type& l)
      {
        return symbol_type (token::TOK_EQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MORE_EQUAL (location_type l)
      {
        return symbol_type (token::TOK_MORE_EQUAL, std::move (l));
      }
#else
      static
      symbol_type
      make_MORE_EQUAL (const location_type& l)
      {
        return symbol_type (token::TOK_MORE_EQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LESS_EQUAL (location_type l)
      {
        return symbol_type (token::TOK_LESS_EQUAL, std::move (l));
      }
#else
      static
      symbol_type
      make_LESS_EQUAL (const location_type& l)
      {
        return symbol_type (token::TOK_LESS_EQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NOT_EQUAL (location_type l)
      {
        return symbol_type (token::TOK_NOT_EQUAL, std::move (l));
      }
#else
      static
      symbol_type
      make_NOT_EQUAL (const location_type& l)
      {
        return symbol_type (token::TOK_NOT_EQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LEFT_SHIFT (location_type l)
      {
        return symbol_type (token::TOK_LEFT_SHIFT, std::move (l));
      }
#else
      static
      symbol_type
      make_LEFT_SHIFT (const location_type& l)
      {
        return symbol_type (token::TOK_LEFT_SHIFT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RIGHT_SHIFT (location_type l)
      {
        return symbol_type (token::TOK_RIGHT_SHIFT, std::move (l));
      }
#else
      static
      symbol_type
      make_RIGHT_SHIFT (const location_type& l)
      {
        return symbol_type (token::TOK_RIGHT_SHIFT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_GOTO_TAG (location_type l)
      {
        return symbol_type (token::TOK_GOTO_TAG, std::move (l));
      }
#else
      static
      symbol_type
      make_GOTO_TAG (const location_type& l)
      {
        return symbol_type (token::TOK_GOTO_TAG, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SEMICOLON (location_type l)
      {
        return symbol_type (token::TOK_SEMICOLON, std::move (l));
      }
#else
      static
      symbol_type
      make_SEMICOLON (const location_type& l)
      {
        return symbol_type (token::TOK_SEMICOLON, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_COLON (location_type l)
      {
        return symbol_type (token::TOK_COLON, std::move (l));
      }
#else
      static
      symbol_type
      make_COLON (const location_type& l)
      {
        return symbol_type (token::TOK_COLON, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_COMMA (location_type l)
      {
        return symbol_type (token::TOK_COMMA, std::move (l));
      }
#else
      static
      symbol_type
      make_COMMA (const location_type& l)
      {
        return symbol_type (token::TOK_COMMA, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DOT (location_type l)
      {
        return symbol_type (token::TOK_DOT, std::move (l));
      }
#else
      static
      symbol_type
      make_DOT (const location_type& l)
      {
        return symbol_type (token::TOK_DOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_XOR (location_type l)
      {
        return symbol_type (token::TOK_XOR, std::move (l));
      }
#else
      static
      symbol_type
      make_XOR (const location_type& l)
      {
        return symbol_type (token::TOK_XOR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MOD (location_type l)
      {
        return symbol_type (token::TOK_MOD, std::move (l));
      }
#else
      static
      symbol_type
      make_MOD (const location_type& l)
      {
        return symbol_type (token::TOK_MOD, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_BITAND (location_type l)
      {
        return symbol_type (token::TOK_BITAND, std::move (l));
      }
#else
      static
      symbol_type
      make_BITAND (const location_type& l)
      {
        return symbol_type (token::TOK_BITAND, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_BITOR (location_type l)
      {
        return symbol_type (token::TOK_BITOR, std::move (l));
      }
#else
      static
      symbol_type
      make_BITOR (const location_type& l)
      {
        return symbol_type (token::TOK_BITOR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_BITNOT (location_type l)
      {
        return symbol_type (token::TOK_BITNOT, std::move (l));
      }
#else
      static
      symbol_type
      make_BITNOT (const location_type& l)
      {
        return symbol_type (token::TOK_BITNOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MORE (location_type l)
      {
        return symbol_type (token::TOK_MORE, std::move (l));
      }
#else
      static
      symbol_type
      make_MORE (const location_type& l)
      {
        return symbol_type (token::TOK_MORE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LESS (location_type l)
      {
        return symbol_type (token::TOK_LESS, std::move (l));
      }
#else
      static
      symbol_type
      make_LESS (const location_type& l)
      {
        return symbol_type (token::TOK_LESS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NUMBER_SIGN (location_type l)
      {
        return symbol_type (token::TOK_NUMBER_SIGN, std::move (l));
      }
#else
      static
      symbol_type
      make_NUMBER_SIGN (const location_type& l)
      {
        return symbol_type (token::TOK_NUMBER_SIGN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IDENTIFIER (std::string v, location_type l)
      {
        return symbol_type (token::TOK_IDENTIFIER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_IDENTIFIER (const std::string& v, const location_type& l)
      {
        return symbol_type (token::TOK_IDENTIFIER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STRING (std::string v, location_type l)
      {
        return symbol_type (token::TOK_STRING, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_STRING (const std::string& v, const location_type& l)
      {
        return symbol_type (token::TOK_STRING, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NUMBER (double v, location_type l)
      {
        return symbol_type (token::TOK_NUMBER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_NUMBER (const double& v, const location_type& l)
      {
        return symbol_type (token::TOK_NUMBER, v, l);
      }
#endif


    class context
    {
    public:
      context (const parser& yyparser, const symbol_type& yyla);
      const symbol_type& lookahead () const YY_NOEXCEPT { return yyla_; }
      symbol_kind_type token () const YY_NOEXCEPT { return yyla_.kind (); }
      const location_type& location () const YY_NOEXCEPT { return yyla_.location; }

      /// Put in YYARG at most YYARGN of the expected tokens, and return the
      /// number of tokens stored in YYARG.  If YYARG is null, return the
      /// number of expected tokens (guaranteed to be less than YYNTOKENS).
      int expected_tokens (symbol_kind_type yyarg[], int yyargn) const;

    private:
      const parser& yyparser_;
      const symbol_type& yyla_;
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
    bool yy_lac_check_ (symbol_kind_type yytoken) const;
    /// Establish the initial context if no initial context currently exists.
    /// \returns  true iff the token will be eventually shifted.
    bool yy_lac_establish_ (symbol_kind_type yytoken);
    /// Discard any previous initial lookahead context because of event.
    /// \param event  the event which caused the lookahead to be discarded.
    ///               Only used for debbuging output.
    void yy_lac_discard_ (const char* event);

    /// Stored state numbers (used for stacks).
    typedef signed char state_type;

    /// The arguments of the error message.
    int yy_syntax_error_arguments_ (const context& yyctx,
                                    symbol_kind_type yyarg[], int yyargn) const;

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
    /// In theory \a t should be a token_kind_type, but character literals
    /// are valid, yet not members of the token_type enum.
    static symbol_kind_type yytranslate_ (int t);



    // Tables.
    // YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
    // STATE-NUM.
    static const short yypact_[];

    // YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
    // Performed when YYTABLE does not specify something else to do.  Zero
    // means the default is an error.
    static const signed char yydefact_[];

    // YYPGOTO[NTERM-NUM].
    static const signed char yypgoto_[];

    // YYDEFGOTO[NTERM-NUM].
    static const signed char yydefgoto_[];

    // YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
    // positive, shift that token.  If negative, reduce the rule whose
    // number is the opposite.  If YYTABLE_NINF, syntax error.
    static const signed char yytable_[];

    static const signed char yycheck_[];

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
    void yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const;
#endif

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param yysym     The symbol.
    template <typename Base>
    void yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const;

  private:
    /// Type access provider for state based symbols.
    struct by_state
    {
      /// Default constructor.
      by_state () YY_NOEXCEPT;

      /// The symbol kind as needed by the constructor.
      typedef state_type kind_type;

      /// Constructor.
      by_state (kind_type s) YY_NOEXCEPT;

      /// Copy constructor.
      by_state (const by_state& that) YY_NOEXCEPT;

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_state& that);

      /// The symbol kind (corresponding to \a state).
      /// \a symbol_kind::S_YYEMPTY when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// The state number used to denote an empty symbol.
      /// We use the initial state, as it does not have a value.
      enum { empty_state = 0 };

      /// The state.
      /// \a empty when empty.
      state_type state;
    };

    /// "Internal" symbol: element of the stack.
    struct stack_symbol_type : basic_symbol<by_state>
    {
      /// Superclass.
      typedef basic_symbol<by_state> super_type;
      /// Construct an empty symbol.
      stack_symbol_type ();
      /// Move or copy construction.
      stack_symbol_type (YY_RVREF (stack_symbol_type) that);
      /// Steal the contents from \a sym to build this.
      stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) sym);
#if YY_CPLUSPLUS < 201103L
      /// Assignment, needed by push_back by some old implementations.
      /// Moves the contents of that.
      stack_symbol_type& operator= (stack_symbol_type& that);

      /// Assignment, needed by push_back by other implementations.
      /// Needed by some other old implementations.
      stack_symbol_type& operator= (const stack_symbol_type& that);
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
      typedef typename std::ptrdiff_t index_type;

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
      operator[] (index_type i) const
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      T&
      operator[] (index_type i)
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
      index_type
      size () const YY_NOEXCEPT
      {
        return index_type (seq_.size ());
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
        slice (const stack& stack, index_type range)
          : stack_ (stack)
          , range_ (range)
        {}

        const T&
        operator[] (index_type i) const
        {
          return stack_[range_ - i];
        }

      private:
        const stack& stack_;
        index_type range_;
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
    typedef stack<stack_symbol_type> stack_type;

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
    void yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym);

    /// Push a new look ahead token on the state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the state
    /// \param sym  the symbol (for its value and location).
    /// \warning the contents of \a sym.value is stolen.
    void yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym);

    /// Pop \a n symbols from the stack.
    void yypop_ (int n = 1);

    /// Constants.
    enum
    {
      yylast_ = 428,     ///< Last index in yytable_.
      yynnts_ = 23,  ///< Number of nonterminal symbols.
      yyfinal_ = 36 ///< Termination state number.
    };


    // User arguments.
    fakelua::myflexer* l;

  };

  inline
  parser::symbol_kind_type
  parser::yytranslate_ (int t)
  {
    return static_cast<symbol_kind_type> (t);
  }

  // basic_symbol.
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value ()
    , location (that.location)
  {
    switch (this->kind ())
    {
      case symbol_kind::S_NUMBER: // "number"
        value.copy< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_retstat: // retstat
      case symbol_kind::S_label: // label
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_explist: // explist
        value.copy< fakelua::syntax_tree_interface_ptr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
        value.copy< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

  }



  template <typename Base>
  parser::symbol_kind_type
  parser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }

  template <typename Base>
  bool
  parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    switch (this->kind ())
    {
      case symbol_kind::S_NUMBER: // "number"
        value.move< double > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_retstat: // retstat
      case symbol_kind::S_label: // label
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_explist: // explist
        value.move< fakelua::syntax_tree_interface_ptr > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
        value.move< std::string > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

    location = YY_MOVE (s.location);
  }

  // by_kind.
  inline
  parser::by_kind::by_kind ()
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  inline
  parser::by_kind::by_kind (by_kind&& that)
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  inline
  parser::by_kind::by_kind (const by_kind& that)
    : kind_ (that.kind_)
  {}

  inline
  parser::by_kind::by_kind (token_kind_type t)
    : kind_ (yytranslate_ (t))
  {}

  inline
  void
  parser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  inline
  void
  parser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  inline
  parser::symbol_kind_type
  parser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }

  inline
  parser::symbol_kind_type
  parser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }

} // yy
#line 2399 "parser.h"




#endif // !YY_YY_PARSER_H_INCLUDED
