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
      case symbol_kind::S_NUMBER: // "number"
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

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
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
        value.YY_MOVE_OR_COPY< fakelua::syntax_tree_interface_ptr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
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
      case symbol_kind::S_NUMBER: // "number"
        value.move< double > (YY_MOVE (that.value));
        break;

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
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
        value.move< fakelua::syntax_tree_interface_ptr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
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
      case symbol_kind::S_NUMBER: // "number"
        value.copy< double > (that.value);
        break;

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
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
        value.copy< fakelua::syntax_tree_interface_ptr > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
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
      case symbol_kind::S_NUMBER: // "number"
        value.move< double > (that.value);
        break;

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
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
        value.move< fakelua::syntax_tree_interface_ptr > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
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
#line 131 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 435 "parser.cpp"
        break;

      case symbol_kind::S_STRING: // "string"
#line 131 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 441 "parser.cpp"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 131 "parser.y"
                 { yyo << yysym.value.template as < double > (); }
#line 447 "parser.cpp"
        break;

      case symbol_kind::S_chunk: // chunk
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 453 "parser.cpp"
        break;

      case symbol_kind::S_block: // block
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 459 "parser.cpp"
        break;

      case symbol_kind::S_stmt: // stmt
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 465 "parser.cpp"
        break;

      case symbol_kind::S_elseifs: // elseifs
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 471 "parser.cpp"
        break;

      case symbol_kind::S_retstat: // retstat
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 477 "parser.cpp"
        break;

      case symbol_kind::S_label: // label
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 483 "parser.cpp"
        break;

      case symbol_kind::S_funcnamelist: // funcnamelist
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 489 "parser.cpp"
        break;

      case symbol_kind::S_funcname: // funcname
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 495 "parser.cpp"
        break;

      case symbol_kind::S_varlist: // varlist
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 501 "parser.cpp"
        break;

      case symbol_kind::S_var: // var
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 507 "parser.cpp"
        break;

      case symbol_kind::S_namelist: // namelist
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 513 "parser.cpp"
        break;

      case symbol_kind::S_explist: // explist
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 519 "parser.cpp"
        break;

      case symbol_kind::S_exp: // exp
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 525 "parser.cpp"
        break;

      case symbol_kind::S_prefixexp: // prefixexp
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 531 "parser.cpp"
        break;

      case symbol_kind::S_functioncall: // functioncall
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 537 "parser.cpp"
        break;

      case symbol_kind::S_args: // args
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 543 "parser.cpp"
        break;

      case symbol_kind::S_funcbody: // funcbody
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 549 "parser.cpp"
        break;

      case symbol_kind::S_parlist: // parlist
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 555 "parser.cpp"
        break;

      case symbol_kind::S_tableconstructor: // tableconstructor
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 561 "parser.cpp"
        break;

      case symbol_kind::S_fieldlist: // fieldlist
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 567 "parser.cpp"
        break;

      case symbol_kind::S_field: // field
#line 131 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 573 "parser.cpp"
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
      case symbol_kind::S_NUMBER: // "number"
        yylhs.value.emplace< double > ();
        break;

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
      case symbol_kind::S_funcbody: // funcbody
      case symbol_kind::S_parlist: // parlist
      case symbol_kind::S_tableconstructor: // tableconstructor
      case symbol_kind::S_fieldlist: // fieldlist
      case symbol_kind::S_field: // field
        yylhs.value.emplace< fakelua::syntax_tree_interface_ptr > ();
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
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
#line 138 "parser.y"
        {
  		LOG(INFO) << "[bison]: chunk: " << "block";
  		l->set_chunk(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
	}
#line 872 "parser.cpp"
    break;

  case 3: // block: %empty
#line 146 "parser.y"
        {
  		LOG(INFO) << "[bison]: block: " << "empty";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_block>(yystack_[0].location);
  	}
#line 881 "parser.cpp"
    break;

  case 4: // block: stmt
#line 152 "parser.y"
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
#line 896 "parser.cpp"
    break;

  case 5: // block: block stmt
#line 164 "parser.y"
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
#line 916 "parser.cpp"
    break;

  case 6: // stmt: retstat
#line 183 "parser.y"
        {
        	LOG(INFO) << "[bison]: stmt: " << "retstat";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
        }
#line 925 "parser.cpp"
    break;

  case 7: // stmt: ";"
#line 189 "parser.y"
        {
		LOG(INFO) << "[bison]: stmt: " << "SEMICOLON";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_empty>(yystack_[0].location);
	}
#line 934 "parser.cpp"
    break;

  case 8: // stmt: varlist "=" explist
#line 195 "parser.y"
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
#line 956 "parser.cpp"
    break;

  case 9: // stmt: label
#line 214 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "label";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
  	}
#line 965 "parser.cpp"
    break;

  case 10: // stmt: "break"
#line 220 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "break";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_break>(yystack_[0].location);
  	}
#line 974 "parser.cpp"
    break;

  case 11: // stmt: "goto" "identifier"
#line 226 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "goto IDENTIFIER";
  		auto go = std::make_shared<fakelua::syntax_tree_goto>(yystack_[0].location);
  		go->set_label(yystack_[0].value.as < std::string > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = go;
  	}
#line 985 "parser.cpp"
    break;

  case 12: // stmt: "do" block "end"
#line 234 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "do block end";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
  	}
#line 994 "parser.cpp"
    break;

  case 13: // stmt: "while" exp "do" block "end"
