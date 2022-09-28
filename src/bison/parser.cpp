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
#line 126 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 415 "parser.cpp"
        break;

      case symbol_kind::S_STRING: // "string"
#line 126 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 421 "parser.cpp"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 126 "parser.y"
                 { yyo << yysym.value.template as < double > (); }
#line 427 "parser.cpp"
        break;

      case symbol_kind::S_chunk: // chunk
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 433 "parser.cpp"
        break;

      case symbol_kind::S_block: // block
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 439 "parser.cpp"
        break;

      case symbol_kind::S_stmt: // stmt
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 445 "parser.cpp"
        break;

      case symbol_kind::S_elseifs: // elseifs
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 451 "parser.cpp"
        break;

      case symbol_kind::S_retstat: // retstat
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 457 "parser.cpp"
        break;

      case symbol_kind::S_label: // label
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 463 "parser.cpp"
        break;

      case symbol_kind::S_varlist: // varlist
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 469 "parser.cpp"
        break;

      case symbol_kind::S_var: // var
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 475 "parser.cpp"
        break;

      case symbol_kind::S_explist: // explist
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 481 "parser.cpp"
        break;

      case symbol_kind::S_exp: // exp
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 487 "parser.cpp"
        break;

      case symbol_kind::S_prefixexp: // prefixexp
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 493 "parser.cpp"
        break;

      case symbol_kind::S_functioncall: // functioncall
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 499 "parser.cpp"
        break;

      case symbol_kind::S_args: // args
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 505 "parser.cpp"
        break;

      case symbol_kind::S_tableconstructor: // tableconstructor
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 511 "parser.cpp"
        break;

      case symbol_kind::S_fieldlist: // fieldlist
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 517 "parser.cpp"
        break;

      case symbol_kind::S_field: // field
#line 126 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 523 "parser.cpp"
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
#line 133 "parser.y"
        {
  		LOG(INFO) << "[bison]: chunk: " << "block";
  		l->set_chunk(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
	}
#line 817 "parser.cpp"
    break;

  case 3: // block: %empty
#line 141 "parser.y"
        {
  		LOG(INFO) << "[bison]: block: " << "empty";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_block>(yystack_[0].location);
  	}
#line 826 "parser.cpp"
    break;

  case 4: // block: stmt
#line 147 "parser.y"
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
#line 841 "parser.cpp"
    break;

  case 5: // block: block stmt
#line 159 "parser.y"
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
#line 861 "parser.cpp"
    break;

  case 6: // stmt: retstat
#line 178 "parser.y"
        {
        	LOG(INFO) << "[bison]: stmt: " << "retstat";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
        }
#line 870 "parser.cpp"
    break;

  case 7: // stmt: ";"
#line 184 "parser.y"
        {
		LOG(INFO) << "[bison]: stmt: " << "SEMICOLON";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_empty>(yystack_[0].location);
	}
#line 879 "parser.cpp"
    break;

  case 8: // stmt: varlist "=" explist
#line 190 "parser.y"
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
		auto assign = std::make_shared<fakelua::syntax_tree_assign>(varlist, explist, yystack_[1].location);
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = assign;
	}
#line 899 "parser.cpp"
    break;

  case 9: // stmt: label
#line 207 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "label";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
  	}
#line 908 "parser.cpp"
    break;

  case 10: // stmt: "break"
#line 213 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "break";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_break>(yystack_[0].location);
  	}
#line 917 "parser.cpp"
    break;

  case 11: // stmt: "goto" "identifier"
#line 219 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "goto IDENTIFIER";
  		auto go = std::make_shared<fakelua::syntax_tree_goto>(yystack_[0].location);
  		go->set_label(yystack_[0].value.as < std::string > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = go;
  	}
#line 928 "parser.cpp"
    break;

  case 12: // stmt: "do" block "end"
#line 227 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "do block end";
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
  	}
#line 937 "parser.cpp"
    break;

  case 13: // stmt: "while" exp "do" block "end"
