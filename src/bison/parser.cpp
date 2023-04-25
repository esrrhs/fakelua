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
#line 33 "parser.y"

#include "compile/myflexer.h"

yy::parser::symbol_type yylex(fakelua::myflexer* l) {
    auto ret = l->my_yylex();
    LOG(INFO) << "[bison]: bison get token: " << ret.name() << " loc:" << ret.location;
    return ret;
}

int yyFlexLexer::yylex() { return -1; }


#line 59 "parser.cpp"


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
#line 151 "parser.cpp"

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
#line 134 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 435 "parser.cpp"
        break;

      case symbol_kind::S_STRING: // "string"
#line 134 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 441 "parser.cpp"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 134 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 447 "parser.cpp"
        break;

      case symbol_kind::S_chunk: // chunk
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 453 "parser.cpp"
        break;

      case symbol_kind::S_block: // block
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 459 "parser.cpp"
        break;

      case symbol_kind::S_stmt: // stmt
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 465 "parser.cpp"
        break;

      case symbol_kind::S_elseifs: // elseifs
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 471 "parser.cpp"
        break;

      case symbol_kind::S_retstat: // retstat
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 477 "parser.cpp"
        break;

      case symbol_kind::S_label: // label
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 483 "parser.cpp"
        break;

      case symbol_kind::S_funcnamelist: // funcnamelist
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 489 "parser.cpp"
        break;

      case symbol_kind::S_funcname: // funcname
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 495 "parser.cpp"
        break;

      case symbol_kind::S_varlist: // varlist
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 501 "parser.cpp"
        break;

      case symbol_kind::S_var: // var
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 507 "parser.cpp"
        break;

      case symbol_kind::S_namelist: // namelist
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 513 "parser.cpp"
        break;

      case symbol_kind::S_explist: // explist
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 519 "parser.cpp"
        break;

      case symbol_kind::S_exp: // exp
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 525 "parser.cpp"
        break;

      case symbol_kind::S_prefixexp: // prefixexp
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 531 "parser.cpp"
        break;

      case symbol_kind::S_functioncall: // functioncall
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 537 "parser.cpp"
        break;

      case symbol_kind::S_args: // args
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 543 "parser.cpp"
        break;

      case symbol_kind::S_functiondef: // functiondef
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 549 "parser.cpp"
        break;

      case symbol_kind::S_funcbody: // funcbody
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 555 "parser.cpp"
        break;

      case symbol_kind::S_parlist: // parlist
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 561 "parser.cpp"
        break;

      case symbol_kind::S_tableconstructor: // tableconstructor
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 567 "parser.cpp"
        break;

      case symbol_kind::S_fieldlist: // fieldlist
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 573 "parser.cpp"
        break;

      case symbol_kind::S_field: // field
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 579 "parser.cpp"
        break;

      case symbol_kind::S_binop: // binop
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 585 "parser.cpp"
        break;

      case symbol_kind::S_unop: // unop
#line 134 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 591 "parser.cpp"
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
#line 141 "parser.y"
    {
          LOG(INFO) << "[bison]: chunk: " << "block";
          l->set_chunk(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
    }
#line 890 "parser.cpp"
    break;

  case 3: // block: %empty
#line 149 "parser.y"
    {
        LOG(INFO) << "[bison]: block: " << "empty";
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_block>(yystack_[0].location);
    }
#line 899 "parser.cpp"
    break;

  case 4: // block: stmt
#line 155 "parser.y"
    {
        LOG(INFO) << "[bison]: block: " << "stmt";
        auto block = std::make_shared<fakelua::syntax_tree_block>(yystack_[0].location);
        auto stmt = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (stmt == nullptr) {
            LOG(ERROR) << "[bison]: block: " << "stmt is nullptr";
        }
        block->add_stmt(stmt);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = block;
    }
#line 914 "parser.cpp"
    break;

  case 5: // block: block stmt
#line 167 "parser.y"
    {
        LOG(INFO) << "[bison]: block: " << "block stmt";
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG(ERROR) << "[bison]: block: " << "block is not a block";
            throw std::runtime_error("block is not a block");
        }
        auto stmt = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (stmt == nullptr) {
            LOG(ERROR) << "[bison]: block: " << "stmt is not a stmt";
            throw std::runtime_error("stmt is not a stmt");
        }
        block->add_stmt(stmt);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = block;
    }
#line 934 "parser.cpp"
    break;

  case 6: // stmt: retstat
#line 186 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "retstat";
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 943 "parser.cpp"
    break;

  case 7: // stmt: ";"
#line 192 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "SEMICOLON";
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_empty>(yystack_[0].location);
    }
#line 952 "parser.cpp"
    break;

  case 8: // stmt: varlist "=" explist
#line 198 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "varlist ASSIGN explist";
        auto varlist = std::dynamic_pointer_cast<fakelua::syntax_tree_varlist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (varlist == nullptr) {
            LOG(ERROR) << "[bison]: stmt: " << "varlist is not a varlist";
            throw std::runtime_error("varlist is not a varlist");
        }
        if (explist == nullptr) {
            LOG(ERROR) << "[bison]: stmt: " << "explist is not a explist";
            throw std::runtime_error("explist is not a explist");
        }
        auto assign = std::make_shared<fakelua::syntax_tree_assign>(yystack_[1].location);
        assign->set_varlist(varlist);
        assign->set_explist(explist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = assign;
    }
#line 974 "parser.cpp"
    break;

  case 9: // stmt: functioncall
#line 217 "parser.y"
    {
	LOG(INFO) << "[bison]: stmt: " << "functioncall";
	yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 983 "parser.cpp"
    break;

  case 10: // stmt: label
#line 223 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "label";
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 992 "parser.cpp"
    break;

  case 11: // stmt: "break"
#line 229 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "break";
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_break>(yystack_[0].location);
    }
#line 1001 "parser.cpp"
    break;

  case 12: // stmt: "goto" "identifier"
#line 235 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "goto IDENTIFIER";
        auto go = std::make_shared<fakelua::syntax_tree_goto>(yystack_[0].location);
        go->set_label(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = go;
    }
#line 1012 "parser.cpp"
    break;

  case 13: // stmt: "do" block "end"
#line 243 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "do block end";
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 1021 "parser.cpp"
    break;

  case 14: // stmt: "while" exp "do" block "end"
#line 249 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "while exp do block end";
        auto while_stmt = std::make_shared<fakelua::syntax_tree_while>(yystack_[4].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "exp is not a exp";
		throw std::runtime_error("exp is not a exp");
        }
        while_stmt->set_exp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "block is not a block";
		throw std::runtime_error("block is not a block");
        }
        while_stmt->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = while_stmt;
    }
#line 1043 "parser.cpp"
    break;

  case 15: // stmt: "repeat" block "until" exp
#line 268 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "repeat block until exp";
        auto repeat = std::make_shared<fakelua::syntax_tree_repeat>(yystack_[3].location);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "block is not a block";
		throw std::runtime_error("block is not a block");
        }
        repeat->set_block(block);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "exp is not a exp";
		throw std::runtime_error("exp is not a exp");
        }
        repeat->set_exp(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = repeat;
    }
#line 1065 "parser.cpp"
    break;

  case 16: // stmt: "if" exp "then" block elseifs "else" block "end"