#line 240 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "while exp do block end";
  		auto while_stmt = std::make_shared<fakelua::syntax_tree_while>(yystack_[4].location);
  		while_stmt->set_exp(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
  		while_stmt->set_block(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = while_stmt;
  	}
#line 1006 "parser.cpp"
    break;

  case 14: // stmt: "repeat" block "until" exp
#line 249 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "repeat block until exp";
  		auto repeat = std::make_shared<fakelua::syntax_tree_repeat>(yystack_[3].location);
  		repeat->set_block(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
  		repeat->set_exp(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = repeat;
  	}
#line 1018 "parser.cpp"
    break;

  case 15: // stmt: "if" exp "then" block elseifs "else" block "end"
#line 258 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "if exp then block elseifs else block end";
  		auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(yystack_[7].location);
  		if_stmt->set_exp(yystack_[6].value.as < fakelua::syntax_tree_interface_ptr > ());
  		if_stmt->set_block(yystack_[4].value.as < fakelua::syntax_tree_interface_ptr > ());
  		auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
  		if (elseifs == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "elseiflist is not a elseiflist";
  			throw std::runtime_error("elseiflist is not a elseiflist");
  		}
  		if_stmt->set_elseiflist(elseifs);
  		if_stmt->set_else_block(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = if_stmt;
  	}
#line 1037 "parser.cpp"
    break;

  case 16: // stmt: "if" exp "then" block elseifs "end"
#line 274 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "if exp then block elseifs end";
  		auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(yystack_[5].location);
  		if_stmt->set_exp(yystack_[4].value.as < fakelua::syntax_tree_interface_ptr > ());
  		if_stmt->set_block(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
  		auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		if (elseifs == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "elseiflist is not a elseiflist";
  			throw std::runtime_error("elseiflist is not a elseiflist");
  		}
  		if_stmt->set_elseiflist(elseifs);
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = if_stmt;
  	}
#line 1055 "parser.cpp"
    break;

  case 17: // stmt: "for" "identifier" "=" exp "," exp "do" block "end"
#line 289 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "for IDENTIFIER assign exp COMMA exp do block end";
  		auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(yystack_[8].location);
  		for_loop_stmt->set_name(yystack_[7].value.as < std::string > ());
  		for_loop_stmt->set_exp_begin(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
  		for_loop_stmt->set_exp_end(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
  		for_loop_stmt->set_block(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_loop_stmt;
  	}
#line 1069 "parser.cpp"
    break;

  case 18: // stmt: "for" "identifier" "=" exp "," exp "," exp "do" block "end"
#line 300 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "for IDENTIFIER assign exp COMMA exp COMMA exp do block end";
  		auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(yystack_[10].location);
  		for_loop_stmt->set_name(yystack_[9].value.as < std::string > ());
  		for_loop_stmt->set_exp_begin(yystack_[7].value.as < fakelua::syntax_tree_interface_ptr > ());
  		for_loop_stmt->set_exp_end(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
  		for_loop_stmt->set_exp_step(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
  		for_loop_stmt->set_block(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_loop_stmt;
  	}
#line 1084 "parser.cpp"
    break;

  case 19: // stmt: "for" namelist "in" explist "do" block "end"
#line 312 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "for namelist in explist do block end";
  		auto for_in_stmt = std::make_shared<fakelua::syntax_tree_for_in>(yystack_[6].location);
  		auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
  		if (namelist == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "namelist is not a namelist";
  			throw std::runtime_error("namelist is not a namelist");
  		}
  		auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
  		if (explist == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "explist is not a explist";
  			throw std::runtime_error("explist is not a explist");
  		}
  		for_in_stmt->set_namelist(namelist);
  		for_in_stmt->set_explist(explist);
  		for_in_stmt->set_block(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_in_stmt;
  	}
#line 1107 "parser.cpp"
    break;

  case 20: // stmt: "function" funcname funcbody
#line 332 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "function funcname funcbody";
  		auto func_stmt = std::make_shared<fakelua::syntax_tree_function>(yystack_[2].location);
  		auto funcname = std::dynamic_pointer_cast<fakelua::syntax_tree_funcname>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		if (funcname == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "funcname is not a funcname";
  			throw std::runtime_error("funcname is not a funcname");
  		}
  		auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
  		if (funcbody == nullptr) {
  			LOG(ERROR) << "[bison]: stmt: " << "funcbody is not a funcbody";
  			throw std::runtime_error("funcbody is not a funcbody");
  		}
  		func_stmt->set_funcname(funcname);
  		func_stmt->set_funcbody(funcbody);
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = func_stmt;
  	}
#line 1129 "parser.cpp"
    break;

  case 21: // elseifs: %empty
#line 353 "parser.y"
        {
		LOG(INFO) << "[bison]: elseifs: " << "empty";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[0].location);
	}
#line 1138 "parser.cpp"
    break;

  case 22: // elseifs: "elseif" exp "then" block
#line 359 "parser.y"
        {
		LOG(INFO) << "[bison]: elseifs: " << "elseif exp then block";
		auto elseifs = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[3].location);
		elseifs->add_elseif(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > (), yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = elseifs;
	}
#line 1149 "parser.cpp"
    break;

  case 23: // elseifs: elseifs "elseif" exp "then" block
#line 367 "parser.y"
        {
		LOG(INFO) << "[bison]: elseifs: " << "elseifs elseif exp then block";
		auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>(yystack_[4].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (elseifs == nullptr) {
			LOG(ERROR) << "[bison]: elseifs: " << "elseifs is not a elseifs";
			throw std::runtime_error("elseifs is not a elseifs");
		}
		elseifs->add_elseif(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > (), yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = elseifs;
	}
#line 1164 "parser.cpp"
    break;

  case 24: // retstat: "return"
#line 381 "parser.y"
        {
		LOG(INFO) << "[bison]: retstat: " << "RETURN";
		auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[0].location);
		ret->set_explist(nullptr);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
	}
#line 1175 "parser.cpp"
    break;

  case 25: // retstat: "return" explist
#line 389 "parser.y"
        {
		LOG(INFO) << "[bison]: retstat: " << "RETURN explist";
		auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[1].location);
		ret->set_explist(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
	}
#line 1186 "parser.cpp"
    break;

  case 26: // label: "::" "identifier" "::"
#line 399 "parser.y"
        {
		LOG(INFO) << "[bison]: bison get label: " << yystack_[1].value.as < std::string > () << " loc: " << yystack_[1].location;
		auto ret = std::make_shared<fakelua::syntax_tree_label>(yystack_[1].location);
		ret->set_name(yystack_[1].value.as < std::string > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
	}
#line 1197 "parser.cpp"
    break;

  case 27: // funcnamelist: "identifier"
#line 409 "parser.y"
        {
		LOG(INFO) << "[bison]: funcnamelist: " << "IDENTIFIER";
		auto funcnamelist = std::make_shared<fakelua::syntax_tree_funcnamelist>(yystack_[0].location);
		funcnamelist->add_name(yystack_[0].value.as < std::string > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcnamelist;
	}
#line 1208 "parser.cpp"
    break;

  case 28: // funcnamelist: funcnamelist "." "identifier"
#line 417 "parser.y"
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
#line 1223 "parser.cpp"
    break;

  case 29: // funcname: funcnamelist
#line 431 "parser.y"
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
#line 1239 "parser.cpp"
    break;

  case 30: // funcname: funcnamelist ":" "identifier"
#line 444 "parser.y"
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
#line 1256 "parser.cpp"
    break;

  case 31: // varlist: var
#line 460 "parser.y"
        {
		LOG(INFO) << "[bison]: varlist: " << "var";
		auto varlist = std::make_shared<fakelua::syntax_tree_varlist>(yystack_[0].location);
		varlist->add_var(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = varlist;
	}
#line 1267 "parser.cpp"
    break;

  case 32: // varlist: varlist "," var
#line 468 "parser.y"
        {
		LOG(INFO) << "[bison]: varlist: " << "varlist COMMA var";
		auto varlist = std::dynamic_pointer_cast<fakelua::syntax_tree_varlist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (varlist == nullptr) {
			LOG(ERROR) << "[bison]: varlist: " << "varlist is not a varlist";
			throw std::runtime_error("varlist is not a varlist");
		}
		varlist->add_var(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = varlist;
	}
#line 1282 "parser.cpp"
    break;

  case 33: // var: "identifier"
#line 482 "parser.y"
        {
		LOG(INFO) << "[bison]: var: " << "IDENTIFIER";
		auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[0].location);
		var->set_name(yystack_[0].value.as < std::string > ());
		var->set_type("simple");
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
	}
#line 1294 "parser.cpp"
    break;

  case 34: // var: prefixexp "[" exp "]"
#line 491 "parser.y"
        {
		LOG(INFO) << "[bison]: var: " << "prefixexp LSQUARE exp RSQUARE";
		auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
		auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (prefixexp == nullptr) {
			LOG(ERROR) << "[bison]: var: " << "prefixexp is not a prefixexp";
			throw std::runtime_error("prefixexp is not a prefixexp");
		}
		if (exp == nullptr) {
			LOG(ERROR) << "[bison]: var: " << "exp is not a exp";
			throw std::runtime_error("exp is not a exp");
		}
		auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[2].location);
		var->set_prefixexp(prefixexp);
		var->set_exp(exp);
		var->set_type("square");
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
	}
#line 1317 "parser.cpp"
    break;

  case 35: // var: prefixexp "." "identifier"
#line 511 "parser.y"
        {
		LOG(INFO) << "[bison]: var: " << "prefixexp DOT IDENTIFIER";
		auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (prefixexp == nullptr) {
			LOG(ERROR) << "[bison]: var: " << "prefixexp is not a prefixexp";
			throw std::runtime_error("prefixexp is not a prefixexp");
		}
		auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[1].location);
		var->set_prefixexp(prefixexp);
		var->set_name(yystack_[0].value.as < std::string > ());
		var->set_type("dot");
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
	}
#line 1335 "parser.cpp"
    break;

  case 36: // namelist: "identifier"
#line 528 "parser.y"
        {
		LOG(INFO) << "[bison]: namelist: " << "IDENTIFIER";
	}
#line 1343 "parser.cpp"
    break;

  case 37: // namelist: namelist "," "identifier"
#line 533 "parser.y"
        {
		LOG(INFO) << "[bison]: namelist: " << "namelist COMMA IDENTIFIER";
	}
#line 1351 "parser.cpp"
    break;

  case 38: // explist: exp
#line 540 "parser.y"
        {
		LOG(INFO) << "[bison]: explist: " << "exp";
	}
#line 1359 "parser.cpp"
    break;

  case 39: // explist: explist "," exp
#line 545 "parser.y"
        {
		LOG(INFO) << "[bison]: explist: " << "explist COMMA exp";
	}
#line 1367 "parser.cpp"
    break;

  case 40: // exp: "nil"
#line 552 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "NIL";
	}
#line 1375 "parser.cpp"
    break;

  case 41: // exp: "true"
#line 557 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "TRUE";
	}
#line 1383 "parser.cpp"
    break;

  case 42: // exp: "false"
#line 562 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "FALSE";
	}
#line 1391 "parser.cpp"
    break;

  case 43: // exp: "number"
#line 567 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "NUMBER";
	}
#line 1399 "parser.cpp"
    break;

  case 44: // exp: "string"
#line 572 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "STRING";
	}
#line 1407 "parser.cpp"
    break;

  case 45: // exp: "..."
#line 577 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "VAR_PARAMS";
	}