#line 233 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "while exp do block end";
  		auto while_stmt = std::make_shared<fakelua::syntax_tree_while>(yystack_[4].location);
  		while_stmt->set_exp(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
  		while_stmt->set_block(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = while_stmt;
  	}
#line 949 "parser.cpp"
    break;

  case 14: // stmt: "repeat" block "until" exp
#line 242 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "repeat block until exp";
  		auto repeat = std::make_shared<fakelua::syntax_tree_repeat>(yystack_[3].location);
  		repeat->set_block(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > ());
  		repeat->set_exp(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = repeat;
  	}
#line 961 "parser.cpp"
    break;

  case 15: // stmt: "if" exp "then" block elseifs "else" block "end"
#line 251 "parser.y"
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
#line 980 "parser.cpp"
    break;

  case 16: // stmt: "if" exp "then" block elseifs "end"
#line 267 "parser.y"
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
#line 998 "parser.cpp"
    break;

  case 17: // stmt: "for" "identifier" "=" exp "," exp "do" block "end"
#line 282 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "for IDENTIFIER assign exp COMMA exp do block end";
  		auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(yystack_[8].location);
  		for_loop_stmt->set_name(yystack_[7].value.as < std::string > ());
  		for_loop_stmt->set_exp_begin(yystack_[5].value.as < fakelua::syntax_tree_interface_ptr > ());
  		for_loop_stmt->set_exp_end(yystack_[3].value.as < fakelua::syntax_tree_interface_ptr > ());
  		for_loop_stmt->set_block(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
  		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = for_loop_stmt;
  	}
#line 1012 "parser.cpp"
    break;

  case 18: // stmt: "for" "identifier" "=" exp "," exp "," exp "do" block "end"
#line 293 "parser.y"
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
#line 1027 "parser.cpp"
    break;

  case 19: // elseifs: %empty
#line 307 "parser.y"
        {
		LOG(INFO) << "[bison]: elseifs: " << "empty";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[0].location);
	}
#line 1036 "parser.cpp"
    break;

  case 20: // elseifs: "elseif" exp "then" block
#line 313 "parser.y"
        {
		LOG(INFO) << "[bison]: elseifs: " << "elseif exp then block";
		auto elseifs = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[3].location);
		elseifs->add_elseif(yystack_[2].value.as < fakelua::syntax_tree_interface_ptr > (), yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = elseifs;
	}
#line 1047 "parser.cpp"
    break;

  case 21: // elseifs: elseifs "elseif" exp "then" block
#line 321 "parser.y"
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
#line 1062 "parser.cpp"
    break;

  case 22: // retstat: "return"
#line 335 "parser.y"
        {
		LOG(INFO) << "[bison]: retstat: " << "RETURN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_return>(nullptr, yystack_[0].location);
	}
#line 1071 "parser.cpp"
    break;

  case 23: // retstat: "return" explist
#line 341 "parser.y"
        {
		LOG(INFO) << "[bison]: retstat: " << "RETURN explist";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_return>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > (), yystack_[1].location);
	}
#line 1080 "parser.cpp"
    break;

  case 24: // label: "::" "identifier" "::"
#line 349 "parser.y"
        {
		LOG(INFO) << "[bison]: bison get label: " << yystack_[1].value.as < std::string > () << " loc: " << yystack_[1].location;
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_label>(yystack_[1].value.as < std::string > (), yystack_[1].location);
	}
#line 1089 "parser.cpp"
    break;

  case 25: // varlist: var
#line 381 "parser.y"
        {
		LOG(INFO) << "[bison]: varlist: " << "var";
		auto varlist = std::make_shared<fakelua::syntax_tree_varlist>(yystack_[0].location);
		varlist->add_var(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = varlist;
	}
#line 1100 "parser.cpp"
    break;

  case 26: // varlist: varlist "," var
#line 389 "parser.y"
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
#line 1115 "parser.cpp"
    break;

  case 27: // var: "identifier"
#line 403 "parser.y"
        {
		LOG(INFO) << "[bison]: var: " << "IDENTIFIER";
		auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[0].location);
		var->set_name(yystack_[0].value.as < std::string > ());
		var->set_type("simple");
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
	}
#line 1127 "parser.cpp"
    break;

  case 28: // var: prefixexp "[" exp "]"
#line 412 "parser.y"
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
#line 1150 "parser.cpp"
    break;

  case 29: // var: prefixexp "." "identifier"
#line 432 "parser.y"
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
#line 1168 "parser.cpp"
    break;

  case 30: // namelist: "identifier"
#line 449 "parser.y"
        {
		LOG(INFO) << "[bison]: namelist: " << "IDENTIFIER";
	}
