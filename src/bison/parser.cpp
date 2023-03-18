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

  case 9: // stmt: label
#line 217 "parser.y"
        {
            LOG(INFO) << "[bison]: stmt: " << "label";
            yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
        }
#line 983 "parser.cpp"
    break;

  case 10: // stmt: "break"
#line 223 "parser.y"
        {
            LOG(INFO) << "[bison]: stmt: " << "break";
            yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_break>(yystack_[0].location);
        }
#line 992 "parser.cpp"
    break;

  case 11: // stmt: "goto" "identifier"
#line 229 "parser.y"
        {
            LOG(INFO) << "[bison]: stmt: " << "goto IDENTIFIER";
            auto go = std::make_shared<fakelua::syntax_tree_goto>(yystack_[0].location);
            go->set_label(yystack_[0].value.as < std::string > ());
            yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = go;
        }
#line 1003 "parser.cpp"
    break;

  case 12: // stmt: "do" block "end"
#line 237 "parser.y"
        {
            LOG(INFO) << "[bison]: stmt: " << "do block end";
            yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
        }
#line 1012 "parser.cpp"
    break;

  case 13: // stmt: "while" exp "do" block "end"
#line 243 "parser.y"
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
#line 1034 "parser.cpp"
    break;

  case 14: // stmt: "repeat" block "until" exp
#line 262 "parser.y"
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
#line 1056 "parser.cpp"
    break;

  case 15: // stmt: "if" exp "then" block elseifs "else" block "end"
#line 281 "parser.y"
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
#line 1090 "parser.cpp"
    break;

  case 16: // stmt: "if" exp "then" block elseifs "end"
#line 312 "parser.y"
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
#line 1118 "parser.cpp"
    break;

  case 17: // stmt: "for" "identifier" "=" exp "," exp "do" block "end"
#line 337 "parser.y"
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
#line 1147 "parser.cpp"
    break;

  case 18: // stmt: "for" "identifier" "=" exp "," exp "," exp "do" block "end"
#line 363 "parser.y"
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
#line 1182 "parser.cpp"
    break;

  case 19: // stmt: "for" namelist "in" explist "do" block "end"
#line 395 "parser.y"
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
#line 1210 "parser.cpp"
    break;

  case 20: // stmt: "function" funcname funcbody
#line 420 "parser.y"
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
#line 1232 "parser.cpp"
    break;

  case 21: // stmt: "local" "function" "identifier" funcbody
#line 439 "parser.y"
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
#line 1249 "parser.cpp"
    break;

  case 22: // stmt: "local" namelist
#line 453 "parser.y"
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
#line 1265 "parser.cpp"
    break;

  case 23: // stmt: "local" namelist "=" explist
#line 466 "parser.y"
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
#line 1287 "parser.cpp"
    break;

  case 24: // elseifs: %empty
#line 487 "parser.y"
    {
        LOG(INFO) << "[bison]: elseifs: " << "empty";
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[0].location);
    }
#line 1296 "parser.cpp"
    break;

  case 25: // elseifs: "elseif" exp "then" block
#line 493 "parser.y"
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
#line 1318 "parser.cpp"
    break;

  case 26: // elseifs: elseifs "elseif" exp "then" block
#line 512 "parser.y"
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
#line 1344 "parser.cpp"
    break;

  case 27: // retstat: "return"
#line 537 "parser.y"
    {
        LOG(INFO) << "[bison]: retstat: " << "RETURN";
        auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[0].location);
        ret->set_explist(nullptr);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1355 "parser.cpp"
    break;

  case 28: // retstat: "return" explist
#line 545 "parser.y"
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
#line 1371 "parser.cpp"
    break;

  case 29: // label: "::" "identifier" "::"
