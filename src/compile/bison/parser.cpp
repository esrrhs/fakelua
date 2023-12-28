// A Bison parser, made by GNU Bison 3.8.

// Skeleton implementation for Bison LALR(1) parsers in C++

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

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "parser.h"


// Unqualified %code blocks.
#line 34 "parser.y"

#include "compile/myflexer.h"

yy::parser::symbol_type yylex(fakelua::myflexer* l) {
    auto ret = l->my_yylex();
    std::stringstream ss;
    ss << ret.location;
    LOG_INFO("[bison]: bison get token: {} loc: {}", ret.name(), ss.str());
    return ret;
}

int yyFlexLexer::yylex() { return -1; }


#line 61 "parser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 153 "parser.cpp"

  /// Build a parser object.
  parser::parser (fakelua::myflexer* l_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      yy_lac_established_ (false),
      l (l_yyarg)
  {}

  parser::~parser ()
  {}

  parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | symbol kinds.  |
  `---------------*/



  // by_state.
  parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser::symbol_kind_type
  parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_attnamelist: // attnamelist
      case symbol_kind::S_elseifs: // elseifs
      case symbol_kind::S_retstat: // retstat
      case symbol_kind::S_label: // label
      case symbol_kind::S_funcnamelist: // funcnamelist
      case symbol_kind::S_funcname: // funcname
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
      case symbol_kind::S_functiondef: // functiondef
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
      case symbol_kind::S_binop: // binop
      case symbol_kind::S_unop: // unop
        value.YY_MOVE_OR_COPY< fakelua::syntax_tree_interface_ptr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_NUMBER: // "number"
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_attnamelist: // attnamelist
      case symbol_kind::S_elseifs: // elseifs
      case symbol_kind::S_retstat: // retstat
      case symbol_kind::S_label: // label
      case symbol_kind::S_funcnamelist: // funcnamelist
      case symbol_kind::S_funcname: // funcname
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
      case symbol_kind::S_functiondef: // functiondef
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
      case symbol_kind::S_binop: // binop
      case symbol_kind::S_unop: // unop
        value.move< fakelua::syntax_tree_interface_ptr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_NUMBER: // "number"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_attnamelist: // attnamelist
      case symbol_kind::S_elseifs: // elseifs
      case symbol_kind::S_retstat: // retstat
      case symbol_kind::S_label: // label
      case symbol_kind::S_funcnamelist: // funcnamelist
      case symbol_kind::S_funcname: // funcname
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
      case symbol_kind::S_functiondef: // functiondef
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
      case symbol_kind::S_binop: // binop
      case symbol_kind::S_unop: // unop
        value.copy< fakelua::syntax_tree_interface_ptr > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_NUMBER: // "number"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_attnamelist: // attnamelist
      case symbol_kind::S_elseifs: // elseifs
      case symbol_kind::S_retstat: // retstat
      case symbol_kind::S_label: // label
      case symbol_kind::S_funcnamelist: // funcnamelist
      case symbol_kind::S_funcname: // funcname
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
      case symbol_kind::S_functiondef: // functiondef
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
      case symbol_kind::S_binop: // binop
      case symbol_kind::S_unop: // unop
        value.move< fakelua::syntax_tree_interface_ptr > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_NUMBER: // "number"
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        switch (yykind)
    {
      case symbol_kind::S_IDENTIFIER: // "identifier"
#line 138 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 441 "parser.cpp"
        break;

      case symbol_kind::S_STRING: // "string"
#line 138 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 447 "parser.cpp"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 138 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 453 "parser.cpp"
        break;

      case symbol_kind::S_chunk: // chunk
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 459 "parser.cpp"
        break;

      case symbol_kind::S_block: // block
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 465 "parser.cpp"
        break;

      case symbol_kind::S_stmt: // stmt
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 471 "parser.cpp"
        break;

      case symbol_kind::S_attnamelist: // attnamelist
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 477 "parser.cpp"
        break;

      case symbol_kind::S_elseifs: // elseifs
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 483 "parser.cpp"
        break;

      case symbol_kind::S_retstat: // retstat
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 489 "parser.cpp"
        break;

      case symbol_kind::S_label: // label
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 495 "parser.cpp"
        break;

      case symbol_kind::S_funcnamelist: // funcnamelist
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 501 "parser.cpp"
        break;

      case symbol_kind::S_funcname: // funcname
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 507 "parser.cpp"
        break;

      case symbol_kind::S_varlist: // varlist
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 513 "parser.cpp"
        break;

      case symbol_kind::S_var: // var
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 519 "parser.cpp"
        break;

      case symbol_kind::S_namelist: // namelist
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 525 "parser.cpp"
        break;

      case symbol_kind::S_explist: // explist
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 531 "parser.cpp"
        break;

      case symbol_kind::S_exp: // exp
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 537 "parser.cpp"
        break;

      case symbol_kind::S_prefixexp: // prefixexp
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 543 "parser.cpp"
        break;

      case symbol_kind::S_functioncall: // functioncall
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 549 "parser.cpp"
        break;

      case symbol_kind::S_args: // args
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 555 "parser.cpp"
        break;

      case symbol_kind::S_functiondef: // functiondef
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 561 "parser.cpp"
        break;

      case symbol_kind::S_funcbody: // funcbody
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 567 "parser.cpp"
        break;

      case symbol_kind::S_parlist: // parlist
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 573 "parser.cpp"
        break;

      case symbol_kind::S_tableconstructor: // tableconstructor
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 579 "parser.cpp"
        break;

      case symbol_kind::S_fieldlist: // fieldlist
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 585 "parser.cpp"
        break;

      case symbol_kind::S_field: // field
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 591 "parser.cpp"
        break;

      case symbol_kind::S_binop: // binop
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 597 "parser.cpp"
        break;

      case symbol_kind::S_unop: // unop
#line 138 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 603 "parser.cpp"
        break;

      default:
        break;
    }
        yyo << ')';
      }
  }
#endif

  void
  parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::operator() ()
  {
    return parse ();
  }

  int
  parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // Discard the LAC context in case there still is one left from a
    // previous invocation.
    yy_lac_discard_ ("init");

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (l));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;

        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    yy_lac_discard_ ("shift");
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_attnamelist: // attnamelist
      case symbol_kind::S_elseifs: // elseifs
      case symbol_kind::S_retstat: // retstat
      case symbol_kind::S_label: // label
      case symbol_kind::S_funcnamelist: // funcnamelist
      case symbol_kind::S_funcname: // funcname
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
      case symbol_kind::S_functiondef: // functiondef
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
      case symbol_kind::S_binop: // binop
      case symbol_kind::S_unop: // unop
        yylhs.value.emplace< fakelua::syntax_tree_interface_ptr > ();
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_NUMBER: // "number"
        yylhs.value.emplace< std::string > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // chunk: block
#line 145 "parser.y"
    {
    LOG_INFO("[bison]: chunk: block");
    l->set_chunk(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
    }
#line 903 "parser.cpp"
    break;

  case 3: // block: %empty
#line 153 "parser.y"
    {
        LOG_INFO("[bison]: block: empty");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_block>(yystack_[0].location);
    }
#line 912 "parser.cpp"
    break;

  case 4: // block: stmt
#line 159 "parser.y"
    {
        LOG_INFO("[bison]: block: stmt");
        auto block = std::make_shared<fakelua::syntax_tree_block>(yystack_[0].location);
        auto stmt = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (stmt == nullptr) {
            LOG_ERROR("[bison]: block: stmt is nullptr");
        }
        block->add_stmt(stmt);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = block;
    }
#line 927 "parser.cpp"
    break;

  case 5: // block: block stmt
#line 171 "parser.y"
    {
        LOG_INFO("[bison]: block: block stmt");
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: block: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        auto stmt = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (stmt == nullptr) {
            LOG_ERROR("[bison]: block: stmt is not a stmt");
            fakelua::throw_fakelua_exception("stmt is not a stmt");
        }
        block->add_stmt(stmt);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = block;
    }
#line 947 "parser.cpp"
    break;

  case 6: // stmt: retstat
#line 190 "parser.y"
    {
        LOG_INFO("[bison]: stmt: retstat");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 956 "parser.cpp"
    break;

  case 7: // stmt: ";"
#line 196 "parser.y"
    {
        LOG_INFO("[bison]: stmt: SEMICOLON");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_empty>(yystack_[0].location);
    }
#line 965 "parser.cpp"
    break;

  case 8: // stmt: varlist "=" explist
#line 202 "parser.y"
    {
        LOG_INFO("[bison]: stmt: varlist ASSIGN explist");
        auto varlist = std::dynamic_pointer_cast<fakelua::syntax_tree_varlist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (varlist == nullptr) {
            LOG_ERROR("[bison]: stmt: varlist is not a varlist");
            fakelua::throw_fakelua_exception("varlist is not a varlist");
        }
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        auto assign = std::make_shared<fakelua::syntax_tree_assign>(yystack_[1].location);
        assign->set_varlist(varlist);
        assign->set_explist(explist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = assign;
    }
#line 987 "parser.cpp"
    break;

  case 9: // stmt: functioncall
#line 221 "parser.y"
    {
        LOG_INFO("[bison]: stmt: functioncall");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 996 "parser.cpp"
    break;

  case 10: // stmt: label
#line 227 "parser.y"
    {
        LOG_INFO("[bison]: stmt: label");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 1005 "parser.cpp"
    break;

  case 11: // stmt: "break"
#line 233 "parser.y"
    {
        LOG_INFO("[bison]: stmt: BREAK");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_break>(yystack_[0].location);
    }
#line 1014 "parser.cpp"
    break;

  case 12: // stmt: "goto" "identifier"
#line 239 "parser.y"
    {
        LOG_INFO("[bison]: stmt: GOTO IDENTIFIER");
        auto go = std::make_shared<fakelua::syntax_tree_goto>(yystack_[0].location);
        go->set_label(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = go;
    }
#line 1025 "parser.cpp"
    break;

  case 13: // stmt: "do" block "end"
#line 247 "parser.y"
    {
        LOG_INFO("[bison]: stmt: DO block END");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 1034 "parser.cpp"
    break;

  case 14: // stmt: "while" exp "do" block "end"
#line 253 "parser.y"
    {
        LOG_INFO("[bison]: stmt: WHILE exp DO block END");
        auto while_stmt = std::make_shared<fakelua::syntax_tree_while>(yystack_[4].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        while_stmt->set_exp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        while_stmt->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = while_stmt;
    }
#line 1056 "parser.cpp"
    break;

  case 15: // stmt: "repeat" block "until" exp
#line 272 "parser.y"
    {
        LOG_INFO("[bison]: stmt: REPEAT block UNTIL exp");
        auto repeat = std::make_shared<fakelua::syntax_tree_repeat>(yystack_[3].location);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        repeat->set_block(block);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        repeat->set_exp(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = repeat;
    }
#line 1078 "parser.cpp"
    break;

  case 16: // stmt: "if" exp "then" block elseifs "else" block "end"
#line 291 "parser.y"
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs ELSE block END");
        auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(yystack_[7].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[6].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        if_stmt->set_exp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[4].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        if_stmt->set_block(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::throw_fakelua_exception("elseiflist is not a elseiflist");
        }
        if_stmt->set_elseiflist(elseifs);
        auto else_block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (else_block == nullptr) {
            LOG_ERROR("[bison]: stmt: else_block is not a block");
            fakelua::throw_fakelua_exception("else_block is not a block");
        }
        if_stmt->set_else_block(else_block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = if_stmt;
    }
#line 1112 "parser.cpp"
    break;

  case 17: // stmt: "if" exp "then" block elseifs "end"
#line 322 "parser.y"
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs END");
        auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(yystack_[5].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[4].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        if_stmt->set_exp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        if_stmt->set_block(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::throw_fakelua_exception("elseiflist is not a elseiflist");
        }
        if_stmt->set_elseiflist(elseifs);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = if_stmt;
    }
#line 1140 "parser.cpp"
    break;

  case 18: // stmt: "for" "identifier" "=" exp "," exp "do" block "end"
#line 347 "parser.y"
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(yystack_[8].location);
        for_loop_stmt->set_name(yystack_[7].value.as < std::string > ());
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        for_loop_stmt->set_exp_begin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::throw_fakelua_exception("end_exp is not a exp");
        }
        for_loop_stmt->set_exp_end(end_exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        for_loop_stmt->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_loop_stmt;
    }
#line 1169 "parser.cpp"
    break;

  case 19: // stmt: "for" "identifier" "=" exp "," exp "," exp "do" block "end"
#line 373 "parser.y"
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(yystack_[10].location);
        for_loop_stmt->set_name(yystack_[9].value.as < std::string > ());
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[7].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        for_loop_stmt->set_exp_begin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::throw_fakelua_exception("end_exp is not a exp");
        }
        for_loop_stmt->set_exp_end(end_exp);
        auto step_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (step_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: step_exp is not a exp");
            fakelua::throw_fakelua_exception("step_exp is not a exp");
        }
        for_loop_stmt->set_exp_step(step_exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        for_loop_stmt->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_loop_stmt;
    }
#line 1204 "parser.cpp"
    break;

  case 20: // stmt: "for" namelist "in" explist "do" block "end"
#line 405 "parser.y"
    {
        LOG_INFO("[bison]: stmt: for namelist in explist do block end");
        auto for_in_stmt = std::make_shared<fakelua::syntax_tree_for_in>(yystack_[6].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        for_in_stmt->set_namelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        for_in_stmt->set_explist(explist);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        for_in_stmt->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_in_stmt;
    }
#line 1232 "parser.cpp"
    break;

  case 21: // stmt: "function" funcname funcbody
#line 430 "parser.y"
    {
        LOG_INFO("[bison]: stmt: function funcname funcbody");
        auto func_stmt = std::make_shared<fakelua::syntax_tree_function>(yystack_[2].location);
        auto funcname = std::dynamic_pointer_cast<fakelua::syntax_tree_funcname>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcname == nullptr) {
            LOG_ERROR("[bison]: stmt: funcname is not a funcname");
            fakelua::throw_fakelua_exception("funcname is not a funcname");
        }
        func_stmt->set_funcname(funcname);
        auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::throw_fakelua_exception("funcbody is not a funcbody");
        }
        func_stmt->set_funcbody(funcbody);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = func_stmt;
    }
#line 1254 "parser.cpp"
    break;

  case 22: // stmt: "local" "function" "identifier" funcbody
#line 449 "parser.y"
    {
        LOG_INFO("[bison]: stmt: local function IDENTIFIER funcbody");
        auto local_func_stmt = std::make_shared<fakelua::syntax_tree_local_function>(yystack_[3].location);
        local_func_stmt->set_name(yystack_[1].value.as < std::string > ());
        auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::throw_fakelua_exception("funcbody is not a funcbody");
        }
        local_func_stmt->set_funcbody(funcbody);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = local_func_stmt;
    }
#line 1271 "parser.cpp"
    break;

  case 23: // stmt: "local" attnamelist
#line 463 "parser.y"
    {
        LOG_INFO("[bison]: stmt: local attnamelist");
        auto local_stmt = std::make_shared<fakelua::syntax_tree_local_var>(yystack_[1].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        local_stmt->set_namelist(namelist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = local_stmt;
    }
#line 1287 "parser.cpp"
    break;

  case 24: // stmt: "local" attnamelist "=" explist
#line 476 "parser.y"
    {
        LOG_INFO("[bison]: stmt: local attnamelist assign explist");
        auto local_stmt = std::make_shared<fakelua::syntax_tree_local_var>(yystack_[3].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        local_stmt->set_namelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        local_stmt->set_explist(explist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = local_stmt;
    }
#line 1309 "parser.cpp"
    break;

  case 25: // attnamelist: "identifier"
#line 497 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(yystack_[0].location);
        namelist->add_name(yystack_[0].value.as < std::string > ());
        namelist->add_attrib("");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1321 "parser.cpp"
    break;

  case 26: // attnamelist: "identifier" "<" "identifier" ">"
#line 506 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(yystack_[3].location);
        namelist->add_name(yystack_[3].value.as < std::string > ());
        namelist->add_attrib(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1333 "parser.cpp"
    break;

  case 27: // attnamelist: attnamelist "," "identifier"
#line 515 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        namelist->add_name(yystack_[0].value.as < std::string > ());
        namelist->add_attrib("");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1349 "parser.cpp"
    break;

  case 28: // attnamelist: attnamelist "," "identifier" "<" "identifier" ">"
#line 528 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        namelist->add_name(yystack_[3].value.as < std::string > ());
        namelist->add_attrib(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1365 "parser.cpp"
    break;

  case 29: // elseifs: %empty
#line 543 "parser.y"
    {
        LOG_INFO("[bison]: elseifs: empty");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[0].location);
    }
#line 1374 "parser.cpp"
    break;

  case 30: // elseifs: "elseif" exp "then" block
#line 549 "parser.y"
    {
        LOG_INFO("[bison]: elseifs: elseif exp then block");
        auto elseifs = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[3].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        elseifs->add_elseif_expr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        elseifs->add_elseif_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = elseifs;
    }
#line 1396 "parser.cpp"
    break;

  case 31: // elseifs: elseifs "elseif" exp "then" block
#line 568 "parser.y"
    {
        LOG_INFO("[bison]: elseifs: elseifs elseif exp then block");
        auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>(yystack_[4].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: elseifs: elseifs is not a elseifs");
            fakelua::throw_fakelua_exception("elseifs is not a elseifs");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        elseifs->add_elseif_expr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        elseifs->add_elseif_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = elseifs;
    }
#line 1422 "parser.cpp"
    break;

  case 32: // retstat: "return"
#line 593 "parser.y"
    {
        LOG_INFO("[bison]: retstat: RETURN");
        auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[0].location);
        ret->set_explist(nullptr);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1433 "parser.cpp"
    break;

  case 33: // retstat: "return" explist
#line 601 "parser.y"
    {
        LOG_INFO("[bison]: retstat: RETURN explist");
        auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[1].location);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: retstat: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        ret->set_explist(explist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1449 "parser.cpp"
    break;

  case 34: // label: "::" "identifier" "::"
#line 616 "parser.y"
    {
    	LOG_INFO("[bison]: label: GOTO_TAG IDENTIFIER GOTO_TAG");
        auto ret = std::make_shared<fakelua::syntax_tree_label>(yystack_[1].location);
        ret->set_name(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1460 "parser.cpp"
    break;

  case 35: // funcnamelist: "identifier"
#line 626 "parser.y"
    {
        LOG_INFO("[bison]: funcnamelist: IDENTIFIER");
        auto funcnamelist = std::make_shared<fakelua::syntax_tree_funcnamelist>(yystack_[0].location);
        funcnamelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcnamelist;
    }
#line 1471 "parser.cpp"
    break;

  case 36: // funcnamelist: funcnamelist "." "identifier"
#line 634 "parser.y"
    {
        LOG_INFO("[bison]: funcnamelist: funcnamelist DOT IDENTIFIER");
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcnamelist: funcnamelist is not a funcnamelist");
            fakelua::throw_fakelua_exception("funcnamelist is not a funcnamelist");
        }
        funcnamelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcnamelist;
    }
#line 1486 "parser.cpp"
    break;

  case 37: // funcname: funcnamelist
#line 648 "parser.y"
    {
        LOG_INFO("[bison]: funcname: funcnamelist");
        auto funcname = std::make_shared<fakelua::syntax_tree_funcname>(yystack_[0].location);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::throw_fakelua_exception("funcnamelist is not a funcnamelist");
        }
        funcname->set_funcnamelist(funcnamelist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcname;
    }
#line 1502 "parser.cpp"
    break;

  case 38: // funcname: funcnamelist ":" "identifier"
#line 661 "parser.y"
    {
        LOG_INFO("[bison]: funcname: funcnamelist COLON IDENTIFIER");
        auto funcname = std::make_shared<fakelua::syntax_tree_funcname>(yystack_[2].location);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::throw_fakelua_exception("funcnamelist is not a funcnamelist");
        }
        funcname->set_funcnamelist(funcnamelist);
        funcname->set_colon_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcname;
    }
#line 1519 "parser.cpp"
    break;

  case 39: // varlist: var
#line 677 "parser.y"
    {
        LOG_INFO("[bison]: varlist: var");
        auto varlist = std::make_shared<fakelua::syntax_tree_varlist>(yystack_[0].location);
        auto var = std::dynamic_pointer_cast<fakelua::syntax_tree_var>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::throw_fakelua_exception("var is not a var");
        }
        varlist->add_var(var);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = varlist;
    }
#line 1535 "parser.cpp"
    break;

  case 40: // varlist: varlist "," var
#line 690 "parser.y"
    {
        LOG_INFO("[bison]: varlist: varlist COMMA var");
        auto varlist = std::dynamic_pointer_cast<fakelua::syntax_tree_varlist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (varlist == nullptr) {
            LOG_ERROR("[bison]: varlist: varlist is not a varlist");
            fakelua::throw_fakelua_exception("varlist is not a varlist");
        }
        auto var = std::dynamic_pointer_cast<fakelua::syntax_tree_var>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::throw_fakelua_exception("var is not a var");
        }
        varlist->add_var(var);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = varlist;
    }
#line 1555 "parser.cpp"
    break;

  case 41: // var: "identifier"
#line 709 "parser.y"
    {
        LOG_INFO("[bison]: var: IDENTIFIER");
        auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[0].location);
        var->set_name(yystack_[0].value.as < std::string > ());
        var->set_type("simple");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
    }
#line 1567 "parser.cpp"
    break;

  case 42: // var: prefixexp "[" exp "]"
#line 718 "parser.y"
    {
        LOG_INFO("[bison]: var: prefixexp LSQUARE exp RSQUARE");
        auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[2].location);
        var->set_type("square");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        var->set_prefixexp(prefixexp);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: var: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        var->set_exp(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
    }
#line 1590 "parser.cpp"
    break;

  case 43: // var: prefixexp "." "identifier"
#line 738 "parser.y"
    {
        LOG_INFO("[bison]: var: prefixexp DOT IDENTIFIER");
        auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[1].location);
        var->set_type("dot");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        var->set_prefixexp(prefixexp);
        var->set_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
    }
#line 1608 "parser.cpp"
    break;

  case 44: // namelist: "identifier"
#line 755 "parser.y"
    {
        LOG_INFO("[bison]: namelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(yystack_[0].location);
        namelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1619 "parser.cpp"
    break;

  case 45: // namelist: namelist "," "identifier"
#line 763 "parser.y"
    {
        LOG_INFO("[bison]: namelist: namelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        namelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1634 "parser.cpp"
    break;

  case 46: // explist: exp
#line 777 "parser.y"
    {
        LOG_INFO("[bison]: explist: exp");
        auto explist = std::make_shared<fakelua::syntax_tree_explist>(yystack_[0].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        explist->add_exp(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = explist;
    }
#line 1650 "parser.cpp"
    break;

  case 47: // explist: explist "," exp
#line 790 "parser.y"
    {
        LOG_INFO("[bison]: explist: explist COMMA exp");
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: explist: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        explist->add_exp(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = explist;
    }
#line 1670 "parser.cpp"
    break;

  case 48: // exp: "nil"
#line 809 "parser.y"
    {
        LOG_INFO("[bison]: exp: NIL");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("nil");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1681 "parser.cpp"
    break;

  case 49: // exp: "true"
#line 817 "parser.y"
    {
        LOG_INFO("[bison]: exp: TRUE");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("true");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1692 "parser.cpp"
    break;

  case 50: // exp: "false"
#line 825 "parser.y"
    {
        LOG_INFO("[bison]: exp: FALSES");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("false");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1703 "parser.cpp"
    break;

  case 51: // exp: "number"
#line 833 "parser.y"
    {
        LOG_INFO("[bison]: exp: NUMBER");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("number");
        exp->set_value(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1715 "parser.cpp"
    break;

  case 52: // exp: "string"
#line 842 "parser.y"
    {
        LOG_INFO("[bison]: exp: STRING");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("string");
        exp->set_value(l->remove_quotes(yystack_[0].value.as < std::string > ()));
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1727 "parser.cpp"
    break;

  case 53: // exp: "..."
#line 851 "parser.y"
    {
        LOG_INFO("[bison]: exp: VAR_PARAMS");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("var_params");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1738 "parser.cpp"
    break;

  case 54: // exp: functiondef
#line 859 "parser.y"
    {
        LOG_INFO("[bison]: exp: functiondef");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("functiondef");
        auto functiondef = std::dynamic_pointer_cast<fakelua::syntax_tree_functiondef>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (functiondef == nullptr) {
            LOG_ERROR("[bison]: exp: functiondef is not a functiondef");
            fakelua::throw_fakelua_exception("functiondef is not a functiondef");
        }
        exp->set_right(functiondef);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1755 "parser.cpp"
    break;

  case 55: // exp: prefixexp
#line 873 "parser.y"
    {
        LOG_INFO("[bison]: exp: prefixexp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("prefixexp");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: exp: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        exp->set_right(prefixexp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1772 "parser.cpp"
    break;

  case 56: // exp: tableconstructor
#line 887 "parser.y"
    {
        LOG_INFO("[bison]: exp: tableconstructor");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("tableconstructor");
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::syntax_tree_tableconstructor>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: exp: tableconstructor is not a tableconstructor");
            fakelua::throw_fakelua_exception("tableconstructor is not a tableconstructor");
        }
        exp->set_right(tableconstructor);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1789 "parser.cpp"
    break;

  case 57: // exp: exp binop exp
#line 901 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp binop exp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[2].location);
        exp->set_type("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::throw_fakelua_exception("left_exp is not a exp");
        }
        exp->set_left(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::throw_fakelua_exception("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        auto binop = std::dynamic_pointer_cast<fakelua::syntax_tree_binop>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (binop == nullptr) {
            LOG_ERROR("[bison]: exp: binop is not a binop");
            fakelua::throw_fakelua_exception("binop is not a binop");
        }
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1818 "parser.cpp"
    break;

  case 58: // exp: unop exp
#line 927 "parser.y"
    {
        LOG_INFO("[bison]: exp: unop exp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[1].location);
        exp->set_type("unop");
        auto unop = std::dynamic_pointer_cast<fakelua::syntax_tree_unop>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (unop == nullptr) {
            LOG_ERROR("[bison]: exp: unop is not a unop");
            fakelua::throw_fakelua_exception("unop is not a unop");
        }
        exp->set_op(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::throw_fakelua_exception("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1841 "parser.cpp"
    break;

  case 59: // prefixexp: var
#line 949 "parser.y"
    {
        LOG_INFO("[bison]: prefixexp: var");
        auto prefixexp = std::make_shared<fakelua::syntax_tree_prefixexp>(yystack_[0].location);
        prefixexp->set_type("var");
        auto var = std::dynamic_pointer_cast<fakelua::syntax_tree_var>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (var == nullptr) {
            LOG_ERROR("[bison]: prefixexp: var is not a var");
            fakelua::throw_fakelua_exception("var is not a var");
        }
        prefixexp->set_value(var);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = prefixexp;
    }
#line 1858 "parser.cpp"
    break;

  case 60: // prefixexp: functioncall
#line 963 "parser.y"
    {
        LOG_INFO("[bison]: prefixexp: functioncall");
        auto prefixexp = std::make_shared<fakelua::syntax_tree_prefixexp>(yystack_[0].location);
        prefixexp->set_type("functioncall");
        auto functioncall = std::dynamic_pointer_cast<fakelua::syntax_tree_functioncall>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (functioncall == nullptr) {
            LOG_ERROR("[bison]: prefixexp: functioncall is not a functioncall");
            fakelua::throw_fakelua_exception("functioncall is not a functioncall");
        }
        prefixexp->set_value(functioncall);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = prefixexp;
    }
#line 1875 "parser.cpp"
    break;

  case 61: // prefixexp: "(" exp ")"
#line 977 "parser.y"
    {
        LOG_INFO("[bison]: prefixexp: LPAREN exp RPAREN");
        auto prefixexp = std::make_shared<fakelua::syntax_tree_prefixexp>(yystack_[2].location);
        prefixexp->set_type("exp");
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: prefixexp: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        prefixexp->set_value(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = prefixexp;
    }
#line 1892 "parser.cpp"
    break;

  case 62: // functioncall: prefixexp args
#line 992 "parser.y"
    {
        LOG_INFO("[bison]: functioncall: prefixexp args");
        auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(yystack_[1].location);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        functioncall->set_prefixexp(prefixexp);
        auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_args>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::throw_fakelua_exception("args is not a args");
        }
        functioncall->set_args(args);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = functioncall;
    }
#line 1914 "parser.cpp"
    break;

  case 63: // functioncall: prefixexp ":" "identifier" args
#line 1011 "parser.y"
    {
        LOG_INFO("[bison]: functioncall: prefixexp COLON IDENTIFIER args");
        auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(yystack_[3].location);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        functioncall->set_prefixexp(prefixexp);
        functioncall->set_name(yystack_[1].value.as < std::string > ());
        auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_args>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::throw_fakelua_exception("args is not a args");
        }
        functioncall->set_args(args);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = functioncall;
    }
#line 1937 "parser.cpp"
    break;

  case 64: // args: "(" explist ")"
#line 1033 "parser.y"
    {
        LOG_INFO("[bison]: args: LPAREN explist RPAREN");
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[2].location);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: args: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        args->set_explist(explist);
        args->set_type("explist");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1954 "parser.cpp"
    break;

  case 65: // args: "(" ")"
#line 1047 "parser.y"
    {
        LOG_INFO("[bison]: args: LPAREN RPAREN");
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[1].location);
        args->set_type("empty");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1965 "parser.cpp"
    break;

  case 66: // args: tableconstructor
#line 1055 "parser.y"
    {
        LOG_INFO("[bison]: args: tableconstructor");
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[0].location);
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::syntax_tree_tableconstructor>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: args: tableconstructor is not a tableconstructor");
            fakelua::throw_fakelua_exception("tableconstructor is not a tableconstructor");
        }
        args->set_tableconstructor(tableconstructor);
        args->set_type("tableconstructor");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1982 "parser.cpp"
    break;

  case 67: // args: "string"
#line 1069 "parser.y"
    {
        LOG_INFO("[bison]: args: STRING");
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[0].location);
        args->set_string(l->remove_quotes(yystack_[0].value.as < std::string > ()));
        args->set_type("string");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1994 "parser.cpp"
    break;

  case 68: // functiondef: "function" funcbody
#line 1080 "parser.y"
    {
        LOG_INFO("[bison]: functiondef: FUNCTION funcbody");
        auto functiondef = std::make_shared<fakelua::syntax_tree_functiondef>(yystack_[1].location);
        auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: functiondef: funcbody is not a funcbody");
            fakelua::throw_fakelua_exception("funcbody is not a funcbody");
        }
        functiondef->set_funcbody(funcbody);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = functiondef;
    }
#line 2010 "parser.cpp"
    break;

  case 69: // funcbody: "(" parlist ")" block "end"
#line 1095 "parser.y"
    {
        LOG_INFO("[bison]: funcbody: LPAREN parlist RPAREN block END");
        auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(yystack_[4].location);
        auto parlist = std::dynamic_pointer_cast<fakelua::syntax_tree_parlist>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (parlist == nullptr) {
            LOG_ERROR("[bison]: funcbody: parlist is not a parlist");
            fakelua::throw_fakelua_exception("parlist is not a parlist");
        }
        funcbody->set_parlist(parlist);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        funcbody->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcbody;
    }
#line 2032 "parser.cpp"
    break;

  case 70: // funcbody: "(" ")" block "end"
#line 1114 "parser.y"
    {
        LOG_INFO("[bison]: funcbody: LPAREN RPAREN block END");
        auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(yystack_[3].location);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        funcbody->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcbody;
    }
#line 2048 "parser.cpp"
    break;

  case 71: // parlist: namelist
#line 1129 "parser.y"
    {
        LOG_INFO("[bison]: parlist: namelist");
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[0].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        parlist->set_namelist(namelist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
    }
#line 2064 "parser.cpp"
    break;

  case 72: // parlist: namelist "," "..."
#line 1142 "parser.y"
    {
        LOG_INFO("[bison]: parlist: namelist COMMA VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[2].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        parlist->set_namelist(namelist);
        parlist->set_var_params(true);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
    }
#line 2081 "parser.cpp"
    break;

  case 73: // parlist: "..."
#line 1156 "parser.y"
    {
        LOG_INFO("[bison]: parlist: VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[0].location);
        parlist->set_var_params(true);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
    }
#line 2092 "parser.cpp"
    break;

  case 74: // tableconstructor: "{" fieldlist "}"
#line 1166 "parser.y"
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY fieldlist RCURLY");
        auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[2].location);
        auto fieldlist = std::dynamic_pointer_cast<fakelua::syntax_tree_fieldlist>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: tableconstructor: fieldlist is not a fieldlist");
            fakelua::throw_fakelua_exception("fieldlist is not a fieldlist");
        }
        tableconstructor->set_fieldlist(fieldlist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
    }
#line 2108 "parser.cpp"
    break;

  case 75: // tableconstructor: "{" "}"
#line 1179 "parser.y"
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY RCURLY");
        auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[1].location);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
    }
#line 2118 "parser.cpp"
    break;

  case 76: // fieldlist: field
#line 1188 "parser.y"
    {
        LOG_INFO("[bison]: fieldlist: field");
        auto fieldlist = std::make_shared<fakelua::syntax_tree_fieldlist>(yystack_[0].location);
        auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_field>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::throw_fakelua_exception("field is not a field");
        }
        fieldlist->add_field(field);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = fieldlist;
    }
#line 2134 "parser.cpp"
    break;

  case 77: // fieldlist: fieldlist fieldsep field
#line 1201 "parser.y"
    {
        LOG_INFO("[bison]: fieldlist: fieldlist fieldsep field");
        auto fieldlist = std::dynamic_pointer_cast<fakelua::syntax_tree_fieldlist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: fieldlist: fieldlist is not a fieldlist");
            fakelua::throw_fakelua_exception("fieldlist is not a fieldlist");
        }
        auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_field>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::throw_fakelua_exception("field is not a field");
        }
        fieldlist->add_field(field);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = fieldlist;
    }
#line 2154 "parser.cpp"
    break;

  case 78: // field: "[" exp "]" "=" exp
#line 1220 "parser.y"
    {
        LOG_INFO("[bison]: field: LSQUARE exp RSQUARE ASSIGN exp");
        auto field = std::make_shared<fakelua::syntax_tree_field>(yystack_[4].location);
        auto key = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (key == nullptr) {
            LOG_ERROR("[bison]: key: key is not a exp");
            fakelua::throw_fakelua_exception("key is not a exp");
        }
        field->set_key(key);
        auto value = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (value == nullptr) {
            LOG_ERROR("[bison]: field: value is not a exp");
            fakelua::throw_fakelua_exception("value is not a exp");
        }
        field->set_value(value);
        field->set_type("array");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = field;
    }
#line 2177 "parser.cpp"
    break;

  case 79: // field: "identifier" "=" exp
#line 1240 "parser.y"
    {
        LOG_INFO("[bison]: field: IDENTIFIER ASSIGN exp");
        auto field = std::make_shared<fakelua::syntax_tree_field>(yystack_[2].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        field->set_name(yystack_[2].value.as < std::string > ());
        field->set_value(exp);
        field->set_type("object");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = field;
    }
#line 2195 "parser.cpp"
    break;

  case 80: // field: exp
#line 1255 "parser.y"
    {
        LOG_INFO("[bison]: field: exp");
        auto field = std::make_shared<fakelua::syntax_tree_field>(yystack_[0].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        field->set_value(exp);
        field->set_type("array");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = field;
    }
#line 2212 "parser.cpp"
    break;

  case 81: // fieldsep: ","
#line 1271 "parser.y"
    {
        LOG_INFO("[bison]: fieldsep: COMMA");
        // nothing to do
    }
#line 2221 "parser.cpp"
    break;

  case 82: // fieldsep: ";"
#line 1277 "parser.y"
    {
        LOG_INFO("[bison]: fieldsep: SEMICOLON");
        // nothing to do
    }
#line 2230 "parser.cpp"
    break;

  case 83: // binop: "+"
#line 1285 "parser.y"
    {
        LOG_INFO("[bison]: binop: PLUS");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("PLUS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2241 "parser.cpp"
    break;

  case 84: // binop: "-"
#line 1293 "parser.y"
    {
        LOG_INFO("[bison]: binop: MINUS");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MINUS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2252 "parser.cpp"
    break;

  case 85: // binop: "*"
#line 1301 "parser.y"
    {
        LOG_INFO("[bison]: binop: STAR");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("STAR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2263 "parser.cpp"
    break;

  case 86: // binop: "/"
#line 1309 "parser.y"
    {
        LOG_INFO("[bison]: binop: SLASH");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("SLASH");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2274 "parser.cpp"
    break;

  case 87: // binop: "//"
#line 1317 "parser.y"
    {
        LOG_INFO("[bison]: binop: DOUBLE_SLASH");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("DOUBLE_SLASH");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2285 "parser.cpp"
    break;

  case 88: // binop: "^"
#line 1325 "parser.y"
    {
        LOG_INFO("[bison]: binop: POW");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("POW");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2296 "parser.cpp"
    break;

  case 89: // binop: "%"
#line 1333 "parser.y"
    {
        LOG_INFO("[bison]: binop: MOD");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MOD");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2307 "parser.cpp"
    break;

  case 90: // binop: "&"
#line 1341 "parser.y"
    {
        LOG_INFO("[bison]: binop: BITAND");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("BITAND");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2318 "parser.cpp"
    break;

  case 91: // binop: "~"
#line 1349 "parser.y"
    {
        LOG_INFO("[bison]: binop: BITNOT");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("XOR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2329 "parser.cpp"
    break;

  case 92: // binop: "|"
#line 1357 "parser.y"
    {
        LOG_INFO("[bison]: binop: BITOR");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("BITOR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2340 "parser.cpp"
    break;

  case 93: // binop: ">>"
#line 1365 "parser.y"
    {
        LOG_INFO("[bison]: binop: RIGHT_SHIFT");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("RIGHT_SHIFT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2351 "parser.cpp"
    break;

  case 94: // binop: "<<"
#line 1373 "parser.y"
    {
        LOG_INFO("[bison]: binop: LEFT_SHIFT");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("LEFT_SHIFT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2362 "parser.cpp"
    break;

  case 95: // binop: ".."
#line 1381 "parser.y"
    {
        LOG_INFO("[bison]: binop: CONCAT");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("CONCAT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2373 "parser.cpp"
    break;

  case 96: // binop: "<"
#line 1389 "parser.y"
    {
        LOG_INFO("[bison]: binop: LESS");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("LESS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2384 "parser.cpp"
    break;

  case 97: // binop: "<="
#line 1397 "parser.y"
    {
        LOG_INFO("[bison]: binop: LESS_EQUAL");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("LESS_EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2395 "parser.cpp"
    break;

  case 98: // binop: ">"
#line 1405 "parser.y"
    {
        LOG_INFO("[bison]: binop: MORE");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MORE");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2406 "parser.cpp"
    break;

  case 99: // binop: ">="
#line 1413 "parser.y"
    {
        LOG_INFO("[bison]: binop: MORE_EQUAL");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MORE_EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2417 "parser.cpp"
    break;

  case 100: // binop: "=="
#line 1421 "parser.y"
    {
        LOG_INFO("[bison]: binop: EQUAL");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2428 "parser.cpp"
    break;

  case 101: // binop: "~="
#line 1429 "parser.y"
    {
        LOG_INFO("[bison]: binop: NOT_EQUAL");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("NOT_EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2439 "parser.cpp"
    break;

  case 102: // binop: "and"
#line 1437 "parser.y"
    {
        LOG_INFO("[bison]: binop: AND");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("AND");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2450 "parser.cpp"
    break;

  case 103: // binop: "or"
#line 1445 "parser.y"
    {
        LOG_INFO("[bison]: binop: OR");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("OR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2461 "parser.cpp"
    break;

  case 104: // unop: "-"
#line 1455 "parser.y"
    {
        LOG_INFO("[bison]: unop: MINUS");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("MINUS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2472 "parser.cpp"
    break;

  case 105: // unop: "not"
#line 1463 "parser.y"
    {
        LOG_INFO("[bison]: unop: NOT");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("NOT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2483 "parser.cpp"
    break;

  case 106: // unop: "#"
#line 1471 "parser.y"
    {
        LOG_INFO("[bison]: unop: NUMBER_SIGN");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("NUMBER_SIGN");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2494 "parser.cpp"
    break;

  case 107: // unop: "~"
#line 1479 "parser.y"
    {
        LOG_INFO("[bison]: unop: BITNOT");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("BITNOT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2505 "parser.cpp"
    break;


#line 2509 "parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      yy_lac_discard_ ("error recovery");
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
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


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  const char *
  parser::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "end of file", "error", "invalid token", "=", "-", "+", "*", "/", "(",
  ")", "{", "}", "[", "]", "and", "break", "do", "else", "elseif", "end",
  "false", "for", "function", "if", "in", "local", "nil", "not", "or",
  "repeat", "return", "then", "true", "until", "while", "goto", "//", "..",
  "...", "==", ">=", "<=", "~=", "<<", ">>", "::", ";", ":", ",", ".", "^",
  "%", "&", "|", "~", ">", "<", "#", "identifier", "string", "number",
  "$accept", "chunk", "block", "stmt", "attnamelist", "elseifs", "retstat",
  "label", "funcnamelist", "funcname", "varlist", "var", "namelist",
  "explist", "exp", "prefixexp", "functioncall", "args", "functiondef",
  "funcbody", "parlist", "tableconstructor", "fieldlist", "field",
  "fieldsep", "binop", "unop", YY_NULLPTR
    };
    return yy_sname[yysymbol];
  }



  // parser::context.
  parser::context::context (const parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

#if YYDEBUG
    // Execute LAC once. We don't care if it is successful, we
    // only do it for the sake of debugging output.
    if (!yyparser_.yy_lac_established_)
      yyparser_.yy_lac_check_ (yyla_.kind ());
#endif

    for (int yyx = 0; yyx < YYNTOKENS; ++yyx)
      {
        symbol_kind_type yysym = YY_CAST (symbol_kind_type, yyx);
        if (yysym != symbol_kind::S_YYerror
            && yysym != symbol_kind::S_YYUNDEF
            && yyparser_.yy_lac_check_ (yysym))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = yysym;
          }
      }
    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }




  bool
  parser::yy_lac_check_ (symbol_kind_type yytoken) const
  {
    // Logically, the yylac_stack's lifetime is confined to this function.
    // Clear it, to get rid of potential left-overs from previous call.
    yylac_stack_.clear ();
    // Reduce until we encounter a shift and thereby accept the token.
#if YYDEBUG
    YYCDEBUG << "LAC: checking lookahead " << symbol_name (yytoken) << ':';
#endif
    std::ptrdiff_t lac_top = 0;
    while (true)
      {
        state_type top_state = (yylac_stack_.empty ()
                                ? yystack_[lac_top].state
                                : yylac_stack_.back ());
        int yyrule = yypact_[+top_state];
        if (yy_pact_value_is_default_ (yyrule)
            || (yyrule += yytoken) < 0 || yylast_ < yyrule
            || yycheck_[yyrule] != yytoken)
          {
            // Use the default action.
            yyrule = yydefact_[+top_state];
            if (yyrule == 0)
              {
                YYCDEBUG << " Err\n";
                return false;
              }
          }
        else
          {
            // Use the action from yytable.
            yyrule = yytable_[yyrule];
            if (yy_table_value_is_error_ (yyrule))
              {
                YYCDEBUG << " Err\n";
                return false;
              }
            if (0 < yyrule)
              {
                YYCDEBUG << " S" << yyrule << '\n';
                return true;
              }
            yyrule = -yyrule;
          }
        // By now we know we have to simulate a reduce.
        YYCDEBUG << " R" << yyrule - 1;
        // Pop the corresponding number of values from the stack.
        {
          std::ptrdiff_t yylen = yyr2_[yyrule];
          // First pop from the LAC stack as many tokens as possible.
          std::ptrdiff_t lac_size = std::ptrdiff_t (yylac_stack_.size ());
          if (yylen < lac_size)
            {
              yylac_stack_.resize (std::size_t (lac_size - yylen));
              yylen = 0;
            }
          else if (lac_size)
            {
              yylac_stack_.clear ();
              yylen -= lac_size;
            }
          // Only afterwards look at the main stack.
          // We simulate popping elements by incrementing lac_top.
          lac_top += yylen;
        }
        // Keep top_state in sync with the updated stack.
        top_state = (yylac_stack_.empty ()
                     ? yystack_[lac_top].state
                     : yylac_stack_.back ());
        // Push the resulting state of the reduction.
        state_type state = yy_lr_goto_state_ (top_state, yyr1_[yyrule]);
        YYCDEBUG << " G" << int (state);
        yylac_stack_.push_back (state);
      }
  }

  // Establish the initial context if no initial context currently exists.
  bool
  parser::yy_lac_establish_ (symbol_kind_type yytoken)
  {
    /* Establish the initial context for the current lookahead if no initial
       context is currently established.

       We define a context as a snapshot of the parser stacks.  We define
       the initial context for a lookahead as the context in which the
       parser initially examines that lookahead in order to select a
       syntactic action.  Thus, if the lookahead eventually proves
       syntactically unacceptable (possibly in a later context reached via a
       series of reductions), the initial context can be used to determine
       the exact set of tokens that would be syntactically acceptable in the
       lookahead's place.  Moreover, it is the context after which any
       further semantic actions would be erroneous because they would be
       determined by a syntactically unacceptable token.

       yy_lac_establish_ should be invoked when a reduction is about to be
       performed in an inconsistent state (which, for the purposes of LAC,
       includes consistent states that don't know they're consistent because
       their default reductions have been disabled).

       For parse.lac=full, the implementation of yy_lac_establish_ is as
       follows.  If no initial context is currently established for the
       current lookahead, then check if that lookahead can eventually be
       shifted if syntactic actions continue from the current context.  */
    if (yy_lac_established_)
      return true;
    else
      {
#if YYDEBUG
        YYCDEBUG << "LAC: initial context established for "
                 << symbol_name (yytoken) << '\n';
#endif
        yy_lac_established_ = true;
        return yy_lac_check_ (yytoken);
      }
  }

  // Discard any previous initial lookahead context.
  void
  parser::yy_lac_discard_ (const char* event)
  {
   /* Discard any previous initial lookahead context because of Event,
      which may be a lookahead change or an invalidation of the currently
      established initial context for the current lookahead.

      The most common example of a lookahead change is a shift.  An example
      of both cases is syntax error recovery.  That is, a syntax error
      occurs when the lookahead is syntactically erroneous for the
      currently established initial context, so error recovery manipulates
      the parser stacks to try to find a new initial context in which the
      current lookahead is syntactically acceptable.  If it fails to find
      such a context, it discards the lookahead.  */
    if (yy_lac_established_)
      {
        YYCDEBUG << "LAC: initial context discarded due to "
                 << event << '\n';
        yy_lac_established_ = false;
      }
  }


  int
  parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
         In the first two cases, it might appear that the current syntax
         error should have been detected in the previous state when
         yy_lac_check was invoked.  However, at that time, there might
         have been a different syntax error that discarded a different
         initial context during error recovery, leaving behind the
         current lookahead.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -60;

  const signed char parser::yytable_ninf_ = -61;

  const short
  parser::yypact_[] =
  {
    1343,   387,   -60,  1343,   -46,   -36,   387,    58,  1343,   387,
     387,   -32,   -12,   -60,   -60,    13,  1343,   -60,   -60,   -60,
      21,    27,     9,    62,   -60,   321,   -60,    44,   -60,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,   -60,   497,     9,   -60,
     -60,   -60,   387,  1023,    78,    -9,   -60,   -20,    44,   550,
      26,    36,    30,  1055,    35,   974,   603,   -60,    48,   -60,
     -60,   387,    12,    28,   387,    41,    43,   -60,   -60,   -60,
     -60,   387,    91,   974,     3,   -60,    19,   -60,   -60,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,
     387,   974,   -60,   387,   387,    49,    52,    60,   -60,  1343,
      44,    61,   387,    64,   387,   387,  1343,   -60,    35,    31,
     -60,    -1,   656,     8,   -60,   709,   387,   -60,   -60,   -60,
     378,  1343,   -60,   -60,    54,   111,   974,   762,    25,   -60,
     -60,   -60,  1087,   -60,    68,    35,    69,   974,   974,  1119,
     -60,   -60,   -60,   121,   974,   -60,  1151,   -27,  1343,   387,
    1343,   387,    72,   -60,    71,   -60,   387,   -60,   -60,  1183,
     444,  1215,   815,  1343,   387,   -60,    76,   974,   -60,  1343,
     387,   -60,  1343,  1247,   868,   -60,  1279,   921,  1343,   -60,
    1343,   -60,  1343,  1343,  1311,   -60
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,    11,     3,     0,     0,     0,     0,     3,    32,
       0,     0,     0,     7,    41,     0,     2,     4,     6,    10,
       0,    59,     0,     9,   104,     0,    50,     0,    48,   105,
      49,    53,   107,   106,    52,    51,    59,     0,    55,    60,
      54,    56,     0,     0,    44,     0,    35,    37,     0,     0,
       0,    25,    23,     0,    33,    46,     0,    12,     0,     1,
       5,     0,     0,     0,     0,     0,     0,    67,    62,    66,
      75,     0,    41,    80,     0,    76,     0,    68,    84,    83,
      85,    86,    61,   102,   103,    87,    95,   100,    99,    97,
     101,    94,    93,    88,    89,    90,    92,    91,    98,    96,
       0,    58,    13,     0,     0,     0,     0,     0,    21,     3,
       0,     0,     0,     0,     0,     0,     3,    34,     8,    59,
      65,     0,     0,     0,    43,     0,     0,    74,    82,    81,
       0,     3,    73,    44,    71,     0,    57,     0,     0,    45,
      38,    36,    29,    22,     0,    24,    27,    15,    47,     0,
      64,    42,    63,     0,    79,    77,     0,     0,     3,     0,
       3,     0,     0,    26,     0,    14,     0,    70,    72,     0,
       0,     0,     0,     3,     0,    17,     0,    78,    69,     3,
       0,    20,     3,     0,     0,    28,     0,     0,    30,    16,
       3,    18,     3,    31,     0,    19
  };

  const short
  parser::yypgoto_[] =
  {
     -60,   -60,    -3,   101,   -60,   -60,   -60,   -60,   -60,   -60,
     -60,     0,    51,   -59,   196,   176,    34,    10,   -60,   -45,
     -60,   -15,   -60,     2,   -60,   -60,   -60
  };

  const unsigned char
  parser::yydefgoto_[] =
  {
       0,    15,    16,    17,    52,   162,    18,    19,    47,    48,
      20,    21,    45,    54,    55,    22,    23,    68,    40,    77,
     135,    41,    74,    75,   130,   100,    42
  };

  const short
  parser::yytable_[] =
  {
      43,    36,   118,   108,   121,    53,    36,    69,   150,    36,
      36,   168,    44,    59,   127,   104,    63,    63,    25,    25,
       1,    64,    46,    69,    61,    36,    57,   106,   131,   107,
     -39,   139,    24,   112,   -40,    39,     1,   120,    25,   105,
      39,   160,    36,    39,    39,   138,    58,   115,    26,   128,
      27,   129,    76,   145,    28,    29,    65,   132,    66,    39,
      30,    36,   119,    36,    36,   143,    31,    67,    67,    62,
      14,    36,   -60,   115,   -60,   -39,    39,   133,   113,   -40,
      50,   103,    32,   115,   110,    33,    14,    34,    35,   173,
     174,   175,   111,   117,   126,    39,    39,    39,    39,   123,
      36,   124,   157,    36,    36,    39,   142,   139,    69,   -60,
     140,   -60,    36,   149,    36,    36,    51,    60,   141,   144,
     158,   -60,   146,   163,   166,   164,    36,   134,   156,   176,
      36,   185,   155,   152,    39,     0,     0,    39,    39,     0,
       0,     0,     0,     0,    60,     0,    39,     0,    39,    39,
       0,     0,     0,     0,    60,   169,     0,   171,     0,    36,
      39,    36,     0,     0,    39,     0,    36,     0,     0,     0,
     183,     0,     0,     0,    36,     0,   186,    38,     0,   188,
      36,     0,    38,     0,     0,    38,    38,   193,     0,   194,
       0,     0,     0,    39,     0,    39,     0,    37,     0,     0,
      39,    38,    49,     0,     0,     0,    56,     0,    39,     0,
       0,     0,     0,     0,    39,     0,     0,     0,    38,     0,
       0,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    38,   101,    38,
      38,     0,     0,    60,     0,     0,     0,    38,     0,     0,
      60,     0,     0,     0,     0,     0,     0,    60,     0,     0,
     122,     0,     0,     0,     0,     0,     0,   125,     0,     0,
      60,     0,    60,     0,     0,     0,    38,     0,     0,    38,
      38,     0,     0,     0,    60,     0,     0,    60,    38,    60,
      38,    38,     0,     0,    60,    60,   136,     0,     0,   137,
       0,     0,    38,     0,     0,     0,    38,     0,     0,     0,
     147,   148,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   154,     0,     0,    24,    73,     0,     0,     1,
       0,    25,    70,    71,     0,    38,     0,    38,     0,     0,
       0,    26,    38,    27,     0,     0,     0,    28,    29,     0,
      38,     0,     0,    30,     0,   170,    38,   172,     0,    31,
       0,     0,   177,     0,     0,     0,     0,     0,     0,     0,
     184,     0,     0,     0,     0,    32,   187,     0,    33,    72,
      34,    35,    24,     0,     0,     0,     1,     0,    25,     0,
      71,    24,     0,     0,     0,     1,     0,    25,    26,     0,
      27,     0,     0,     0,    28,    29,     0,    26,     0,    27,
      30,     0,     0,    28,    29,     0,    31,     0,     0,    30,
       0,     0,     0,     0,     0,    31,     0,     0,     0,     0,
       0,     0,    32,     0,     0,    33,    72,    34,    35,     0,
       0,    32,     0,     0,    33,    14,    34,    35,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,    83,     0,
     179,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,     0,     0,     0,
      85,    86,     0,    87,    88,    89,    90,    91,    92,     0,
       0,     0,   180,     0,    93,    94,    95,    96,    97,    98,
      99,    78,    79,    80,    81,     0,    82,     0,     0,     0,
       0,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,    85,    86,     0,    87,    88,    89,    90,
      91,    92,     0,     0,     0,     0,     0,    93,    94,    95,
      96,    97,    98,    99,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
       0,   109,     0,     0,     0,     0,    85,    86,     0,    87,
      88,    89,    90,    91,    92,     0,     0,     0,     0,     0,
      93,    94,    95,    96,    97,    98,    99,    78,    79,    80,
      81,     0,     0,     0,     0,     0,     0,    83,     0,   116,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,     0,    85,
      86,     0,    87,    88,    89,    90,    91,    92,     0,     0,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
      78,    79,    80,    81,     0,     0,     0,     0,     0,   151,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,    87,    88,    89,    90,    91,
      92,     0,     0,     0,     0,     0,    93,    94,    95,    96,
      97,    98,    99,    78,    79,    80,    81,     0,     0,     0,
       0,     0,   153,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,     0,    85,    86,     0,    87,    88,
      89,    90,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,    85,    86,
       0,    87,    88,    89,    90,    91,    92,     0,     0,     0,
     159,     0,    93,    94,    95,    96,    97,    98,    99,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,   182,     0,     0,     0,
       0,    85,    86,     0,    87,    88,    89,    90,    91,    92,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,   190,
       0,     0,     0,     0,    85,    86,     0,    87,    88,    89,
      90,    91,    92,     0,     0,     0,     0,     0,    93,    94,
      95,    96,    97,    98,    99,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,    83,     0,   192,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,     0,     0,     0,    85,    86,     0,
      87,    88,    89,    90,    91,    92,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,     0,     0,     0,
      85,    86,     0,    87,    88,    89,    90,    91,    92,     0,
       0,     0,     0,     0,    93,    94,    95,    96,    97,    98,
      99,     1,     0,     0,     0,     0,     0,     0,     2,     3,
       0,     0,   102,     0,     4,     5,     6,     0,     7,     0,
       0,     0,     8,     9,     0,     0,     0,    10,    11,     0,
       0,     0,     0,     1,     0,     0,     0,     0,    12,    13,
       2,     3,     0,     0,     0,     0,     4,     5,     6,     0,
       7,    14,     0,     0,     8,     9,     0,     0,   114,    10,
      11,     0,     0,     0,     0,     1,     0,     0,     0,     0,
      12,    13,     2,     3,     0,   161,     0,     0,     4,     5,
       6,     0,     7,    14,     0,     0,     8,     9,     0,     0,
       0,    10,    11,     0,     0,     0,     0,     1,     0,     0,
       0,     0,    12,    13,     2,     3,     0,     0,   165,     0,
       4,     5,     6,     0,     7,    14,     0,     0,     8,     9,
       0,     0,     0,    10,    11,     0,     0,     0,     0,     1,
       0,     0,     0,     0,    12,    13,     2,     3,     0,     0,
     167,     0,     4,     5,     6,     0,     7,    14,     0,     0,
       8,     9,     0,     0,     0,    10,    11,     0,     0,     0,
       0,     1,     0,     0,     0,     0,    12,    13,     2,     3,
       0,     0,   178,     0,     4,     5,     6,     0,     7,    14,
       0,     0,     8,     9,     0,     0,     0,    10,    11,     0,
       0,     0,     0,     1,     0,     0,     0,     0,    12,    13,
       2,     3,     0,     0,   181,     0,     4,     5,     6,     0,
       7,    14,     0,     0,     8,     9,     0,     0,     0,    10,
      11,     0,     0,     0,     0,     1,     0,     0,     0,     0,
      12,    13,     2,     3,     0,     0,   189,     0,     4,     5,
       6,     0,     7,    14,     0,     0,     8,     9,     0,     0,
       0,    10,    11,     0,     0,     0,     0,     1,     0,     0,
       0,     0,    12,    13,     2,     3,     0,     0,   191,     0,
       4,     5,     6,     0,     7,    14,     0,     0,     8,     9,
       0,     0,     0,    10,    11,     0,     0,     0,     0,     1,
       0,     0,     0,     0,    12,    13,     2,     3,     0,     0,
     195,     0,     4,     5,     6,     0,     7,    14,     0,     0,
       8,     9,     0,     0,     0,    10,    11,     0,     0,     0,
       0,     1,     0,     0,     0,     0,    12,    13,     2,     3,
       0,     0,     0,     0,     4,     5,     6,     0,     7,    14,
       0,     0,     8,     9,     0,     0,     0,    10,    11,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14
  };

  const short
  parser::yycheck_[] =
  {
       3,     1,    61,    48,    63,     8,     6,    22,     9,     9,
      10,    38,    58,     0,    11,    24,     8,     8,    10,    10,
       8,    12,    58,    38,     3,    25,    58,    47,     9,    49,
       3,    58,     4,     3,     3,     1,     8,     9,    10,    48,
       6,    16,    42,     9,    10,   104,    58,    48,    20,    46,
      22,    48,     8,   112,    26,    27,    47,    38,    49,    25,
      32,    61,    62,    63,    64,   110,    38,    59,    59,    48,
      58,    71,    10,    48,    12,    48,    42,    58,    48,    48,
      22,     3,    54,    48,    58,    57,    58,    59,    60,    17,
      18,    19,    56,    45,     3,    61,    62,    63,    64,    58,
     100,    58,    48,   103,   104,    71,   109,    58,   123,    47,
      58,    49,   112,   116,   114,   115,    58,    16,    58,    58,
       9,    59,    58,    55,     3,    56,   126,    76,   131,    58,
     130,    55,   130,   123,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,    -1,    -1,    43,    -1,   112,    -1,   114,   115,
      -1,    -1,    -1,    -1,    53,   158,    -1,   160,    -1,   159,
     126,   161,    -1,    -1,   130,    -1,   166,    -1,    -1,    -1,
     173,    -1,    -1,    -1,   174,    -1,   179,     1,    -1,   182,
     180,    -1,     6,    -1,    -1,     9,    10,   190,    -1,   192,
      -1,    -1,    -1,   159,    -1,   161,    -1,     1,    -1,    -1,
     166,    25,     6,    -1,    -1,    -1,    10,    -1,   174,    -1,
      -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,    42,    -1,
      -1,    25,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,    42,    63,
      64,    -1,    -1,   142,    -1,    -1,    -1,    71,    -1,    -1,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,
     169,    -1,   171,    -1,    -1,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   183,    -1,    -1,   186,   112,   188,
     114,   115,    -1,    -1,   193,   194,   100,    -1,    -1,   103,
      -1,    -1,   126,    -1,    -1,    -1,   130,    -1,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   126,    -1,    -1,     4,   130,    -1,    -1,     8,
      -1,    10,    11,    12,    -1,   159,    -1,   161,    -1,    -1,
      -1,    20,   166,    22,    -1,    -1,    -1,    26,    27,    -1,
     174,    -1,    -1,    32,    -1,   159,   180,   161,    -1,    38,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    54,   180,    -1,    57,    58,
      59,    60,     4,    -1,    -1,    -1,     8,    -1,    10,    -1,
      12,     4,    -1,    -1,    -1,     8,    -1,    10,    20,    -1,
      22,    -1,    -1,    -1,    26,    27,    -1,    20,    -1,    22,
      32,    -1,    -1,    26,    27,    -1,    38,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    -1,    -1,    57,    58,    59,    60,    -1,
      -1,    54,    -1,    -1,    57,    58,    59,    60,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,     4,     5,     6,     7,    -1,     9,    -1,    -1,    -1,
      -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,
      53,    54,    55,    56,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      50,    51,    52,    53,    54,    55,    56,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    54,    55,    56,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,
      51,    52,    53,    54,    55,    56,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,    51,
      52,    53,    54,    55,    56,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,
      -1,    -1,    19,    -1,    21,    22,    23,    -1,    25,    -1,
      -1,    -1,    29,    30,    -1,    -1,    -1,    34,    35,    -1,
      -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    45,    46,
      15,    16,    -1,    -1,    -1,    -1,    21,    22,    23,    -1,
      25,    58,    -1,    -1,    29,    30,    -1,    -1,    33,    34,
      35,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      45,    46,    15,    16,    -1,    18,    -1,    -1,    21,    22,
      23,    -1,    25,    58,    -1,    -1,    29,    30,    -1,    -1,
      -1,    34,    35,    -1,    -1,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    45,    46,    15,    16,    -1,    -1,    19,    -1,
      21,    22,    23,    -1,    25,    58,    -1,    -1,    29,    30,
      -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,     8,
      -1,    -1,    -1,    -1,    45,    46,    15,    16,    -1,    -1,
      19,    -1,    21,    22,    23,    -1,    25,    58,    -1,    -1,
      29,    30,    -1,    -1,    -1,    34,    35,    -1,    -1,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    45,    46,    15,    16,
      -1,    -1,    19,    -1,    21,    22,    23,    -1,    25,    58,
      -1,    -1,    29,    30,    -1,    -1,    -1,    34,    35,    -1,
      -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    45,    46,
      15,    16,    -1,    -1,    19,    -1,    21,    22,    23,    -1,
      25,    58,    -1,    -1,    29,    30,    -1,    -1,    -1,    34,
      35,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      45,    46,    15,    16,    -1,    -1,    19,    -1,    21,    22,
      23,    -1,    25,    58,    -1,    -1,    29,    30,    -1,    -1,
      -1,    34,    35,    -1,    -1,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    45,    46,    15,    16,    -1,    -1,    19,    -1,
      21,    22,    23,    -1,    25,    58,    -1,    -1,    29,    30,
      -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,     8,
      -1,    -1,    -1,    -1,    45,    46,    15,    16,    -1,    -1,
      19,    -1,    21,    22,    23,    -1,    25,    58,    -1,    -1,
      29,    30,    -1,    -1,    -1,    34,    35,    -1,    -1,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    45,    46,    15,    16,
      -1,    -1,    -1,    -1,    21,    22,    23,    -1,    25,    58,
      -1,    -1,    29,    30,    -1,    -1,    -1,    34,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58
  };

  const signed char
  parser::yystos_[] =
  {
       0,     8,    15,    16,    21,    22,    23,    25,    29,    30,
      34,    35,    45,    46,    58,    62,    63,    64,    67,    68,
      71,    72,    76,    77,     4,    10,    20,    22,    26,    27,
      32,    38,    54,    57,    59,    60,    72,    75,    76,    77,
      79,    82,    87,    63,    58,    73,    58,    69,    70,    75,
      22,    58,    65,    63,    74,    75,    75,    58,    58,     0,
      64,     3,    48,     8,    12,    47,    49,    59,    78,    82,
      11,    12,    58,    75,    83,    84,     8,    80,     4,     5,
       6,     7,     9,    14,    28,    36,    37,    39,    40,    41,
      42,    43,    44,    50,    51,    52,    53,    54,    55,    56,
      86,    75,    19,     3,    24,    48,    47,    49,    80,    31,
      58,    56,     3,    48,    33,    48,    16,    45,    74,    72,
       9,    74,    75,    58,    58,    75,     3,    11,    46,    48,
      85,     9,    38,    58,    73,    81,    75,    75,    74,    58,
      58,    58,    63,    80,    58,    74,    58,    75,    75,    63,
       9,    13,    78,    13,    75,    84,    63,    48,     9,    48,
      16,    18,    66,    55,    56,    19,     3,    19,    38,    63,
      75,    63,    75,    17,    18,    19,    58,    75,    19,    16,
      48,    19,    31,    63,    75,    55,    63,    75,    63,    19,
      31,    19,    16,    63,    63,    19
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    61,    62,    63,    63,    63,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    65,    65,    65,    65,    66,
      66,    66,    67,    67,    68,    69,    69,    70,    70,    71,
      71,    72,    72,    72,    73,    73,    74,    74,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    76,
      76,    76,    77,    77,    78,    78,    78,    78,    79,    80,
      80,    81,    81,    81,    82,    82,    83,    83,    84,    84,
      84,    85,    85,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    87,    87,    87,    87
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     1,     2,     1,     1,     3,     1,
       1,     1,     2,     3,     5,     4,     8,     6,     9,    11,
       7,     3,     4,     2,     4,     1,     4,     3,     6,     0,
       4,     5,     1,     2,     3,     1,     3,     1,     3,     1,
       3,     1,     4,     3,     1,     3,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     3,     2,     4,     3,     2,     1,     1,     2,     5,
       4,     1,     3,     1,     3,     2,     1,     3,     5,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
  };




#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   144,   144,   152,   158,   170,   189,   195,   201,   220,
     226,   232,   238,   246,   252,   271,   290,   321,   346,   372,
     404,   429,   448,   462,   475,   496,   505,   514,   527,   542,
     548,   567,   592,   600,   615,   625,   633,   647,   660,   676,
     689,   708,   717,   737,   754,   762,   776,   789,   808,   816,
     824,   832,   841,   850,   858,   872,   886,   900,   926,   948,
     962,   976,   991,  1010,  1032,  1046,  1054,  1068,  1079,  1094,
    1113,  1128,  1141,  1155,  1165,  1178,  1187,  1200,  1219,  1239,
    1254,  1270,  1276,  1284,  1292,  1300,  1308,  1316,  1324,  1332,
    1340,  1348,  1356,  1364,  1372,  1380,  1388,  1396,  1404,  1412,
    1420,  1428,  1436,  1444,  1454,  1462,  1470,  1478
  };

  void
  parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


} // yy
#line 3440 "parser.cpp"

#line 1486 "parser.y"


void
yy::parser::error (const location_type& l, const std::string& m)
{
    std::stringstream ss;
    ss << l;
    fakelua::throw_fakelua_exception(std::format("{}: {}", ss.str(), m));
}