#line 1176 "parser.cpp"
    break;

  case 31: // namelist: namelist "," "identifier"
#line 454 "parser.y"
        {
		LOG(INFO) << "[bison]: namelist: " << "namelist COMMA IDENTIFIER";
	}
#line 1184 "parser.cpp"
    break;

  case 32: // explist: exp
#line 461 "parser.y"
        {
		LOG(INFO) << "[bison]: explist: " << "exp";
	}
#line 1192 "parser.cpp"
    break;

  case 33: // explist: explist "," exp
#line 466 "parser.y"
        {
		LOG(INFO) << "[bison]: explist: " << "explist COMMA exp";
	}
#line 1200 "parser.cpp"
    break;

  case 34: // exp: "nil"
#line 473 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "NIL";
	}
#line 1208 "parser.cpp"
    break;

  case 35: // exp: "true"
#line 478 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "TRUE";
	}
#line 1216 "parser.cpp"
    break;

  case 36: // exp: "false"
#line 483 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "FALSE";
	}
#line 1224 "parser.cpp"
    break;

  case 37: // exp: "number"
#line 488 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "NUMBER";
	}
#line 1232 "parser.cpp"
    break;

  case 38: // exp: "string"
#line 493 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "STRING";
	}
#line 1240 "parser.cpp"
    break;

  case 39: // exp: "..."
#line 498 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "VAR_PARAMS";
	}
#line 1248 "parser.cpp"
    break;

  case 40: // exp: functiondef
#line 503 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "functiondef";
	}
#line 1256 "parser.cpp"
    break;

  case 41: // exp: prefixexp
#line 508 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "prefixexp";
	}
#line 1264 "parser.cpp"
    break;

  case 42: // exp: tableconstructor
#line 513 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "tableconstructor";
	}
#line 1272 "parser.cpp"
    break;

  case 43: // exp: exp binop exp
#line 518 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "exp binop exp";
	}
#line 1280 "parser.cpp"
    break;

  case 44: // exp: unop exp
#line 523 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "unop exp";
	}
#line 1288 "parser.cpp"
    break;

  case 45: // prefixexp: var
#line 530 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "var";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1297 "parser.cpp"
    break;

  case 46: // prefixexp: functioncall
#line 536 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "functioncall";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1306 "parser.cpp"
    break;

  case 47: // prefixexp: "(" exp ")"
#line 542 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "LPAREN exp RPAREN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1315 "parser.cpp"
    break;

  case 48: // functioncall: prefixexp args
#line 549 "parser.y"
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
#line 1337 "parser.cpp"
    break;

  case 49: // functioncall: prefixexp ":" "identifier" args
#line 568 "parser.y"
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
#line 1360 "parser.cpp"
    break;

  case 50: // args: "(" explist ")"
#line 590 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "LPAREN explist RPAREN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1369 "parser.cpp"
    break;

  case 51: // args: "(" ")"
#line 596 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "LPAREN RPAREN";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_empty>(yystack_[1].location);
	}
#line 1378 "parser.cpp"
    break;

  case 52: // args: tableconstructor
#line 602 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "tableconstructor";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1387 "parser.cpp"
    break;

  case 53: // args: "string"
#line 608 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "STRING";
	}
#line 1395 "parser.cpp"
    break;

  case 54: // functiondef: "function" funcbody
#line 615 "parser.y"
        {
		LOG(INFO) << "[bison]: functiondef: " << "FUNCTION funcbody";
	}
#line 1403 "parser.cpp"
    break;

  case 55: // funcbody: "(" parlist ")" block "end"
#line 622 "parser.y"
        {
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN parlist RPAREN block END";
	}
#line 1411 "parser.cpp"
    break;

  case 56: // funcbody: "(" ")" block "end"
#line 627 "parser.y"
        {
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN RPAREN block END";
	}
#line 1419 "parser.cpp"
    break;

  case 57: // parlist: namelist
#line 634 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "namelist";
	}
#line 1427 "parser.cpp"
    break;

  case 58: // parlist: namelist "," "..."
#line 639 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "namelist COMMA VAR_PARAMS";
	}
#line 1435 "parser.cpp"
    break;

  case 59: // parlist: "..."
#line 644 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "VAR_PARAMS";
	}
#line 1443 "parser.cpp"
    break;

  case 60: // tableconstructor: "{" fieldlist "}"
