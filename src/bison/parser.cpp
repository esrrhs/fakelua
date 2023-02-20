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
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
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
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
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
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
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
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
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
#line 127 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 419 "parser.cpp"
        break;

      case symbol_kind::S_STRING: // "string"
#line 127 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 425 "parser.cpp"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 127 "parser.y"
                 { yyo << yysym.value.template as < double > (); }
#line 431 "parser.cpp"
        break;

      case symbol_kind::S_chunk: // chunk
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 437 "parser.cpp"
        break;

      case symbol_kind::S_block: // block
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 443 "parser.cpp"
        break;

      case symbol_kind::S_stmt: // stmt
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 449 "parser.cpp"
        break;

      case symbol_kind::S_elseifs: // elseifs
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 455 "parser.cpp"
        break;

      case symbol_kind::S_retstat: // retstat
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 461 "parser.cpp"
        break;

      case symbol_kind::S_label: // label
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 467 "parser.cpp"
        break;

      case symbol_kind::S_varlist: // varlist
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 473 "parser.cpp"
        break;

      case symbol_kind::S_var: // var
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 479 "parser.cpp"
        break;

      case symbol_kind::S_namelist: // namelist
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 485 "parser.cpp"
        break;

      case symbol_kind::S_explist: // explist
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 491 "parser.cpp"
        break;

      case symbol_kind::S_exp: // exp
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 497 "parser.cpp"
        break;

      case symbol_kind::S_prefixexp: // prefixexp
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 503 "parser.cpp"
        break;

      case symbol_kind::S_functioncall: // functioncall
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 509 "parser.cpp"
        break;

      case symbol_kind::S_args: // args
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 515 "parser.cpp"
        break;

      case symbol_kind::S_tableconstructor: // tableconstructor
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 521 "parser.cpp"
        break;

      case symbol_kind::S_fieldlist: // fieldlist
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 527 "parser.cpp"
        break;

      case symbol_kind::S_field: // field
#line 127 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 533 "parser.cpp"
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
      case symbol_kind::S_varlist: // varlist
      case symbol_kind::S_var: // var
      case symbol_kind::S_namelist: // namelist
      case symbol_kind::S_explist: // explist
      case symbol_kind::S_exp: // exp
      case symbol_kind::S_prefixexp: // prefixexp
      case symbol_kind::S_functioncall: // functioncall
      case symbol_kind::S_args: // args
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
#line 134 "parser.y"
        {
  		LOG(INFO) << "[bison]: chunk: " << "block";
  		l->set_chunk(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
	}
#line 828 "parser.cpp"
    break;

  case 3: // block: %empty
#line 142 "parser.y"
        {
  		LOG(INFO) << "[bison]: block: " << "empty";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_block>(yystack_[0].location);
  	}
#line 837 "parser.cpp"
    break;

  case 4: // block: stmt
#line 148 "parser.y"
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
#line 852 "parser.cpp"
    break;

  case 5: // block: block stmt
#line 160 "parser.y"
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
#line 872 "parser.cpp"
    break;

  case 6: // stmt: retstat
#line 179 "parser.y"
        {
        	LOG(INFO) << "[bison]: stmt: " << "retstat";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
        }
#line 881 "parser.cpp"
    break;

  case 7: // stmt: ";"
#line 185 "parser.y"
        {
		LOG(INFO) << "[bison]: stmt: " << "SEMICOLON";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_empty>(yystack_[0].location);
	}
#line 890 "parser.cpp"
    break;

  case 8: // stmt: varlist "=" explist
#line 191 "parser.y"
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
#line 912 "parser.cpp"
    break;

  case 9: // stmt: label
#line 210 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "label";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
  	}
#line 921 "parser.cpp"
    break;

  case 10: // stmt: "break"
#line 216 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "break";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_break>(yystack_[0].location);
  	}
#line 930 "parser.cpp"
    break;

  case 11: // stmt: "goto" "identifier"
#line 222 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "goto IDENTIFIER";
  		auto go = std::make_shared<fakelua::syntax_tree_goto>(yystack_[0].location);
  		go->set_label(yystack_[0].value.as < std::string > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = go;
  	}
#line 941 "parser.cpp"
    break;

  case 12: // stmt: "do" block "end"
#line 230 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "do block end";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
  	}
#line 950 "parser.cpp"
    break;

  case 13: // stmt: "while" exp "do" block "end"