#line 287 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "if exp then block elseifs else block end";
        auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(yystack_[7].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[6].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "exp is not a exp";
		throw std::runtime_error("exp is not a exp");
        }
        if_stmt->set_exp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[4].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "block is not a block";
		throw std::runtime_error("block is not a block");
        }
        if_stmt->set_block(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (elseifs == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "elseiflist is not a elseiflist";
		throw std::runtime_error("elseiflist is not a elseiflist");
        }
        if_stmt->set_elseiflist(elseifs);
        auto else_block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (else_block == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "else_block is not a block";
		throw std::runtime_error("else_block is not a block");
        }
        if_stmt->set_else_block(else_block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = if_stmt;
    }
#line 1099 "parser.cpp"
    break;

  case 17: // stmt: "if" exp "then" block elseifs "end"
#line 318 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "if exp then block elseifs end";
        auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(yystack_[5].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[4].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "exp is not a exp";
		throw std::runtime_error("exp is not a exp");
        }
        if_stmt->set_exp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "block is not a block";
		throw std::runtime_error("block is not a block");
        }
        if_stmt->set_block(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (elseifs == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "elseiflist is not a elseiflist";
		throw std::runtime_error("elseiflist is not a elseiflist");
        }
        if_stmt->set_elseiflist(elseifs);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = if_stmt;
    }
#line 1127 "parser.cpp"
    break;

  case 18: // stmt: "for" "identifier" "=" exp "," exp "do" block "end"
#line 343 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "for IDENTIFIER assign exp COMMA exp do block end";
        auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(yystack_[8].location);
        for_loop_stmt->set_name(yystack_[7].value.as < std::string > ());
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "exp is not a exp";
		throw std::runtime_error("exp is not a exp");
        }
        for_loop_stmt->set_exp_begin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (end_exp == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "end_exp is not a exp";
		throw std::runtime_error("end_exp is not a exp");
        }
        for_loop_stmt->set_exp_end(end_exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "block is not a block";
		throw std::runtime_error("block is not a block");
        }
        for_loop_stmt->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_loop_stmt;
    }
#line 1156 "parser.cpp"
    break;

  case 19: // stmt: "for" "identifier" "=" exp "," exp "," exp "do" block "end"
#line 369 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "for IDENTIFIER assign exp COMMA exp COMMA exp do block end";
        auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(yystack_[10].location);
        for_loop_stmt->set_name(yystack_[9].value.as < std::string > ());
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[7].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "exp is not a exp";
		throw std::runtime_error("exp is not a exp");
        }
        for_loop_stmt->set_exp_begin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (end_exp == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "end_exp is not a exp";
		throw std::runtime_error("end_exp is not a exp");
        }
        for_loop_stmt->set_exp_end(end_exp);
        auto step_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (step_exp == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "step_exp is not a exp";
		throw std::runtime_error("step_exp is not a exp");
        }
        for_loop_stmt->set_exp_step(step_exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "block is not a block";
		throw std::runtime_error("block is not a block");
        }
        for_loop_stmt->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_loop_stmt;
    }
#line 1191 "parser.cpp"
    break;

  case 20: // stmt: "for" namelist "in" explist "do" block "end"
#line 401 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "for namelist in explist do block end";
        auto for_in_stmt = std::make_shared<fakelua::syntax_tree_for_in>(yystack_[6].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "namelist is not a namelist";
		throw std::runtime_error("namelist is not a namelist");
        }
        for_in_stmt->set_namelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "explist is not a explist";
		throw std::runtime_error("explist is not a explist");
        }
        for_in_stmt->set_explist(explist);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "block is not a block";
		throw std::runtime_error("block is not a block");
        }
        for_in_stmt->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_in_stmt;
    }
#line 1219 "parser.cpp"
    break;

  case 21: // stmt: "function" funcname funcbody
#line 426 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "function funcname funcbody";
        auto func_stmt = std::make_shared<fakelua::syntax_tree_function>(yystack_[2].location);
        auto funcname = std::dynamic_pointer_cast<fakelua::syntax_tree_funcname>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcname == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "funcname is not a funcname";
		throw std::runtime_error("funcname is not a funcname");
        }
        func_stmt->set_funcname(funcname);
        auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcbody == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "funcbody is not a funcbody";
		throw std::runtime_error("funcbody is not a funcbody");
        }
        func_stmt->set_funcbody(funcbody);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = func_stmt;
    }
#line 1241 "parser.cpp"
    break;

  case 22: // stmt: "local" "function" "identifier" funcbody
#line 445 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "local function NAME funcbody";
        auto local_func_stmt = std::make_shared<fakelua::syntax_tree_local_function>(yystack_[3].location);
        local_func_stmt->set_name(yystack_[1].value.as < std::string > ());
        auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcbody == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "funcbody is not a funcbody";
		throw std::runtime_error("funcbody is not a funcbody");
        }
        local_func_stmt->set_funcbody(funcbody);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = local_func_stmt;
    }
#line 1258 "parser.cpp"
    break;

  case 23: // stmt: "local" namelist
#line 459 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "local namelist";
        auto local_stmt = std::make_shared<fakelua::syntax_tree_local_var>(yystack_[1].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "namelist is not a namelist";
		throw std::runtime_error("namelist is not a namelist");
        }
        local_stmt->set_namelist(namelist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = local_stmt;
    }
#line 1274 "parser.cpp"
    break;

  case 24: // stmt: "local" namelist "=" explist
#line 472 "parser.y"
    {
        LOG(INFO) << "[bison]: stmt: " << "local namelist assign explist";
        auto local_stmt = std::make_shared<fakelua::syntax_tree_local_var>(yystack_[3].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "namelist is not a namelist";
		throw std::runtime_error("namelist is not a namelist");
        }
        local_stmt->set_namelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
		LOG(ERROR) << "[bison]: stmt: " << "explist is not a explist";
		throw std::runtime_error("explist is not a explist");
        }
        local_stmt->set_explist(explist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = local_stmt;
    }
#line 1296 "parser.cpp"
    break;

  case 25: // elseifs: %empty
#line 493 "parser.y"
    {
        LOG(INFO) << "[bison]: elseifs: " << "empty";
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[0].location);
    }
#line 1305 "parser.cpp"
    break;

  case 26: // elseifs: "elseif" exp "then" block
#line 499 "parser.y"
    {
        LOG(INFO) << "[bison]: elseifs: " << "elseif exp then block";
        auto elseifs = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[3].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG(ERROR) << "[bison]: elseifs: " << "exp is not a exp";
            throw std::runtime_error("exp is not a exp");
        }
        elseifs->add_elseif_expr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG(ERROR) << "[bison]: elseifs: " << "block is not a block";
            throw std::runtime_error("block is not a block");
        }
        elseifs->add_elseif_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = elseifs;
    }
#line 1327 "parser.cpp"
    break;

  case 27: // elseifs: elseifs "elseif" exp "then" block
#line 518 "parser.y"
    {
        LOG(INFO) << "[bison]: elseifs: " << "elseifs elseif exp then block";
        auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>(yystack_[4].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (elseifs == nullptr) {
            LOG(ERROR) << "[bison]: elseifs: " << "elseifs is not a elseifs";
            throw std::runtime_error("elseifs is not a elseifs");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG(ERROR) << "[bison]: elseifs: " << "exp is not a exp";
            throw std::runtime_error("exp is not a exp");
        }
        elseifs->add_elseif_expr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG(ERROR) << "[bison]: elseifs: " << "block is not a block";
            throw std::runtime_error("block is not a block");
        }
        elseifs->add_elseif_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = elseifs;
    }
#line 1353 "parser.cpp"
    break;

  case 28: // retstat: "return"
#line 543 "parser.y"
    {
        LOG(INFO) << "[bison]: retstat: " << "RETURN";
        auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[0].location);
        ret->set_explist(nullptr);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1364 "parser.cpp"
    break;

  case 29: // retstat: "return" explist