#line 651 "parser.y"
        {
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY fieldlist RCURLY";
		auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[2].location);
		tableconstructor->set_fieldlist(yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ());
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
	}
#line 1454 "parser.cpp"
    break;

  case 61: // tableconstructor: "{" "}"
#line 659 "parser.y"
        {
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY RCURLY";
		auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[1].location);
		tableconstructor->set_fieldlist(std::make_shared<fakelua::syntax_tree_empty>(yystack_[1].location));
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
	}
#line 1465 "parser.cpp"
    break;

  case 62: // fieldlist: field
#line 669 "parser.y"
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
#line 1481 "parser.cpp"
    break;

  case 63: // fieldlist: fieldlist fieldsep field
#line 682 "parser.y"
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
#line 1501 "parser.cpp"
    break;

  case 64: // field: "[" exp "]" "=" exp
#line 701 "parser.y"
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
#line 1523 "parser.cpp"
    break;

  case 65: // field: "identifier" "=" exp
#line 720 "parser.y"
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
#line 1540 "parser.cpp"
    break;

  case 66: // field: exp
#line 734 "parser.y"
        {
		LOG(INFO) << "[bison]: field: " << "exp";
		yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
	}
#line 1549 "parser.cpp"
    break;

  case 67: // fieldsep: ","
#line 743 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldsep: " << "COMMA";
	}
#line 1557 "parser.cpp"
    break;

  case 68: // fieldsep: ";"
#line 748 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldsep: " << "SEMICOLON";
	}
#line 1565 "parser.cpp"
    break;

  case 69: // binop: "+"
#line 755 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "PLUS";
	}
#line 1573 "parser.cpp"
    break;

  case 70: // binop: "-"
#line 760 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MINUS";
	}
#line 1581 "parser.cpp"
    break;

  case 71: // binop: "*"
#line 765 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "STAR";
	}
#line 1589 "parser.cpp"
    break;

  case 72: // binop: "/"
#line 770 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "SLASH";
	}
#line 1597 "parser.cpp"
    break;

  case 73: // binop: "//"
#line 775 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "DOUBLE_SLASH";
	}
#line 1605 "parser.cpp"
    break;

  case 74: // binop: "^"
#line 780 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "XOR";
	}
#line 1613 "parser.cpp"
    break;

  case 75: // binop: "%"
#line 785 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MOD";
	}
#line 1621 "parser.cpp"
    break;

  case 76: // binop: "&"
#line 790 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITAND";
	}
#line 1629 "parser.cpp"
    break;

  case 77: // binop: "~"
#line 795 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITNOT";
	}
#line 1637 "parser.cpp"
    break;

  case 78: // binop: "|"
#line 800 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITOR";
	}
#line 1645 "parser.cpp"
    break;

  case 79: // binop: ">>"
#line 805 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "RIGHT_SHIFT";
	}
#line 1653 "parser.cpp"
    break;

  case 80: // binop: "<<"
#line 810 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LEFT_SHIFT";
	}
#line 1661 "parser.cpp"
    break;

  case 81: // binop: ".."
#line 815 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "CONCAT";
	}
#line 1669 "parser.cpp"
    break;

  case 82: // binop: "<"
#line 820 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LESS";
	}
#line 1677 "parser.cpp"
    break;

  case 83: // binop: "<="
#line 825 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LESS_EQUAL";
	}
#line 1685 "parser.cpp"
    break;

  case 84: // binop: ">"
#line 830 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MORE";
	}
#line 1693 "parser.cpp"
    break;

  case 85: // binop: ">="
#line 835 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MORE_EQUAL";
	}
#line 1701 "parser.cpp"
    break;

  case 86: // binop: "=="
#line 840 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "EQUAL";
	}
#line 1709 "parser.cpp"
    break;

  case 87: // binop: "~="
#line 845 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "NOT_EQUAL";
	}
#line 1717 "parser.cpp"
    break;

  case 88: // binop: "and"
#line 850 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "AND";
	}
#line 1725 "parser.cpp"
    break;

  case 89: // binop: "or"
#line 855 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "OR";
	}
#line 1733 "parser.cpp"
    break;

  case 90: // unop: "-"
#line 862 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "MINUS";
	}
#line 1741 "parser.cpp"
    break;

  case 91: // unop: "not"