#line 236 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "while exp do block end";
  		auto while_stmt = std::make_shared<fakelua::syntax_tree_while>(yystack_[4].location);
  		while_stmt->set_exp(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
  		while_stmt->set_block(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = while_stmt;
  	}
#line 962 "parser.cpp"
    break;

  case 14: // stmt: "repeat" block "until" exp
#line 245 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "repeat block until exp";
  		auto repeat = std::make_shared<fakelua::syntax_tree_repeat>(yystack_[3].location);
  		repeat->set_block(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
  		repeat->set_exp(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = repeat;
  	}
#line 974 "parser.cpp"
    break;

  case 15: // stmt: "if" exp "then" block elseifs "else" block "end"
#line 254 "parser.y"
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
#line 993 "parser.cpp"
    break;

  case 16: // stmt: "if" exp "then" block elseifs "end"
#line 270 "parser.y"
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
#line 1011 "parser.cpp"
    break;

  case 17: // stmt: "for" "identifier" "=" exp "," exp "do" block "end"
#line 285 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "for IDENTIFIER assign exp COMMA exp do block end";
  		auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(yystack_[8].location);
  		for_loop_stmt->set_name(yystack_[7].value.as < std::string > ());
  		for_loop_stmt->set_exp_begin(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
  		for_loop_stmt->set_exp_end(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
  		for_loop_stmt->set_block(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_loop_stmt;
  	}
#line 1025 "parser.cpp"
    break;

  case 18: // stmt: "for" "identifier" "=" exp "," exp "," exp "do" block "end"
#line 296 "parser.y"
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
#line 1040 "parser.cpp"
    break;

  case 19: // stmt: "for" namelist "in" explist "do" block "end"
#line 308 "parser.y"
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
#line 1063 "parser.cpp"
    break;

  case 20: // elseifs: %empty
#line 330 "parser.y"
        {
		LOG(INFO) << "[bison]: elseifs: " << "empty";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[0].location);
	}
#line 1072 "parser.cpp"
    break;

  case 21: // elseifs: "elseif" exp "then" block
#line 336 "parser.y"
        {
		LOG(INFO) << "[bison]: elseifs: " << "elseif exp then block";
		auto elseifs = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[3].location);
		elseifs->add_elseif(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > (), yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = elseifs;
	}
#line 1083 "parser.cpp"
    break;

  case 22: // elseifs: elseifs "elseif" exp "then" block
#line 344 "parser.y"
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
#line 1098 "parser.cpp"
    break;

  case 23: // retstat: "return"
#line 358 "parser.y"
        {
		LOG(INFO) << "[bison]: retstat: " << "RETURN";
		auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[0].location);
		ret->set_explist(nullptr);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
	}
#line 1109 "parser.cpp"
    break;

  case 24: // retstat: "return" explist
#line 366 "parser.y"
        {
		LOG(INFO) << "[bison]: retstat: " << "RETURN explist";
		auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[1].location);
		ret->set_explist(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
	}
#line 1120 "parser.cpp"
    break;

  case 25: // label: "::" "identifier" "::"
#line 376 "parser.y"
        {
		LOG(INFO) << "[bison]: bison get label: " << yystack_[1].value.as < std::string > () << " loc: " << yystack_[1].location;
		auto ret = std::make_shared<fakelua::syntax_tree_label>(yystack_[1].location);
		ret->set_name(yystack_[1].value.as < std::string > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
	}
#line 1131 "parser.cpp"
    break;

  case 26: // varlist: var
#line 410 "parser.y"
        {
		LOG(INFO) << "[bison]: varlist: " << "var";
		auto varlist = std::make_shared<fakelua::syntax_tree_varlist>(yystack_[0].location);
		varlist->add_var(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = varlist;
	}
#line 1142 "parser.cpp"
    break;

  case 27: // varlist: varlist "," var
#line 418 "parser.y"
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
#line 1157 "parser.cpp"
    break;

  case 28: // var: "identifier"
#line 432 "parser.y"
        {
		LOG(INFO) << "[bison]: var: " << "IDENTIFIER";
		auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[0].location);
		var->set_name(yystack_[0].value.as < std::string > ());
		var->set_type("simple");
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
	}
#line 1169 "parser.cpp"
    break;

  case 29: // var: prefixexp "[" exp "]"
#line 441 "parser.y"
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
#line 1192 "parser.cpp"
    break;

  case 30: // var: prefixexp "." "identifier"
#line 461 "parser.y"
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
#line 1210 "parser.cpp"
    break;

  case 31: // namelist: "identifier"
#line 478 "parser.y"
        {
		LOG(INFO) << "[bison]: namelist: " << "IDENTIFIER";
	}
#line 1218 "parser.cpp"
    break;

  case 32: // namelist: namelist "," "identifier"
#line 483 "parser.y"
        {
		LOG(INFO) << "[bison]: namelist: " << "namelist COMMA IDENTIFIER";
	}
#line 1226 "parser.cpp"
    break;

  case 33: // explist: exp
#line 490 "parser.y"
        {
		LOG(INFO) << "[bison]: explist: " << "exp";
	}
#line 1234 "parser.cpp"
    break;

  case 34: // explist: explist "," exp
#line 495 "parser.y"
        {
		LOG(INFO) << "[bison]: explist: " << "explist COMMA exp";
	}
#line 1242 "parser.cpp"
    break;

  case 35: // exp: "nil"
#line 502 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "NIL";
	}
#line 1250 "parser.cpp"
    break;

  case 36: // exp: "true"
#line 507 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "TRUE";
	}
#line 1258 "parser.cpp"
    break;

  case 37: // exp: "false"
#line 512 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "FALSE";
	}