#line 551 "parser.y"
    {
        LOG(INFO) << "[bison]: retstat: " << "RETURN explist";
        auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[1].location);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
            LOG(ERROR) << "[bison]: retstat: " << "explist is not a explist";
            throw std::runtime_error("explist is not a explist");
        }
        ret->set_explist(explist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1380 "parser.cpp"
    break;

  case 30: // label: "::" "identifier" "::"
#line 566 "parser.y"
    {
        LOG(INFO) << "[bison]: bison get label: " << yystack_[1].value.as < std::string > () << " loc: " << yystack_[1].location;
        auto ret = std::make_shared<fakelua::syntax_tree_label>(yystack_[1].location);
        ret->set_name(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1391 "parser.cpp"
    break;

  case 31: // funcnamelist: "identifier"
#line 576 "parser.y"
    {
        LOG(INFO) << "[bison]: funcnamelist: " << "IDENTIFIER";
        auto funcnamelist = std::make_shared<fakelua::syntax_tree_funcnamelist>(yystack_[0].location);
        funcnamelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcnamelist;
    }
#line 1402 "parser.cpp"
    break;

  case 32: // funcnamelist: funcnamelist "." "identifier"
#line 584 "parser.y"
    {
        LOG(INFO) << "[bison]: funcnamelist: " << "funcnamelist DOT IDENTIFIER";
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcnamelist == nullptr) {
            LOG(ERROR) << "[bison]: funcnamelist: " << "funcnamelist is not a funcnamelist";
            throw std::runtime_error("funcnamelist is not a funcnamelist");
        }
        funcnamelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcnamelist;
    }
#line 1417 "parser.cpp"
    break;

  case 33: // funcname: funcnamelist
#line 598 "parser.y"
    {
        LOG(INFO) << "[bison]: funcname: " << "funcnamelist";
        auto funcname = std::make_shared<fakelua::syntax_tree_funcname>(yystack_[0].location);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcnamelist == nullptr) {
            LOG(ERROR) << "[bison]: funcname: " << "funcnamelist is not a funcnamelist";
            throw std::runtime_error("funcnamelist is not a funcnamelist");
        }
        funcname->set_funcnamelist(funcnamelist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcname;
    }
#line 1433 "parser.cpp"
    break;

  case 34: // funcname: funcnamelist ":" "identifier"
#line 611 "parser.y"
    {
        LOG(INFO) << "[bison]: funcname: " << "funcnamelist COLON IDENTIFIER";
        auto funcname = std::make_shared<fakelua::syntax_tree_funcname>(yystack_[2].location);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcnamelist == nullptr) {
            LOG(ERROR) << "[bison]: funcname: " << "funcnamelist is not a funcnamelist";
            throw std::runtime_error("funcnamelist is not a funcnamelist");
        }
        funcname->set_funcnamelist(funcnamelist);
        funcname->set_colon_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcname;
    }
#line 1450 "parser.cpp"
    break;

  case 35: // varlist: var
#line 627 "parser.y"
    {
        LOG(INFO) << "[bison]: varlist: " << "var";
        auto varlist = std::make_shared<fakelua::syntax_tree_varlist>(yystack_[0].location);
        auto var = std::dynamic_pointer_cast<fakelua::syntax_tree_var>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (var == nullptr) {
            LOG(ERROR) << "[bison]: varlist: " << "var is not a var";
            throw std::runtime_error("var is not a var");
        }
        varlist->add_var(var);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = varlist;
    }
#line 1466 "parser.cpp"
    break;

  case 36: // varlist: varlist "," var
#line 640 "parser.y"
    {
        LOG(INFO) << "[bison]: varlist: " << "varlist COMMA var";
        auto varlist = std::dynamic_pointer_cast<fakelua::syntax_tree_varlist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (varlist == nullptr) {
            LOG(ERROR) << "[bison]: varlist: " << "varlist is not a varlist";
            throw std::runtime_error("varlist is not a varlist");
        }
        auto var = std::dynamic_pointer_cast<fakelua::syntax_tree_var>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (var == nullptr) {
            LOG(ERROR) << "[bison]: varlist: " << "var is not a var";
            throw std::runtime_error("var is not a var");
        }
        varlist->add_var(var);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = varlist;
    }
#line 1486 "parser.cpp"
    break;

  case 37: // var: "identifier"
#line 659 "parser.y"
    {
        LOG(INFO) << "[bison]: var: " << "IDENTIFIER";
        auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[0].location);
        var->set_name(yystack_[0].value.as < std::string > ());
        var->set_type("simple");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
    }
#line 1498 "parser.cpp"
    break;

  case 38: // var: prefixexp "[" exp "]"
#line 668 "parser.y"
    {
        LOG(INFO) << "[bison]: var: " << "prefixexp LSQUARE exp RSQUARE";
        auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[2].location);
        var->set_type("square");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG(ERROR) << "[bison]: var: " << "prefixexp is not a prefixexp";
            throw std::runtime_error("prefixexp is not a prefixexp");
        }
        var->set_prefixexp(prefixexp);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG(ERROR) << "[bison]: var: " << "exp is not a exp";
            throw std::runtime_error("exp is not a exp");
        }
        var->set_exp(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
    }
#line 1521 "parser.cpp"
    break;

  case 39: // var: prefixexp "." "identifier"
#line 688 "parser.y"
    {
        LOG(INFO) << "[bison]: var: " << "prefixexp DOT IDENTIFIER";
        auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[1].location);
        var->set_type("dot");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG(ERROR) << "[bison]: var: " << "prefixexp is not a prefixexp";
            throw std::runtime_error("prefixexp is not a prefixexp");
        }
        var->set_prefixexp(prefixexp);
        var->set_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
    }
#line 1539 "parser.cpp"
    break;

  case 40: // namelist: "identifier"
