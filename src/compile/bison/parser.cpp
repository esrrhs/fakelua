// A Bison parser, made by GNU Bison 3.8.2.

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

#include "compile/my_flexer.h"

yy::parser::symbol_type yylex(fakelua::MyFlexer* l) {
    auto ret = l->MyYylex();
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
  parser::parser (fakelua::MyFlexer* l_yyarg)
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

  /*---------.
  | symbol.  |
  `---------*/



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
        value.YY_MOVE_OR_COPY< fakelua::SyntaxTreeInterfacePtr > (YY_MOVE (that.value));
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
        value.move< fakelua::SyntaxTreeInterfacePtr > (YY_MOVE (that.value));
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
        value.copy< fakelua::SyntaxTreeInterfacePtr > (that.value);
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
        value.move< fakelua::SyntaxTreeInterfacePtr > (that.value);
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
#line 157 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 433 "parser.cpp"
        break;

      case symbol_kind::S_STRING: // "string"
#line 157 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 439 "parser.cpp"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 157 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 445 "parser.cpp"
        break;

      case symbol_kind::S_chunk: // chunk
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 451 "parser.cpp"
        break;

      case symbol_kind::S_block: // block
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 457 "parser.cpp"
        break;

      case symbol_kind::S_stmt: // stmt
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 463 "parser.cpp"
        break;

      case symbol_kind::S_attnamelist: // attnamelist
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 469 "parser.cpp"
        break;

      case symbol_kind::S_elseifs: // elseifs
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 475 "parser.cpp"
        break;

      case symbol_kind::S_retstat: // retstat
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 481 "parser.cpp"
        break;

      case symbol_kind::S_label: // label
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 487 "parser.cpp"
        break;

      case symbol_kind::S_funcnamelist: // funcnamelist
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 493 "parser.cpp"
        break;

      case symbol_kind::S_funcname: // funcname
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 499 "parser.cpp"
        break;

      case symbol_kind::S_varlist: // varlist
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 505 "parser.cpp"
        break;

      case symbol_kind::S_var: // var
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 511 "parser.cpp"
        break;

      case symbol_kind::S_namelist: // namelist
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 517 "parser.cpp"
        break;

      case symbol_kind::S_explist: // explist
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 523 "parser.cpp"
        break;

      case symbol_kind::S_exp: // exp
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 529 "parser.cpp"
        break;

      case symbol_kind::S_prefixexp: // prefixexp
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 535 "parser.cpp"
        break;

      case symbol_kind::S_functioncall: // functioncall
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 541 "parser.cpp"
        break;

      case symbol_kind::S_args: // args
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 547 "parser.cpp"
        break;

      case symbol_kind::S_functiondef: // functiondef
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 553 "parser.cpp"
        break;

      case symbol_kind::S_funcbody: // funcbody
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 559 "parser.cpp"
        break;

      case symbol_kind::S_parlist: // parlist
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 565 "parser.cpp"
        break;

      case symbol_kind::S_tableconstructor: // tableconstructor
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 571 "parser.cpp"
        break;

      case symbol_kind::S_fieldlist: // fieldlist
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 577 "parser.cpp"
        break;

      case symbol_kind::S_field: // field
#line 157 "parser.y"
                 { yyo << yysym.value.template as < fakelua::SyntaxTreeInterfacePtr > (); }
#line 583 "parser.cpp"
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
  parser::yypop_ (int n) YY_NOEXCEPT
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
  parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
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
    (void) yynerrs_;

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
        yylhs.value.emplace< fakelua::SyntaxTreeInterfacePtr > ();
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
#line 164 "parser.y"
    {
    LOG_INFO("[bison]: chunk: block");
    l->SetChunk(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
    }
#line 881 "parser.cpp"
    break;

  case 3: // block: %empty
#line 172 "parser.y"
    {
        LOG_INFO("[bison]: block: empty");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = std::make_shared<fakelua::SyntaxTreeBlock>(yystack_[0].location);
    }
#line 890 "parser.cpp"
    break;

  case 4: // block: block stmt
#line 178 "parser.y"
    {
        LOG_INFO("[bison]: block: block stmt");
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: block: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        auto stmt = std::dynamic_pointer_cast<fakelua::SyntaxTreeInterface>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (stmt == nullptr) {
            LOG_ERROR("[bison]: block: stmt is not a stmt");
            fakelua::ThrowFakeluaException("stmt is not a stmt");
        }
        if (block->Stmts().empty()) {
            block->SetLoc(yystack_[0].location);
        }
        block->AddStmt(stmt);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = block;
    }
#line 913 "parser.cpp"
    break;

  case 5: // stmt: retstat
#line 200 "parser.y"
    {
        LOG_INFO("[bison]: stmt: retstat");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ();
    }
#line 922 "parser.cpp"
    break;

  case 6: // stmt: ";"
#line 206 "parser.y"
    {
        LOG_INFO("[bison]: stmt: SEMICOLON");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = std::make_shared<fakelua::SyntaxTreeEmpty>(yystack_[0].location);
    }
#line 931 "parser.cpp"
    break;

  case 7: // stmt: varlist "=" explist
#line 212 "parser.y"
    {
        LOG_INFO("[bison]: stmt: varlist ASSIGN explist");
        auto varlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeVarlist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (varlist == nullptr) {
            LOG_ERROR("[bison]: stmt: varlist is not a varlist");
            fakelua::ThrowFakeluaException("varlist is not a varlist");
        }
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        auto assign = std::make_shared<fakelua::SyntaxTreeAssign>(yystack_[1].location);
        assign->SetVarlist(varlist);
        assign->SetExplist(explist);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = assign;
    }
#line 953 "parser.cpp"
    break;

  case 8: // stmt: functioncall
#line 231 "parser.y"
    {
        LOG_INFO("[bison]: stmt: functioncall");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ();
    }
#line 962 "parser.cpp"
    break;

  case 9: // stmt: label
#line 237 "parser.y"
    {
        LOG_INFO("[bison]: stmt: label");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ();
    }
#line 971 "parser.cpp"
    break;

  case 10: // stmt: "break"
#line 243 "parser.y"
    {
        LOG_INFO("[bison]: stmt: BREAK");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = std::make_shared<fakelua::SyntaxTreeBreak>(yystack_[0].location);
    }
#line 980 "parser.cpp"
    break;

  case 11: // stmt: "goto" "identifier"
#line 249 "parser.y"
    {
        LOG_INFO("[bison]: stmt: GOTO IDENTIFIER");
        auto go = std::make_shared<fakelua::SyntaxTreeGoto>(yystack_[0].location);
        go->SetLabel(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = go;
    }
#line 991 "parser.cpp"
    break;

  case 12: // stmt: "do" block "end"
#line 257 "parser.y"
    {
        LOG_INFO("[bison]: stmt: DO block END");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ();
    }
#line 1000 "parser.cpp"
    break;

  case 13: // stmt: "while" exp "do" block "end"
#line 263 "parser.y"
    {
        LOG_INFO("[bison]: stmt: WHILE exp DO block END");
        auto while_stmt = std::make_shared<fakelua::SyntaxTreeWhile>(yystack_[4].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[3].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        while_stmt->SetExp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        while_stmt->SetBlock(block);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = while_stmt;
    }
#line 1022 "parser.cpp"
    break;

  case 14: // stmt: "repeat" block "until" exp
#line 282 "parser.y"
    {
        LOG_INFO("[bison]: stmt: REPEAT block UNTIL exp");
        auto repeat = std::make_shared<fakelua::SyntaxTreeRepeat>(yystack_[3].location);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        repeat->SetBlock(block);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        repeat->SetExp(exp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = repeat;
    }
#line 1044 "parser.cpp"
    break;

  case 15: // stmt: "if" exp "then" block elseifs "else" block "end"
#line 301 "parser.y"
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs ELSE block END");
        auto if_stmt = std::make_shared<fakelua::SyntaxTreeIf>(yystack_[7].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[6].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        if_stmt->SetExp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[4].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        if_stmt->SetBlock(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>(yystack_[3].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::ThrowFakeluaException("elseiflist is not a elseiflist");
        }
        if_stmt->SetElseiflist(elseifs);
        auto else_block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (else_block == nullptr) {
            LOG_ERROR("[bison]: stmt: else_block is not a block");
            fakelua::ThrowFakeluaException("else_block is not a block");
        }
        if_stmt->SetElseBlock(else_block);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = if_stmt;
    }
#line 1078 "parser.cpp"
    break;

  case 16: // stmt: "if" exp "then" block elseifs "end"
#line 332 "parser.y"
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs END");
        auto if_stmt = std::make_shared<fakelua::SyntaxTreeIf>(yystack_[5].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[4].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        if_stmt->SetExp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        if_stmt->SetBlock(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::ThrowFakeluaException("elseiflist is not a elseiflist");
        }
        if_stmt->SetElseiflist(elseifs);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = if_stmt;
    }
#line 1106 "parser.cpp"
    break;

  case 17: // stmt: "for" "identifier" "=" exp "," exp "do" block "end"
#line 357 "parser.y"
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::SyntaxTreeForLoop>(yystack_[8].location);
        for_loop_stmt->SetName(yystack_[7].value.as < std::string > ());
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[5].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        for_loop_stmt->SetExpBegin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[3].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::ThrowFakeluaException("end_exp is not a exp");
        }
        for_loop_stmt->SetExpEnd(end_exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        for_loop_stmt->SetBlock(block);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = for_loop_stmt;
    }
#line 1135 "parser.cpp"
    break;

  case 18: // stmt: "for" "identifier" "=" exp "," exp "," exp "do" block "end"
#line 383 "parser.y"
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::SyntaxTreeForLoop>(yystack_[10].location);
        for_loop_stmt->SetName(yystack_[9].value.as < std::string > ());
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[7].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        for_loop_stmt->SetExpBegin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[5].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::ThrowFakeluaException("end_exp is not a exp");
        }
        for_loop_stmt->SetExpEnd(end_exp);
        auto step_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[3].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (step_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: step_exp is not a exp");
            fakelua::ThrowFakeluaException("step_exp is not a exp");
        }
        for_loop_stmt->SetExpStep(step_exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        for_loop_stmt->SetBlock(block);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = for_loop_stmt;
    }
#line 1170 "parser.cpp"
    break;

  case 19: // stmt: "for" namelist "in" explist "do" block "end"
#line 415 "parser.y"
    {
        LOG_INFO("[bison]: stmt: for namelist in explist do block end");
        auto for_in_stmt = std::make_shared<fakelua::SyntaxTreeForIn>(yystack_[6].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[5].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        for_in_stmt->SetNamelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[3].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        for_in_stmt->SetExplist(explist);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        for_in_stmt->SetBlock(block);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = for_in_stmt;
    }
#line 1198 "parser.cpp"
    break;

  case 20: // stmt: "function" funcname funcbody
#line 440 "parser.y"
    {
        LOG_INFO("[bison]: stmt: function funcname funcbody");
        auto func_stmt = std::make_shared<fakelua::SyntaxTreeFunction>(yystack_[2].location);
        auto funcname = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncname>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (funcname == nullptr) {
            LOG_ERROR("[bison]: stmt: funcname is not a funcname");
            fakelua::ThrowFakeluaException("funcname is not a funcname");
        }
        func_stmt->SetFuncname(funcname);
        auto funcbody = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::ThrowFakeluaException("funcbody is not a funcbody");
        }
        func_stmt->SetFuncbody(funcbody);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = func_stmt;
    }
#line 1220 "parser.cpp"
    break;

  case 21: // stmt: "local" "function" "identifier" funcbody
#line 459 "parser.y"
    {
        LOG_INFO("[bison]: stmt: local function IDENTIFIER funcbody");
        auto local_func_stmt = std::make_shared<fakelua::SyntaxTreeLocalFunction>(yystack_[3].location);
        local_func_stmt->SetName(yystack_[1].value.as < std::string > ());
        auto funcbody = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::ThrowFakeluaException("funcbody is not a funcbody");
        }
        local_func_stmt->SetFuncbody(funcbody);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = local_func_stmt;
    }
#line 1237 "parser.cpp"
    break;

  case 22: // stmt: "local" attnamelist
#line 473 "parser.y"
    {
        LOG_INFO("[bison]: stmt: local attnamelist");
        auto local_stmt = std::make_shared<fakelua::SyntaxTreeLocalVar>(yystack_[1].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        local_stmt->SetNamelist(namelist);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = local_stmt;
    }
#line 1253 "parser.cpp"
    break;

  case 23: // stmt: "local" attnamelist "=" explist
#line 486 "parser.y"
    {
        LOG_INFO("[bison]: stmt: local attnamelist assign explist");
        auto local_stmt = std::make_shared<fakelua::SyntaxTreeLocalVar>(yystack_[3].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        local_stmt->SetNamelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        local_stmt->SetExplist(explist);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = local_stmt;
    }
#line 1275 "parser.cpp"
    break;

  case 24: // attnamelist: "identifier"
#line 507 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::SyntaxTreeNamelist>(yystack_[0].location);
        namelist->AddName(yystack_[0].value.as < std::string > ());
        namelist->AddAttrib("");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1287 "parser.cpp"
    break;

  case 25: // attnamelist: "identifier" "<" "identifier" ">"
#line 516 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::make_shared<fakelua::SyntaxTreeNamelist>(yystack_[3].location);
        namelist->AddName(yystack_[3].value.as < std::string > ());
        namelist->AddAttrib(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1299 "parser.cpp"
    break;

  case 26: // attnamelist: attnamelist "," "identifier"
#line 525 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        namelist->AddName(yystack_[0].value.as < std::string > ());
        namelist->AddAttrib("");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1315 "parser.cpp"
    break;

  case 27: // attnamelist: attnamelist "," "identifier" "<" "identifier" ">"
#line 538 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[5].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        namelist->AddName(yystack_[3].value.as < std::string > ());
        namelist->AddAttrib(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1331 "parser.cpp"
    break;

  case 28: // elseifs: %empty
#line 553 "parser.y"
    {
        LOG_INFO("[bison]: elseifs: empty");
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = std::make_shared<fakelua::SyntaxTreeElseiflist>(yystack_[0].location);
    }
#line 1340 "parser.cpp"
    break;

  case 29: // elseifs: "elseif" exp "then" block
#line 559 "parser.y"
    {
        LOG_INFO("[bison]: elseifs: elseif exp then block");
        auto elseifs = std::make_shared<fakelua::SyntaxTreeElseiflist>(yystack_[3].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        elseifs->AddElseifExpr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        elseifs->AddElseifBlock(block);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = elseifs;
    }
#line 1362 "parser.cpp"
    break;

  case 30: // elseifs: elseifs "elseif" exp "then" block
#line 578 "parser.y"
    {
        LOG_INFO("[bison]: elseifs: elseifs elseif exp then block");
        auto elseifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>(yystack_[4].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: elseifs: elseifs is not a elseifs");
            fakelua::ThrowFakeluaException("elseifs is not a elseifs");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        elseifs->AddElseifExpr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        elseifs->AddElseifBlock(block);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = elseifs;
    }
#line 1388 "parser.cpp"
    break;

  case 31: // retstat: "return"
#line 603 "parser.y"
    {
        LOG_INFO("[bison]: retstat: RETURN");
        auto ret = std::make_shared<fakelua::SyntaxTreeReturn>(yystack_[0].location);
        ret->SetExplist(nullptr);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = ret;
    }
#line 1399 "parser.cpp"
    break;

  case 32: // retstat: "return" explist
#line 611 "parser.y"
    {
        LOG_INFO("[bison]: retstat: RETURN explist");
        auto ret = std::make_shared<fakelua::SyntaxTreeReturn>(yystack_[1].location);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: retstat: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        ret->SetExplist(explist);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = ret;
    }
#line 1415 "parser.cpp"
    break;

  case 33: // label: "::" "identifier" "::"
#line 626 "parser.y"
    {
            LOG_INFO("[bison]: label: GOTO_TAG IDENTIFIER GOTO_TAG");
        auto ret = std::make_shared<fakelua::SyntaxTreeLabel>(yystack_[1].location);
        ret->SetName(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = ret;
    }
#line 1426 "parser.cpp"
    break;

  case 34: // funcnamelist: "identifier"
#line 636 "parser.y"
    {
        LOG_INFO("[bison]: funcnamelist: IDENTIFIER");
        auto funcnamelist = std::make_shared<fakelua::SyntaxTreeFuncnamelist>(yystack_[0].location);
        funcnamelist->AddName(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = funcnamelist;
    }
#line 1437 "parser.cpp"
    break;

  case 35: // funcnamelist: funcnamelist "." "identifier"
#line 644 "parser.y"
    {
        LOG_INFO("[bison]: funcnamelist: funcnamelist DOT IDENTIFIER");
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncnamelist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcnamelist: funcnamelist is not a funcnamelist");
            fakelua::ThrowFakeluaException("funcnamelist is not a funcnamelist");
        }
        funcnamelist->AddName(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = funcnamelist;
    }
#line 1452 "parser.cpp"
    break;

  case 36: // funcname: funcnamelist
#line 658 "parser.y"
    {
        LOG_INFO("[bison]: funcname: funcnamelist");
        auto funcname = std::make_shared<fakelua::SyntaxTreeFuncname>(yystack_[0].location);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncnamelist>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::ThrowFakeluaException("funcnamelist is not a funcnamelist");
        }
        funcname->SetFuncNameList(funcnamelist);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = funcname;
    }
#line 1468 "parser.cpp"
    break;

  case 37: // funcname: funcnamelist ":" "identifier"
#line 671 "parser.y"
    {
        LOG_INFO("[bison]: funcname: funcnamelist COLON IDENTIFIER");
        auto funcname = std::make_shared<fakelua::SyntaxTreeFuncname>(yystack_[2].location);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncnamelist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::ThrowFakeluaException("funcnamelist is not a funcnamelist");
        }
        funcname->SetFuncNameList(funcnamelist);
        funcname->SetColonName(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = funcname;
    }
#line 1485 "parser.cpp"
    break;

  case 38: // varlist: var
#line 687 "parser.y"
    {
        LOG_INFO("[bison]: varlist: var");
        auto varlist = std::make_shared<fakelua::SyntaxTreeVarlist>(yystack_[0].location);
        auto var = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::ThrowFakeluaException("var is not a var");
        }
        varlist->AddVar(var);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = varlist;
    }
#line 1501 "parser.cpp"
    break;

  case 39: // varlist: varlist "," var
#line 700 "parser.y"
    {
        LOG_INFO("[bison]: varlist: varlist COMMA var");
        auto varlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeVarlist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (varlist == nullptr) {
            LOG_ERROR("[bison]: varlist: varlist is not a varlist");
            fakelua::ThrowFakeluaException("varlist is not a varlist");
        }
        auto var = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::ThrowFakeluaException("var is not a var");
        }
        varlist->AddVar(var);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = varlist;
    }
#line 1521 "parser.cpp"
    break;

  case 40: // var: "identifier"
#line 719 "parser.y"
    {
        LOG_INFO("[bison]: var: IDENTIFIER");
        auto var = std::make_shared<fakelua::SyntaxTreeVar>(yystack_[0].location);
        var->SetName(yystack_[0].value.as < std::string > ());
        var->SetVarKind(VarKind::kSimple);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = var;
    }
#line 1533 "parser.cpp"
    break;

  case 41: // var: prefixexp "[" exp "]"
#line 728 "parser.y"
    {
        LOG_INFO("[bison]: var: prefixexp LSQUARE exp RSQUARE");
        auto var = std::make_shared<fakelua::SyntaxTreeVar>(yystack_[2].location);
        var->SetVarKind(VarKind::kSquare);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[3].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        var->SetPrefixexp(prefixexp);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: var: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        var->SetExp(exp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = var;
    }
#line 1556 "parser.cpp"
    break;

  case 42: // var: prefixexp "." "identifier"
#line 748 "parser.y"
    {
        LOG_INFO("[bison]: var: prefixexp DOT IDENTIFIER");
        auto var = std::make_shared<fakelua::SyntaxTreeVar>(yystack_[1].location);
        var->SetVarKind(VarKind::kDot);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        var->SetPrefixexp(prefixexp);
        var->SetName(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = var;
    }
#line 1574 "parser.cpp"
    break;

  case 43: // namelist: "identifier"
#line 765 "parser.y"
    {
        LOG_INFO("[bison]: namelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::SyntaxTreeNamelist>(yystack_[0].location);
        namelist->AddName(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1585 "parser.cpp"
    break;

  case 44: // namelist: namelist "," "identifier"
#line 773 "parser.y"
    {
        LOG_INFO("[bison]: namelist: namelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        namelist->AddName(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1600 "parser.cpp"
    break;

  case 45: // explist: exp
#line 787 "parser.y"
    {
        LOG_INFO("[bison]: explist: exp");
        auto explist = std::make_shared<fakelua::SyntaxTreeExplist>(yystack_[0].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        explist->AddExp(exp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = explist;
    }
#line 1616 "parser.cpp"
    break;

  case 46: // explist: explist "," exp
#line 800 "parser.y"
    {
        LOG_INFO("[bison]: explist: explist COMMA exp");
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: explist: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        explist->AddExp(exp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = explist;
    }
#line 1636 "parser.cpp"
    break;

  case 47: // exp: "nil"
#line 819 "parser.y"
    {
        LOG_INFO("[bison]: exp: NIL");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kNil);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1647 "parser.cpp"
    break;

  case 48: // exp: "true"
#line 827 "parser.y"
    {
        LOG_INFO("[bison]: exp: TRUE");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kTrue);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1658 "parser.cpp"
    break;

  case 49: // exp: "false"
#line 835 "parser.y"
    {
        LOG_INFO("[bison]: exp: FALSES");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kFalse);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1669 "parser.cpp"
    break;

  case 50: // exp: "number"
#line 843 "parser.y"
    {
        LOG_INFO("[bison]: exp: NUMBER");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kNumber);
        exp->SetValue(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1681 "parser.cpp"
    break;

  case 51: // exp: "string"
#line 852 "parser.y"
    {
        LOG_INFO("[bison]: exp: STRING");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kString);
        exp->SetValue(l->RemoveQuotes(yystack_[0].value.as < std::string > ()));
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1693 "parser.cpp"
    break;

  case 52: // exp: "..."
#line 861 "parser.y"
    {
        LOG_INFO("[bison]: exp: VAR_PARAMS");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kVarParams);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1704 "parser.cpp"
    break;

  case 53: // exp: functiondef
#line 869 "parser.y"
    {
        LOG_INFO("[bison]: exp: functiondef");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kFunctionDef);
        auto functiondef = std::dynamic_pointer_cast<fakelua::SyntaxTreeFunctiondef>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (functiondef == nullptr) {
            LOG_ERROR("[bison]: exp: functiondef is not a functiondef");
            fakelua::ThrowFakeluaException("functiondef is not a functiondef");
        }
        exp->SetRight(functiondef);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1721 "parser.cpp"
    break;

  case 54: // exp: prefixexp
#line 883 "parser.y"
    {
        LOG_INFO("[bison]: exp: prefixexp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kPrefixExp);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: exp: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        exp->SetRight(prefixexp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1738 "parser.cpp"
    break;

  case 55: // exp: tableconstructor
#line 897 "parser.y"
    {
        LOG_INFO("[bison]: exp: tableconstructor");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kTableConstructor);
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::SyntaxTreeTableconstructor>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: exp: tableconstructor is not a tableconstructor");
            fakelua::ThrowFakeluaException("tableconstructor is not a tableconstructor");
        }
        exp->SetRight(tableconstructor);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1755 "parser.cpp"
    break;

  case 56: // exp: exp "+" exp
#line 911 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp PLUS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kPlus);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1781 "parser.cpp"
    break;

  case 57: // exp: exp "-" exp
#line 934 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp MINUS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kMinus);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1807 "parser.cpp"
    break;

  case 58: // exp: exp "*" exp
#line 957 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp STAR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kStar);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1833 "parser.cpp"
    break;

  case 59: // exp: exp "/" exp
#line 980 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp SLASH exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kSlash);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1859 "parser.cpp"
    break;

  case 60: // exp: exp "//" exp
#line 1003 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp DOUBLE_SLASH exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kDoubleSlash);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1885 "parser.cpp"
    break;

  case 61: // exp: exp "^" exp
#line 1026 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp POW exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kPow);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1911 "parser.cpp"
    break;

  case 62: // exp: exp "%" exp
#line 1049 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp MOD exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kMod);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1937 "parser.cpp"
    break;

  case 63: // exp: exp "&" exp
#line 1072 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp BITAND exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kBitAnd);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1963 "parser.cpp"
    break;

  case 64: // exp: exp "~" exp
#line 1095 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp XOR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kXor);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1989 "parser.cpp"
    break;

  case 65: // exp: exp "|" exp
#line 1118 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp BITOR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kBitOr);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2015 "parser.cpp"
    break;

  case 66: // exp: exp ">>" exp
#line 1141 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp RIGHT_SHIFT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kRightShift);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2041 "parser.cpp"
    break;

  case 67: // exp: exp "<<" exp
#line 1164 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp LEFT_SHIFT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kLeftShift);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2067 "parser.cpp"
    break;

  case 68: // exp: exp ".." exp
#line 1187 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp CONCAT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kConcat);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2093 "parser.cpp"
    break;

  case 69: // exp: exp "<" exp
#line 1210 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp LESS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kLess);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2119 "parser.cpp"
    break;

  case 70: // exp: exp "<=" exp
#line 1233 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp LESS_EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kLessEqual);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2145 "parser.cpp"
    break;

  case 71: // exp: exp ">" exp
#line 1256 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp MORE exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kMore);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2171 "parser.cpp"
    break;

  case 72: // exp: exp ">=" exp
#line 1279 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp MORE_EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kMoreEqual);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2197 "parser.cpp"
    break;

  case 73: // exp: exp "==" exp
#line 1302 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kEqual);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2223 "parser.cpp"
    break;

  case 74: // exp: exp "~=" exp
#line 1325 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp NOT_EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kNotEqual);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2249 "parser.cpp"
    break;

  case 75: // exp: exp "and" exp
#line 1348 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp AND exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kAnd);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2275 "parser.cpp"
    break;

  case 76: // exp: exp "or" exp
#line 1371 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp OR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kOr);
        exp->SetOp(binop);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2301 "parser.cpp"
    break;

  case 77: // exp: "-" exp
#line 1394 "parser.y"
    {
        LOG_INFO("[bison]: exp: MINUS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[1].location);
        exp->SetExpKind(ExpKind::kUnop);
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(yystack_[1].location);
        unop->SetOpKind(UnOpKind::kMinus);
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2321 "parser.cpp"
    break;

  case 78: // exp: "not" exp
#line 1411 "parser.y"
    {
        LOG_INFO("[bison]: exp: NOT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[1].location);
        exp->SetExpKind(ExpKind::kUnop);
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(yystack_[1].location);
        unop->SetOpKind(UnOpKind::kNot);
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2341 "parser.cpp"
    break;

  case 79: // exp: "#" exp
#line 1428 "parser.y"
    {
        LOG_INFO("[bison]: exp: NUMBER_SIGN exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[1].location);
        exp->SetExpKind(ExpKind::kUnop);
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(yystack_[1].location);
        unop->SetOpKind(UnOpKind::kNumberSign);
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2361 "parser.cpp"
    break;

  case 80: // exp: "~" exp
#line 1445 "parser.y"
    {
        LOG_INFO("[bison]: exp: BITNOT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[1].location);
        exp->SetExpKind(ExpKind::kUnop);
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(yystack_[1].location);
        unop->SetOpKind(UnOpKind::kBitNot);
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2381 "parser.cpp"
    break;

  case 81: // prefixexp: var
#line 1464 "parser.y"
    {
        LOG_INFO("[bison]: prefixexp: var");
        auto prefixexp = std::make_shared<fakelua::SyntaxTreePrefixexp>(yystack_[0].location);
        prefixexp->SetPrefixKind(PrefixExpKind::kVar);
        auto var = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (var == nullptr) {
            LOG_ERROR("[bison]: prefixexp: var is not a var");
            fakelua::ThrowFakeluaException("var is not a var");
        }
        prefixexp->SetValue(var);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = prefixexp;
    }
#line 2398 "parser.cpp"
    break;

  case 82: // prefixexp: functioncall
#line 1478 "parser.y"
    {
        LOG_INFO("[bison]: prefixexp: functioncall");
        auto prefixexp = std::make_shared<fakelua::SyntaxTreePrefixexp>(yystack_[0].location);
        prefixexp->SetPrefixKind(PrefixExpKind::kFunctionCall);
        auto functioncall = std::dynamic_pointer_cast<fakelua::SyntaxTreeFunctioncall>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (functioncall == nullptr) {
            LOG_ERROR("[bison]: prefixexp: functioncall is not a functioncall");
            fakelua::ThrowFakeluaException("functioncall is not a functioncall");
        }
        prefixexp->SetValue(functioncall);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = prefixexp;
    }
#line 2415 "parser.cpp"
    break;

  case 83: // prefixexp: "(" exp ")"
#line 1492 "parser.y"
    {
        LOG_INFO("[bison]: prefixexp: LPAREN exp RPAREN");
        auto prefixexp = std::make_shared<fakelua::SyntaxTreePrefixexp>(yystack_[2].location);
        prefixexp->SetPrefixKind(PrefixExpKind::kExp);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: prefixexp: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        prefixexp->SetValue(exp);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = prefixexp;
    }
#line 2432 "parser.cpp"
    break;

  case 84: // functioncall: prefixexp args
#line 1507 "parser.y"
    {
        LOG_INFO("[bison]: functioncall: prefixexp args");
        auto functioncall = std::make_shared<fakelua::SyntaxTreeFunctioncall>(yystack_[1].location);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        functioncall->SetPrefixexp(prefixexp);
        auto args = std::dynamic_pointer_cast<fakelua::SyntaxTreeArgs>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::ThrowFakeluaException("args is not a args");
        }
        functioncall->SetArgs(args);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = functioncall;
    }
#line 2454 "parser.cpp"
    break;

  case 85: // functioncall: prefixexp ":" "identifier" args
#line 1526 "parser.y"
    {
        LOG_INFO("[bison]: functioncall: prefixexp COLON IDENTIFIER args");
        auto functioncall = std::make_shared<fakelua::SyntaxTreeFunctioncall>(yystack_[3].location);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[3].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        functioncall->SetPrefixexp(prefixexp);
        functioncall->SetName(yystack_[1].value.as < std::string > ());
        auto args = std::dynamic_pointer_cast<fakelua::SyntaxTreeArgs>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::ThrowFakeluaException("args is not a args");
        }
        functioncall->SetArgs(args);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = functioncall;
    }
#line 2477 "parser.cpp"
    break;

  case 86: // args: "(" explist ")"
#line 1548 "parser.y"
    {
        LOG_INFO("[bison]: args: LPAREN explist RPAREN");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(yystack_[2].location);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: args: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        args->SetExplist(explist);
        args->SetArgsKind(ArgsKind::kExpList);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = args;
    }
#line 2494 "parser.cpp"
    break;

  case 87: // args: "(" ")"
#line 1562 "parser.y"
    {
        LOG_INFO("[bison]: args: LPAREN RPAREN");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(yystack_[1].location);
        args->SetArgsKind(ArgsKind::kEmpty);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = args;
    }
#line 2505 "parser.cpp"
    break;

  case 88: // args: tableconstructor
#line 1570 "parser.y"
    {
        LOG_INFO("[bison]: args: tableconstructor");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(yystack_[0].location);
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::SyntaxTreeTableconstructor>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: args: tableconstructor is not a tableconstructor");
            fakelua::ThrowFakeluaException("tableconstructor is not a tableconstructor");
        }
        args->SetTableconstructor(tableconstructor);
        args->SetArgsKind(ArgsKind::kTableConstructor);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = args;
    }
#line 2522 "parser.cpp"
    break;

  case 89: // args: "string"
#line 1584 "parser.y"
    {
        LOG_INFO("[bison]: args: STRING");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(yystack_[0].location);
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kString);
        exp->SetValue(l->RemoveQuotes(yystack_[0].value.as < std::string > ()));
        args->SetString(exp);
        args->SetArgsKind(ArgsKind::kString);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = args;
    }
#line 2537 "parser.cpp"
    break;

  case 90: // functiondef: "function" funcbody
#line 1598 "parser.y"
    {
        LOG_INFO("[bison]: functiondef: FUNCTION funcbody");
        auto functiondef = std::make_shared<fakelua::SyntaxTreeFunctiondef>(yystack_[1].location);
        auto funcbody = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: functiondef: funcbody is not a funcbody");
            fakelua::ThrowFakeluaException("funcbody is not a funcbody");
        }
        functiondef->SetFuncbody(funcbody);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = functiondef;
    }
#line 2553 "parser.cpp"
    break;

  case 91: // funcbody: "(" parlist ")" block "end"
#line 1613 "parser.y"
    {
        LOG_INFO("[bison]: funcbody: LPAREN parlist RPAREN block END");
        auto funcbody = std::make_shared<fakelua::SyntaxTreeFuncbody>(yystack_[4].location);
        auto parlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeParlist>(yystack_[3].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (parlist == nullptr) {
            LOG_ERROR("[bison]: funcbody: parlist is not a parlist");
            fakelua::ThrowFakeluaException("parlist is not a parlist");
        }
        funcbody->SetParlist(parlist);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        funcbody->SetBlock(block);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = funcbody;
    }
#line 2575 "parser.cpp"
    break;

  case 92: // funcbody: "(" ")" block "end"
#line 1632 "parser.y"
    {
        LOG_INFO("[bison]: funcbody: LPAREN RPAREN block END");
        auto funcbody = std::make_shared<fakelua::SyntaxTreeFuncbody>(yystack_[3].location);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        funcbody->SetBlock(block);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = funcbody;
    }
#line 2591 "parser.cpp"
    break;

  case 93: // parlist: namelist
#line 1647 "parser.y"
    {
        LOG_INFO("[bison]: parlist: namelist");
        auto parlist = std::make_shared<fakelua::SyntaxTreeParlist>(yystack_[0].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        parlist->SetNamelist(namelist);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = parlist;
    }
#line 2607 "parser.cpp"
    break;

  case 94: // parlist: namelist "," "..."
#line 1660 "parser.y"
    {
        LOG_INFO("[bison]: parlist: namelist COMMA VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::SyntaxTreeParlist>(yystack_[2].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        parlist->SetNamelist(namelist);
        parlist->SetVarParams(true);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = parlist;
    }
#line 2624 "parser.cpp"
    break;

  case 95: // parlist: "..."
#line 1674 "parser.y"
    {
        LOG_INFO("[bison]: parlist: VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::SyntaxTreeParlist>(yystack_[0].location);
        parlist->SetVarParams(true);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = parlist;
    }
#line 2635 "parser.cpp"
    break;

  case 96: // tableconstructor: "{" fieldlist "}"
#line 1684 "parser.y"
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY fieldlist RCURLY");
        auto tableconstructor = std::make_shared<fakelua::SyntaxTreeTableconstructor>(yystack_[2].location);
        auto fieldlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFieldlist>(yystack_[1].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: tableconstructor: fieldlist is not a fieldlist");
            fakelua::ThrowFakeluaException("fieldlist is not a fieldlist");
        }
        tableconstructor->SetFieldlist(fieldlist);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = tableconstructor;
    }
#line 2651 "parser.cpp"
    break;

  case 97: // tableconstructor: "{" "}"
#line 1697 "parser.y"
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY RCURLY");
        auto tableconstructor = std::make_shared<fakelua::SyntaxTreeTableconstructor>(yystack_[1].location);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = tableconstructor;
    }
#line 2661 "parser.cpp"
    break;

  case 98: // fieldlist: field
#line 1706 "parser.y"
    {
        LOG_INFO("[bison]: fieldlist: field");
        auto fieldlist = std::make_shared<fakelua::SyntaxTreeFieldlist>(yystack_[0].location);
        auto field = std::dynamic_pointer_cast<fakelua::SyntaxTreeField>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::ThrowFakeluaException("field is not a field");
        }
        fieldlist->AddField(field);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = fieldlist;
    }
#line 2677 "parser.cpp"
    break;

  case 99: // fieldlist: fieldlist fieldsep field
#line 1719 "parser.y"
    {
        LOG_INFO("[bison]: fieldlist: fieldlist fieldsep field");
        auto fieldlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFieldlist>(yystack_[2].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: fieldlist: fieldlist is not a fieldlist");
            fakelua::ThrowFakeluaException("fieldlist is not a fieldlist");
        }
        auto field = std::dynamic_pointer_cast<fakelua::SyntaxTreeField>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::ThrowFakeluaException("field is not a field");
        }
        fieldlist->AddField(field);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = fieldlist;
    }
#line 2697 "parser.cpp"
    break;

  case 100: // field: "[" exp "]" "=" exp
#line 1738 "parser.y"
    {
        LOG_INFO("[bison]: field: LSQUARE exp RSQUARE ASSIGN exp");
        auto field = std::make_shared<fakelua::SyntaxTreeField>(yystack_[4].location);
        auto key = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[3].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (key == nullptr) {
            LOG_ERROR("[bison]: key: key is not a exp");
            fakelua::ThrowFakeluaException("key is not a exp");
        }
        field->SetKey(key);
        auto value = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (value == nullptr) {
            LOG_ERROR("[bison]: field: value is not a exp");
            fakelua::ThrowFakeluaException("value is not a exp");
        }
        field->SetValue(value);
        field->SetFieldKind(FieldKind::kArray);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = field;
    }
#line 2720 "parser.cpp"
    break;

  case 101: // field: "identifier" "=" exp
#line 1758 "parser.y"
    {
        LOG_INFO("[bison]: field: IDENTIFIER ASSIGN exp");
        auto field = std::make_shared<fakelua::SyntaxTreeField>(yystack_[2].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        field->SetName(yystack_[2].value.as < std::string > ());
        field->SetValue(exp);
        field->SetFieldKind(FieldKind::kObject);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = field;
    }
#line 2738 "parser.cpp"
    break;

  case 102: // field: exp
#line 1773 "parser.y"
    {
        LOG_INFO("[bison]: field: exp");
        auto field = std::make_shared<fakelua::SyntaxTreeField>(yystack_[0].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as < fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        field->SetValue(exp);
        field->SetFieldKind(FieldKind::kArray);
        yylhs.value.as < fakelua::SyntaxTreeInterfacePtr > () = field;
    }
#line 2755 "parser.cpp"
    break;

  case 103: // fieldsep: ","
#line 1789 "parser.y"
    {
        LOG_INFO("[bison]: fieldsep: COMMA");
        // nothing to do
    }
#line 2764 "parser.cpp"
    break;

  case 104: // fieldsep: ";"
#line 1795 "parser.y"
    {
        LOG_INFO("[bison]: fieldsep: SEMICOLON");
        // nothing to do
    }
#line 2773 "parser.cpp"
    break;


#line 2777 "parser.cpp"

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
  "%", "&", "|", "~", ">", "<", "#", "UNARY", "identifier", "string",
  "number", "$accept", "chunk", "block", "stmt", "attnamelist", "elseifs",
  "retstat", "label", "funcnamelist", "funcname", "varlist", "var",
  "namelist", "explist", "exp", "prefixexp", "functioncall", "args",
  "functiondef", "funcbody", "parlist", "tableconstructor", "fieldlist",
  "field", "fieldsep", YY_NULLPTR
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


  const signed char parser::yypact_ninf_ = -57;

  const signed char parser::yytable_ninf_ = -83;

  const short
  parser::yypact_[] =
  {
     -57,    20,  1302,   -57,    73,   -57,   -57,   -32,   -30,    73,
      -9,   -57,    73,    73,   -23,   -19,   -57,   -57,   -57,   -57,
     -57,     6,    31,    49,    96,    73,    11,   -57,    38,   -57,
      73,   -57,   -57,    73,    73,   -57,   -57,   -57,   439,    49,
     -57,   -57,   -57,  1018,    25,   -18,   -57,   -31,    38,   492,
      21,    10,    36,  1046,    40,   916,   545,   -57,    44,    73,
       4,    65,    73,    35,    43,   -57,   -57,   -57,    54,   -57,
      73,    87,   916,    -4,   -57,    69,   -57,    54,    54,    54,
      73,    73,    73,    73,   -57,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,   -57,    73,    73,    51,    61,    62,   -57,   -57,
      38,    70,    73,    72,    73,    73,   -57,   -57,    40,    64,
     -57,     5,   598,     0,   -57,   651,    73,   -57,   -57,   -57,
     325,   -57,   -57,   -57,    75,   104,   220,   220,    54,    54,
     225,   969,    54,   361,   132,   132,   132,   132,   361,   361,
      54,    54,   464,   111,   337,   132,   132,   704,    66,   -57,
     -57,   -57,  1078,   -57,    80,    40,    84,   916,   916,  1106,
     -57,   -57,   -57,   138,   916,   -57,  1134,   -27,   -57,    73,
     -57,    73,     7,   -57,    83,   -57,    73,   -57,   -57,  1162,
     386,  1190,   757,   -57,    73,   -57,    89,   916,   -57,   -57,
      73,   -57,   -57,  1218,   810,   -57,  1246,   863,  1302,   -57,
     -57,   -57,   -57,  1302,  1274,   -57
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,     2,     1,     0,    10,     3,     0,     0,     0,
       0,     3,    31,     0,     0,     0,     6,    40,     4,     5,
       9,     0,    81,     0,     8,     0,     0,    49,     0,    47,
       0,    48,    52,     0,     0,    51,    50,    81,     0,    54,
      82,    53,    55,     0,    43,     0,    34,    36,     0,     0,
       0,    24,    22,     0,    32,    45,     0,    11,     0,     0,
       0,     0,     0,     0,     0,    89,    84,    88,    77,    97,
       0,    40,   102,     0,    98,     0,    90,    78,    80,    79,
       0,     0,     0,     0,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,     0,     0,     0,     0,     0,    20,     3,
       0,     0,     0,     0,     0,     0,     3,    33,     7,    81,
      87,     0,     0,     0,    42,     0,     0,    96,   104,   103,
       0,     3,    95,    43,    93,     0,    57,    56,    58,    59,
      75,    76,    60,    68,    73,    72,    70,    74,    67,    66,
      61,    62,    63,    65,    64,    71,    69,     0,     0,    44,
      37,    35,    28,    21,     0,    23,    26,    14,    46,     0,
      86,    41,    85,     0,   101,    99,     0,     0,     3,     0,
       3,     0,     0,    25,     0,    13,     0,    92,    94,     0,
       0,     0,     0,     3,     0,    16,     0,   100,    91,     3,
       0,    19,     3,     0,     0,    27,     0,     0,    29,    15,
       3,    17,     3,    30,     0,    18
  };

  const short
  parser::yypgoto_[] =
  {
     -57,   -57,    41,   -57,   -57,   -57,   -57,   -57,   -57,   -57,
     -57,    -2,    71,   -56,   224,     2,    33,    26,   -57,   -46,
     -57,   -22,   -57,    22,   -57
  };

  const unsigned char
  parser::yydefgoto_[] =
  {
       0,     1,     2,    18,    52,   182,    19,    20,    47,    48,
      21,    37,    45,    54,    55,    39,    40,    66,    41,    76,
     135,    42,    73,    74,   130
  };

  const short
  parser::yytable_[] =
  {
      22,    67,   108,   118,    23,   121,   104,   127,    61,    59,
      26,   188,     4,    50,   170,    25,   106,    67,   107,     4,
       3,    26,    69,    70,   193,   194,   195,    44,   103,    46,
     105,    27,   159,    28,   -38,    24,    57,    29,    30,   112,
      58,    22,   128,    31,   129,    23,    75,    43,   158,    32,
      51,    22,    53,   115,    60,    23,   165,    61,   119,    26,
      65,    62,    23,    17,   163,    33,   111,   -39,    34,    25,
      71,    35,    36,     4,   120,    26,    24,    25,   131,   -38,
     110,     4,   180,    26,   113,    27,    24,    28,   115,   117,
     126,    29,    30,    27,   123,    28,    63,    31,    64,    29,
      30,    67,   124,    32,    95,    31,   -82,   132,   -82,    65,
     159,    32,   -39,   178,   115,    80,    81,    82,    83,    33,
     160,   161,    34,   177,    17,    35,    36,    33,   133,   164,
      34,   166,    17,    35,    36,   183,    80,    81,    82,    83,
     184,   186,   196,   -82,   205,   -82,   134,    87,    88,   172,
     162,     0,   175,     0,    93,    94,   -82,   169,     0,     0,
      22,    95,    96,    97,    23,    99,     0,    22,    87,    88,
       0,    23,   176,     0,    22,    93,    94,     0,    23,     0,
       0,     0,    95,    96,    97,    98,    99,    22,     0,    22,
       0,    23,     0,    23,     0,    24,     0,     0,     0,     0,
       0,    22,    24,     0,    22,    23,    22,     0,    23,    24,
      23,    22,    22,     0,     0,    23,    23,     0,     0,   189,
       0,   191,    24,     0,    24,     0,    82,    83,    38,    80,
      81,    82,    83,    49,   203,     0,    24,    56,     0,    24,
     206,    24,     0,   208,     0,     0,    24,    24,     0,    68,
      72,   213,     0,   214,    77,     0,    87,    78,    79,     0,
       0,    87,    88,     0,    89,    90,    91,    92,    93,    94,
      95,    96,     0,     0,     0,    95,    96,    97,    98,    99,
     100,   101,     0,     0,     0,     0,   122,     0,     0,     0,
       0,     0,     0,     0,   125,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   136,   137,   138,   139,     0,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,     0,   157,     0,    25,
       0,     0,     0,     4,     0,    26,     0,    70,   167,   168,
       0,    80,    81,    82,    83,    27,     0,    28,     0,     0,
     174,    29,    30,     0,    72,     0,     0,    31,     0,     0,
       0,     0,     0,    32,     0,    80,    81,    82,    83,     0,
       0,     0,     0,    87,    88,     0,     0,     0,     0,    33,
      93,    94,    34,     0,    71,    35,    36,    95,    96,    97,
      80,    81,    82,    83,     0,     0,     0,    87,    88,     0,
      85,     0,   199,   190,     0,   192,     0,     0,     0,     0,
     197,    95,    96,     0,    86,     0,     0,     0,   204,     0,
       0,     0,    87,    88,   207,    89,    90,    91,    92,    93,
      94,     0,     0,     0,   200,     0,    95,    96,    97,    98,
      99,   100,   101,    80,    81,    82,    83,     0,    84,     0,
       0,     0,     0,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    86,    80,    81,
      82,    83,     0,     0,     0,    87,    88,     0,    89,    90,
      91,    92,    93,    94,     0,     0,     0,     0,     0,    95,
      96,    97,    98,    99,   100,   101,    80,    81,    82,    83,
      87,    88,     0,     0,     0,     0,    85,    93,    94,     0,
       0,     0,     0,     0,    95,    96,     0,     0,     0,     0,
      86,     0,     0,   109,     0,     0,     0,     0,    87,    88,
       0,    89,    90,    91,    92,    93,    94,     0,     0,     0,
       0,     0,    95,    96,    97,    98,    99,   100,   101,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,    85,
       0,   116,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    86,     0,     0,     0,     0,     0,     0,
       0,    87,    88,     0,    89,    90,    91,    92,    93,    94,
       0,     0,     0,     0,     0,    95,    96,    97,    98,    99,
     100,   101,    80,    81,    82,    83,     0,     0,     0,     0,
       0,   171,    85,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    86,     0,     0,     0,
       0,     0,     0,     0,    87,    88,     0,    89,    90,    91,
      92,    93,    94,     0,     0,     0,     0,     0,    95,    96,
      97,    98,    99,   100,   101,    80,    81,    82,    83,     0,
       0,     0,     0,     0,   173,    85,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    86,
       0,     0,     0,     0,     0,     0,     0,    87,    88,     0,
      89,    90,    91,    92,    93,    94,     0,     0,     0,     0,
       0,    95,    96,    97,    98,    99,   100,   101,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    86,     0,     0,     0,     0,     0,     0,     0,
      87,    88,     0,    89,    90,    91,    92,    93,    94,     0,
       0,     0,   179,     0,    95,    96,    97,    98,    99,   100,
     101,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    86,     0,     0,   202,     0,
       0,     0,     0,    87,    88,     0,    89,    90,    91,    92,
      93,    94,     0,     0,     0,     0,     0,    95,    96,    97,
      98,    99,   100,   101,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,    85,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    86,     0,
       0,   210,     0,     0,     0,     0,    87,    88,     0,    89,
      90,    91,    92,    93,    94,     0,     0,     0,     0,     0,
      95,    96,    97,    98,    99,   100,   101,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,    85,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    86,     0,     0,     0,     0,     0,     0,     0,    87,
      88,     0,    89,    90,    91,    92,    93,    94,     0,     0,
       0,     0,     0,    95,    96,    97,    98,    99,   100,   101,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    86,     0,     0,     0,     0,     0,
       0,     0,    87,    88,     0,    89,    90,    91,    92,    93,
      94,     0,     0,     0,     0,     0,    95,    96,    97,    98,
      99,   100,   101,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    87,    88,     0,    89,    90,
      91,    92,    93,    94,     0,     0,     0,     0,     0,    95,
      96,    97,    98,    99,   100,   101,     4,     0,     0,     0,
       0,     0,     0,     5,     6,     0,     0,   102,     0,     7,
       8,     9,     0,    10,     0,     0,     0,    11,    12,     0,
       0,     0,    13,    14,     4,     0,     0,     0,     0,     0,
       0,     5,     6,    15,    16,     0,     0,     7,     8,     9,
       0,    10,     0,     0,     0,    11,    12,    17,     0,   114,
      13,    14,     0,     0,     0,     0,     4,     0,     0,     0,
       0,    15,    16,     5,     6,     0,   181,     0,     0,     7,
       8,     9,     0,    10,     0,    17,     0,    11,    12,     0,
       0,     0,    13,    14,     4,     0,     0,     0,     0,     0,
       0,     5,     6,    15,    16,   185,     0,     7,     8,     9,
       0,    10,     0,     0,     0,    11,    12,    17,     0,     0,
      13,    14,     4,     0,     0,     0,     0,     0,     0,     5,
       6,    15,    16,   187,     0,     7,     8,     9,     0,    10,
       0,     0,     0,    11,    12,    17,     0,     0,    13,    14,
       4,     0,     0,     0,     0,     0,     0,     5,     6,    15,
      16,   198,     0,     7,     8,     9,     0,    10,     0,     0,
       0,    11,    12,    17,     0,     0,    13,    14,     4,     0,
       0,     0,     0,     0,     0,     5,     6,    15,    16,   201,
       0,     7,     8,     9,     0,    10,     0,     0,     0,    11,
      12,    17,     0,     0,    13,    14,     4,     0,     0,     0,
       0,     0,     0,     5,     6,    15,    16,   209,     0,     7,
       8,     9,     0,    10,     0,     0,     0,    11,    12,    17,
       0,     0,    13,    14,     4,     0,     0,     0,     0,     0,
       0,     5,     6,    15,    16,   211,     0,     7,     8,     9,
       0,    10,     0,     0,     0,    11,    12,    17,     0,     0,
      13,    14,     4,     0,     0,     0,     0,     0,     0,     5,
       6,    15,    16,   215,     0,     7,     8,     9,     0,    10,
       0,     0,     0,    11,    12,    17,     0,     0,    13,    14,
       4,     0,     0,     0,     0,     0,     0,     5,     6,    15,
      16,     0,     0,     7,     8,     9,     0,    10,     0,     0,
       0,    11,    12,    17,     0,     0,    13,    14,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17
  };

  const short
  parser::yycheck_[] =
  {
       2,    23,    48,    59,     2,    61,    24,    11,     8,     3,
      10,    38,     8,    22,     9,     4,    47,    39,    49,     8,
       0,    10,    11,    12,    17,    18,    19,    59,     3,    59,
      48,    20,    59,    22,     3,     2,    59,    26,    27,     3,
      59,    43,    46,    32,    48,    43,     8,     6,   104,    38,
      59,    53,    11,    48,    48,    53,   112,     8,    60,    10,
      60,    12,    60,    59,   110,    54,    56,     3,    57,     4,
      59,    60,    61,     8,     9,    10,    43,     4,     9,    48,
      59,     8,    16,    10,    48,    20,    53,    22,    48,    45,
       3,    26,    27,    20,    59,    22,    47,    32,    49,    26,
      27,   123,    59,    38,    50,    32,    10,    38,    12,    60,
      59,    38,    48,     9,    48,     4,     5,     6,     7,    54,
      59,    59,    57,    48,    59,    60,    61,    54,    59,    59,
      57,    59,    59,    60,    61,    55,     4,     5,     6,     7,
      56,     3,    59,    47,    55,    49,    75,    36,    37,   123,
     109,    -1,   130,    -1,    43,    44,    60,   116,    -1,    -1,
     162,    50,    51,    52,   162,    54,    -1,   169,    36,    37,
      -1,   169,   131,    -1,   176,    43,    44,    -1,   176,    -1,
      -1,    -1,    50,    51,    52,    53,    54,   189,    -1,   191,
      -1,   189,    -1,   191,    -1,   162,    -1,    -1,    -1,    -1,
      -1,   203,   169,    -1,   206,   203,   208,    -1,   206,   176,
     208,   213,   214,    -1,    -1,   213,   214,    -1,    -1,   178,
      -1,   180,   189,    -1,   191,    -1,     6,     7,     4,     4,
       5,     6,     7,     9,   193,    -1,   203,    13,    -1,   206,
     199,   208,    -1,   202,    -1,    -1,   213,   214,    -1,    25,
      26,   210,    -1,   212,    30,    -1,    36,    33,    34,    -1,
      -1,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      50,    51,    -1,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,    -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,    -1,   103,    -1,     4,
      -1,    -1,    -1,     8,    -1,    10,    -1,    12,   114,   115,
      -1,     4,     5,     6,     7,    20,    -1,    22,    -1,    -1,
     126,    26,    27,    -1,   130,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    38,    -1,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    36,    37,    -1,    -1,    -1,    -1,    54,
      43,    44,    57,    -1,    59,    60,    61,    50,    51,    52,
       4,     5,     6,     7,    -1,    -1,    -1,    36,    37,    -1,
      14,    -1,    16,   179,    -1,   181,    -1,    -1,    -1,    -1,
     186,    50,    51,    -1,    28,    -1,    -1,    -1,   194,    -1,
      -1,    -1,    36,    37,   200,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,     4,     5,     6,     7,    -1,     9,    -1,
      -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,     4,     5,
       6,     7,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,
      51,    52,    53,    54,    55,    56,     4,     5,     6,     7,
      36,    37,    -1,    -1,    -1,    -1,    14,    43,    44,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
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
      54,    55,    56,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,
      51,    52,    53,    54,    55,    56,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    -1,    -1,    19,    -1,    21,
      22,    23,    -1,    25,    -1,    -1,    -1,    29,    30,    -1,
      -1,    -1,    34,    35,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    16,    45,    46,    -1,    -1,    21,    22,    23,
      -1,    25,    -1,    -1,    -1,    29,    30,    59,    -1,    33,
      34,    35,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    45,    46,    15,    16,    -1,    18,    -1,    -1,    21,
      22,    23,    -1,    25,    -1,    59,    -1,    29,    30,    -1,
      -1,    -1,    34,    35,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    16,    45,    46,    19,    -1,    21,    22,    23,
      -1,    25,    -1,    -1,    -1,    29,    30,    59,    -1,    -1,
      34,    35,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,
      16,    45,    46,    19,    -1,    21,    22,    23,    -1,    25,
      -1,    -1,    -1,    29,    30,    59,    -1,    -1,    34,    35,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    45,
      46,    19,    -1,    21,    22,    23,    -1,    25,    -1,    -1,
      -1,    29,    30,    59,    -1,    -1,    34,    35,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    16,    45,    46,    19,
      -1,    21,    22,    23,    -1,    25,    -1,    -1,    -1,    29,
      30,    59,    -1,    -1,    34,    35,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    45,    46,    19,    -1,    21,
      22,    23,    -1,    25,    -1,    -1,    -1,    29,    30,    59,
      -1,    -1,    34,    35,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    16,    45,    46,    19,    -1,    21,    22,    23,
      -1,    25,    -1,    -1,    -1,    29,    30,    59,    -1,    -1,
      34,    35,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,
      16,    45,    46,    19,    -1,    21,    22,    23,    -1,    25,
      -1,    -1,    -1,    29,    30,    59,    -1,    -1,    34,    35,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    45,
      46,    -1,    -1,    21,    22,    23,    -1,    25,    -1,    -1,
      -1,    29,    30,    59,    -1,    -1,    34,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59
  };

  const signed char
  parser::yystos_[] =
  {
       0,    63,    64,     0,     8,    15,    16,    21,    22,    23,
      25,    29,    30,    34,    35,    45,    46,    59,    65,    68,
      69,    72,    73,    77,    78,     4,    10,    20,    22,    26,
      27,    32,    38,    54,    57,    60,    61,    73,    76,    77,
      78,    80,    83,    64,    59,    74,    59,    70,    71,    76,
      22,    59,    66,    64,    75,    76,    76,    59,    59,     3,
      48,     8,    12,    47,    49,    60,    79,    83,    76,    11,
      12,    59,    76,    84,    85,     8,    81,    76,    76,    76,
       4,     5,     6,     7,     9,    14,    28,    36,    37,    39,
      40,    41,    42,    43,    44,    50,    51,    52,    53,    54,
      55,    56,    19,     3,    24,    48,    47,    49,    81,    31,
      59,    56,     3,    48,    33,    48,    16,    45,    75,    73,
       9,    75,    76,    59,    59,    76,     3,    11,    46,    48,
      86,     9,    38,    59,    74,    82,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    75,    59,
      59,    59,    64,    81,    59,    75,    59,    76,    76,    64,
       9,    13,    79,    13,    76,    85,    64,    48,     9,    48,
      16,    18,    67,    55,    56,    19,     3,    19,    38,    64,
      76,    64,    76,    17,    18,    19,    59,    76,    19,    16,
      48,    19,    31,    64,    76,    55,    64,    76,    64,    19,
      31,    19,    16,    64,    64,    19
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    62,    63,    64,    64,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    66,    66,    66,    66,    67,    67,
      67,    68,    68,    69,    70,    70,    71,    71,    72,    72,
      73,    73,    73,    74,    74,    75,    75,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    77,    77,    77,    78,    78,    79,    79,    79,    79,
      80,    81,    81,    82,    82,    82,    83,    83,    84,    84,
      85,    85,    85,    86,    86
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     2,     1,     1,     3,     1,     1,
       1,     2,     3,     5,     4,     8,     6,     9,    11,     7,
       3,     4,     2,     4,     1,     4,     3,     6,     0,     4,
       5,     1,     2,     3,     1,     3,     1,     3,     1,     3,
       1,     4,     3,     1,     3,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     1,     1,     3,     2,     4,     3,     2,     1,     1,
       2,     5,     4,     1,     3,     1,     3,     2,     1,     3,
       5,     3,     1,     1,     1
  };




#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   163,   163,   171,   177,   199,   205,   211,   230,   236,
     242,   248,   256,   262,   281,   300,   331,   356,   382,   414,
     439,   458,   472,   485,   506,   515,   524,   537,   552,   558,
     577,   602,   610,   625,   635,   643,   657,   670,   686,   699,
     718,   727,   747,   764,   772,   786,   799,   818,   826,   834,
     842,   851,   860,   868,   882,   896,   910,   933,   956,   979,
    1002,  1025,  1048,  1071,  1094,  1117,  1140,  1163,  1186,  1209,
    1232,  1255,  1278,  1301,  1324,  1347,  1370,  1393,  1410,  1427,
    1444,  1463,  1477,  1491,  1506,  1525,  1547,  1561,  1569,  1583,
    1597,  1612,  1631,  1646,  1659,  1673,  1683,  1696,  1705,  1718,
    1737,  1757,  1772,  1788,  1794
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
#line 3706 "parser.cpp"

#line 1801 "parser.y"


void
yy::parser::error (const location_type& l, const std::string& m)
{
    std::stringstream ss;
    ss << l;
    fakelua::ThrowFakeluaException(std::format("{}: {}", ss.str(), m));
}