#line 560 "parser.y"
    {
        LOG(INFO) << "[bison]: bison get label: " << yystack_[1].value.as < std::string > () << " loc: " << yystack_[1].location;
        auto ret = std::make_shared<fakelua::syntax_tree_label>(yystack_[1].location);
        ret->set_name(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1382 "parser.cpp"
    break;

  case 30: // funcnamelist: "identifier"
#line 570 "parser.y"
    {
        LOG(INFO) << "[bison]: funcnamelist: " << "IDENTIFIER";
        auto funcnamelist = std::make_shared<fakelua::syntax_tree_funcnamelist>(yystack_[0].location);
        funcnamelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcnamelist;
    }
#line 1393 "parser.cpp"
    break;

  case 31: // funcnamelist: funcnamelist "." "identifier"
#line 578 "parser.y"
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
#line 1408 "parser.cpp"
    break;

  case 32: // funcname: funcnamelist
#line 592 "parser.y"
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
#line 1424 "parser.cpp"
    break;

  case 33: // funcname: funcnamelist ":" "identifier"
#line 605 "parser.y"
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
#line 1441 "parser.cpp"
    break;

  case 34: // varlist: var
#line 621 "parser.y"
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
#line 1457 "parser.cpp"
    break;

  case 35: // varlist: varlist "," var
#line 634 "parser.y"
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
#line 1477 "parser.cpp"
    break;

  case 36: // var: "identifier"
#line 653 "parser.y"
    {
        LOG(INFO) << "[bison]: var: " << "IDENTIFIER";
        auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[0].location);
        var->set_name(yystack_[0].value.as < std::string > ());
        var->set_type("simple");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
    }
#line 1489 "parser.cpp"
    break;

  case 37: // var: prefixexp "[" exp "]"
#line 662 "parser.y"
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
#line 1512 "parser.cpp"
    break;

  case 38: // var: prefixexp "." "identifier"
#line 682 "parser.y"
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
#line 1530 "parser.cpp"
    break;

  case 39: // namelist: "identifier"
#line 699 "parser.y"
    {
        LOG(INFO) << "[bison]: namelist: " << "IDENTIFIER";
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(yystack_[0].location);
        namelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1541 "parser.cpp"
    break;

  case 40: // namelist: namelist "," "identifier"
#line 707 "parser.y"
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
#line 1556 "parser.cpp"
    break;

  case 41: // explist: exp
#line 721 "parser.y"
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
#line 1572 "parser.cpp"
    break;

  case 42: // explist: explist "," exp
#line 734 "parser.y"
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
#line 1592 "parser.cpp"
    break;

  case 43: // exp: "nil"
#line 753 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "NIL";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("nil");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1603 "parser.cpp"
    break;

  case 44: // exp: "true"
#line 761 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "TRUE";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("true");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1614 "parser.cpp"
    break;

  case 45: // exp: "false"
#line 769 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "FALSES";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("false");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1625 "parser.cpp"
    break;

  case 46: // exp: "number"
#line 777 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "NUMBER";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("number");
        exp->set_value(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1637 "parser.cpp"
    break;

  case 47: // exp: "string"
#line 786 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "STRING";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("string");
        exp->set_value(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1649 "parser.cpp"
    break;

  case 48: // exp: "..."
#line 795 "parser.y"
    {
        LOG(INFO) << "[bison]: exp: " << "VAR_PARAMS";
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("var_params");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1660 "parser.cpp"
    break;

  case 49: // exp: functiondef
#line 803 "parser.y"
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
#line 1677 "parser.cpp"
    break;

  case 50: // exp: prefixexp
#line 817 "parser.y"
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
#line 1694 "parser.cpp"
    break;

  case 51: // exp: tableconstructor
#line 831 "parser.y"
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
#line 1711 "parser.cpp"
    break;

  case 52: // exp: exp binop exp
#line 845 "parser.y"
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
#line 1740 "parser.cpp"
    break;

  case 53: // exp: unop exp
#line 871 "parser.y"
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
#line 1763 "parser.cpp"
    break;

  case 54: // prefixexp: var
#line 893 "parser.y"
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
#line 1780 "parser.cpp"
    break;

  case 55: // prefixexp: functioncall
#line 907 "parser.y"
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
#line 1797 "parser.cpp"
    break;

  case 56: // prefixexp: "(" exp ")"
#line 921 "parser.y"
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
#line 1814 "parser.cpp"
    break;

  case 57: // functioncall: prefixexp args
#line 936 "parser.y"
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
#line 1836 "parser.cpp"
    break;

  case 58: // functioncall: prefixexp ":" "identifier" args
#line 955 "parser.y"
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
#line 1859 "parser.cpp"
    break;

  case 59: // args: "(" explist ")"
#line 977 "parser.y"
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
#line 1876 "parser.cpp"
    break;

  case 60: // args: "(" ")"
#line 991 "parser.y"
    {
        LOG(INFO) << "[bison]: args: " << "LPAREN RPAREN";
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[1].location);
        args->set_type("empty");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1887 "parser.cpp"
    break;

  case 61: // args: tableconstructor
#line 999 "parser.y"
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
#line 1904 "parser.cpp"
    break;

  case 62: // args: "string"
#line 1013 "parser.y"
    {
        LOG(INFO) << "[bison]: args: " << "STRING";
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[0].location);
        args->set_string(yystack_[0].value.as < std::string > ());
        args->set_type("string");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 1916 "parser.cpp"
    break;

  case 63: // functiondef: "function" funcbody
#line 1024 "parser.y"
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
#line 1932 "parser.cpp"
    break;

  case 64: // funcbody: "(" parlist ")" block "end"
#line 1039 "parser.y"
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
#line 1954 "parser.cpp"
    break;

  case 65: // funcbody: "(" ")" block "end"
#line 1058 "parser.y"
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
#line 1970 "parser.cpp"
    break;

  case 66: // parlist: namelist
#line 1073 "parser.y"
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
#line 1986 "parser.cpp"
    break;

  case 67: // parlist: namelist "," "..."
#line 1086 "parser.y"
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
#line 2003 "parser.cpp"
    break;

  case 68: // parlist: "..."
#line 1100 "parser.y"
    {
        LOG(INFO) << "[bison]: parlist: " << "VAR_PARAMS";
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[0].location);
        parlist->set_var_params(true);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
    }
#line 2014 "parser.cpp"
    break;

  case 69: // tableconstructor: "{" fieldlist "}"
#line 1110 "parser.y"
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
#line 2030 "parser.cpp"
    break;

  case 70: // tableconstructor: "{" "}"
#line 1123 "parser.y"
    {
        LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY RCURLY";
        auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[1].location);
        tableconstructor->set_fieldlist(std::make_shared<fakelua::syntax_tree_empty>(yystack_[1].location));
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
    }
#line 2041 "parser.cpp"
    break;

  case 71: // fieldlist: field
#line 1133 "parser.y"
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
#line 2057 "parser.cpp"
    break;

  case 72: // fieldlist: fieldlist fieldsep field
#line 1146 "parser.y"
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
#line 2077 "parser.cpp"
    break;

  case 73: // field: "[" exp "]" "=" exp
#line 1165 "parser.y"
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
#line 2100 "parser.cpp"
    break;

  case 74: // field: "identifier" "=" exp
#line 1185 "parser.y"
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
#line 2118 "parser.cpp"
    break;

  case 75: // field: exp
#line 1200 "parser.y"
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
#line 2135 "parser.cpp"
    break;

  case 76: // fieldsep: ","
#line 1216 "parser.y"
    {
        LOG(INFO) << "[bison]: fieldsep: " << "COMMA";
        // nothing to do
    }
#line 2144 "parser.cpp"
    break;

  case 77: // fieldsep: ";"
#line 1222 "parser.y"
    {
        LOG(INFO) << "[bison]: fieldsep: " << "SEMICOLON";
        // nothing to do
    }
#line 2153 "parser.cpp"
    break;

  case 78: // binop: "+"
#line 1230 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "PLUS";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("PLUS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2164 "parser.cpp"
    break;

  case 79: // binop: "-"
#line 1238 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "MINUS";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MINUS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2175 "parser.cpp"
    break;

  case 80: // binop: "*"
#line 1246 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "STAR";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("STAR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2186 "parser.cpp"
    break;

  case 81: // binop: "/"
#line 1254 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "SLASH";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("SLASH");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2197 "parser.cpp"
    break;

  case 82: // binop: "//"
#line 1262 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "DOUBLE_SLASH";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("DOUBLE_SLASH");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2208 "parser.cpp"
    break;

  case 83: // binop: "^"
#line 1270 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "XOR";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("XOR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2219 "parser.cpp"
    break;

  case 84: // binop: "%"
#line 1278 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "MOD";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MOD");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2230 "parser.cpp"
    break;

  case 85: // binop: "&"
#line 1286 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "BITAND";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("BITAND");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2241 "parser.cpp"
    break;

  case 86: // binop: "~"
#line 1294 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "BITNOT";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("BITNOT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2252 "parser.cpp"
    break;

  case 87: // binop: "|"
#line 1302 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "BITOR";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("BITOR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2263 "parser.cpp"
    break;

  case 88: // binop: ">>"
#line 1310 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "RIGHT_SHIFT";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("RIGHT_SHIFT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2274 "parser.cpp"
    break;

  case 89: // binop: "<<"
#line 1318 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "LEFT_SHIFT";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("LEFT_SHIFT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2285 "parser.cpp"
    break;

  case 90: // binop: ".."
#line 1326 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "CONCAT";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("CONCAT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2296 "parser.cpp"
    break;

  case 91: // binop: "<"
#line 1334 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "LESS";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("LESS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2307 "parser.cpp"
    break;

  case 92: // binop: "<="
#line 1342 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "LESS_EQUAL";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("LESS_EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2318 "parser.cpp"
    break;

  case 93: // binop: ">"
#line 1350 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "MORE";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MORE");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2329 "parser.cpp"
    break;

  case 94: // binop: ">="
#line 1358 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "MORE_EQUAL";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("MORE_EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2340 "parser.cpp"
    break;

  case 95: // binop: "=="
#line 1366 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "EQUAL";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2351 "parser.cpp"
    break;

  case 96: // binop: "~="
#line 1374 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "NOT_EQUAL";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("NOT_EQUAL");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2362 "parser.cpp"
    break;

  case 97: // binop: "and"
#line 1382 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "AND";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("AND");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2373 "parser.cpp"
    break;

  case 98: // binop: "or"
#line 1390 "parser.y"
    {
        LOG(INFO) << "[bison]: binop: " << "OR";
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[0].location);
        binop->set_op("OR");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = binop;
    }
#line 2384 "parser.cpp"
    break;

  case 99: // unop: "-"
#line 1400 "parser.y"
    {
        LOG(INFO) << "[bison]: unop: " << "MINUS";
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("MINUS");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2395 "parser.cpp"
    break;

  case 100: // unop: "not"
#line 1408 "parser.y"
    {
        LOG(INFO) << "[bison]: unop: " << "NOT";
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("NOT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2406 "parser.cpp"
    break;

  case 101: // unop: "#"
#line 1416 "parser.y"
    {
        LOG(INFO) << "[bison]: unop: " << "NUMBER_SIGN";
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("NUMBER_SIGN");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2417 "parser.cpp"
    break;

  case 102: // unop: "~"
#line 1424 "parser.y"
    {
        LOG(INFO) << "[bison]: unop: " << "BITNOT";
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[0].location);
        unop->set_op("BITNOT");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = unop;
    }
#line 2428 "parser.cpp"
    break;


#line 2432 "parser.cpp"

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


  const signed char parser::yypact_ninf_ = -46;

  const signed char parser::yytable_ninf_ = -36;

  const short
  parser::yypact_[] =
  {
    1275,   319,   -46,  1275,   -32,   -30,   319,    -2,  1275,   319,
     319,   -29,   -18,   -46,   -46,    45,  1275,   -46,   -46,   -46,
      13,    15,     0,   -46,   -46,   236,   -46,    44,   -46,   -46,
     -46,   -46,   -46,   -46,   -46,   -46,   -46,   429,     0,   -46,
     -46,   319,   955,    50,     3,   -46,   -10,    44,   482,    20,
     -46,    28,   987,    12,   906,   535,   -46,    34,   -46,   -46,
     319,    14,   277,   319,    22,    23,   -46,   -46,   -46,   -46,
     319,    79,   906,     2,   -46,    16,   -46,   -46,   -46,   -46,
     -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,
     -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   319,
     906,   -46,   319,   319,    25,    26,    27,   -46,  1275,    44,
     319,   319,   319,  1275,   -46,    12,    29,   -46,    -5,   588,
      -1,   -46,   641,   319,   -46,   -46,   -46,   306,  1275,   -46,
      38,    78,   906,   694,     7,   -46,   -46,   -46,  1019,   -46,
      12,   906,   906,  1051,   -46,   -46,   -46,    85,   906,   -46,
    1083,   -14,  1275,   319,  1275,   319,    17,   -46,   319,   -46,
     -46,  1115,   376,  1147,   747,  1275,   319,   -46,   906,   -46,
    1275,   319,   -46,  1275,  1179,   800,  1211,   853,  1275,   -46,
    1275,   -46,  1275,  1275,  1243,   -46
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,    10,     3,     0,     0,     0,     0,     3,    27,
       0,     0,     0,     7,    36,     0,     2,     4,     6,     9,
       0,    54,     0,    55,    99,     0,    45,     0,    43,   100,
      44,    48,   102,   101,    47,    46,    54,     0,    50,    49,
      51,     0,     0,    39,     0,    30,    32,     0,     0,     0,
      39,    22,     0,    28,    41,     0,    11,     0,     1,     5,
       0,     0,     0,     0,     0,     0,    62,    57,    61,    70,
       0,    36,    75,     0,    71,     0,    63,    79,    78,    80,
      81,    56,    97,    98,    82,    90,    95,    94,    92,    96,
      89,    88,    83,    84,    85,    87,    86,    93,    91,     0,
      53,    12,     0,     0,     0,     0,     0,    20,     3,     0,
       0,     0,     0,     3,    29,     8,    54,    60,     0,     0,
       0,    38,     0,     0,    69,    77,    76,     0,     3,    68,
      66,     0,    52,     0,     0,    40,    33,    31,    24,    21,
      23,    14,    42,     0,    59,    37,    58,     0,    74,    72,
       0,     0,     3,     0,     3,     0,     0,    13,     0,    65,
      67,     0,     0,     0,     0,     3,     0,    16,    73,    64,
       3,     0,    19,     3,     0,     0,     0,     0,    25,    15,
       3,    17,     3,    26,     0,    18
  };

  const signed char
  parser::yypgoto_[] =
  {
     -46,   -46,    -3,    54,   -46,   -46,   -46,   -46,   -46,   -46,
       5,    -4,   -41,   112,    32,   -46,   -31,   -46,   -45,   -46,
     -21,   -46,   -37,   -46,   -46,   -46
  };

  const unsigned char
  parser::yydefgoto_[] =
  {
       0,    15,    16,    17,   156,    18,    19,    46,    47,    20,
      21,    44,    53,    54,    22,    23,    67,    39,    76,   131,
      40,    73,    74,   127,    99,    41
  };

  const short
  parser::yytable_[] =
  {
      42,    68,   107,    51,   144,    52,    36,    62,    62,    25,
      25,    36,    63,   124,    36,    36,    60,    68,   -34,   115,
      49,   118,     1,   154,   160,   128,    43,   103,    45,    56,
      36,   110,   -35,    38,   165,   166,   167,   105,    38,   106,
      57,    38,    38,   112,   135,    58,    36,    64,   125,    65,
     126,   104,    75,   102,   129,   112,    50,    38,    66,    66,
     112,    61,   134,   -34,   139,    36,   116,    36,    36,   140,
      59,   130,    14,    38,    50,    36,   104,   -35,   109,   114,
     120,   121,   123,   135,   136,   137,   151,   152,   158,   146,
     149,     0,    38,     0,    38,    38,    59,     0,     0,    68,
       0,     0,    38,     0,    36,   138,    59,    36,    36,     0,
     143,     0,     0,    37,     0,    36,    36,    36,    48,     0,
       0,     0,    55,     0,     0,   150,     0,     0,    36,     0,
       0,    38,    36,     0,    38,    38,     0,    72,     0,     0,
       0,     0,    38,    38,    38,     0,     0,     0,     0,   161,
       0,   163,     0,   100,     0,    38,     0,     0,    36,    38,
      36,     0,   174,    36,     0,     0,     0,   176,     0,     0,
     178,    36,     0,     0,     0,   119,    36,   183,     0,   184,
       0,     0,   122,     0,     0,    38,     0,    38,     0,     0,
      38,     0,    59,     0,     0,     0,     0,    59,    38,     0,
       0,     0,     0,    38,    59,     0,     0,     0,     0,     0,
       0,   132,     0,     0,   133,    59,     0,    59,     0,     0,
       0,     0,     0,   141,   142,     0,     0,     0,    59,     0,
      59,     0,    59,     0,     0,   148,     0,    59,    59,    72,
      24,     0,     0,     0,     1,     0,    25,    69,    70,     0,
       0,     0,     0,     0,     0,     0,    26,     0,    27,     0,
       0,     0,    28,    29,     0,   162,     0,   164,    30,     0,
     168,     0,     0,     0,    31,     0,     0,     0,   175,     0,
       0,    24,     0,   177,     0,     1,   117,    25,     0,     0,
      32,     0,     0,    33,    71,    34,    35,    26,     0,    27,
       0,     0,     0,    28,    29,     0,     0,     0,     0,    30,
      24,     0,     0,     0,     1,    31,    25,     0,    70,     0,
       0,     0,     0,    24,     0,     0,    26,     1,    27,    25,
       0,    32,    28,    29,    33,    14,    34,    35,    30,    26,
       0,    27,     0,     0,    31,    28,    29,     0,     0,     0,
       0,    30,     0,     0,     0,     0,     0,    31,     0,     0,
      32,     0,     0,    33,    71,    34,    35,     0,     0,     0,
       0,     0,     0,    32,     0,     0,    33,    14,    34,    35,
      77,    78,    79,    80,     0,     0,     0,     0,     0,     0,
      82,     0,   170,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,     0,
       0,     0,    84,    85,     0,    86,    87,    88,    89,    90,
      91,     0,     0,     0,   171,     0,    92,    93,    94,    95,
      96,    97,    98,    77,    78,    79,    80,     0,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,     0,     0,     0,    84,    85,     0,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,     0,    92,
      93,    94,    95,    96,    97,    98,    77,    78,    79,    80,
       0,     0,     0,     0,     0,     0,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,     0,     0,   108,     0,     0,     0,     0,    84,    85,
       0,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,    96,    97,    98,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,    82,
       0,   113,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,     0,     0,
       0,    84,    85,     0,    86,    87,    88,    89,    90,    91,
       0,     0,     0,     0,     0,    92,    93,    94,    95,    96,
      97,    98,    77,    78,    79,    80,     0,     0,     0,     0,
       0,   145,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,     0,     0,     0,    84,    85,     0,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,    98,    77,    78,    79,    80,     0,
       0,     0,     0,     0,   147,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,     0,     0,     0,    84,    85,     0,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
       0,    92,    93,    94,    95,    96,    97,    98,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,     0,     0,     0,
      84,    85,     0,    86,    87,    88,    89,    90,    91,     0,
       0,     0,   153,     0,    92,    93,    94,    95,    96,    97,
      98,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,    82,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    83,     0,     0,   173,     0,
       0,     0,     0,    84,    85,     0,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,     0,    92,    93,    94,
      95,    96,    97,    98,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,     0,
       0,   180,     0,     0,     0,     0,    84,    85,     0,    86,
      87,    88,    89,    90,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,    96,    97,    98,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,    82,     0,   182,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,     0,     0,     0,    84,
      85,     0,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,     0,    92,    93,    94,    95,    96,    97,    98,
      77,    78,    79,    80,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,     0,
       0,     0,    84,    85,     0,    86,    87,    88,    89,    90,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
      96,    97,    98,     1,     0,     0,     0,     0,     0,     0,
       2,     3,     0,     0,   101,     0,     4,     5,     6,     0,
       7,     0,     0,     0,     8,     9,     0,     0,     0,    10,
      11,     0,     0,     0,     0,     1,     0,     0,     0,     0,
      12,    13,     2,     3,     0,     0,     0,     0,     4,     5,
       6,     0,     7,    14,     0,     0,     8,     9,     0,     0,
     111,    10,    11,     0,     0,     0,     0,     1,     0,     0,
       0,     0,    12,    13,     2,     3,     0,   155,     0,     0,
       4,     5,     6,     0,     7,    14,     0,     0,     8,     9,
       0,     0,     0,    10,    11,     0,     0,     0,     0,     1,
       0,     0,     0,     0,    12,    13,     2,     3,     0,     0,
     157,     0,     4,     5,     6,     0,     7,    14,     0,     0,
       8,     9,     0,     0,     0,    10,    11,     0,     0,     0,
       0,     1,     0,     0,     0,     0,    12,    13,     2,     3,
       0,     0,   159,     0,     4,     5,     6,     0,     7,    14,
       0,     0,     8,     9,     0,     0,     0,    10,    11,     0,
       0,     0,     0,     1,     0,     0,     0,     0,    12,    13,
       2,     3,     0,     0,   169,     0,     4,     5,     6,     0,
       7,    14,     0,     0,     8,     9,     0,     0,     0,    10,
      11,     0,     0,     0,     0,     1,     0,     0,     0,     0,
      12,    13,     2,     3,     0,     0,   172,     0,     4,     5,
       6,     0,     7,    14,     0,     0,     8,     9,     0,     0,
       0,    10,    11,     0,     0,     0,     0,     1,     0,     0,
       0,     0,    12,    13,     2,     3,     0,     0,   179,     0,
       4,     5,     6,     0,     7,    14,     0,     0,     8,     9,
       0,     0,     0,    10,    11,     0,     0,     0,     0,     1,
       0,     0,     0,     0,    12,    13,     2,     3,     0,     0,
     181,     0,     4,     5,     6,     0,     7,    14,     0,     0,
       8,     9,     0,     0,     0,    10,    11,     0,     0,     0,
       0,     1,     0,     0,     0,     0,    12,    13,     2,     3,
       0,     0,   185,     0,     4,     5,     6,     0,     7,    14,
       0,     0,     8,     9,     0,     0,     0,    10,    11,     0,
       0,     0,     0,     1,     0,     0,     0,     0,    12,    13,
       2,     3,     0,     0,     0,     0,     4,     5,     6,     0,
       7,    14,     0,     0,     8,     9,     0,     0,     0,    10,
      11,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14
  };

  const short
  parser::yycheck_[] =
  {
       3,    22,    47,     7,     9,     8,     1,     8,     8,    10,
      10,     6,    12,    11,     9,    10,     3,    38,     3,    60,
      22,    62,     8,    16,    38,     9,    58,    24,    58,    58,
      25,     3,     3,     1,    17,    18,    19,    47,     6,    49,
      58,     9,    10,    48,    58,     0,    41,    47,    46,    49,
      48,    48,     8,     3,    38,    48,    58,    25,    59,    59,
      48,    48,   103,    48,   109,    60,    61,    62,    63,   110,
      16,    75,    58,    41,    58,    70,    48,    48,    58,    45,
      58,    58,     3,    58,    58,    58,    48,     9,     3,   120,
     127,    -1,    60,    -1,    62,    63,    42,    -1,    -1,   120,
      -1,    -1,    70,    -1,    99,   108,    52,   102,   103,    -1,
     113,    -1,    -1,     1,    -1,   110,   111,   112,     6,    -1,
      -1,    -1,    10,    -1,    -1,   128,    -1,    -1,   123,    -1,
      -1,    99,   127,    -1,   102,   103,    -1,    25,    -1,    -1,
      -1,    -1,   110,   111,   112,    -1,    -1,    -1,    -1,   152,
      -1,   154,    -1,    41,    -1,   123,    -1,    -1,   153,   127,
     155,    -1,   165,   158,    -1,    -1,    -1,   170,    -1,    -1,
     173,   166,    -1,    -1,    -1,    63,   171,   180,    -1,   182,
      -1,    -1,    70,    -1,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   138,    -1,    -1,    -1,    -1,   143,   166,    -1,
      -1,    -1,    -1,   171,   150,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,   102,   161,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   111,   112,    -1,    -1,    -1,   174,    -1,
     176,    -1,   178,    -1,    -1,   123,    -1,   183,   184,   127,
       4,    -1,    -1,    -1,     8,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,    22,    -1,
      -1,    -1,    26,    27,    -1,   153,    -1,   155,    32,    -1,
     158,    -1,    -1,    -1,    38,    -1,    -1,    -1,   166,    -1,
      -1,     4,    -1,   171,    -1,     8,     9,    10,    -1,    -1,
      54,    -1,    -1,    57,    58,    59,    60,    20,    -1,    22,
      -1,    -1,    -1,    26,    27,    -1,    -1,    -1,    -1,    32,
       4,    -1,    -1,    -1,     8,    38,    10,    -1,    12,    -1,
      -1,    -1,    -1,     4,    -1,    -1,    20,     8,    22,    10,
      -1,    54,    26,    27,    57,    58,    59,    60,    32,    20,
      -1,    22,    -1,    -1,    38,    26,    27,    -1,    -1,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      54,    -1,    -1,    57,    58,    59,    60,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    -1,    -1,    57,    58,    59,    60,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,     4,     5,     6,     7,    -1,     9,    -1,
      -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
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
      -1,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,    51,
      52,    53,    54,    55,    56,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    31,    -1,
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
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    16,    -1,    -1,    19,    -1,    21,    22,    23,    -1,
      25,    -1,    -1,    -1,    29,    30,    -1,    -1,    -1,    34,
      35,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      45,    46,    15,    16,    -1,    -1,    -1,    -1,    21,    22,
      23,    -1,    25,    58,    -1,    -1,    29,    30,    -1,    -1,
      33,    34,    35,    -1,    -1,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    45,    46,    15,    16,    -1,    18,    -1,    -1,
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
      -1,    -1,    19,    -1,    21,    22,    23,    -1,    25,    58,
      -1,    -1,    29,    30,    -1,    -1,    -1,    34,    35,    -1,
      -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    45,    46,
      15,    16,    -1,    -1,    -1,    -1,    21,    22,    23,    -1,
      25,    58,    -1,    -1,    29,    30,    -1,    -1,    -1,    34,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    58
  };

  const signed char
  parser::yystos_[] =
  {
       0,     8,    15,    16,    21,    22,    23,    25,    29,    30,
      34,    35,    45,    46,    58,    62,    63,    64,    66,    67,
      70,    71,    75,    76,     4,    10,    20,    22,    26,    27,
      32,    38,    54,    57,    59,    60,    71,    74,    75,    78,
      81,    86,    63,    58,    72,    58,    68,    69,    74,    22,
      58,    72,    63,    73,    74,    74,    58,    58,     0,    64,
       3,    48,     8,    12,    47,    49,    59,    77,    81,    11,
      12,    58,    74,    82,    83,     8,    79,     4,     5,     6,
       7,     9,    14,    28,    36,    37,    39,    40,    41,    42,
      43,    44,    50,    51,    52,    53,    54,    55,    56,    85,
      74,    19,     3,    24,    48,    47,    49,    79,    31,    58,
       3,    33,    48,    16,    45,    73,    71,     9,    73,    74,
      58,    58,    74,     3,    11,    46,    48,    84,     9,    38,
      72,    80,    74,    74,    73,    58,    58,    58,    63,    79,
      73,    74,    74,    63,     9,    13,    77,    13,    74,    83,
      63,    48,     9,    48,    16,    18,    65,    19,     3,    19,
      38,    63,    74,    63,    74,    17,    18,    19,    74,    19,
      16,    48,    19,    31,    63,    74,    63,    74,    63,    19,
      31,    19,    16,    63,    63,    19
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    61,    62,    63,    63,    63,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    65,    65,    65,    66,    66,    67,
      68,    68,    69,    69,    70,    70,    71,    71,    71,    72,
      72,    73,    73,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    75,    75,    75,    76,    76,    77,
      77,    77,    77,    78,    79,    79,    80,    80,    80,    81,
      81,    82,    82,    83,    83,    83,    84,    84,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    86,
      86,    86,    86
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     1,     2,     1,     1,     3,     1,
       1,     2,     3,     5,     4,     8,     6,     9,    11,     7,
       3,     4,     2,     4,     0,     4,     5,     1,     2,     3,
       1,     3,     1,     3,     1,     3,     1,     4,     3,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     3,     2,     4,     3,
       2,     1,     1,     2,     5,     4,     1,     3,     1,     3,
       2,     1,     3,     5,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1
  };




#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   140,   140,   148,   154,   166,   185,   191,   197,   216,
     222,   228,   236,   242,   261,   280,   311,   336,   362,   394,
     419,   438,   452,   465,   486,   492,   511,   536,   544,   559,
     569,   577,   591,   604,   620,   633,   652,   661,   681,   698,
     706,   720,   733,   752,   760,   768,   776,   785,   794,   802,
     816,   830,   844,   870,   892,   906,   920,   935,   954,   976,
     990,   998,  1012,  1023,  1038,  1057,  1072,  1085,  1099,  1109,
    1122,  1132,  1145,  1164,  1184,  1199,  1215,  1221,  1229,  1237,
    1245,  1253,  1261,  1269,  1277,  1285,  1293,  1301,  1309,  1317,
    1325,  1333,  1341,  1349,  1357,  1365,  1373,  1381,  1389,  1399,
    1407,  1415,  1423
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
#line 3346 "parser.cpp"

#line 1431 "parser.y"


void
yy::parser::error (const location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