#line 1415 "parser.cpp"
    break;

  case 46: // exp: functiondef
#line 582 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "functiondef";
	}
#line 1423 "parser.cpp"
    break;

  case 47: // exp: prefixexp
#line 587 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "prefixexp";
	}
#line 1431 "parser.cpp"
    break;

  case 48: // exp: tableconstructor
#line 592 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "tableconstructor";
	}
#line 1439 "parser.cpp"
    break;

  case 49: // exp: exp binop exp
#line 597 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "exp binop exp";
	}
#line 1447 "parser.cpp"
    break;

  case 50: // exp: unop exp
#line 602 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "unop exp";
	}
#line 1455 "parser.cpp"
    break;

  case 51: // prefixexp: var
#line 609 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "var";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1464 "parser.cpp"
    break;

  case 52: // prefixexp: functioncall
#line 615 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "functioncall";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1473 "parser.cpp"
    break;

  case 53: // prefixexp: "(" exp ")"
#line 621 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "LPAREN exp RPAREN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1482 "parser.cpp"
    break;

  case 54: // functioncall: prefixexp args
#line 628 "parser.y"
        {
		LOG(INFO) << "[bison]: functioncall: " << "prefixexp args";
		auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
		auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (prefixexp == nullptr) {
			LOG(ERROR) << "[bison]: functioncall: " << "prefixexp is not a prefixexp";
			throw std::runtime_error("prefixexp is not a prefixexp");
		}
		if (args == nullptr) {
			LOG(ERROR) << "[bison]: functioncall: " << "args is not a args";
			throw std::runtime_error("args is not a args");
		}
		auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(yystack_[1].location);
		functioncall->set_prefixexp(prefixexp);
		functioncall->set_args(args);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = functioncall;
	}