#line 1266 "parser.cpp"
    break;

  case 38: // exp: "number"
#line 517 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "NUMBER";
	}
#line 1274 "parser.cpp"
    break;

  case 39: // exp: "string"
#line 522 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "STRING";
	}
#line 1282 "parser.cpp"
    break;

  case 40: // exp: "..."
#line 527 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "VAR_PARAMS";
	}
#line 1290 "parser.cpp"
    break;

  case 41: // exp: functiondef
#line 532 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "functiondef";
	}
#line 1298 "parser.cpp"
    break;

  case 42: // exp: prefixexp
#line 537 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "prefixexp";
	}
#line 1306 "parser.cpp"
    break;

  case 43: // exp: tableconstructor
#line 542 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "tableconstructor";
	}
#line 1314 "parser.cpp"
    break;

  case 44: // exp: exp binop exp
#line 547 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "exp binop exp";
	}
#line 1322 "parser.cpp"
    break;

  case 45: // exp: unop exp
#line 552 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "unop exp";
	}
#line 1330 "parser.cpp"
    break;

  case 46: // prefixexp: var
#line 559 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "var";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1339 "parser.cpp"
    break;

  case 47: // prefixexp: functioncall
#line 565 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "functioncall";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1348 "parser.cpp"
    break;

  case 48: // prefixexp: "(" exp ")"
#line 571 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "LPAREN exp RPAREN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1357 "parser.cpp"
    break;

  case 49: // functioncall: prefixexp args
#line 578 "parser.y"
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
#line 1379 "parser.cpp"
    break;

  case 50: // functioncall: prefixexp ":" "identifier" args
#line 597 "parser.y"
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
#line 1402 "parser.cpp"
    break;

  case 51: // args: "(" explist ")"
#line 619 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "LPAREN explist RPAREN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1411 "parser.cpp"
    break;

  case 52: // args: "(" ")"
#line 625 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "LPAREN RPAREN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_empty>(yystack_[1].location);
	}
#line 1420 "parser.cpp"
    break;

  case 53: // args: tableconstructor
#line 631 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "tableconstructor";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1429 "parser.cpp"
    break;

  case 54: // args: "string"
#line 637 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "STRING";
	}
#line 1437 "parser.cpp"
    break;

  case 55: // functiondef: "function" funcbody
#line 644 "parser.y"
        {
		LOG(INFO) << "[bison]: functiondef: " << "FUNCTION funcbody";
	}
#line 1445 "parser.cpp"
    break;

  case 56: // funcbody: "(" parlist ")" block "end"
#line 651 "parser.y"
        {
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN parlist RPAREN block END";
	}
#line 1453 "parser.cpp"
    break;

  case 57: // funcbody: "(" ")" block "end"
#line 656 "parser.y"
        {
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN RPAREN block END";
	}
#line 1461 "parser.cpp"
    break;

  case 58: // parlist: namelist
#line 663 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "namelist";
	}
#line 1469 "parser.cpp"
    break;

  case 59: // parlist: namelist "," "..."
#line 668 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "namelist COMMA VAR_PARAMS";
	}
#line 1477 "parser.cpp"
    break;

  case 60: // parlist: "..."
#line 673 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "VAR_PARAMS";
	}
#line 1485 "parser.cpp"
    break;

  case 61: // tableconstructor: "{" fieldlist "}"
#line 680 "parser.y"
        {
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY fieldlist RCURLY";
		auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[2].location);
		tableconstructor->set_fieldlist(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
	}
#line 1496 "parser.cpp"
    break;

  case 62: // tableconstructor: "{" "}"
#line 688 "parser.y"
        {
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY RCURLY";
		auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[1].location);
		tableconstructor->set_fieldlist(std::make_shared<fakelua::syntax_tree_empty>(yystack_[1].location));
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
	}
#line 1507 "parser.cpp"
    break;

  case 63: // fieldlist: field
#line 698 "parser.y"
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
#line 1523 "parser.cpp"
    break;

  case 64: // fieldlist: fieldlist fieldsep field