#line 705 "parser.y"
    {
        LOG(INFO) << "[bison]: namelist: " << "IDENTIFIER";
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(yystack_[0].location);
        namelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1550 "parser.cpp"
    break;

  case 41: // namelist: namelist "," "identifier"
#line 713 "parser.y"
    {
        LOG(INFO) << "[bison]: namelist: " << "namelist COMMA IDENTIFIER";
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG(ERROR) << "[bison]: namelist: " << "namelist is not a namelist";
            throw std::runtime_error("namelist is not a namelist");
        }
        namelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1565 "parser.cpp"
    break;

  case 42: // explist: exp
#line 727 "parser.y"
    {
        LOG(INFO) << "[bison]: explist: " << "exp";
        auto explist = std::make_shared<fakelua::syntax_tree_explist>(yystack_[0].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG(ERROR) << "[bison]: explist: " << "exp is not a exp";
            throw std::runtime_error("exp is not a exp");
        }
        explist->add_exp(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = explist;
    }
#line 1581 "parser.cpp"
    break;

  case 43: // explist: explist "," exp
#line 740 "parser.y"
    {
        LOG(INFO) << "[bison]: explist: " << "explist COMMA exp";
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
            LOG(ERROR) << "[bison]: explist: " << "explist is not a explist";
            throw std::runtime_error("explist is not a explist");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG(ERROR) << "[bison]: explist: " << "exp is not a exp";
            throw std::runtime_error("exp is not a exp");
        }
        explist->add_exp(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = explist;
    }
#line 1601 "parser.cpp"
    break;

  case 44: // exp: "nil"
#line 759 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "NIL";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("nil");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1612 "parser.cpp"
    break;

  case 45: // exp: "true"
#line 767 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "TRUE";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("true");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1623 "parser.cpp"
    break;

  case 46: // exp: "false"
#line 775 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "FALSES";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("false");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1634 "parser.cpp"
    break;

  case 47: // exp: "number"
#line 783 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "NUMBER";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("number");
        exp->set_value(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1646 "parser.cpp"
    break;

  case 48: // exp: "string"
#line 792 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "STRING";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("string");
        exp->set_value(l->remove_quotes(yystack_[0].value.as < std::string > ()));
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1658 "parser.cpp"
    break;

  case 49: // exp: "..."
#line 801 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "VAR_PARAMS";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("var_params");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1669 "parser.cpp"
    break;

  case 50: // exp: functiondef
#line 809 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "functiondef";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("functiondef");
        auto functiondef = std::dynamic_pointer_cast<fakelua::syntax_tree_functiondef>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (functiondef == nullptr) {
            LOG(ERROR) << "[bison]: exp: " << "functiondef is not a functiondef";
            throw std::runtime_error("functiondef is not a functiondef");
        }
        exp->set_right(functiondef);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1686 "parser.cpp"
    break;

  case 51: // exp: prefixexp
#line 823 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "prefixexp";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("prefixexp");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG(ERROR) << "[bison]: exp: " << "prefixexp is not a prefixexp";
            throw std::runtime_error("prefixexp is not a prefixexp");
        }
        exp->set_right(prefixexp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1703 "parser.cpp"
    break;

  case 52: // exp: tableconstructor
#line 837 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "tableconstructor";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("tableconstructor");
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::syntax_tree_tableconstructor>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (tableconstructor == nullptr) {
            LOG(ERROR) << "[bison]: exp: " << "tableconstructor is not a tableconstructor";
            throw std::runtime_error("tableconstructor is not a tableconstructor");
        }
        exp->set_right(tableconstructor);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1720 "parser.cpp"
    break;

  case 53: // exp: exp binop exp
#line 851 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "exp binop exp";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[2].location);
        exp->set_type("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (left_exp == nullptr) {
            LOG(ERROR) << "[bison]: exp: " << "left_exp is not a exp";
            throw std::runtime_error("left_exp is not a exp");
        }
        exp->set_left(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (right_exp == nullptr) {
            LOG(ERROR) << "[bison]: exp: " << "right_exp is not a exp";
            throw std::runtime_error("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        auto binop = std::dynamic_pointer_cast<fakelua::syntax_tree_binop>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (binop == nullptr) {
            LOG(ERROR) << "[bison]: exp: " << "binop is not a binop";
            throw std::runtime_error("binop is not a binop");
        }
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1749 "parser.cpp"
    break;

  case 54: // exp: unop exp
#line 877 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "unop exp";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[1].location);
        exp->set_type("unop");
        auto unop = std::dynamic_pointer_cast<fakelua::syntax_tree_unop>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (unop == nullptr) {
            LOG(ERROR) << "[bison]: exp: " << "unop is not a unop";
            throw std::runtime_error("unop is not a unop");
        }
        exp->set_op(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (right_exp == nullptr) {
            LOG(ERROR) << "[bison]: exp: " << "right_exp is not a exp";
            throw std::runtime_error("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1772 "parser.cpp"
    break;

  case 55: // prefixexp: var
#line 899 "parser.y"
    {
        LOG(INFO) << "[bison]: prefixexp: " << "var";
        auto prefixexp = std::make_shared<fakelua::syntax_tree_prefixexp>(yystack_[0].location);
        prefixexp->set_type("var");
        auto var = std::dynamic_pointer_cast<fakelua::syntax_tree_var>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (var == nullptr) {
            LOG(ERROR) << "[bison]: prefixexp: " << "var is not a var";
            throw std::runtime_error("var is not a var");
        }
        prefixexp->set_value(var);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = prefixexp;
    }
#line 1789 "parser.cpp"
    break;

  case 56: // prefixexp: functioncall
#line 913 "parser.y"
    {
        LOG(INFO) << "[bison]: prefixexp: " << "functioncall";
        auto prefixexp = std::make_shared<fakelua::syntax_tree_prefixexp>(yystack_[0].location);
        prefixexp->set_type("functioncall");
        auto functioncall = std::dynamic_pointer_cast<fakelua::syntax_tree_functioncall>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (functioncall == nullptr) {
            LOG(ERROR) << "[bison]: prefixexp: " << "functioncall is not a functioncall";
            throw std::runtime_error("functioncall is not a functioncall");
        }
        prefixexp->set_value(functioncall);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = prefixexp;
    }
#line 1806 "parser.cpp"
    break;

  case 57: // prefixexp: "(" exp ")"
#line 927 "parser.y"
    {
        LOG(INFO) << "[bison]: prefixexp: " << "LPAREN exp RPAREN";
        auto prefixexp = std::make_shared<fakelua::syntax_tree_prefixexp>(yystack_[2].location);
        prefixexp->set_type("exp");
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG(ERROR) << "[bison]: prefixexp: " << "exp is not a exp";
            throw std::runtime_error("exp is not a exp");
        }
        prefixexp->set_value(exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = prefixexp;
    }
#line 1823 "parser.cpp"
    break;

  case 58: // functioncall: prefixexp args
#line 942 "parser.y"
    {
        LOG(INFO) << "[bison]: functioncall: " << "prefixexp args";
        auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(yystack_[1].location);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG(ERROR) << "[bison]: functioncall: " << "prefixexp is not a prefixexp";
            throw std::runtime_error("prefixexp is not a prefixexp");
        }
        functioncall->set_prefixexp(prefixexp);
        auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_args>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (args == nullptr) {
            LOG(ERROR) << "[bison]: functioncall: " << "args is not a args";
            throw std::runtime_error("args is not a args");
        }
        functioncall->set_args(args);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = functioncall;
    }
#line 1845 "parser.cpp"
    break;

  case 59: // functioncall: prefixexp ":" "identifier" args
#line 961 "parser.y"
    {
        LOG(INFO) << "[bison]: functioncall: " << "prefixexp COLON IDENTIFIER args";
        auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(yystack_[3].location);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (prefixexp == nullptr) {
            LOG(ERROR) << "[bison]: functioncall: " << "prefixexp is not a prefixexp";
            throw std::runtime_error("prefixexp is not a prefixexp");
        }
        functioncall->set_prefixexp(prefixexp);
        functioncall->set_name(yystack_[1].value.as < std::string > ());
        auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_args>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (args == nullptr) {
            LOG(ERROR) << "[bison]: functioncall: " << "args is not a args";
            throw std::runtime_error("args is not a args");
        }
        functioncall->set_args(args);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = functioncall;
    }
#line 1868 "parser.cpp"
    break;

  case 60: // args: "(" explist ")"
#line 983 "parser.y"
    {
        LOG(INFO) << "[bison]: args: " << "LPAREN explist RPAREN";
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[2].location);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (explist == nullptr) {
            LOG(ERROR) << "[bison]: args: " << "explist is not a explist";
            throw std::runtime_error("explist is not a explist");
        }
        args->set_explist(explist);
        args->set_type("explist");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1885 "parser.cpp"
    break;

  case 61: // args: "(" ")"
#line 997 "parser.y"
    {
        LOG(INFO) << "[bison]: args: " << "LPAREN RPAREN";
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[1].location);
        args->set_type("empty");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1896 "parser.cpp"
    break;

  case 62: // args: tableconstructor
#line 1005 "parser.y"
    {
        LOG(INFO) << "[bison]: args: " << "tableconstructor";
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[0].location);
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::syntax_tree_tableconstructor>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (tableconstructor == nullptr) {
            LOG(ERROR) << "[bison]: args: " << "tableconstructor is not a tableconstructor";
            throw std::runtime_error("tableconstructor is not a tableconstructor");
        }
        args->set_tableconstructor(tableconstructor);
        args->set_type("tableconstructor");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1913 "parser.cpp"
    break;

  case 63: // args: "string"
#line 1019 "parser.y"
    {
        LOG(INFO) << "[bison]: args: " << "STRING";
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[0].location);
        args->set_string(l->remove_quotes(yystack_[0].value.as < std::string > ()));
        args->set_type("string");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1925 "parser.cpp"
    break;

  case 64: // functiondef: "function" funcbody
#line 1030 "parser.y"
    {
        LOG(INFO) << "[bison]: functiondef: " << "FUNCTION funcbody";
        auto functiondef = std::make_shared<fakelua::syntax_tree_functiondef>(yystack_[1].location);
        auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (funcbody == nullptr) {
            LOG(ERROR) << "[bison]: functiondef: " << "funcbody is not a funcbody";
            throw std::runtime_error("funcbody is not a funcbody");
        }
        functiondef->set_funcbody(funcbody);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = functiondef;
    }
#line 1941 "parser.cpp"
    break;

  case 65: // funcbody: "(" parlist ")" block "end"
#line 1045 "parser.y"
    {
        LOG(INFO) << "[bison]: funcbody: " << "LPAREN parlist RPAREN block END";
        auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(yystack_[4].location);
        auto parlist = std::dynamic_pointer_cast<fakelua::syntax_tree_parlist>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (parlist == nullptr) {
            LOG(ERROR) << "[bison]: funcbody: " << "parlist is not a parlist";
            throw std::runtime_error("parlist is not a parlist");
        }
        funcbody->set_parlist(parlist);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG(ERROR) << "[bison]: funcbody: " << "block is not a block";
            throw std::runtime_error("block is not a block");
        }
        funcbody->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcbody;
    }
#line 1963 "parser.cpp"
    break;

  case 66: // funcbody: "(" ")" block "end"
#line 1064 "parser.y"
    {
        LOG(INFO) << "[bison]: funcbody: " << "LPAREN RPAREN block END";
        auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(yystack_[3].location);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (block == nullptr) {
            LOG(ERROR) << "[bison]: funcbody: " << "block is not a block";
            throw std::runtime_error("block is not a block");
        }
        funcbody->set_block(block);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcbody;
    }
#line 1979 "parser.cpp"
    break;

  case 67: // parlist: namelist
#line 1079 "parser.y"
    {
        LOG(INFO) << "[bison]: parlist: " << "namelist";
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[0].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG(ERROR) << "[bison]: parlist: " << "namelist is not a namelist";
            throw std::runtime_error("namelist is not a namelist");
        }
        parlist->set_namelist(namelist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
    }
#line 1995 "parser.cpp"
    break;

  case 68: // parlist: namelist "," "..."
#line 1092 "parser.y"
    {
        LOG(INFO) << "[bison]: parlist: " << "namelist COMMA VAR_PARAMS";
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[2].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (namelist == nullptr) {
            LOG(ERROR) << "[bison]: parlist: " << "namelist is not a namelist";
            throw std::runtime_error("namelist is not a namelist");
        }
        parlist->set_namelist(namelist);
        parlist->set_var_params(true);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
    }
#line 2012 "parser.cpp"
    break;

  case 69: // parlist: "..."
#line 1106 "parser.y"
    {
        LOG(INFO) << "[bison]: parlist: " << "VAR_PARAMS";
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[0].location);
        parlist->set_var_params(true);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
    }
#line 2023 "parser.cpp"
    break;

  case 70: // tableconstructor: "{" fieldlist "}"
#line 1116 "parser.y"
    {
        LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY fieldlist RCURLY";
        auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[2].location);
        auto fieldlist = std::dynamic_pointer_cast<fakelua::syntax_tree_fieldlist>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (fieldlist == nullptr) {
            LOG(ERROR) << "[bison]: tableconstructor: " << "fieldlist is not a fieldlist";
            throw std::runtime_error("fieldlist is not a fieldlist");
        }
        tableconstructor->set_fieldlist(fieldlist);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
    }
#line 2039 "parser.cpp"
    break;

  case 71: // tableconstructor: "{" "}"
#line 1129 "parser.y"
    {
        LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY RCURLY";
        auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[1].location);
        tableconstructor->set_fieldlist(std::make_shared<fakelua::syntax_tree_empty>(yystack_[1].location));
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
    }
#line 2050 "parser.cpp"
    break;

  case 72: // fieldlist: field
#line 1139 "parser.y"
    {
        LOG(INFO) << "[bison]: fieldlist: " << "field";
        auto fieldlist = std::make_shared<fakelua::syntax_tree_fieldlist>(yystack_[0].location);
        auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_field>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (field == nullptr) {
            LOG(ERROR) << "[bison]: fieldlist: " << "field is not a field";
            throw std::runtime_error("field is not a field");
        }
        fieldlist->add_field(field);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = fieldlist;
    }
#line 2066 "parser.cpp"
    break;

  case 73: // fieldlist: fieldlist fieldsep field
#line 1152 "parser.y"
    {
        LOG(INFO) << "[bison]: fieldlist: " << "fieldlist fieldsep field";
        auto fieldlist = std::dynamic_pointer_cast<fakelua::syntax_tree_fieldlist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (fieldlist == nullptr) {
            LOG(ERROR) << "[bison]: fieldlist: " << "fieldlist is not a fieldlist";
            throw std::runtime_error("fieldlist is not a fieldlist");
        }
        auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_field>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (field == nullptr) {
            LOG(ERROR) << "[bison]: fieldlist: " << "field is not a field";
            throw std::runtime_error("field is not a field");
        }
        fieldlist->add_field(field);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = fieldlist;
    }
#line 2086 "parser.cpp"
    break;

  case 74: // field: "[" exp "]" "=" exp
#line 1171 "parser.y"
    {
        LOG(INFO) << "[bison]: field: " << "LSQUARE exp RSQUARE ASSIGN exp";
        auto field = std::make_shared<fakelua::syntax_tree_field>(yystack_[4].location);
        auto key = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (key == nullptr) {
            LOG(ERROR) << "[bison]: key: " << "key is not a exp";
            throw std::runtime_error("key is not a exp");
        }
        field->set_key(key);
        auto value = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (value == nullptr) {
            LOG(ERROR) << "[bison]: field: " << "value is not a exp";
            throw std::runtime_error("value is not a exp");
        }
        field->set_value(value);
        field->set_type("array");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = field;
    }
#line 2109 "parser.cpp"
    break;

  case 75: // field: "identifier" "=" exp
#line 1191 "parser.y"
    {
        LOG(INFO) << "[bison]: field: " << "IDENTIFIER ASSIGN exp";
        auto field = std::make_shared<fakelua::syntax_tree_field>(yystack_[2].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG(ERROR) << "[bison]: field: " << "exp is not a exp";
            throw std::runtime_error("exp is not a exp");
        }
        field->set_name(yystack_[2].value.as < std::string > ());
        field->set_value(exp);
        field->set_type("object");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = field;
    }
#line 2127 "parser.cpp"
    break;

  case 76: // field: exp
#line 1206 "parser.y"
    {
        LOG(INFO) << "[bison]: field: " << "exp";
        auto field = std::make_shared<fakelua::syntax_tree_field>(yystack_[0].location);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (exp == nullptr) {
            LOG(ERROR) << "[bison]: field: " << "exp is not a exp";
            throw std::runtime_error("exp is not a exp");
        }
        field->set_value(exp);
        field->set_type("array");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = field;
    }
#line 2144 "parser.cpp"
    break;

  case 77: // fieldsep: ","
#line 1222 "parser.y"
    {
        LOG(INFO) << "[bison]: fieldsep: " << "COMMA";
        // nothing to do
    }
#line 2153 "parser.cpp"
    break;

  case 78: // fieldsep: ";"
#line 1228 "parser.y"
    {
        LOG(INFO) << "[bison]: fieldsep: " << "SEMICOLON";
        // nothing to do
    }
#line 2162 "parser.cpp"
    break;

  case 79: // binop: "+"
#line 1236 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "PLUS";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("PLUS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2173 "parser.cpp"
    break;

  case 80: // binop: "-"
#line 1244 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "MINUS";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MINUS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2184 "parser.cpp"
    break;

  case 81: // binop: "*"
#line 1252 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "STAR";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("STAR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2195 "parser.cpp"
    break;

  case 82: // binop: "/"
#line 1260 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "SLASH";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("SLASH");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2206 "parser.cpp"
    break;

  case 83: // binop: "//"
#line 1268 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "DOUBLE_SLASH";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("DOUBLE_SLASH");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2217 "parser.cpp"
    break;

  case 84: // binop: "^"
#line 1276 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "XOR";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("XOR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2228 "parser.cpp"
    break;

  case 85: // binop: "%"
#line 1284 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "MOD";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MOD");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2239 "parser.cpp"
    break;

  case 86: // binop: "&"
#line 1292 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "BITAND";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("BITAND");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2250 "parser.cpp"
    break;

  case 87: // binop: "~"
#line 1300 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "BITNOT";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("BITNOT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2261 "parser.cpp"
    break;

  case 88: // binop: "|"
#line 1308 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "BITOR";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("BITOR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2272 "parser.cpp"
    break;

  case 89: // binop: ">>"
#line 1316 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "RIGHT_SHIFT";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("RIGHT_SHIFT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2283 "parser.cpp"
    break;

  case 90: // binop: "<<"
#line 1324 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "LEFT_SHIFT";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("LEFT_SHIFT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2294 "parser.cpp"
    break;

  case 91: // binop: ".."
#line 1332 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "CONCAT";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("CONCAT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2305 "parser.cpp"
    break;

  case 92: // binop: "<"
#line 1340 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "LESS";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("LESS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2316 "parser.cpp"
    break;

  case 93: // binop: "<="
#line 1348 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "LESS_EQUAL";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("LESS_EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2327 "parser.cpp"
    break;

  case 94: // binop: ">"
#line 1356 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "MORE";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MORE");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2338 "parser.cpp"
    break;

  case 95: // binop: ">="
#line 1364 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "MORE_EQUAL";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MORE_EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2349 "parser.cpp"
    break;

  case 96: // binop: "=="
#line 1372 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "EQUAL";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2360 "parser.cpp"
    break;

  case 97: // binop: "~="
#line 1380 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "NOT_EQUAL";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("NOT_EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2371 "parser.cpp"
    break;

  case 98: // binop: "and"
#line 1388 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "AND";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("AND");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2382 "parser.cpp"
    break;

  case 99: // binop: "or"
#line 1396 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "OR";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("OR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2393 "parser.cpp"
    break;

  case 100: // unop: "-"
#line 1406 "parser.y"
    {
        LOG(INFO) << "[bison]: unop: " << "MINUS";
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("MINUS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2404 "parser.cpp"
    break;

  case 101: // unop: "not"
#line 1414 "parser.y"
    {
        LOG(INFO) << "[bison]: unop: " << "NOT";
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("NOT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2415 "parser.cpp"
    break;

  case 102: // unop: "#"
#line 1422 "parser.y"
    {
        LOG(INFO) << "[bison]: unop: " << "NUMBER_SIGN";
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("NUMBER_SIGN");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2426 "parser.cpp"
    break;

  case 103: // unop: "~"
#line 1430 "parser.y"
    {
        LOG(INFO) << "[bison]: unop: " << "BITNOT";
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("BITNOT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2437 "parser.cpp"
    break;


#line 2441 "parser.cpp"

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
  "$accept", "chunk", "block", "stmt", "elseifs", "retstat", "label",
  "funcnamelist", "funcname", "varlist", "var", "namelist", "explist",
  "exp", "prefixexp", "functioncall", "args", "functiondef", "funcbody",
  "parlist", "tableconstructor", "fieldlist", "field", "fieldsep", "binop",
  "unop", YY_NULLPTR
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


  const signed char parser::yypact_ninf_ = -50;

  const signed char parser::yytable_ninf_ = -57;

  const short
  parser::yypact_[] =
  {
    1287,   353,   -50,  1287,   -49,   -46,   353,    -9,  1287,   353,
     353,   -36,   -33,   -50,   -50,    27,  1287,   -50,   -50,   -50,
       4,    37,    11,    14,   -50,    24,   -50,    29,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   441,    11,   -50,
     -50,   -50,   353,   967,    36,    56,   -50,   -29,    29,   494,
      28,   -50,    40,   999,    41,   918,   547,   -50,     8,   -50,
     -50,   353,    -4,   283,   353,    33,    34,   -50,   -50,   -50,
     -50,   353,    95,   918,    53,   -50,    39,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     353,   918,   -50,   353,   353,    52,    62,    63,   -50,  1287,
      29,   353,   353,   353,  1287,   -50,    41,    42,   -50,     7,
     600,     0,   -50,   653,   353,   -50,   -50,   -50,   312,  1287,
     -50,    74,   114,   918,   706,    71,   -50,   -50,   -50,  1031,
     -50,    41,   918,   918,  1063,   -50,   -50,   -50,   121,   918,
     -50,  1095,    69,  1287,   353,  1287,   353,    96,   -50,   353,
     -50,   -50,  1127,   388,  1159,   759,  1287,   353,   -50,   918,
     -50,  1287,   353,   -50,  1287,  1191,   812,  1223,   865,  1287,
     -50,  1287,   -50,  1287,  1287,  1255,   -50
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,    11,     3,     0,     0,     0,     0,     3,    28,
       0,     0,     0,     7,    37,     0,     2,     4,     6,    10,
       0,    55,     0,     9,   100,     0,    46,     0,    44,   101,
      45,    49,   103,   102,    48,    47,    55,     0,    51,    56,
      50,    52,     0,     0,    40,     0,    31,    33,     0,     0,
       0,    40,    23,     0,    29,    42,     0,    12,     0,     1,
       5,     0,     0,     0,     0,     0,     0,    63,    58,    62,
      71,     0,    37,    76,     0,    72,     0,    64,    80,    79,
      81,    82,    57,    98,    99,    83,    91,    96,    95,    93,
      97,    90,    89,    84,    85,    86,    88,    87,    94,    92,
       0,    54,    13,     0,     0,     0,     0,     0,    21,     3,
       0,     0,     0,     0,     3,    30,     8,    55,    61,     0,
       0,     0,    39,     0,     0,    70,    78,    77,     0,     3,
      69,    67,     0,    53,     0,     0,    41,    34,    32,    25,
      22,    24,    15,    43,     0,    60,    38,    59,     0,    75,
      73,     0,     0,     3,     0,     3,     0,     0,    14,     0,
      66,    68,     0,     0,     0,     0,     3,     0,    17,    74,
      65,     3,     0,    20,     3,     0,     0,     0,     0,    26,
      16,     3,    18,     3,    27,     0,    19
  };

  const short
  parser::yypgoto_[] =
  {
     -50,   -50,    -3,    59,   -50,   -50,   -50,   -50,   -50,   -50,
       5,    -5,   -32,   195,   145,    32,     9,   -50,   -45,   -50,
     -21,   -50,     3,   -50,   -50,   -50
  };

  const unsigned char
  parser::yydefgoto_[] =
  {
       0,    15,    16,    17,   157,    18,    19,    47,    48,    20,
      21,    45,    54,    55,    22,    23,    68,    40,    77,   132,
      41,    74,    75,   128,   100,    42
  };

  const short
  parser::yytable_[] =
  {
      43,    69,    52,   108,     1,    53,    36,    61,    63,    44,
      25,    36,    46,    50,    36,    36,   145,    69,   106,    63,
     107,    25,    57,    64,   -56,    58,   -56,    59,    24,   116,
      36,   119,     1,    39,    25,    70,    71,    76,    39,   103,
     -35,    39,    39,   111,    26,   -36,    27,    36,   129,    51,
      28,    29,    62,   115,    14,   113,    30,    39,    65,    67,
      66,   -56,    31,   -56,   125,   140,    36,   117,    36,    36,
      67,   131,   135,   -56,    39,    60,    36,   130,    32,   141,
     104,    33,    72,    34,    35,   -35,   110,   155,   105,   113,
     -36,   121,   122,    39,    39,    39,    39,    51,   124,   126,
      69,   127,    60,    39,   105,    36,   139,   161,    36,    36,
     136,   144,    60,   166,   167,   168,    36,    36,    36,   113,
     137,   138,   152,   153,   159,     0,   151,   136,     0,    36,
     147,   150,    39,    36,     0,    39,    39,     0,     0,     0,
       0,     0,     0,    39,    39,    39,    38,     0,     0,     0,
     162,    38,   164,     0,    38,    38,    39,     0,     0,    36,
      39,    36,     0,   175,    36,     0,     0,     0,   177,     0,
      38,   179,    36,     0,     0,     0,     0,    36,   184,     0,
     185,     0,     0,     0,     0,     0,    39,    38,    39,     0,
       0,    39,     0,     0,     0,     0,    37,     0,    60,    39,
       0,    49,     0,    60,    39,    56,    38,     0,    38,    38,
      60,     0,     0,     0,     0,     0,    38,     0,     0,     0,
      73,    60,     0,    60,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    60,     0,    60,   101,    60,     0,
       0,     0,     0,    60,    60,    38,     0,     0,    38,    38,
       0,     0,     0,     0,     0,     0,    38,    38,    38,   120,
       0,     0,     0,     0,     0,     0,   123,     0,     0,    38,
       0,     0,     0,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    24,     0,     0,
       0,     1,   118,    25,     0,   133,     0,     0,   134,    38,
       0,    38,     0,    26,    38,    27,     0,   142,   143,    28,
      29,     0,    38,     0,     0,    30,    24,    38,     0,   149,
       1,    31,    25,    73,    71,     0,     0,     0,     0,     0,
       0,     0,    26,     0,    27,     0,     0,    32,    28,    29,
      33,    14,    34,    35,    30,     0,     0,     0,     0,   163,
      31,   165,     0,     0,   169,     0,     0,    24,     0,     0,
       0,     1,   176,    25,     0,     0,    32,   178,     0,    33,
      72,    34,    35,    26,     0,    27,     0,     0,     0,    28,
      29,     0,     0,     0,     0,    30,     0,     0,     0,     0,
       0,    31,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,    83,     0,   171,     0,     0,    32,     0,     0,
      33,    14,    34,    35,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,    85,    86,     0,    87,    88,    89,
      90,    91,    92,     0,     0,     0,   172,     0,    93,    94,
      95,    96,    97,    98,    99,    78,    79,    80,    81,     0,
      82,     0,     0,     0,     0,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,     0,     0,     0,    85,    86,     0,
      87,    88,    89,    90,    91,    92,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,     0,   109,     0,     0,     0,     0,
      85,    86,     0,    87,    88,    89,    90,    91,    92,     0,
       0,     0,     0,     0,    93,    94,    95,    96,    97,    98,
      99,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,    83,     0,   114,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,    85,    86,     0,    87,    88,    89,    90,
      91,    92,     0,     0,     0,     0,     0,    93,    94,    95,
      96,    97,    98,    99,    78,    79,    80,    81,     0,     0,
       0,     0,     0,   146,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,     0,     0,     0,    85,    86,     0,    87,
      88,    89,    90,    91,    92,     0,     0,     0,     0,     0,
      93,    94,    95,    96,    97,    98,    99,    78,    79,    80,
      81,     0,     0,     0,     0,     0,   148,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,     0,    85,
      86,     0,    87,    88,    89,    90,    91,    92,     0,     0,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
      78,    79,    80,    81,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,    87,    88,    89,    90,    91,
      92,     0,     0,     0,   154,     0,    93,    94,    95,    96,
      97,    98,    99,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
     174,     0,     0,     0,     0,    85,    86,     0,    87,    88,
      89,    90,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,   181,     0,     0,     0,     0,    85,    86,
       0,    87,    88,    89,    90,    91,    92,     0,     0,     0,
       0,     0,    93,    94,    95,    96,    97,    98,    99,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,    83,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,     0,     0,
       0,    85,    86,     0,    87,    88,    89,    90,    91,    92,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,    85,    86,     0,    87,    88,    89,
      90,    91,    92,     0,     0,     0,     0,     0,    93,    94,
      95,    96,    97,    98,    99,     1,     0,     0,     0,     0,
       0,     0,     2,     3,     0,     0,   102,     0,     4,     5,
       6,     0,     7,     0,     0,     0,     8,     9,     0,     0,
       0,    10,    11,     0,     0,     0,     0,     1,     0,     0,
       0,     0,    12,    13,     2,     3,     0,     0,     0,     0,
       4,     5,     6,     0,     7,    14,     0,     0,     8,     9,
       0,     0,   112,    10,    11,     0,     0,     0,     0,     1,
       0,     0,     0,     0,    12,    13,     2,     3,     0,   156,
       0,     0,     4,     5,     6,     0,     7,    14,     0,     0,
       8,     9,     0,     0,     0,    10,    11,     0,     0,     0,
       0,     1,     0,     0,     0,     0,    12,    13,     2,     3,
       0,     0,   158,     0,     4,     5,     6,     0,     7,    14,
       0,     0,     8,     9,     0,     0,     0,    10,    11,     0,
       0,     0,     0,     1,     0,     0,     0,     0,    12,    13,
       2,     3,     0,     0,   160,     0,     4,     5,     6,     0,
       7,    14,     0,     0,     8,     9,     0,     0,     0,    10,
      11,     0,     0,     0,     0,     1,     0,     0,     0,     0,
      12,    13,     2,     3,     0,     0,   170,     0,     4,     5,
       6,     0,     7,    14,     0,     0,     8,     9,     0,     0,
       0,    10,    11,     0,     0,     0,     0,     1,     0,     0,
       0,     0,    12,    13,     2,     3,     0,     0,   173,     0,
       4,     5,     6,     0,     7,    14,     0,     0,     8,     9,
       0,     0,     0,    10,    11,     0,     0,     0,     0,     1,
       0,     0,     0,     0,    12,    13,     2,     3,     0,     0,
     180,     0,     4,     5,     6,     0,     7,    14,     0,     0,
       8,     9,     0,     0,     0,    10,    11,     0,     0,     0,
       0,     1,     0,     0,     0,     0,    12,    13,     2,     3,
       0,     0,   182,     0,     4,     5,     6,     0,     7,    14,
       0,     0,     8,     9,     0,     0,     0,    10,    11,     0,
       0,     0,     0,     1,     0,     0,     0,     0,    12,    13,
       2,     3,     0,     0,   186,     0,     4,     5,     6,     0,
       7,    14,     0,     0,     8,     9,     0,     0,     0,    10,
      11,     0,     0,     0,     0,     1,     0,     0,     0,     0,
      12,    13,     2,     3,     0,     0,     0,     0,     4,     5,
       6,     0,     7,    14,     0,     0,     8,     9,     0,     0,
       0,    10,    11,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14
  };

  const short
  parser::yycheck_[] =
  {
       3,    22,     7,    48,     8,     8,     1,     3,     8,    58,
      10,     6,    58,    22,     9,    10,     9,    38,    47,     8,
      49,    10,    58,    12,    10,    58,    12,     0,     4,    61,
      25,    63,     8,     1,    10,    11,    12,     8,     6,     3,
       3,     9,    10,     3,    20,     3,    22,    42,     9,    58,
      26,    27,    48,    45,    58,    48,    32,    25,    47,    59,
      49,    47,    38,    49,    11,   110,    61,    62,    63,    64,
      59,    76,   104,    59,    42,    16,    71,    38,    54,   111,
      24,    57,    58,    59,    60,    48,    58,    16,    48,    48,
      48,    58,    58,    61,    62,    63,    64,    58,     3,    46,
     121,    48,    43,    71,    48,   100,   109,    38,   103,   104,
      58,   114,    53,    17,    18,    19,   111,   112,   113,    48,
      58,    58,    48,     9,     3,    -1,   129,    58,    -1,   124,
     121,   128,   100,   128,    -1,   103,   104,    -1,    -1,    -1,
      -1,    -1,    -1,   111,   112,   113,     1,    -1,    -1,    -1,
     153,     6,   155,    -1,     9,    10,   124,    -1,    -1,   154,
     128,   156,    -1,   166,   159,    -1,    -1,    -1,   171,    -1,
      25,   174,   167,    -1,    -1,    -1,    -1,   172,   181,    -1,
     183,    -1,    -1,    -1,    -1,    -1,   154,    42,   156,    -1,
      -1,   159,    -1,    -1,    -1,    -1,     1,    -1,   139,   167,
      -1,     6,    -1,   144,   172,    10,    61,    -1,    63,    64,
     151,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,
      25,   162,    -1,   164,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   175,    -1,   177,    42,   179,    -1,
      -1,    -1,    -1,   184,   185,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,    -1,    -1,    -1,   111,   112,   113,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,   124,
      -1,    -1,    -1,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,
      -1,     8,     9,    10,    -1,   100,    -1,    -1,   103,   154,
      -1,   156,    -1,    20,   159,    22,    -1,   112,   113,    26,
      27,    -1,   167,    -1,    -1,    32,     4,   172,    -1,   124,
       8,    38,    10,   128,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    -1,    22,    -1,    -1,    54,    26,    27,
      57,    58,    59,    60,    32,    -1,    -1,    -1,    -1,   154,
      38,   156,    -1,    -1,   159,    -1,    -1,     4,    -1,    -1,
      -1,     8,   167,    10,    -1,    -1,    54,   172,    -1,    57,
      58,    59,    60,    20,    -1,    22,    -1,    -1,    -1,    26,
      27,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    38,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    -1,    54,    -1,    -1,
      57,    58,    59,    60,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,     4,     5,     6,     7,    -1,
       9,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      -1,    14,    -1,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,
      53,    54,    55,    56,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      50,    51,    52,    53,    54,    55,    56,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    54,    55,    56,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,
      51,    52,    53,    54,    55,    56,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    31,    -1,    -1,    -1,    -1,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    50,    51,    52,    53,    54,    55,    56,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,    51,
      52,    53,    54,    55,    56,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    16,    -1,    -1,    19,    -1,    21,    22,
      23,    -1,    25,    -1,    -1,    -1,    29,    30,    -1,    -1,
      -1,    34,    35,    -1,    -1,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    45,    46,    15,    16,    -1,    -1,    -1,    -1,
      21,    22,    23,    -1,    25,    58,    -1,    -1,    29,    30,
      -1,    -1,    33,    34,    35,    -1,    -1,    -1,    -1,     8,
      -1,    -1,    -1,    -1,    45,    46,    15,    16,    -1,    18,
      -1,    -1,    21,    22,    23,    -1,    25,    58,    -1,    -1,
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
      -1,    -1,    19,    -1,    21,    22,    23,    -1,    25,    58,
      -1,    -1,    29,    30,    -1,    -1,    -1,    34,    35,    -1,
      -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    45,    46,
      15,    16,    -1,    -1,    19,    -1,    21,    22,    23,    -1,
      25,    58,    -1,    -1,    29,    30,    -1,    -1,    -1,    34,
      35,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      45,    46,    15,    16,    -1,    -1,    -1,    -1,    21,    22,
      23,    -1,    25,    58,    -1,    -1,    29,    30,    -1,    -1,
      -1,    34,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    58
  };

  const signed char
  parser::yystos_[] =
  {
       0,     8,    15,    16,    21,    22,    23,    25,    29,    30,
      34,    35,    45,    46,    58,    62,    63,    64,    66,    67,
      70,    71,    75,    76,     4,    10,    20,    22,    26,    27,
      32,    38,    54,    57,    59,    60,    71,    74,    75,    76,
      78,    81,    86,    63,    58,    72,    58,    68,    69,    74,
      22,    58,    72,    63,    73,    74,    74,    58,    58,     0,
      64,     3,    48,     8,    12,    47,    49,    59,    77,    81,
      11,    12,    58,    74,    82,    83,     8,    79,     4,     5,
       6,     7,     9,    14,    28,    36,    37,    39,    40,    41,
      42,    43,    44,    50,    51,    52,    53,    54,    55,    56,
      85,    74,    19,     3,    24,    48,    47,    49,    79,    31,
      58,     3,    33,    48,    16,    45,    73,    71,     9,    73,
      74,    58,    58,    74,     3,    11,    46,    48,    84,     9,
      38,    72,    80,    74,    74,    73,    58,    58,    58,    63,
      79,    73,    74,    74,    63,     9,    13,    77,    13,    74,
      83,    63,    48,     9,    48,    16,    18,    65,    19,     3,
      19,    38,    63,    74,    63,    74,    17,    18,    19,    74,
      19,    16,    48,    19,    31,    63,    74,    63,    74,    63,
      19,    31,    19,    16,    63,    63,    19
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    61,    62,    63,    63,    63,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    65,    65,    65,    66,    66,
      67,    68,    68,    69,    69,    70,    70,    71,    71,    71,
      72,    72,    73,    73,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    75,    75,    75,    76,    76,
      77,    77,    77,    77,    78,    79,    79,    80,    80,    80,
      81,    81,    82,    82,    83,    83,    83,    84,    84,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      86,    86,    86,    86
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     1,     2,     1,     1,     3,     1,
       1,     1,     2,     3,     5,     4,     8,     6,     9,    11,
       7,     3,     4,     2,     4,     0,     4,     5,     1,     2,
       3,     1,     3,     1,     3,     1,     3,     1,     4,     3,
       1,     3,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     2,     1,     1,     3,     2,     4,
       3,     2,     1,     1,     2,     5,     4,     1,     3,     1,
       3,     2,     1,     3,     5,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1
  };




#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   140,   140,   148,   154,   166,   185,   191,   197,   216,
     222,   228,   234,   242,   248,   267,   286,   317,   342,   368,
     400,   425,   444,   458,   471,   492,   498,   517,   542,   550,
     565,   575,   583,   597,   610,   626,   639,   658,   667,   687,
     704,   712,   726,   739,   758,   766,   774,   782,   791,   800,
     808,   822,   836,   850,   876,   898,   912,   926,   941,   960,
     982,   996,  1004,  1018,  1029,  1044,  1063,  1078,  1091,  1105,
    1115,  1128,  1138,  1151,  1170,  1190,  1205,  1221,  1227,  1235,
    1243,  1251,  1259,  1267,  1275,  1283,  1291,  1299,  1307,  1315,
    1323,  1331,  1339,  1347,  1355,  1363,  1371,  1379,  1387,  1395,
    1405,  1413,  1421,  1429
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
#line 3357 "parser.cpp"

#line 1437 "parser.y"


void
yy::parser::error (const location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