#line 1504 "parser.cpp"
    break;

  case 55: // functioncall: prefixexp ":" "identifier" args
#line 647 "parser.y"
        {
		LOG(INFO) << "[bison]: functioncall: " << "prefixexp COLON IDENTIFIER args";
		auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (prefixexp == nullptr) {
			LOG(ERROR) << "[bison]: functioncall: " << "prefixexp is not a prefixexp";
			throw std::runtime_error("prefixexp is not a prefixexp");
		}
		auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(yystack_[3].location);
		functioncall->set_prefixexp(prefixexp);
		functioncall->set_name(yystack_[1].value.as < std::string > ());
		auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (args == nullptr) {
			LOG(ERROR) << "[bison]: functioncall: " << "args is not a args";
			throw std::runtime_error("args is not a args");
		}
		functioncall->set_args(args);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = functioncall;
	}
#line 1527 "parser.cpp"
    break;

  case 56: // args: "(" explist ")"
#line 669 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "LPAREN explist RPAREN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1536 "parser.cpp"
    break;

  case 57: // args: "(" ")"
#line 675 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "LPAREN RPAREN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_empty>(yystack_[1].location);
	}
#line 1545 "parser.cpp"
    break;

  case 58: // args: tableconstructor
#line 681 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "tableconstructor";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1554 "parser.cpp"
    break;

  case 59: // args: "string"
#line 687 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "STRING";
	}
#line 1562 "parser.cpp"
    break;

  case 60: // functiondef: "function" funcbody
#line 694 "parser.y"
        {
		LOG(INFO) << "[bison]: functiondef: " << "FUNCTION funcbody";
	}
#line 1570 "parser.cpp"
    break;

  case 61: // funcbody: "(" parlist ")" block "end"
#line 701 "parser.y"
        {
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN parlist RPAREN block END";
		auto parlist = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (parlist == nullptr) {
			LOG(ERROR) << "[bison]: funcbody: " << "parlist is not a parlist";
			throw std::runtime_error("parlist is not a parlist");
		}
		auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (block == nullptr) {
			LOG(ERROR) << "[bison]: funcbody: " << "block is not a block";
			throw std::runtime_error("block is not a block");
		}
		auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(yystack_[4].location);
		funcbody->set_parlist(parlist);
		funcbody->set_block(block);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcbody;
	}
#line 1592 "parser.cpp"
    break;

  case 62: // funcbody: "(" ")" block "end"
#line 720 "parser.y"
        {
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN RPAREN block END";
		auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (block == nullptr) {
			LOG(ERROR) << "[bison]: funcbody: " << "block is not a block";
			throw std::runtime_error("block is not a block");
		}
		auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(yystack_[3].location);
		funcbody->set_block(block);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcbody;
	}
#line 1608 "parser.cpp"
    break;

  case 63: // parlist: namelist
#line 735 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "namelist";
		auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (namelist == nullptr) {
			LOG(ERROR) << "[bison]: parlist: " << "namelist is not a namelist";
			throw std::runtime_error("namelist is not a namelist");
		}
		auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[0].location);
		parlist->set_namelist(namelist);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
	}
#line 1624 "parser.cpp"
    break;

  case 64: // parlist: namelist "," "..."
#line 748 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "namelist COMMA VAR_PARAMS";
		auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (namelist == nullptr) {
			LOG(ERROR) << "[bison]: parlist: " << "namelist is not a namelist";
			throw std::runtime_error("namelist is not a namelist");
		}
		auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[2].location);
		parlist->set_namelist(namelist);
		parlist->set_var_params(true);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
	}
#line 1641 "parser.cpp"
    break;

  case 65: // parlist: "..."
#line 762 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "VAR_PARAMS";
		auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[0].location);
		parlist->set_var_params(true);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
	}
#line 1652 "parser.cpp"
    break;

  case 66: // tableconstructor: "{" fieldlist "}"
#line 772 "parser.y"
        {
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY fieldlist RCURLY";
		auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[2].location);
		tableconstructor->set_fieldlist(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
	}
#line 1663 "parser.cpp"
    break;

  case 67: // tableconstructor: "{" "}"
#line 780 "parser.y"
        {
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY RCURLY";
		auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[1].location);
		tableconstructor->set_fieldlist(std::make_shared<fakelua::syntax_tree_empty>(yystack_[1].location));
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
	}
#line 1674 "parser.cpp"
    break;

  case 68: // fieldlist: field
#line 790 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldlist: " << "field";
		auto fieldlist = std::make_shared<fakelua::syntax_tree_fieldlist>(yystack_[0].location);
		auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (field == nullptr) {
			LOG(ERROR) << "[bison]: fieldlist: " << "field is not a field";
			throw std::runtime_error("field is not a field");
		}
		fieldlist->add_field(field);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = fieldlist;
	}
#line 1690 "parser.cpp"
    break;

  case 69: // fieldlist: fieldlist fieldsep field
#line 803 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldlist: " << "fieldlist fieldsep field";
		auto fieldlist = std::dynamic_pointer_cast<fakelua::syntax_tree_fieldlist>(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (fieldlist == nullptr) {
			LOG(ERROR) << "[bison]: fieldlist: " << "fieldlist is not a fieldlist";
			throw std::runtime_error("fieldlist is not a fieldlist");
		}
		auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (field == nullptr) {
			LOG(ERROR) << "[bison]: fieldlist: " << "field is not a field";
			throw std::runtime_error("field is not a field");
		}
		fieldlist->add_field(field);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = fieldlist;
	}