#line 711 "parser.y"
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
#line 1543 "parser.cpp"
    break;

  case 65: // field: "[" exp "]" "=" exp
#line 730 "parser.y"
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
#line 1565 "parser.cpp"
    break;

  case 66: // field: "identifier" "=" exp
#line 749 "parser.y"
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
#line 1582 "parser.cpp"
    break;

  case 67: // field: exp
#line 763 "parser.y"
        {
		LOG(INFO) << "[bison]: field: " << "exp";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1591 "parser.cpp"
    break;

  case 68: // fieldsep: ","
#line 772 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldsep: " << "COMMA";
	}
#line 1599 "parser.cpp"
    break;

  case 69: // fieldsep: ";"
#line 777 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldsep: " << "SEMICOLON";
	}
#line 1607 "parser.cpp"
    break;

  case 70: // binop: "+"
#line 784 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "PLUS";
	}
#line 1615 "parser.cpp"
    break;

  case 71: // binop: "-"
#line 789 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MINUS";
	}
#line 1623 "parser.cpp"
    break;

  case 72: // binop: "*"
#line 794 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "STAR";
	}
#line 1631 "parser.cpp"
    break;

  case 73: // binop: "/"
#line 799 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "SLASH";
	}
#line 1639 "parser.cpp"
    break;

  case 74: // binop: "//"
#line 804 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "DOUBLE_SLASH";
	}
#line 1647 "parser.cpp"
    break;

  case 75: // binop: "^"
#line 809 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "XOR";
	}
#line 1655 "parser.cpp"
    break;

  case 76: // binop: "%"
#line 814 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MOD";
	}
#line 1663 "parser.cpp"
    break;

  case 77: // binop: "&"
#line 819 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITAND";
	}
#line 1671 "parser.cpp"
    break;

  case 78: // binop: "~"
#line 824 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITNOT";
	}
#line 1679 "parser.cpp"
    break;

  case 79: // binop: "|"
#line 829 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITOR";
	}
#line 1687 "parser.cpp"
    break;

  case 80: // binop: ">>"
#line 834 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "RIGHT_SHIFT";
	}
#line 1695 "parser.cpp"
    break;

  case 81: // binop: "<<"
#line 839 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LEFT_SHIFT";
	}
#line 1703 "parser.cpp"
    break;

  case 82: // binop: ".."
#line 844 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "CONCAT";
	}
#line 1711 "parser.cpp"
    break;

  case 83: // binop: "<"
#line 849 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LESS";
	}
#line 1719 "parser.cpp"
    break;

  case 84: // binop: "<="
#line 854 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LESS_EQUAL";
	}
#line 1727 "parser.cpp"
    break;

  case 85: // binop: ">"
#line 859 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MORE";
	}
#line 1735 "parser.cpp"
    break;

  case 86: // binop: ">="
#line 864 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MORE_EQUAL";
	}
#line 1743 "parser.cpp"
    break;

  case 87: // binop: "=="
#line 869 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "EQUAL";
	}
#line 1751 "parser.cpp"
    break;

  case 88: // binop: "~="
#line 874 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "NOT_EQUAL";
	}
#line 1759 "parser.cpp"
    break;

  case 89: // binop: "and"
#line 879 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "AND";
	}
#line 1767 "parser.cpp"
    break;

  case 90: // binop: "or"
#line 884 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "OR";
	}
#line 1775 "parser.cpp"
    break;

  case 91: // unop: "-"
#line 891 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "MINUS";
	}
#line 1783 "parser.cpp"
    break;

  case 92: // unop: "not"
#line 896 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "NOT";
	}
#line 1791 "parser.cpp"
    break;

  case 93: // unop: "#"
#line 901 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "NUMBER_SIGN";
	}
#line 1799 "parser.cpp"
    break;

  case 94: // unop: "~"
#line 906 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "BITNOT";
	}
#line 1807 "parser.cpp"
    break;