#line 867 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "NOT";
	}
#line 1749 "parser.cpp"
    break;

  case 92: // unop: "#"
#line 872 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "NUMBER_SIGN";
	}
#line 1757 "parser.cpp"
    break;

  case 93: // unop: "~"
#line 877 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "BITNOT";
	}
#line 1765 "parser.cpp"
    break;


#line 1769 "parser.cpp"

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


  const signed char parser::yypact_ninf_ = -49;

  const signed char parser::yytable_ninf_ = -27;

  const short
  parser::yypact_[] =
  {
    1186,   262,   -49,  1186,   -43,   262,  1186,   262,   262,   -42,
     -36,   -49,   -49,    26,  1186,   -49,   -49,   -49,    17,    21,
       9,   -49,   -49,   189,   -49,    19,   -49,   -49,   -49,   -49,
     -49,   -49,   -49,   -49,   -49,   372,     9,   -49,   -49,   262,
     898,    33,   425,   930,    -7,   849,   478,   -49,    -2,   -49,
     -49,   262,     1,   204,   262,   -16,   -14,   -49,   -49,   -49,
     -49,   262,    43,   849,    -1,   -49,     2,   -49,   -49,   -49,
     -49,   -49,   -49,   -49,   -49,   -49,   -49,   -49,   -49,   -49,
     -49,   -49,   -49,   -49,   -49,   -49,   -49,   -49,   -49,   -49,
     262,   849,   -49,   262,  1186,   262,   262,  1186,   -49,    -7,
      22,   -49,    28,   531,    -4,   -49,   584,   262,   -49,   -49,
     -49,   247,  1186,   -49,   -49,    14,    39,   849,   637,   962,
     849,   849,   994,   -49,   -49,   -49,    46,   849,   -49,  1026,
     -25,  1186,   262,   262,    12,   -49,   262,   -49,   -49,   -49,
    1058,   319,   690,  1186,   262,   -49,   849,   -49,  1186,   262,
    1186,  1090,   743,  1122,   796,  1186,   -49,  1186,   -49,  1186,
    1186,  1154,   -49
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,    10,     3,     0,     0,     3,    22,     0,     0,
       0,     7,    27,     0,     2,     4,     6,     9,     0,    45,
       0,    46,    90,     0,    36,     0,    34,    91,    35,    39,
      93,    92,    38,    37,    45,     0,    41,    40,    42,     0,
       0,     0,     0,     0,    23,    32,     0,    11,     0,     1,
       5,     0,     0,     0,     0,     0,     0,    53,    48,    52,
      61,     0,    27,    66,     0,    62,     0,    54,    70,    69,
      71,    72,    47,    88,    89,    73,    81,    86,    85,    83,
      87,    80,    79,    74,    75,    76,    78,    77,    84,    82,
       0,    44,    12,     0,     3,     0,     0,     3,    24,     8,
      45,    51,     0,     0,     0,    29,     0,     0,    60,    68,
      67,     0,     3,    59,    30,    57,     0,    43,     0,    19,
      14,    33,     0,    50,    28,    49,     0,    65,    63,     0,
       0,     3,     0,     0,     0,    13,     0,    56,    58,    31,
       0,     0,     0,     3,     0,    16,    64,    55,     3,     0,
       3,     0,     0,     0,     0,    20,    15,     3,    17,     3,
      21,     0,    18
  };

  const signed char
  parser::yypgoto_[] =
  {
     -49,   -49,    -3,    24,   -49,   -49,   -49,   -49,     0,   -49,
     -39,    96,    27,   -49,   -47,   -49,   -49,   -49,   -18,   -49,
     -48,   -49,   -49,   -49
  };

  const unsigned char
  parser::yydefgoto_[] =
  {
       0,    13,    14,    15,   134,    16,    17,    18,    19,   115,
      44,    45,    20,    21,    58,    37,    67,   116,    38,    64,
      65,   111,    90,    39
  };

  const short
  parser::yytable_[] =
  {
      40,    34,    59,    43,    53,    34,    23,    34,    34,     1,
     108,   112,    99,   138,   102,    41,    47,    53,    59,    23,
      51,    54,    48,    34,   -25,   -26,    49,    66,    36,   143,
     144,   145,    36,   139,    36,    36,    93,   123,    50,    34,
     113,    96,   104,    98,   105,   109,   107,   110,   131,   136,
      36,    34,   100,    34,    34,    57,    55,   125,    56,    12,
     114,    34,   130,   128,    50,    52,    36,    50,    57,   -25,
     -26,     0,     0,     0,     0,     0,    96,     0,    36,     0,
      36,    36,     0,     0,     0,     0,    59,     0,    36,     0,
      34,   119,     0,    34,   122,    34,    34,    35,     0,     0,
       0,    42,     0,     0,    46,     0,     0,    34,     0,   129,
       0,    34,     0,     0,     0,     0,     0,    36,     0,    63,
      36,     0,    36,    36,     0,     0,     0,     0,   140,     0,
       0,     0,    34,    34,    36,    91,    34,     0,    36,     0,
     151,     0,     0,    50,    34,   153,    50,   155,     0,    34,
     103,     0,     0,    50,   160,     0,   161,   106,     0,    36,
      36,     0,     0,    36,    50,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,    50,    36,    50,     0,    50,
       0,     0,     0,     0,    50,    50,   117,     0,     0,   118,
       0,   120,   121,    22,     0,     0,     0,     1,     0,    23,
      60,    61,     0,   127,     0,     0,     0,    63,    22,    24,
       0,    25,     1,   101,    23,    26,    27,     0,     0,     0,
       0,    28,     0,     0,    24,     0,    25,    29,   141,   142,
      26,    27,   146,     0,     0,     0,    28,     0,     0,     0,
     152,     0,    29,    30,     0,   154,    31,    62,    32,    33,
       0,    22,     0,     0,     0,     1,     0,    23,    30,    61,
       0,    31,    12,    32,    33,     0,    22,    24,     0,    25,
       1,     0,    23,    26,    27,     0,     0,     0,     0,    28,
       0,     0,    24,     0,    25,    29,     0,     0,    26,    27,
       0,     0,     0,     0,    28,     0,     0,     0,     0,     0,
      29,    30,     0,     0,    31,    62,    32,    33,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,    31,
      12,    32,    33,    68,    69,    70,    71,     0,     0,     0,
       0,     0,     0,    73,     0,   148,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,     0,     0,     0,    75,    76,     0,    77,    78,
      79,    80,    81,    82,     0,     0,     0,   149,     0,    83,
      84,    85,    86,    87,    88,    89,    68,    69,    70,    71,
       0,    72,     0,     0,     0,     0,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,     0,     0,     0,    75,    76,
       0,    77,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    68,
      69,    70,    71,     0,     0,     0,     0,     0,     0,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,     0,     0,    94,     0,     0,     0,
       0,    75,    76,     0,    77,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    68,    69,    70,    71,     0,     0,     0,     0,
       0,     0,    73,     0,    97,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,     0,     0,     0,    75,    76,     0,    77,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    68,    69,    70,    71,     0,
       0,     0,     0,     0,   124,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,     0,     0,     0,    75,    76,     0,
      77,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    68,    69,
      70,    71,     0,     0,     0,     0,     0,   126,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,     0,     0,     0,
      75,    76,     0,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    68,    69,    70,    71,     0,     0,     0,     0,     0,
       0,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
       0,     0,     0,    75,    76,     0,    77,    78,    79,    80,
      81,    82,     0,     0,     0,   132,     0,    83,    84,    85,
      86,    87,    88,    89,    68,    69,    70,    71,     0,     0,
       0,     0,     0,     0,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,     0,
       0,   150,     0,     0,     0,     0,    75,    76,     0,    77,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    68,    69,    70,
      71,     0,     0,     0,     0,     0,     0,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,     0,     0,   157,     0,     0,     0,     0,    75,
      76,     0,    77,    78,    79,    80,    81,    82,     0,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      68,    69,    70,    71,     0,     0,     0,     0,     0,     0,
      73,     0,   159,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,     0,
       0,     0,    75,    76,     0,    77,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,    83,    84,    85,    86,
      87,    88,    89,    68,    69,    70,    71,     0,     0,     0,
       0,     0,     0,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,     0,     0,     0,    75,    76,     0,    77,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,     1,     0,     0,     0,
       0,     0,     0,     2,     3,     0,     0,    92,     0,     4,
       0,     5,     0,     0,     0,     0,     0,     6,     7,     0,
       0,     0,     8,     9,     0,     0,     0,     0,     1,     0,
       0,     0,     0,    10,    11,     2,     3,     0,     0,     0,
       0,     4,     0,     5,     0,     0,    12,     0,     0,     6,
       7,     0,     0,    95,     8,     9,     0,     0,     0,     0,
       1,     0,     0,     0,     0,    10,    11,     2,     3,     0,
     133,     0,     0,     4,     0,     5,     0,     0,    12,     0,
       0,     6,     7,     0,     0,     0,     8,     9,     0,     0,
       0,     0,     1,     0,     0,     0,     0,    10,    11,     2,
       3,     0,     0,   135,     0,     4,     0,     5,     0,     0,
      12,     0,     0,     6,     7,     0,     0,     0,     8,     9,
       0,     0,     0,     0,     1,     0,     0,     0,     0,    10,
      11,     2,     3,     0,     0,   137,     0,     4,     0,     5,
       0,     0,    12,     0,     0,     6,     7,     0,     0,     0,
       8,     9,     0,     0,     0,     0,     1,     0,     0,     0,
       0,    10,    11,     2,     3,     0,     0,   147,     0,     4,
       0,     5,     0,     0,    12,     0,     0,     6,     7,     0,
       0,     0,     8,     9,     0,     0,     0,     0,     1,     0,
       0,     0,     0,    10,    11,     2,     3,     0,     0,   156,
       0,     4,     0,     5,     0,     0,    12,     0,     0,     6,
       7,     0,     0,     0,     8,     9,     0,     0,     0,     0,
       1,     0,     0,     0,     0,    10,    11,     2,     3,     0,
       0,   158,     0,     4,     0,     5,     0,     0,    12,     0,
       0,     6,     7,     0,     0,     0,     8,     9,     0,     0,
       0,     0,     1,     0,     0,     0,     0,    10,    11,     2,
       3,     0,     0,   162,     0,     4,     0,     5,     0,     0,
      12,     0,     0,     6,     7,     0,     0,     0,     8,     9,
       0,     0,     0,     0,     1,     0,     0,     0,     0,    10,
      11,     2,     3,     0,     0,     0,     0,     4,     0,     5,
       0,     0,    12,     0,     0,     6,     7,     0,     0,     0,
       8,     9,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    10,    11,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12
  };

  const short
  parser::yycheck_[] =
  {
       3,     1,    20,     6,     8,     5,    10,     7,     8,     8,
      11,     9,    51,    38,    53,    58,    58,     8,    36,    10,
       3,    12,    58,    23,     3,     3,     0,     8,     1,    17,
      18,    19,     5,    58,     7,     8,     3,     9,    14,    39,
      38,    48,    58,    45,    58,    46,     3,    48,     9,     3,
      23,    51,    52,    53,    54,    59,    47,   104,    49,    58,
      58,    61,    48,   111,    40,    48,    39,    43,    59,    48,
      48,    -1,    -1,    -1,    -1,    -1,    48,    -1,    51,    -1,
      53,    54,    -1,    -1,    -1,    -1,   104,    -1,    61,    -1,
      90,    94,    -1,    93,    97,    95,    96,     1,    -1,    -1,
      -1,     5,    -1,    -1,     8,    -1,    -1,   107,    -1,   112,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    90,    -1,    23,
      93,    -1,    95,    96,    -1,    -1,    -1,    -1,   131,    -1,
      -1,    -1,   132,   133,   107,    39,   136,    -1,   111,    -1,
     143,    -1,    -1,   119,   144,   148,   122,   150,    -1,   149,
      54,    -1,    -1,   129,   157,    -1,   159,    61,    -1,   132,
     133,    -1,    -1,   136,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   144,    -1,    -1,    -1,   151,   149,   153,    -1,   155,
      -1,    -1,    -1,    -1,   160,   161,    90,    -1,    -1,    93,
      -1,    95,    96,     4,    -1,    -1,    -1,     8,    -1,    10,
      11,    12,    -1,   107,    -1,    -1,    -1,   111,     4,    20,
      -1,    22,     8,     9,    10,    26,    27,    -1,    -1,    -1,
      -1,    32,    -1,    -1,    20,    -1,    22,    38,   132,   133,
      26,    27,   136,    -1,    -1,    -1,    32,    -1,    -1,    -1,
     144,    -1,    38,    54,    -1,   149,    57,    58,    59,    60,
      -1,     4,    -1,    -1,    -1,     8,    -1,    10,    54,    12,
      -1,    57,    58,    59,    60,    -1,     4,    20,    -1,    22,
       8,    -1,    10,    26,    27,    -1,    -1,    -1,    -1,    32,
      -1,    -1,    20,    -1,    22,    38,    -1,    -1,    26,    27,
      -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      38,    54,    -1,    -1,    57,    58,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    57,
      58,    59,    60,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,     4,     5,     6,     7,
      -1,     9,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
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
      -1,    -1,    -1,    -1,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      50,    51,    52,    53,    54,    55,    56,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    31,    -1,    -1,    -1,    -1,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    54,    55,    56,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,
      51,    52,    53,    54,    55,    56,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    -1,    -1,    19,    -1,    21,
      -1,    23,    -1,    -1,    -1,    -1,    -1,    29,    30,    -1,
      -1,    -1,    34,    35,    -1,    -1,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    45,    46,    15,    16,    -1,    -1,    -1,
      -1,    21,    -1,    23,    -1,    -1,    58,    -1,    -1,    29,
      30,    -1,    -1,    33,    34,    35,    -1,    -1,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    45,    46,    15,    16,    -1,
      18,    -1,    -1,    21,    -1,    23,    -1,    -1,    58,    -1,
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
      46,    15,    16,    -1,    -1,    -1,    -1,    21,    -1,    23,
      -1,    -1,    58,    -1,    -1,    29,    30,    -1,    -1,    -1,
      34,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    58
  };

  const signed char
  parser::yystos_[] =
  {
       0,     8,    15,    16,    21,    23,    29,    30,    34,    35,
      45,    46,    58,    62,    63,    64,    66,    67,    68,    69,
      73,    74,     4,    10,    20,    22,    26,    27,    32,    38,
      54,    57,    59,    60,    69,    72,    73,    76,    79,    84,
      63,    58,    72,    63,    71,    72,    72,    58,    58,     0,
      64,     3,    48,     8,    12,    47,    49,    59,    75,    79,
      11,    12,    58,    72,    80,    81,     8,    77,     4,     5,
       6,     7,     9,    14,    28,    36,    37,    39,    40,    41,
      42,    43,    44,    50,    51,    52,    53,    54,    55,    56,
      83,    72,    19,     3,    31,    33,    48,    16,    45,    71,
      69,     9,    71,    72,    58,    58,    72,     3,    11,    46,
      48,    82,     9,    38,    58,    70,    78,    72,    72,    63,
      72,    72,    63,     9,    13,    75,    13,    72,    81,    63,
      48,     9,    48,    18,    65,    19,     3,    19,    38,    58,
      63,    72,    72,    17,    18,    19,    72,    19,    16,    48,
      31,    63,    72,    63,    72,    63,    19,    31,    19,    16,
      63,    63,    19
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    61,    62,    63,    63,    63,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    65,
      65,    65,    66,    66,    67,    68,    68,    69,    69,    69,
      70,    70,    71,    71,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    73,    73,    73,    74,    74,
      75,    75,    75,    75,    76,    77,    77,    78,    78,    78,
      79,    79,    80,    80,    81,    81,    81,    82,    82,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      84,    84,    84,    84
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     1,     2,     1,     1,     3,     1,
       1,     2,     3,     5,     4,     8,     6,     9,    11,     0,
       4,     5,     1,     2,     3,     1,     3,     1,     4,     3,
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
       0,   132,   132,   140,   146,   158,   177,   183,   189,   206,
     212,   218,   226,   232,   241,   250,   266,   281,   292,   306,
     312,   320,   334,   340,   348,   380,   388,   402,   411,   431,
     448,   453,   460,   465,   472,   477,   482,   487,   492,   497,
     502,   507,   512,   517,   522,   529,   535,   541,   548,   567,
     589,   595,   601,   607,   614,   621,   626,   633,   638,   643,
     650,   658,   668,   681,   700,   719,   733,   742,   747,   754,
     759,   764,   769,   774,   779,   784,   789,   794,   799,   804,
     809,   814,   819,   824,   829,   834,   839,   844,   849,   854,
     861,   866,   871,   876
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
#line 2655 "parser.cpp"

#line 881 "parser.y"


void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