#line 1710 "parser.cpp"
    break;

  case 70: // field: "[" exp "]" "=" exp
#line 822 "parser.y"
        {
		LOG(INFO) << "[bison]: field: " << "LSQUARE exp RSQUARE ASSIGN exp";
		auto assignment = std::make_shared<fakelua::syntax_tree_fieldassignment>(yystack_[4].location);
		auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (field == nullptr) {
			LOG(ERROR) << "[bison]: field: " << "field is not a field";
			throw std::runtime_error("field is not a field");
		}
		assignment->set_field(field);
		auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (exp == nullptr) {
			LOG(ERROR) << "[bison]: field: " << "exp is not a exp";
			throw std::runtime_error("exp is not a exp");
		}
		assignment->set_exp(exp);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = assignment;
	}
#line 1732 "parser.cpp"
    break;

  case 71: // field: "identifier" "=" exp
#line 841 "parser.y"
        {
		LOG(INFO) << "[bison]: field: " << "IDENTIFIER ASSIGN exp";
		auto assignment = std::make_shared<fakelua::syntax_tree_fieldassignment>(yystack_[2].location);
		auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		if (exp == nullptr) {
			LOG(ERROR) << "[bison]: field: " << "exp is not a exp";
			throw std::runtime_error("exp is not a exp");
		}
		assignment->set_name(yystack_[2].value.as < std::string > ());
		assignment->set_exp(exp);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = assignment;
	}
#line 1749 "parser.cpp"
    break;

  case 72: // field: exp
#line 855 "parser.y"
        {
		LOG(INFO) << "[bison]: field: " << "exp";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1758 "parser.cpp"
    break;

  case 73: // fieldsep: ","
#line 864 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldsep: " << "COMMA";
	}
#line 1766 "parser.cpp"
    break;

  case 74: // fieldsep: ";"
#line 869 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldsep: " << "SEMICOLON";
	}
#line 1774 "parser.cpp"
    break;

  case 75: // binop: "+"
#line 876 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "PLUS";
	}
#line 1782 "parser.cpp"
    break;

  case 76: // binop: "-"
#line 881 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MINUS";
	}
#line 1790 "parser.cpp"
    break;

  case 77: // binop: "*"
#line 886 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "STAR";
	}
#line 1798 "parser.cpp"
    break;

  case 78: // binop: "/"
#line 891 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "SLASH";
	}
#line 1806 "parser.cpp"
    break;

  case 79: // binop: "//"
#line 896 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "DOUBLE_SLASH";
	}
#line 1814 "parser.cpp"
    break;

  case 80: // binop: "^"
#line 901 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "XOR";
	}
#line 1822 "parser.cpp"
    break;

  case 81: // binop: "%"
#line 906 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MOD";
	}
#line 1830 "parser.cpp"
    break;

  case 82: // binop: "&"
#line 911 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITAND";
	}
#line 1838 "parser.cpp"
    break;

  case 83: // binop: "~"
#line 916 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITNOT";
	}
#line 1846 "parser.cpp"
    break;

  case 84: // binop: "|"
#line 921 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITOR";
	}
#line 1854 "parser.cpp"
    break;

  case 85: // binop: ">>"
#line 926 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "RIGHT_SHIFT";
	}
#line 1862 "parser.cpp"
    break;

  case 86: // binop: "<<"
#line 931 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LEFT_SHIFT";
	}
#line 1870 "parser.cpp"
    break;

  case 87: // binop: ".."
#line 936 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "CONCAT";
	}
#line 1878 "parser.cpp"
    break;

  case 88: // binop: "<"
#line 941 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LESS";
	}
#line 1886 "parser.cpp"
    break;

  case 89: // binop: "<="
#line 946 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LESS_EQUAL";
	}
#line 1894 "parser.cpp"
    break;

  case 90: // binop: ">"
#line 951 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MORE";
	}
#line 1902 "parser.cpp"
    break;

  case 91: // binop: ">="
#line 956 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MORE_EQUAL";
	}
#line 1910 "parser.cpp"
    break;

  case 92: // binop: "=="
#line 961 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "EQUAL";
	}
#line 1918 "parser.cpp"
    break;

  case 93: // binop: "~="
#line 966 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "NOT_EQUAL";
	}
#line 1926 "parser.cpp"
    break;

  case 94: // binop: "and"
#line 971 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "AND";
	}
#line 1934 "parser.cpp"
    break;

  case 95: // binop: "or"
#line 976 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "OR";
	}
#line 1942 "parser.cpp"
    break;

  case 96: // unop: "-"
#line 983 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "MINUS";
	}
#line 1950 "parser.cpp"
    break;

  case 97: // unop: "not"
#line 988 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "NOT";
	}
#line 1958 "parser.cpp"
    break;

  case 98: // unop: "#"
#line 993 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "NUMBER_SIGN";
	}
#line 1966 "parser.cpp"
    break;

  case 99: // unop: "~"
#line 998 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "BITNOT";
	}
#line 1974 "parser.cpp"
    break;