#line 1811 "parser.cpp"

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
  "varlist", "var", "namelist", "explist", "exp", "prefixexp",
  "functioncall", "args", "functiondef", "funcbody", "parlist",
  "tableconstructor", "fieldlist", "field", "fieldsep", "binop", "unop", YY_NULLPTR
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


  const signed char parser::yypact_ninf_ = -40;

  const signed char parser::yytable_ninf_ = -28;

  const short
  parser::yypact_[] =
  {
    1280,   324,   -40,  1280,   -38,   324,  1280,   324,   324,   -36,
     -32,   -40,   -40,    32,  1280,   -40,   -40,   -40,     2,     8,
       6,   -40,   -40,   241,   -40,    30,   -40,   -40,   -40,   -40,
     -40,   -40,   -40,   -40,   -40,   434,     6,   -40,   -40,   324,
     960,    26,   -15,   487,   992,    -9,   911,   540,   -40,     0,
     -40,   -40,   324,    -4,   282,   324,   -11,   -10,   -40,   -40,
     -40,   -40,   324,    46,   911,    16,   -40,    -7,   -40,   -40,
     -40,   -40,   -40,   -40,   -40,   -40,   -40,   -40,   -40,   -40,
     -40,   -40,   -40,   -40,   -40,   -40,   -40,   -40,   -40,   -40,
     -40,   324,   911,   -40,   324,   324,     3,  1280,   324,   324,
    1280,   -40,    -9,    21,   -40,    -2,   593,    11,   -40,   646,
     324,   -40,   -40,   -40,   311,  1280,   -40,   -40,    23,    63,
     911,   699,    -8,   -40,  1024,   911,   911,  1056,   -40,   -40,
     -40,    70,   911,   -40,  1088,   -23,  1280,   324,  1280,   324,
      24,   -40,   324,   -40,   -40,  1120,   381,  1152,   752,  1280,
     324,   -40,   911,   -40,  1280,   324,   -40,  1280,  1184,   805,
    1216,   858,  1280,   -40,  1280,   -40,  1280,  1280,  1248,   -40
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,    10,     3,     0,     0,     3,    23,     0,     0,
       0,     7,    28,     0,     2,     4,     6,     9,     0,    46,
       0,    47,    91,     0,    37,     0,    35,    92,    36,    40,
      94,    93,    39,    38,    46,     0,    42,    41,    43,     0,
       0,    31,     0,     0,     0,    24,    33,     0,    11,     0,
       1,     5,     0,     0,     0,     0,     0,     0,    54,    49,
      53,    62,     0,    28,    67,     0,    63,     0,    55,    71,
      70,    72,    73,    48,    89,    90,    74,    82,    87,    86,
      84,    88,    81,    80,    75,    76,    77,    79,    78,    85,
      83,     0,    45,    12,     0,     0,     0,     3,     0,     0,
       3,    25,     8,    46,    52,     0,     0,     0,    30,     0,
       0,    61,    69,    68,     0,     3,    60,    31,    58,     0,
      44,     0,     0,    32,    20,    14,    34,     0,    51,    29,
      50,     0,    66,    64,     0,     0,     3,     0,     3,     0,
       0,    13,     0,    57,    59,     0,     0,     0,     0,     3,
       0,    16,    65,    56,     3,     0,    19,     3,     0,     0,
       0,     0,    21,    15,     3,    17,     3,    22,     0,    18
  };

  const short
  parser::yypgoto_[] =
  {
     -40,   -40,    -3,    49,   -40,   -40,   -40,   -40,     5,     7,
     -29,   133,    29,   -40,   -31,   -40,   -40,   -40,   -19,   -40,
     -39,   -40,   -40,   -40
  };

  const unsigned char
  parser::yydefgoto_[] =
  {
       0,    13,    14,    15,   140,    16,    17,    18,    19,    42,
      45,    46,    20,    21,    59,    37,    68,   119,    38,    65,
      66,   114,    91,    39
  };

  const short
  parser::yytable_[] =
  {
      40,    60,   115,    44,     1,    52,    34,   128,   138,    95,
      34,   -26,    34,    34,    54,   144,    23,    60,    55,    54,
      41,    23,    48,   102,   -27,   105,    49,   111,    34,    94,
      36,   116,    50,    96,    36,   123,    36,    36,    67,    99,
      99,   149,   150,   151,    34,   101,    99,   107,   108,   110,
      53,   117,    36,    56,    12,    57,   -26,    34,   103,    34,
      34,   123,   112,    51,   113,    58,   122,    34,    36,   -27,
      58,   135,   136,   142,   118,   133,   130,     0,     0,     0,
       0,    36,     0,    36,    36,     0,     0,     0,    60,    51,
       0,    36,     0,    51,   124,     0,    34,   127,     0,    34,
      34,     0,     0,    34,    34,     0,     0,     0,     0,     0,
       0,     0,   134,     0,     0,    34,     0,     0,     0,    34,
      36,     0,     0,    36,    36,     0,     0,    36,    36,     0,
       0,     0,     0,   145,    35,   147,     0,     0,    43,    36,
       0,    47,    34,    36,    34,     0,   158,    34,     0,     0,
       0,   160,     0,     0,   162,    34,    64,     0,     0,     0,
      34,   167,     0,   168,     0,     0,    36,     0,    36,     0,
       0,    36,    92,    51,     0,     0,    51,     0,     0,    36,
       0,     0,     0,    51,    36,     0,     0,     0,   106,     0,
       0,     0,     0,     0,    51,   109,    51,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,    51,
       0,    51,     0,     0,     0,     0,    51,    51,     0,     0,
       0,     0,     0,     0,   120,     0,     0,   121,     0,     0,
       0,   125,   126,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   132,     0,    22,     0,    64,     0,     1,
       0,    23,    61,    62,     0,     0,     0,     0,     0,     0,
       0,    24,     0,    25,     0,     0,     0,    26,    27,     0,
     146,     0,   148,    28,     0,   152,     0,     0,     0,    29,
       0,     0,     0,   159,     0,     0,    22,     0,   161,     0,
       1,   104,    23,     0,     0,    30,     0,     0,    31,    63,
      32,    33,    24,     0,    25,     0,     0,     0,    26,    27,
       0,     0,     0,     0,    28,    22,     0,     0,     0,     1,
      29,    23,     0,    62,     0,     0,     0,     0,    22,     0,
       0,    24,     1,    25,    23,     0,    30,    26,    27,    31,
      12,    32,    33,    28,    24,     0,    25,     0,     0,    29,
      26,    27,     0,     0,     0,     0,    28,     0,     0,     0,
       0,     0,    29,     0,     0,    30,     0,     0,    31,    63,
      32,    33,     0,     0,     0,     0,     0,     0,    30,     0,
       0,    31,    12,    32,    33,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,    74,     0,   154,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,     0,     0,     0,    76,    77,     0,
      78,    79,    80,    81,    82,    83,     0,     0,     0,   155,
       0,    84,    85,    86,    87,    88,    89,    90,    69,    70,
      71,    72,     0,    73,     0,     0,     0,     0,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,     0,     0,     0,
      76,    77,     0,    78,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,    84,    85,    86,    87,    88,    89,
      90,    69,    70,    71,    72,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,     0,     0,    97,     0,
       0,     0,     0,    76,    77,     0,    78,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,    84,    85,    86,
      87,    88,    89,    90,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,    74,     0,   100,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,     0,     0,     0,    76,    77,     0,    78,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,    69,    70,    71,
      72,     0,     0,     0,     0,     0,   129,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,     0,     0,     0,    76,
      77,     0,    78,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
      69,    70,    71,    72,     0,     0,     0,     0,     0,   131,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,     0,
       0,     0,    76,    77,     0,    78,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,    69,    70,    71,    72,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,     0,     0,     0,    76,    77,     0,    78,    79,
      80,    81,    82,    83,     0,     0,     0,   137,     0,    84,
      85,    86,    87,    88,    89,    90,    69,    70,    71,    72,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,     0,     0,   157,     0,     0,     0,     0,    76,    77,
       0,    78,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,    69,
      70,    71,    72,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,     0,     0,   164,     0,     0,     0,
       0,    76,    77,     0,    78,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,    84,    85,    86,    87,    88,
      89,    90,    69,    70,    71,    72,     0,     0,     0,     0,
       0,     0,    74,     0,   166,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,     0,     0,     0,    76,    77,     0,    78,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,    84,    85,
      86,    87,    88,    89,    90,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,     0,     0,     0,    76,    77,     0,
      78,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,    84,    85,    86,    87,    88,    89,    90,     1,     0,
       0,     0,     0,     0,     0,     2,     3,     0,     0,    93,
       0,     4,     0,     5,     0,     0,     0,     0,     0,     6,
       7,     0,     0,     0,     8,     9,     0,     0,     0,     0,
       1,     0,     0,     0,     0,    10,    11,     2,     3,     0,
       0,     0,     0,     4,     0,     5,     0,     0,    12,     0,
       0,     6,     7,     0,     0,    98,     8,     9,     0,     0,
       0,     0,     1,     0,     0,     0,     0,    10,    11,     2,
       3,     0,   139,     0,     0,     4,     0,     5,     0,     0,
      12,     0,     0,     6,     7,     0,     0,     0,     8,     9,
       0,     0,     0,     0,     1,     0,     0,     0,     0,    10,
      11,     2,     3,     0,     0,   141,     0,     4,     0,     5,
       0,     0,    12,     0,     0,     6,     7,     0,     0,     0,
       8,     9,     0,     0,     0,     0,     1,     0,     0,     0,
       0,    10,    11,     2,     3,     0,     0,   143,     0,     4,
       0,     5,     0,     0,    12,     0,     0,     6,     7,     0,
       0,     0,     8,     9,     0,     0,     0,     0,     1,     0,
       0,     0,     0,    10,    11,     2,     3,     0,     0,   153,
       0,     4,     0,     5,     0,     0,    12,     0,     0,     6,
       7,     0,     0,     0,     8,     9,     0,     0,     0,     0,
       1,     0,     0,     0,     0,    10,    11,     2,     3,     0,
       0,   156,     0,     4,     0,     5,     0,     0,    12,     0,
       0,     6,     7,     0,     0,     0,     8,     9,     0,     0,
       0,     0,     1,     0,     0,     0,     0,    10,    11,     2,
       3,     0,     0,   163,     0,     4,     0,     5,     0,     0,
      12,     0,     0,     6,     7,     0,     0,     0,     8,     9,
       0,     0,     0,     0,     1,     0,     0,     0,     0,    10,
      11,     2,     3,     0,     0,   165,     0,     4,     0,     5,
       0,     0,    12,     0,     0,     6,     7,     0,     0,     0,
       8,     9,     0,     0,     0,     0,     1,     0,     0,     0,
       0,    10,    11,     2,     3,     0,     0,   169,     0,     4,
       0,     5,     0,     0,    12,     0,     0,     6,     7,     0,
       0,     0,     8,     9,     0,     0,     0,     0,     1,     0,
       0,     0,     0,    10,    11,     2,     3,     0,     0,     0,
       0,     4,     0,     5,     0,     0,    12,     0,     0,     6,
       7,     0,     0,     0,     8,     9,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    10,    11,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12
  };

  const short
  parser::yycheck_[] =
  {
       3,    20,     9,     6,     8,     3,     1,     9,    16,    24,
       5,     3,     7,     8,     8,    38,    10,    36,    12,     8,
      58,    10,    58,    52,     3,    54,    58,    11,    23,     3,
       1,    38,     0,    48,     5,    58,     7,     8,     8,    48,
      48,    17,    18,    19,    39,    45,    48,    58,    58,     3,
      48,    58,    23,    47,    58,    49,    48,    52,    53,    54,
      55,    58,    46,    14,    48,    59,    95,    62,    39,    48,
      59,    48,     9,     3,    67,   114,   107,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    -1,    -1,    -1,   107,    40,
      -1,    62,    -1,    44,    97,    -1,    91,   100,    -1,    94,
      95,    -1,    -1,    98,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,   110,    -1,    -1,    -1,   114,
      91,    -1,    -1,    94,    95,    -1,    -1,    98,    99,    -1,
      -1,    -1,    -1,   136,     1,   138,    -1,    -1,     5,   110,
      -1,     8,   137,   114,   139,    -1,   149,   142,    -1,    -1,
      -1,   154,    -1,    -1,   157,   150,    23,    -1,    -1,    -1,
     155,   164,    -1,   166,    -1,    -1,   137,    -1,   139,    -1,
      -1,   142,    39,   124,    -1,    -1,   127,    -1,    -1,   150,
      -1,    -1,    -1,   134,   155,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,   145,    62,   147,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,   160,
      -1,   162,    -1,    -1,    -1,    -1,   167,   168,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    94,    -1,    -1,
      -1,    98,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   110,    -1,     4,    -1,   114,    -1,     8,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    -1,    22,    -1,    -1,    -1,    26,    27,    -1,
     137,    -1,   139,    32,    -1,   142,    -1,    -1,    -1,    38,
      -1,    -1,    -1,   150,    -1,    -1,     4,    -1,   155,    -1,
       8,     9,    10,    -1,    -1,    54,    -1,    -1,    57,    58,
      59,    60,    20,    -1,    22,    -1,    -1,    -1,    26,    27,
      -1,    -1,    -1,    -1,    32,     4,    -1,    -1,    -1,     8,
      38,    10,    -1,    12,    -1,    -1,    -1,    -1,     4,    -1,
      -1,    20,     8,    22,    10,    -1,    54,    26,    27,    57,
      58,    59,    60,    32,    20,    -1,    22,    -1,    -1,    38,
      26,    27,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    54,    -1,    -1,    57,    58,
      59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    57,    58,    59,    60,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,     4,     5,
       6,     7,    -1,     9,    -1,    -1,    -1,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       7,    -1,    -1,    -1,    -1,    -1,    13,    14,    -1,    -1,
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
      -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    31,    -1,    -1,    -1,    -1,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    50,    51,    52,    53,    54,    55,    56,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,    51,
      52,    53,    54,    55,    56,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    16,    -1,    -1,    19,
      -1,    21,    -1,    23,    -1,    -1,    -1,    -1,    -1,    29,
      30,    -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    45,    46,    15,    16,    -1,
      -1,    -1,    -1,    21,    -1,    23,    -1,    -1,    58,    -1,
      -1,    29,    30,    -1,    -1,    33,    34,    35,    -1,    -1,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    45,    46,    15,
      16,    -1,    18,    -1,    -1,    21,    -1,    23,    -1,    -1,
      58,    -1,    -1,    29,    30,    -1,    -1,    -1,    34,    35,
      -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    45,
      46,    15,    16,    -1,    -1,    19,    -1,    21,    -1,    23,
      -1,    -1,    58,    -1,    -1,    29,    30,    -1,    -1,    -1,
      34,    35,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    45,    46,    15,    16,    -1,    -1,    19,    -1,    21,
      -1,    23,    -1,    -1,    58,    -1,    -1,    29,    30,    -1,
      -1,    -1,    34,    35,    -1,    -1,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    45,    46,    15,    16,    -1,    -1,    19,
      -1,    21,    -1,    23,    -1,    -1,    58,    -1,    -1,    29,
      30,    -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    45,    46,    15,    16,    -1,
      -1,    19,    -1,    21,    -1,    23,    -1,    -1,    58,    -1,
      -1,    29,    30,    -1,    -1,    -1,    34,    35,    -1,    -1,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    45,    46,    15,
      16,    -1,    -1,    19,    -1,    21,    -1,    23,    -1,    -1,
      58,    -1,    -1,    29,    30,    -1,    -1,    -1,    34,    35,
      -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    45,
      46,    15,    16,    -1,    -1,    19,    -1,    21,    -1,    23,
      -1,    -1,    58,    -1,    -1,    29,    30,    -1,    -1,    -1,
      34,    35,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    45,    46,    15,    16,    -1,    -1,    19,    -1,    21,
      -1,    23,    -1,    -1,    58,    -1,    -1,    29,    30,    -1,
      -1,    -1,    34,    35,    -1,    -1,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    45,    46,    15,    16,    -1,    -1,    -1,
      -1,    21,    -1,    23,    -1,    -1,    58,    -1,    -1,    29,
      30,    -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58
  };

  const signed char
  parser::yystos_[] =
  {
       0,     8,    15,    16,    21,    23,    29,    30,    34,    35,
      45,    46,    58,    62,    63,    64,    66,    67,    68,    69,
      73,    74,     4,    10,    20,    22,    26,    27,    32,    38,
      54,    57,    59,    60,    69,    72,    73,    76,    79,    84,
      63,    58,    70,    72,    63,    71,    72,    72,    58,    58,
       0,    64,     3,    48,     8,    12,    47,    49,    59,    75,
      79,    11,    12,    58,    72,    80,    81,     8,    77,     4,
       5,     6,     7,     9,    14,    28,    36,    37,    39,    40,
      41,    42,    43,    44,    50,    51,    52,    53,    54,    55,
      56,    83,    72,    19,     3,    24,    48,    31,    33,    48,
      16,    45,    71,    69,     9,    71,    72,    58,    58,    72,
       3,    11,    46,    48,    82,     9,    38,    58,    70,    78,
      72,    72,    71,    58,    63,    72,    72,    63,     9,    13,
      75,    13,    72,    81,    63,    48,     9,    48,    16,    18,
      65,    19,     3,    19,    38,    63,    72,    63,    72,    17,
      18,    19,    72,    19,    16,    48,    19,    31,    63,    72,
      63,    72,    63,    19,    31,    19,    16,    63,    63,    19
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    61,    62,    63,    63,    63,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      65,    65,    65,    66,    66,    67,    68,    68,    69,    69,
      69,    70,    70,    71,    71,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    73,    73,    73,    74,
      74,    75,    75,    75,    75,    76,    77,    77,    78,    78,
      78,    79,    79,    80,    80,    81,    81,    81,    82,    82,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    84,    84,    84,    84
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     1,     2,     1,     1,     3,     1,
       1,     2,     3,     5,     4,     8,     6,     9,    11,     7,
       0,     4,     5,     1,     2,     3,     1,     3,     1,     4,
       3,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     3,     2,
       4,     3,     2,     1,     1,     2,     5,     4,     1,     3,
       1,     3,     2,     1,     3,     5,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1
  };




#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   133,   133,   141,   147,   159,   178,   184,   190,   209,
     215,   221,   229,   235,   244,   253,   269,   284,   295,   307,
     329,   335,   343,   357,   365,   375,   409,   417,   431,   440,
     460,   477,   482,   489,   494,   501,   506,   511,   516,   521,
     526,   531,   536,   541,   546,   551,   558,   564,   570,   577,
     596,   618,   624,   630,   636,   643,   650,   655,   662,   667,
     672,   679,   687,   697,   710,   729,   748,   762,   771,   776,
     783,   788,   793,   798,   803,   808,   813,   818,   823,   828,
     833,   838,   843,   848,   853,   858,   863,   868,   873,   878,
     883,   890,   895,   900,   905
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
#line 2715 "parser.cpp"

#line 910 "parser.y"


void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