#line 1978 "parser.cpp"

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


  const signed char parser::yypact_ninf_ = -47;

  const signed char parser::yytable_ninf_ = -33;

  const short
  parser::yypact_[] =
  {
    1212,   278,   -47,  1212,   -46,   -41,   278,  1212,   278,   278,
     -23,   -20,   -47,   -47,    69,  1212,   -47,   -47,   -47,    12,
      17,    -5,   -47,   -47,   236,   -47,    66,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   366,    -5,   -47,   -47,
     278,   892,    73,    24,   -47,    26,    66,   419,   924,    31,
     843,   472,   -47,    40,   -47,   -47,   278,     6,    23,   278,
      32,    33,   -47,   -47,   -47,   -47,   278,    85,   843,     5,
     -47,     9,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   278,   843,   -47,   278,   278,
      35,    44,    49,   -47,  1212,   278,   278,  1212,   -47,    31,
      22,   -47,    -7,   525,     3,   -47,   578,   278,   -47,   -47,
     -47,   249,  1212,   -47,   -47,    60,   100,   843,   631,    30,
     -47,   -47,   -47,   956,   843,   843,   988,   -47,   -47,   -47,
     107,   843,   -47,  1020,   -19,  1212,   278,  1212,   278,     4,
     -47,   278,   -47,   -47,  1052,   313,  1084,   684,  1212,   278,
     -47,   843,   -47,  1212,   278,   -47,  1212,  1116,   737,  1148,
     790,  1212,   -47,  1212,   -47,  1212,  1212,  1180,   -47
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,    10,     3,     0,     0,     0,     3,    24,     0,
       0,     0,     7,    33,     0,     2,     4,     6,     9,     0,
      51,     0,    52,    96,     0,    42,     0,    40,    97,    41,
      45,    99,    98,    44,    43,    51,     0,    47,    46,    48,
       0,     0,    36,     0,    27,    29,     0,     0,     0,    25,
      38,     0,    11,     0,     1,     5,     0,     0,     0,     0,
       0,     0,    59,    54,    58,    67,     0,    33,    72,     0,
      68,     0,    60,    76,    75,    77,    78,    53,    94,    95,
      79,    87,    92,    91,    89,    93,    86,    85,    80,    81,
      82,    84,    83,    90,    88,     0,    50,    12,     0,     0,
       0,     0,     0,    20,     3,     0,     0,     3,    26,     8,
      51,    57,     0,     0,     0,    35,     0,     0,    66,    74,
      73,     0,     3,    65,    36,    63,     0,    49,     0,     0,
      37,    30,    28,    21,    14,    39,     0,    56,    34,    55,
       0,    71,    69,     0,     0,     3,     0,     3,     0,     0,
      13,     0,    62,    64,     0,     0,     0,     0,     3,     0,
      16,    70,    61,     3,     0,    19,     3,     0,     0,     0,
       0,    22,    15,     3,    17,     3,    23,     0,    18
  };

  const signed char
  parser::yypgoto_[] =
  {
     -47,   -47,    -3,    48,   -47,   -47,   -47,   -47,   -47,   -47,
       0,    41,   -28,    91,    28,   -47,    -1,   -47,    65,   -47,
     -11,   -47,     1,   -47,   -47,   -47
  };

  const unsigned char
  parser::yydefgoto_[] =
  {
       0,    14,    15,    16,   149,    17,    18,    45,    46,    19,
      20,    43,    49,    50,    21,    22,    63,    38,    72,   126,
      39,    69,    70,   121,    95,    40
  };

  const short
  parser::yytable_[] =
  {
      41,    35,   137,    58,    48,    24,    35,    59,    35,    35,
      64,    58,    42,    24,     1,    56,   118,    44,   122,   153,
     -31,   158,   159,   160,    35,   -32,    64,    23,   109,    37,
     112,     1,   111,    24,    37,    52,    37,    37,    53,   130,
      35,   106,    60,    25,    61,    26,   147,   123,    99,    27,
      28,   119,    37,   120,    62,    29,    35,   110,    35,    35,
      57,    30,    62,    55,    13,   -31,    35,   124,    37,    54,
     -32,   129,   100,   101,    71,   102,    98,    31,   106,   106,
      32,    13,    33,    34,    37,   108,    37,    37,   117,    55,
     114,   115,    36,   130,    37,    35,    55,    47,    35,    35,
      51,   133,   131,    64,   136,    35,    35,   132,   144,   145,
     151,   103,   125,   139,     0,    68,     0,    35,     0,   143,
       0,    35,   142,    37,     0,     0,    37,    37,     0,     0,
       0,    96,     0,    37,    37,     0,     0,     0,     0,     0,
       0,     0,   154,     0,   156,    37,    35,     0,    35,    37,
     113,    35,     0,     0,     0,   167,     0,   116,     0,    35,
     169,     0,     0,   171,    35,     0,     0,     0,     0,     0,
     176,     0,   177,     0,    37,     0,    37,     0,     0,    37,
       0,    55,     0,     0,    55,     0,   127,    37,     0,   128,
       0,    55,    37,     0,     0,     0,   134,   135,     0,     0,
       0,     0,    55,     0,    55,     0,     0,     0,   141,     0,
       0,     0,    68,     0,     0,    55,     0,    55,     0,    55,
       0,     0,     0,     0,    55,    55,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   155,     0,   157,
      23,     0,   161,     0,     1,     0,    24,    65,    66,     0,
     168,     0,     0,    23,     0,   170,    25,     1,    26,    24,
       0,    66,    27,    28,     0,     0,     0,     0,    29,    25,
       0,    26,     0,     0,    30,    27,    28,     0,     0,     0,
       0,    29,    23,     0,     0,     0,     1,    30,    24,     0,
      31,     0,     0,    32,    67,    33,    34,     0,    25,     0,
      26,     0,     0,    31,    27,    28,    32,    67,    33,    34,
      29,     0,     0,     0,     0,     0,    30,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    78,     0,   163,
       0,     0,    31,     0,     0,    32,    13,    33,    34,     0,
       0,    79,     0,     0,     0,     0,     0,     0,     0,    80,
      81,     0,    82,    83,    84,    85,    86,    87,     0,     0,
       0,   164,     0,    88,    89,    90,    91,    92,    93,    94,
      73,    74,    75,    76,     0,    77,     0,     0,     0,     0,
      78,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,     0,
       0,     0,    80,    81,     0,    82,    83,    84,    85,    86,
      87,     0,     0,     0,     0,     0,    88,    89,    90,    91,
      92,    93,    94,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    79,     0,     0,
     104,     0,     0,     0,     0,    80,    81,     0,    82,    83,
      84,    85,    86,    87,     0,     0,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    78,     0,   107,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,     0,     0,     0,    80,    81,
       0,    82,    83,    84,    85,    86,    87,     0,     0,     0,
       0,     0,    88,    89,    90,    91,    92,    93,    94,    73,
      74,    75,    76,     0,     0,     0,     0,     0,   138,    78,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    79,     0,     0,     0,     0,     0,     0,
       0,    80,    81,     0,    82,    83,    84,    85,    86,    87,
       0,     0,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,    73,    74,    75,    76,     0,     0,     0,     0,
       0,   140,    78,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,     0,     0,     0,    80,    81,     0,    82,    83,    84,
      85,    86,    87,     0,     0,     0,     0,     0,    88,    89,
      90,    91,    92,    93,    94,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    78,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    79,
       0,     0,     0,     0,     0,     0,     0,    80,    81,     0,
      82,    83,    84,    85,    86,    87,     0,     0,     0,   146,
       0,    88,    89,    90,    91,    92,    93,    94,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    79,     0,     0,   166,     0,     0,     0,     0,
      80,    81,     0,    82,    83,    84,    85,    86,    87,     0,
       0,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    79,     0,     0,   173,     0,
       0,     0,     0,    80,    81,     0,    82,    83,    84,    85,
      86,    87,     0,     0,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    78,     0,   175,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,     0,     0,     0,    80,    81,     0,    82,
      83,    84,    85,    86,    87,     0,     0,     0,     0,     0,
      88,    89,    90,    91,    92,    93,    94,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    79,     0,     0,     0,     0,     0,     0,     0,    80,
      81,     0,    82,    83,    84,    85,    86,    87,     0,     0,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
       1,     0,     0,     0,     0,     0,     0,     2,     3,     0,
       0,    97,     0,     4,     5,     6,     0,     0,     0,     0,
       0,     7,     8,     0,     0,     0,     9,    10,     0,     0,
       0,     0,     1,     0,     0,     0,     0,    11,    12,     2,
       3,     0,     0,     0,     0,     4,     5,     6,     0,     0,
      13,     0,     0,     7,     8,     0,     0,   105,     9,    10,
       0,     0,     0,     0,     1,     0,     0,     0,     0,    11,
      12,     2,     3,     0,   148,     0,     0,     4,     5,     6,
       0,     0,    13,     0,     0,     7,     8,     0,     0,     0,
       9,    10,     0,     0,     0,     0,     1,     0,     0,     0,
       0,    11,    12,     2,     3,     0,     0,   150,     0,     4,
       5,     6,     0,     0,    13,     0,     0,     7,     8,     0,
       0,     0,     9,    10,     0,     0,     0,     0,     1,     0,
       0,     0,     0,    11,    12,     2,     3,     0,     0,   152,
       0,     4,     5,     6,     0,     0,    13,     0,     0,     7,
       8,     0,     0,     0,     9,    10,     0,     0,     0,     0,
       1,     0,     0,     0,     0,    11,    12,     2,     3,     0,
       0,   162,     0,     4,     5,     6,     0,     0,    13,     0,
       0,     7,     8,     0,     0,     0,     9,    10,     0,     0,
       0,     0,     1,     0,     0,     0,     0,    11,    12,     2,
       3,     0,     0,   165,     0,     4,     5,     6,     0,     0,
      13,     0,     0,     7,     8,     0,     0,     0,     9,    10,
       0,     0,     0,     0,     1,     0,     0,     0,     0,    11,
      12,     2,     3,     0,     0,   172,     0,     4,     5,     6,
       0,     0,    13,     0,     0,     7,     8,     0,     0,     0,
       9,    10,     0,     0,     0,     0,     1,     0,     0,     0,
       0,    11,    12,     2,     3,     0,     0,   174,     0,     4,
       5,     6,     0,     0,    13,     0,     0,     7,     8,     0,
       0,     0,     9,    10,     0,     0,     0,     0,     1,     0,
       0,     0,     0,    11,    12,     2,     3,     0,     0,   178,
       0,     4,     5,     6,     0,     0,    13,     0,     0,     7,
       8,     0,     0,     0,     9,    10,     0,     0,     0,     0,
       1,     0,     0,     0,     0,    11,    12,     2,     3,     0,
       0,     0,     0,     4,     5,     6,     0,     0,    13,     0,
       0,     7,     8,     0,     0,     0,     9,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13
  };

  const short
  parser::yycheck_[] =
  {
       3,     1,     9,     8,     7,    10,     6,    12,     8,     9,
      21,     8,    58,    10,     8,     3,    11,    58,     9,    38,
       3,    17,    18,    19,    24,     3,    37,     4,    56,     1,
      58,     8,     9,    10,     6,    58,     8,     9,    58,    58,
      40,    48,    47,    20,    49,    22,    16,    38,    24,    26,
      27,    46,    24,    48,    59,    32,    56,    57,    58,    59,
      48,    38,    59,    15,    58,    48,    66,    58,    40,     0,
      48,    99,    48,    47,     8,    49,     3,    54,    48,    48,
      57,    58,    59,    60,    56,    45,    58,    59,     3,    41,
      58,    58,     1,    58,    66,    95,    48,     6,    98,    99,
       9,   104,    58,   114,   107,   105,   106,    58,    48,     9,
       3,    46,    71,   114,    -1,    24,    -1,   117,    -1,   122,
      -1,   121,   121,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,    40,    -1,   105,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,   147,   117,   146,    -1,   148,   121,
      59,   151,    -1,    -1,    -1,   158,    -1,    66,    -1,   159,
     163,    -1,    -1,   166,   164,    -1,    -1,    -1,    -1,    -1,
     173,    -1,   175,    -1,   146,    -1,   148,    -1,    -1,   151,
      -1,   133,    -1,    -1,   136,    -1,    95,   159,    -1,    98,
      -1,   143,   164,    -1,    -1,    -1,   105,   106,    -1,    -1,
      -1,    -1,   154,    -1,   156,    -1,    -1,    -1,   117,    -1,
      -1,    -1,   121,    -1,    -1,   167,    -1,   169,    -1,   171,
      -1,    -1,    -1,    -1,   176,   177,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,   148,
       4,    -1,   151,    -1,     8,    -1,    10,    11,    12,    -1,
     159,    -1,    -1,     4,    -1,   164,    20,     8,    22,    10,
      -1,    12,    26,    27,    -1,    -1,    -1,    -1,    32,    20,
      -1,    22,    -1,    -1,    38,    26,    27,    -1,    -1,    -1,
      -1,    32,     4,    -1,    -1,    -1,     8,    38,    10,    -1,
      54,    -1,    -1,    57,    58,    59,    60,    -1,    20,    -1,
      22,    -1,    -1,    54,    26,    27,    57,    58,    59,    60,
      32,    -1,    -1,    -1,    -1,    -1,    38,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    54,    -1,    -1,    57,    58,    59,    60,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
       4,     5,     6,     7,    -1,     9,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,
      51,    52,    53,    54,    55,    56,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    50,    51,    52,    53,    54,    55,    56,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,    51,
      52,    53,    54,    55,    56,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,
      53,    54,    55,    56,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      50,    51,    52,    53,    54,    55,    56,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    54,    55,    56,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    -1,
      -1,    19,    -1,    21,    22,    23,    -1,    -1,    -1,    -1,
      -1,    29,    30,    -1,    -1,    -1,    34,    35,    -1,    -1,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    45,    46,    15,
      16,    -1,    -1,    -1,    -1,    21,    22,    23,    -1,    -1,
      58,    -1,    -1,    29,    30,    -1,    -1,    33,    34,    35,
      -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    45,
      46,    15,    16,    -1,    18,    -1,    -1,    21,    22,    23,
      -1,    -1,    58,    -1,    -1,    29,    30,    -1,    -1,    -1,
      34,    35,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    45,    46,    15,    16,    -1,    -1,    19,    -1,    21,
      22,    23,    -1,    -1,    58,    -1,    -1,    29,    30,    -1,
      -1,    -1,    34,    35,    -1,    -1,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    45,    46,    15,    16,    -1,    -1,    19,
      -1,    21,    22,    23,    -1,    -1,    58,    -1,    -1,    29,
      30,    -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    45,    46,    15,    16,    -1,
      -1,    19,    -1,    21,    22,    23,    -1,    -1,    58,    -1,
      -1,    29,    30,    -1,    -1,    -1,    34,    35,    -1,    -1,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    45,    46,    15,
      16,    -1,    -1,    19,    -1,    21,    22,    23,    -1,    -1,
      58,    -1,    -1,    29,    30,    -1,    -1,    -1,    34,    35,
      -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    45,
      46,    15,    16,    -1,    -1,    19,    -1,    21,    22,    23,
      -1,    -1,    58,    -1,    -1,    29,    30,    -1,    -1,    -1,
      34,    35,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    45,    46,    15,    16,    -1,    -1,    19,    -1,    21,
      22,    23,    -1,    -1,    58,    -1,    -1,    29,    30,    -1,
      -1,    -1,    34,    35,    -1,    -1,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    45,    46,    15,    16,    -1,    -1,    19,
      -1,    21,    22,    23,    -1,    -1,    58,    -1,    -1,    29,
      30,    -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    45,    46,    15,    16,    -1,
      -1,    -1,    -1,    21,    22,    23,    -1,    -1,    58,    -1,
      -1,    29,    30,    -1,    -1,    -1,    34,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      58
  };

  const signed char
  parser::yystos_[] =
  {
       0,     8,    15,    16,    21,    22,    23,    29,    30,    34,
      35,    45,    46,    58,    62,    63,    64,    66,    67,    70,
      71,    75,    76,     4,    10,    20,    22,    26,    27,    32,
      38,    54,    57,    59,    60,    71,    74,    75,    78,    81,
      86,    63,    58,    72,    58,    68,    69,    74,    63,    73,
      74,    74,    58,    58,     0,    64,     3,    48,     8,    12,
      47,    49,    59,    77,    81,    11,    12,    58,    74,    82,
      83,     8,    79,     4,     5,     6,     7,     9,    14,    28,
      36,    37,    39,    40,    41,    42,    43,    44,    50,    51,
      52,    53,    54,    55,    56,    85,    74,    19,     3,    24,
      48,    47,    49,    79,    31,    33,    48,    16,    45,    73,
      71,     9,    73,    74,    58,    58,    74,     3,    11,    46,
      48,    84,     9,    38,    58,    72,    80,    74,    74,    73,
      58,    58,    58,    63,    74,    74,    63,     9,    13,    77,
      13,    74,    83,    63,    48,     9,    48,    16,    18,    65,
      19,     3,    19,    38,    63,    74,    63,    74,    17,    18,
      19,    74,    19,    16,    48,    19,    31,    63,    74,    63,
      74,    63,    19,    31,    19,    16,    63,    63,    19
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    61,    62,    63,    63,    63,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    65,    65,    65,    66,    66,    67,    68,    68,    69,
      69,    70,    70,    71,    71,    71,    72,    72,    73,    73,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    75,    75,    75,    76,    76,    77,    77,    77,    77,
      78,    79,    79,    80,    80,    80,    81,    81,    82,    82,
      83,    83,    83,    84,    84,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    86,    86,    86,    86
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     1,     2,     1,     1,     3,     1,
       1,     2,     3,     5,     4,     8,     6,     9,    11,     7,
       3,     0,     4,     5,     1,     2,     3,     1,     3,     1,
       3,     1,     3,     1,     4,     3,     1,     3,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     3,     2,     4,     3,     2,     1,     1,
       2,     5,     4,     1,     3,     1,     3,     2,     1,     3,
       5,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
  };




#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   137,   137,   145,   151,   163,   182,   188,   194,   213,
     219,   225,   233,   239,   248,   257,   273,   288,   299,   311,
     331,   352,   358,   366,   380,   388,   398,   408,   416,   430,
     443,   459,   467,   481,   490,   510,   527,   532,   539,   544,
     551,   556,   561,   566,   571,   576,   581,   586,   591,   596,
     601,   608,   614,   620,   627,   646,   668,   674,   680,   686,
     693,   700,   719,   734,   747,   761,   771,   779,   789,   802,
     821,   840,   854,   863,   868,   875,   880,   885,   890,   895,
     900,   905,   910,   915,   920,   925,   930,   935,   940,   945,
     950,   955,   960,   965,   970,   975,   982,   987,   992,   997
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
#line 2874 "parser.cpp"

#line 1002 "parser.y"


void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
