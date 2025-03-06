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
#line 149 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 433 "parser.cpp"
        break;

      case symbol_kind::S_STRING: // "string"
#line 149 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 439 "parser.cpp"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 149 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 445 "parser.cpp"
        break;

      case symbol_kind::S_chunk: // chunk
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 451 "parser.cpp"
        break;

      case symbol_kind::S_block: // block
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 457 "parser.cpp"
        break;

      case symbol_kind::S_stmt: // stmt
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 463 "parser.cpp"
        break;

      case symbol_kind::S_attnamelist: // attnamelist
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 469 "parser.cpp"
        break;

      case symbol_kind::S_elseifs: // elseifs
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 475 "parser.cpp"
        break;

      case symbol_kind::S_retstat: // retstat
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 481 "parser.cpp"
        break;

      case symbol_kind::S_label: // label
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 487 "parser.cpp"
        break;

      case symbol_kind::S_funcnamelist: // funcnamelist
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 493 "parser.cpp"
        break;

      case symbol_kind::S_funcname: // funcname
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 499 "parser.cpp"
        break;

      case symbol_kind::S_varlist: // varlist
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 505 "parser.cpp"
        break;

      case symbol_kind::S_var: // var
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 511 "parser.cpp"
        break;

      case symbol_kind::S_namelist: // namelist
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 517 "parser.cpp"
        break;

      case symbol_kind::S_explist: // explist
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 523 "parser.cpp"
        break;

      case symbol_kind::S_exp: // exp
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 529 "parser.cpp"
        break;

      case symbol_kind::S_prefixexp: // prefixexp
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 535 "parser.cpp"
        break;

      case symbol_kind::S_functioncall: // functioncall
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 541 "parser.cpp"
        break;

      case symbol_kind::S_args: // args
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 547 "parser.cpp"
        break;

      case symbol_kind::S_functiondef: // functiondef
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 553 "parser.cpp"
        break;

      case symbol_kind::S_funcbody: // funcbody
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 559 "parser.cpp"
        break;

      case symbol_kind::S_parlist: // parlist
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 565 "parser.cpp"
        break;

      case symbol_kind::S_tableconstructor: // tableconstructor
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 571 "parser.cpp"
        break;

      case symbol_kind::S_fieldlist: // fieldlist
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
#line 577 "parser.cpp"
        break;

      case symbol_kind::S_field: // field
#line 149 "parser.y"
                 { yyo << yysym.value.template as < fakelua::syntax_tree_interface_ptr > (); }
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
#line 156 "parser.y"
    {
    LOG_INFO("[bison]: chunk: block");
    l->set_chunk(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
    }
#line 881 "parser.cpp"
    break;

  case 3: // block: %empty
#line 164 "parser.y"
    {
        LOG_INFO("[bison]: block: empty");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_block>(yystack_[0].location);
    }
#line 890 "parser.cpp"
    break;

  case 4: // block: stmt
#line 170 "parser.y"
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
#line 905 "parser.cpp"
    break;

  case 5: // block: block stmt
#line 182 "parser.y"
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
#line 925 "parser.cpp"
    break;

  case 6: // stmt: retstat
#line 201 "parser.y"
    {
        LOG_INFO("[bison]: stmt: retstat");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 934 "parser.cpp"
    break;

  case 7: // stmt: ";"
#line 207 "parser.y"
    {
        LOG_INFO("[bison]: stmt: SEMICOLON");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_empty>(yystack_[0].location);
    }
#line 943 "parser.cpp"
    break;

  case 8: // stmt: varlist "=" explist
#line 213 "parser.y"
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
#line 965 "parser.cpp"
    break;

  case 9: // stmt: functioncall
#line 232 "parser.y"
    {
        LOG_INFO("[bison]: stmt: functioncall");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 974 "parser.cpp"
    break;

  case 10: // stmt: label
#line 238 "parser.y"
    {
        LOG_INFO("[bison]: stmt: label");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 983 "parser.cpp"
    break;

  case 11: // stmt: "break"
#line 244 "parser.y"
    {
        LOG_INFO("[bison]: stmt: BREAK");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_break>(yystack_[0].location);
    }
#line 992 "parser.cpp"
    break;

  case 12: // stmt: "goto" "identifier"
#line 250 "parser.y"
    {
        LOG_INFO("[bison]: stmt: GOTO IDENTIFIER");
        auto go = std::make_shared<fakelua::syntax_tree_goto>(yystack_[0].location);
        go->set_label(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = go;
    }
#line 1003 "parser.cpp"
    break;

  case 13: // stmt: "do" block "end"
#line 258 "parser.y"
    {
        LOG_INFO("[bison]: stmt: DO block END");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = yystack_[1].value.as < fakelua::syntax_tree_interface_ptr > ();
    }
#line 1012 "parser.cpp"
    break;

  case 14: // stmt: "while" exp "do" block "end"
#line 264 "parser.y"
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
#line 1034 "parser.cpp"
    break;

  case 15: // stmt: "repeat" block "until" exp
#line 283 "parser.y"
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
#line 1056 "parser.cpp"
    break;

  case 16: // stmt: "if" exp "then" block elseifs "else" block "end"
#line 302 "parser.y"
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
#line 1090 "parser.cpp"
    break;

  case 17: // stmt: "if" exp "then" block elseifs "end"
#line 333 "parser.y"
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
#line 1118 "parser.cpp"
    break;

  case 18: // stmt: "for" "identifier" "=" exp "," exp "do" block "end"
#line 358 "parser.y"
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
#line 1147 "parser.cpp"
    break;

  case 19: // stmt: "for" "identifier" "=" exp "," exp "," exp "do" block "end"
#line 384 "parser.y"
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
#line 1182 "parser.cpp"
    break;

  case 20: // stmt: "for" namelist "in" explist "do" block "end"
#line 416 "parser.y"
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
#line 1210 "parser.cpp"
    break;

  case 21: // stmt: "function" funcname funcbody
#line 441 "parser.y"
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
#line 1232 "parser.cpp"
    break;

  case 22: // stmt: "local" "function" "identifier" funcbody
#line 460 "parser.y"
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
#line 1249 "parser.cpp"
    break;

  case 23: // stmt: "local" attnamelist
#line 474 "parser.y"
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
#line 1265 "parser.cpp"
    break;

  case 24: // stmt: "local" attnamelist "=" explist
#line 487 "parser.y"
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
#line 1287 "parser.cpp"
    break;

  case 25: // attnamelist: "identifier"
#line 508 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(yystack_[0].location);
        namelist->add_name(yystack_[0].value.as < std::string > ());
        namelist->add_attrib("");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1299 "parser.cpp"
    break;

  case 26: // attnamelist: "identifier" "<" "identifier" ">"
#line 517 "parser.y"
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(yystack_[3].location);
        namelist->add_name(yystack_[3].value.as < std::string > ());
        namelist->add_attrib(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1311 "parser.cpp"
    break;

  case 27: // attnamelist: attnamelist "," "identifier"
#line 526 "parser.y"
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
#line 1327 "parser.cpp"
    break;

  case 28: // attnamelist: attnamelist "," "identifier" "<" "identifier" ">"
#line 539 "parser.y"
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
#line 1343 "parser.cpp"
    break;

  case 29: // elseifs: %empty
#line 554 "parser.y"
    {
        LOG_INFO("[bison]: elseifs: empty");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = std::make_shared<fakelua::syntax_tree_elseiflist>(yystack_[0].location);
    }
#line 1352 "parser.cpp"
    break;

  case 30: // elseifs: "elseif" exp "then" block
#line 560 "parser.y"
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
#line 1374 "parser.cpp"
    break;

  case 31: // elseifs: elseifs "elseif" exp "then" block
#line 579 "parser.y"
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
#line 1400 "parser.cpp"
    break;

  case 32: // retstat: "return"
#line 604 "parser.y"
    {
        LOG_INFO("[bison]: retstat: RETURN");
        auto ret = std::make_shared<fakelua::syntax_tree_return>(yystack_[0].location);
        ret->set_explist(nullptr);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1411 "parser.cpp"
    break;

  case 33: // retstat: "return" explist
#line 612 "parser.y"
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
#line 1427 "parser.cpp"
    break;

  case 34: // label: "::" "identifier" "::"
#line 627 "parser.y"
    {
            LOG_INFO("[bison]: label: GOTO_TAG IDENTIFIER GOTO_TAG");
        auto ret = std::make_shared<fakelua::syntax_tree_label>(yystack_[1].location);
        ret->set_name(yystack_[1].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = ret;
    }
#line 1438 "parser.cpp"
    break;

  case 35: // funcnamelist: "identifier"
#line 637 "parser.y"
    {
        LOG_INFO("[bison]: funcnamelist: IDENTIFIER");
        auto funcnamelist = std::make_shared<fakelua::syntax_tree_funcnamelist>(yystack_[0].location);
        funcnamelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = funcnamelist;
    }
#line 1449 "parser.cpp"
    break;

  case 36: // funcnamelist: funcnamelist "." "identifier"
#line 645 "parser.y"
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
#line 1464 "parser.cpp"
    break;

  case 37: // funcname: funcnamelist
#line 659 "parser.y"
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
#line 1480 "parser.cpp"
    break;

  case 38: // funcname: funcnamelist ":" "identifier"
#line 672 "parser.y"
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
#line 1497 "parser.cpp"
    break;

  case 39: // varlist: var
#line 688 "parser.y"
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
#line 1513 "parser.cpp"
    break;

  case 40: // varlist: varlist "," var
#line 701 "parser.y"
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
#line 1533 "parser.cpp"
    break;

  case 41: // var: "identifier"
#line 720 "parser.y"
    {
        LOG_INFO("[bison]: var: IDENTIFIER");
        auto var = std::make_shared<fakelua::syntax_tree_var>(yystack_[0].location);
        var->set_name(yystack_[0].value.as < std::string > ());
        var->set_type("simple");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = var;
    }
#line 1545 "parser.cpp"
    break;

  case 42: // var: prefixexp "[" exp "]"
#line 729 "parser.y"
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
#line 1568 "parser.cpp"
    break;

  case 43: // var: prefixexp "." "identifier"
#line 749 "parser.y"
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
#line 1586 "parser.cpp"
    break;

  case 44: // namelist: "identifier"
#line 766 "parser.y"
    {
        LOG_INFO("[bison]: namelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(yystack_[0].location);
        namelist->add_name(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = namelist;
    }
#line 1597 "parser.cpp"
    break;

  case 45: // namelist: namelist "," "identifier"
#line 774 "parser.y"
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
#line 1612 "parser.cpp"
    break;

  case 46: // explist: exp
#line 788 "parser.y"
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
#line 1628 "parser.cpp"
    break;

  case 47: // explist: explist "," exp
#line 801 "parser.y"
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
#line 1648 "parser.cpp"
    break;

  case 48: // exp: "nil"
#line 820 "parser.y"
    {
        LOG_INFO("[bison]: exp: NIL");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("nil");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1659 "parser.cpp"
    break;

  case 49: // exp: "true"
#line 828 "parser.y"
    {
        LOG_INFO("[bison]: exp: TRUE");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("true");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1670 "parser.cpp"
    break;

  case 50: // exp: "false"
#line 836 "parser.y"
    {
        LOG_INFO("[bison]: exp: FALSES");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("false");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1681 "parser.cpp"
    break;

  case 51: // exp: "number"
#line 844 "parser.y"
    {
        LOG_INFO("[bison]: exp: NUMBER");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("number");
        exp->set_value(yystack_[0].value.as < std::string > ());
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1693 "parser.cpp"
    break;

  case 52: // exp: "string"
#line 853 "parser.y"
    {
        LOG_INFO("[bison]: exp: STRING");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("string");
        exp->set_value(l->remove_quotes(yystack_[0].value.as < std::string > ()));
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1705 "parser.cpp"
    break;

  case 53: // exp: "..."
#line 862 "parser.y"
    {
        LOG_INFO("[bison]: exp: VAR_PARAMS");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("var_params");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1716 "parser.cpp"
    break;

  case 54: // exp: functiondef
#line 870 "parser.y"
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
#line 1733 "parser.cpp"
    break;

  case 55: // exp: prefixexp
#line 884 "parser.y"
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
#line 1750 "parser.cpp"
    break;

  case 56: // exp: tableconstructor
#line 898 "parser.y"
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
#line 1767 "parser.cpp"
    break;

  case 57: // exp: exp "+" exp
#line 912 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp PLUS exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("PLUS");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1793 "parser.cpp"
    break;

  case 58: // exp: exp "-" exp
#line 935 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp MINUS exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("MINUS");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1819 "parser.cpp"
    break;

  case 59: // exp: exp "*" exp
#line 958 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp STAR exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("STAR");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1845 "parser.cpp"
    break;

  case 60: // exp: exp "/" exp
#line 981 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp SLASH exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("SLASH");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1871 "parser.cpp"
    break;

  case 61: // exp: exp "//" exp
#line 1004 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp DOUBLE_SLASH exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("DOUBLE_SLASH");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1897 "parser.cpp"
    break;

  case 62: // exp: exp "^" exp
#line 1027 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp POW exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("POW");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1923 "parser.cpp"
    break;

  case 63: // exp: exp "%" exp
#line 1050 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp MOD exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("MOD");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1949 "parser.cpp"
    break;

  case 64: // exp: exp "&" exp
#line 1073 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp BITAND exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("BITAND");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 1975 "parser.cpp"
    break;

  case 65: // exp: exp "~" exp
#line 1096 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp XOR exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("XOR");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2001 "parser.cpp"
    break;

  case 66: // exp: exp "|" exp
#line 1119 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp BITOR exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("BITOR");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2027 "parser.cpp"
    break;

  case 67: // exp: exp ">>" exp
#line 1142 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp RIGHT_SHIFT exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("RIGHT_SHIFT");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2053 "parser.cpp"
    break;

  case 68: // exp: exp "<<" exp
#line 1165 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp LEFT_SHIFT exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("LEFT_SHIFT");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2079 "parser.cpp"
    break;

  case 69: // exp: exp ".." exp
#line 1188 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp CONCAT exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("CONCAT");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2105 "parser.cpp"
    break;

  case 70: // exp: exp "<" exp
#line 1211 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp LESS exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("LESS");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2131 "parser.cpp"
    break;

  case 71: // exp: exp "<=" exp
#line 1234 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp LESS_EQUAL exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("LESS_EQUAL");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2157 "parser.cpp"
    break;

  case 72: // exp: exp ">" exp
#line 1257 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp MORE exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("MORE");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2183 "parser.cpp"
    break;

  case 73: // exp: exp ">=" exp
#line 1280 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp MORE_EQUAL exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("MORE_EQUAL");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2209 "parser.cpp"
    break;

  case 74: // exp: exp "==" exp
#line 1303 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp EQUAL exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("EQUAL");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2235 "parser.cpp"
    break;

  case 75: // exp: exp "~=" exp
#line 1326 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp NOT_EQUAL exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("NOT_EQUAL");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2261 "parser.cpp"
    break;

  case 76: // exp: exp "and" exp
#line 1349 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp AND exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("AND");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2287 "parser.cpp"
    break;

  case 77: // exp: exp "or" exp
#line 1372 "parser.y"
    {
        LOG_INFO("[bison]: exp: exp OR exp");
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
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(yystack_[1].location);
        binop->set_op("OR");
        exp->set_op(binop);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2313 "parser.cpp"
    break;

  case 78: // exp: "-" exp
#line 1395 "parser.y"
    {
        LOG_INFO("[bison]: exp: MINUS exp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[1].location);
        exp->set_type("unop");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[1].location);
        unop->set_op("MINUS");
        exp->set_op(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::throw_fakelua_exception("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2333 "parser.cpp"
    break;

  case 79: // exp: "not" exp
#line 1412 "parser.y"
    {
        LOG_INFO("[bison]: exp: NOT exp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[1].location);
        exp->set_type("unop");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[1].location);
        unop->set_op("NOT");
        exp->set_op(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::throw_fakelua_exception("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2353 "parser.cpp"
    break;

  case 80: // exp: "#" exp
#line 1429 "parser.y"
    {
        LOG_INFO("[bison]: exp: NUMBER_SIGN exp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[1].location);
        exp->set_type("unop");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[1].location);
        unop->set_op("NUMBER_SIGN");
        exp->set_op(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::throw_fakelua_exception("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2373 "parser.cpp"
    break;

  case 81: // exp: "~" exp
#line 1446 "parser.y"
    {
        LOG_INFO("[bison]: exp: BITNOT exp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[1].location);
        exp->set_type("unop");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(yystack_[1].location);
        unop->set_op("BITNOT");
        exp->set_op(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>(yystack_[0].value.as < fakelua::syntax_tree_interface_ptr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::throw_fakelua_exception("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = exp;
    }
#line 2393 "parser.cpp"
    break;

  case 82: // prefixexp: var
#line 1465 "parser.y"
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
#line 2410 "parser.cpp"
    break;

  case 83: // prefixexp: functioncall
#line 1479 "parser.y"
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
#line 2427 "parser.cpp"
    break;

  case 84: // prefixexp: "(" exp ")"
#line 1493 "parser.y"
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
#line 2444 "parser.cpp"
    break;

  case 85: // functioncall: prefixexp args
#line 1508 "parser.y"
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
#line 2466 "parser.cpp"
    break;

  case 86: // functioncall: prefixexp ":" "identifier" args
#line 1527 "parser.y"
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
#line 2489 "parser.cpp"
    break;

  case 87: // args: "(" explist ")"
#line 1549 "parser.y"
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
#line 2506 "parser.cpp"
    break;

  case 88: // args: "(" ")"
#line 1563 "parser.y"
    {
        LOG_INFO("[bison]: args: LPAREN RPAREN");
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[1].location);
        args->set_type("empty");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 2517 "parser.cpp"
    break;

  case 89: // args: tableconstructor
#line 1571 "parser.y"
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
#line 2534 "parser.cpp"
    break;

  case 90: // args: "string"
#line 1585 "parser.y"
    {
        LOG_INFO("[bison]: args: STRING");
        auto args = std::make_shared<fakelua::syntax_tree_args>(yystack_[0].location);
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(yystack_[0].location);
        exp->set_type("string");
        exp->set_value(l->remove_quotes(yystack_[0].value.as < std::string > ()));
        args->set_string(exp);
        args->set_type("string");
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = args;
    }
#line 2549 "parser.cpp"
    break;

  case 91: // functiondef: "function" funcbody
#line 1599 "parser.y"
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
#line 2565 "parser.cpp"
    break;

  case 92: // funcbody: "(" parlist ")" block "end"
#line 1614 "parser.y"
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
#line 2587 "parser.cpp"
    break;

  case 93: // funcbody: "(" ")" block "end"
#line 1633 "parser.y"
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
#line 2603 "parser.cpp"
    break;

  case 94: // parlist: namelist
#line 1648 "parser.y"
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
#line 2619 "parser.cpp"
    break;

  case 95: // parlist: namelist "," "..."
#line 1661 "parser.y"
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
#line 2636 "parser.cpp"
    break;

  case 96: // parlist: "..."
#line 1675 "parser.y"
    {
        LOG_INFO("[bison]: parlist: VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(yystack_[0].location);
        parlist->set_var_params(true);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = parlist;
    }
#line 2647 "parser.cpp"
    break;

  case 97: // tableconstructor: "{" fieldlist "}"
#line 1685 "parser.y"
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
#line 2663 "parser.cpp"
    break;

  case 98: // tableconstructor: "{" "}"
#line 1698 "parser.y"
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY RCURLY");
        auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(yystack_[1].location);
        yylhs.value.as < fakelua::syntax_tree_interface_ptr > () = tableconstructor;
    }
#line 2673 "parser.cpp"
    break;

  case 99: // fieldlist: field
#line 1707 "parser.y"
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
#line 2689 "parser.cpp"
    break;

  case 100: // fieldlist: fieldlist fieldsep field
#line 1720 "parser.y"
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
#line 2709 "parser.cpp"
    break;

  case 101: // field: "[" exp "]" "=" exp
#line 1739 "parser.y"
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
#line 2732 "parser.cpp"
    break;

  case 102: // field: "identifier" "=" exp
#line 1759 "parser.y"
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
#line 2750 "parser.cpp"
    break;

  case 103: // field: exp
#line 1774 "parser.y"
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
#line 2767 "parser.cpp"
    break;

  case 104: // fieldsep: ","
#line 1790 "parser.y"
    {
        LOG_INFO("[bison]: fieldsep: COMMA");
        // nothing to do
    }
#line 2776 "parser.cpp"
    break;

  case 105: // fieldsep: ";"
#line 1796 "parser.y"
    {
        LOG_INFO("[bison]: fieldsep: SEMICOLON");
        // nothing to do
    }
#line 2785 "parser.cpp"
    break;


#line 2789 "parser.cpp"

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


  const signed char parser::yypact_ninf_ = -56;

  const signed char parser::yytable_ninf_ = -84;

  const short
  parser::yypact_[] =
  {
    1407,   459,   -56,  1407,   -49,   -37,   459,   -11,  1407,   459,
     459,   -36,   -33,   -56,   -56,    33,  1407,   -56,   -56,   -56,
      11,    37,    26,    20,   459,    17,   -56,    27,   -56,   459,
     -56,   -56,   459,   459,   -56,   -56,   -56,   551,    26,   -56,
     -56,   -56,   319,    50,    40,   -56,   -34,    27,   604,     6,
      23,    42,  1151,    36,  1028,   657,   -56,    44,   -56,   -56,
     459,     1,   414,   459,    28,    34,   -56,   -56,   -56,    45,
     -56,   459,    89,  1028,    61,   -56,     9,   -56,    45,    45,
      45,   459,   459,   459,   459,   -56,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   -56,   459,   459,    35,    46,    53,   -56,
    1407,    27,    55,   459,    59,   459,   459,  1407,   -56,    36,
      43,   -56,     3,   710,    -4,   -56,   763,   459,   -56,   -56,
     -56,   115,  1407,   -56,   -56,    56,   104,    95,    95,    45,
      45,  1102,  1081,    45,    93,   576,   576,   576,   576,    93,
      93,    45,    45,   525,   632,   369,   576,   576,   816,    15,
     -56,   -56,   -56,  1183,   -56,    65,    36,    66,  1028,  1028,
    1211,   -56,   -56,   -56,   118,  1028,   -56,  1239,   -18,  1407,
     459,  1407,   459,    64,   -56,    69,   -56,   459,   -56,   -56,
    1267,   498,  1295,   869,  1407,   459,   -56,    79,  1028,   -56,
    1407,   459,   -56,  1407,  1323,   922,   -56,  1351,   975,  1407,
     -56,  1407,   -56,  1407,  1407,  1379,   -56
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,    11,     3,     0,     0,     0,     0,     3,    32,
       0,     0,     0,     7,    41,     0,     2,     4,     6,    10,
       0,    82,     0,     9,     0,     0,    50,     0,    48,     0,
      49,    53,     0,     0,    52,    51,    82,     0,    55,    83,
      54,    56,     0,    44,     0,    35,    37,     0,     0,     0,
      25,    23,     0,    33,    46,     0,    12,     0,     1,     5,
       0,     0,     0,     0,     0,     0,    90,    85,    89,    78,
      98,     0,    41,   103,     0,    99,     0,    91,    79,    81,
      80,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,     0,     0,     0,     0,     0,    21,
       3,     0,     0,     0,     0,     0,     0,     3,    34,     8,
      82,    88,     0,     0,     0,    43,     0,     0,    97,   105,
     104,     0,     3,    96,    44,    94,     0,    58,    57,    59,
      60,    76,    77,    61,    69,    74,    73,    71,    75,    68,
      67,    62,    63,    64,    66,    65,    72,    70,     0,     0,
      45,    38,    36,    29,    22,     0,    24,    27,    15,    47,
       0,    87,    42,    86,     0,   102,   100,     0,     0,     3,
       0,     3,     0,     0,    26,     0,    14,     0,    93,    95,
       0,     0,     0,     0,     3,     0,    17,     0,   101,    92,
       3,     0,    20,     3,     0,     0,    28,     0,     0,    30,
      16,     3,    18,     3,    31,     0,    19
  };

  const short
  parser::yypgoto_[] =
  {
     -56,   -56,    16,   240,   -56,   -56,   -56,   -56,   -56,   -56,
     -56,     0,    60,   -55,   300,    54,   108,    14,   -56,   -45,
     -56,   -21,   -56,     8,   -56
  };

  const unsigned char
  parser::yydefgoto_[] =
  {
       0,    15,    16,    17,    51,   183,    18,    19,    46,    47,
      20,    36,    44,    53,    54,    38,    39,    67,    40,    77,
     136,    41,    74,    75,   131
  };

  const short
  parser::yytable_[] =
  {
      21,    68,   109,    21,    62,   119,    25,   122,    21,     1,
      43,    49,   171,   107,    60,   108,    21,    68,   132,    42,
     189,    24,    45,    56,    52,     1,    57,    25,    70,    71,
     -83,   181,   -83,    58,    62,    76,    25,    26,    63,    27,
     -39,   160,    21,    28,    29,   113,   -40,   133,    50,    30,
     159,   116,    21,   104,    22,    31,    66,    22,   166,    61,
      14,   120,    22,   116,   105,   111,   164,   -83,   134,   -83,
      22,    32,   128,    64,    33,    65,    72,    34,    35,   112,
     -83,   194,   195,   196,   116,   -39,    66,   124,   106,   118,
     114,   -40,   127,   125,   160,    96,    22,    81,    82,    83,
      84,    83,    84,    68,   178,   161,    22,   129,    23,   130,
      21,    23,   162,   179,   165,    22,    23,    21,   167,    24,
     184,   187,   185,     1,    23,    25,   163,    71,   197,    88,
      89,    88,    21,   170,   206,    26,   135,    27,   173,   176,
       0,    28,    29,    96,    97,    96,    97,    30,   177,     0,
      23,     0,     0,    31,     0,     0,     0,     0,     0,     0,
      23,     0,     0,    21,    22,     0,     0,     0,     0,    32,
      21,    22,    33,     0,    72,    34,    35,    21,     0,    21,
       0,    21,     0,     0,     0,     0,    22,     0,     0,     0,
      21,     0,    21,     0,    21,   190,     0,   192,     0,     0,
      21,     0,     0,    21,    21,     0,     0,    21,     0,    21,
     204,    21,     0,    21,    21,    21,   207,    22,    23,   209,
       0,     0,     0,     0,    22,    23,     0,   214,     0,   215,
       0,    22,     0,    22,     0,    22,     0,     0,     0,     0,
      23,     0,     0,     0,    22,     0,    22,     0,    22,     0,
       0,     0,     0,     0,    22,     0,    59,    22,    22,     0,
       0,    22,     0,    22,     0,    22,     0,    22,    22,    22,
       0,    23,     0,     0,     0,     0,     0,     0,    23,     0,
       0,     0,    59,     0,     0,    23,     0,    23,     0,    23,
       0,     0,    59,     0,     0,     0,     0,     0,    23,     0,
      23,    37,    23,     0,     0,     0,    48,     0,    23,     0,
      55,    23,    23,     0,     0,    23,     0,    23,     0,    23,
       0,    23,    23,    23,    69,    73,     0,     1,     0,    78,
       0,     0,    79,    80,     2,     3,     0,     0,   103,     0,
       4,     5,     6,     0,     7,     0,     0,     0,     8,     9,
       0,     0,     0,    10,    11,     0,     0,     0,     0,     0,
       0,     0,     0,   123,    12,    13,     0,     0,     0,     0,
       0,   126,     0,    81,    82,    83,    84,     0,    14,     0,
       0,   137,   138,   139,   140,     0,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    59,   158,    88,    89,     0,     0,     0,
      59,     0,    94,    95,     0,   168,   169,    59,    24,    96,
      97,    98,     1,   121,    25,     0,     0,   175,     0,     0,
      59,    73,    59,     0,    26,     0,    27,     0,     0,     0,
      28,    29,     0,     0,    59,     0,    30,    59,     0,    59,
       0,     0,    31,     0,    59,    59,     0,     0,     0,     0,
       0,     0,     0,    24,     0,     0,     0,     1,    32,    25,
       0,    33,     0,    14,    34,    35,     0,     0,     0,    26,
     191,    27,   193,     0,     0,    28,    29,   198,     0,     0,
       0,    30,     0,     0,     0,   205,     0,    31,     0,     0,
       0,   208,    81,    82,    83,    84,     0,     0,     0,     0,
       0,     0,    86,    32,   200,     0,    33,     0,    14,    34,
      35,     0,     0,     0,     0,     0,    87,     0,     0,    81,
      82,    83,    84,     0,    88,    89,     0,    90,    91,    92,
      93,    94,    95,     0,     0,     0,   201,     0,    96,    97,
      98,    99,   100,   101,   102,    81,    82,    83,    84,     0,
      85,    88,    89,     0,     0,    86,     0,     0,    94,    95,
       0,     0,     0,     0,     0,    96,    97,     0,     0,    87,
      81,    82,    83,    84,     0,     0,     0,    88,    89,     0,
      90,    91,    92,    93,    94,    95,     0,     0,     0,     0,
       0,    96,    97,    98,    99,   100,   101,   102,    81,    82,
      83,    84,    88,    89,     0,     0,     0,     0,    86,    94,
      95,     0,     0,     0,     0,     0,    96,    97,    98,    99,
     100,     0,    87,     0,     0,   110,    81,    82,    83,    84,
      88,    89,     0,    90,    91,    92,    93,    94,    95,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,    81,    82,    83,    84,     0,     0,     0,    88,    89,
       0,    86,     0,   117,     0,    94,    95,     0,     0,     0,
       0,     0,    96,    97,    98,    87,   100,     0,     0,     0,
       0,     0,     0,    88,    89,     0,    90,    91,    92,    93,
      94,    95,     0,     0,     0,     0,     0,    96,    97,    98,
      99,   100,   101,   102,    81,    82,    83,    84,     0,     0,
       0,     0,     0,   172,    86,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,     0,     0,     0,    88,    89,     0,    90,
      91,    92,    93,    94,    95,     0,     0,     0,     0,     0,
      96,    97,    98,    99,   100,   101,   102,    81,    82,    83,
      84,     0,     0,     0,     0,     0,   174,    86,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,     0,     0,     0,    88,
      89,     0,    90,    91,    92,    93,    94,    95,     0,     0,
       0,     0,     0,    96,    97,    98,    99,   100,   101,   102,
      81,    82,    83,    84,     0,     0,     0,     0,     0,     0,
      86,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,     0,
       0,     0,    88,    89,     0,    90,    91,    92,    93,    94,
      95,     0,     0,     0,   180,     0,    96,    97,    98,    99,
     100,   101,   102,    81,    82,    83,    84,     0,     0,     0,
       0,     0,     0,    86,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    87,     0,     0,
     203,     0,     0,     0,     0,    88,    89,     0,    90,    91,
      92,    93,    94,    95,     0,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,    81,    82,    83,    84,
       0,     0,     0,     0,     0,     0,    86,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      87,     0,     0,   211,     0,     0,     0,     0,    88,    89,
       0,    90,    91,    92,    93,    94,    95,     0,     0,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,    81,
      82,    83,    84,     0,     0,     0,     0,     0,     0,    86,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,     0,     0,
       0,    88,    89,     0,    90,    91,    92,    93,    94,    95,
       0,     0,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,    81,    82,    83,    84,     0,     0,     0,     0,
       0,     0,    86,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,     0,     0,     0,    88,    89,     0,    90,    91,    92,
      93,    94,    95,     0,     0,     0,     0,     0,    96,    97,
      98,    99,   100,   101,   102,    81,    82,    83,    84,     0,
       0,     0,     0,     0,     0,    86,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    81,    82,    83,    84,
       0,     0,     0,     0,     0,     0,     0,    88,    89,     0,
      90,    91,    92,    93,    94,    95,     0,     0,     0,     0,
       0,    96,    97,    98,    99,   100,   101,   102,    88,    89,
       0,    90,    91,    92,    93,    94,    95,     0,     0,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,     1,
       0,     0,     0,     0,     0,     0,     2,     3,     0,     0,
       0,     0,     4,     5,     6,     0,     7,     0,     0,     0,
       8,     9,     0,     0,   115,    10,    11,     0,     0,     0,
       0,     1,     0,     0,     0,     0,    12,    13,     2,     3,
       0,   182,     0,     0,     4,     5,     6,     0,     7,     0,
      14,     0,     8,     9,     0,     0,     0,    10,    11,     1,
       0,     0,     0,     0,     0,     0,     2,     3,    12,    13,
     186,     0,     4,     5,     6,     0,     7,     0,     0,     0,
       8,     9,    14,     0,     0,    10,    11,     1,     0,     0,
       0,     0,     0,     0,     2,     3,    12,    13,   188,     0,
       4,     5,     6,     0,     7,     0,     0,     0,     8,     9,
      14,     0,     0,    10,    11,     1,     0,     0,     0,     0,
       0,     0,     2,     3,    12,    13,   199,     0,     4,     5,
       6,     0,     7,     0,     0,     0,     8,     9,    14,     0,
       0,    10,    11,     1,     0,     0,     0,     0,     0,     0,
       2,     3,    12,    13,   202,     0,     4,     5,     6,     0,
       7,     0,     0,     0,     8,     9,    14,     0,     0,    10,
      11,     1,     0,     0,     0,     0,     0,     0,     2,     3,
      12,    13,   210,     0,     4,     5,     6,     0,     7,     0,
       0,     0,     8,     9,    14,     0,     0,    10,    11,     1,
       0,     0,     0,     0,     0,     0,     2,     3,    12,    13,
     212,     0,     4,     5,     6,     0,     7,     0,     0,     0,
       8,     9,    14,     0,     0,    10,    11,     1,     0,     0,
       0,     0,     0,     0,     2,     3,    12,    13,   216,     0,
       4,     5,     6,     0,     7,     0,     0,     0,     8,     9,
      14,     0,     0,    10,    11,     1,     0,     0,     0,     0,
       0,     0,     2,     3,    12,    13,     0,     0,     4,     5,
       6,     0,     7,     0,     0,     0,     8,     9,    14,     0,
       0,    10,    11,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14
  };

  const short
  parser::yycheck_[] =
  {
       0,    22,    47,     3,     8,    60,    10,    62,     8,     8,
      59,    22,     9,    47,     3,    49,    16,    38,     9,     3,
      38,     4,    59,    59,     8,     8,    59,    10,    11,    12,
      10,    16,    12,     0,     8,     8,    10,    20,    12,    22,
       3,    59,    42,    26,    27,     3,     3,    38,    59,    32,
     105,    48,    52,     3,     0,    38,    60,     3,   113,    48,
      59,    61,     8,    48,    24,    59,   111,    47,    59,    49,
      16,    54,    11,    47,    57,    49,    59,    60,    61,    56,
      60,    17,    18,    19,    48,    48,    60,    59,    48,    45,
      48,    48,     3,    59,    59,    50,    42,     4,     5,     6,
       7,     6,     7,   124,    48,    59,    52,    46,     0,    48,
     110,     3,    59,     9,    59,    61,     8,   117,    59,     4,
      55,     3,    56,     8,    16,    10,   110,    12,    59,    36,
      37,    36,   132,   117,    55,    20,    76,    22,   124,   131,
      -1,    26,    27,    50,    51,    50,    51,    32,   132,    -1,
      42,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,   163,   110,    -1,    -1,    -1,    -1,    54,
     170,   117,    57,    -1,    59,    60,    61,   177,    -1,   179,
      -1,   181,    -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,
     190,    -1,   192,    -1,   194,   179,    -1,   181,    -1,    -1,
     200,    -1,    -1,   203,   204,    -1,    -1,   207,    -1,   209,
     194,   211,    -1,   213,   214,   215,   200,   163,   110,   203,
      -1,    -1,    -1,    -1,   170,   117,    -1,   211,    -1,   213,
      -1,   177,    -1,   179,    -1,   181,    -1,    -1,    -1,    -1,
     132,    -1,    -1,    -1,   190,    -1,   192,    -1,   194,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    16,   203,   204,    -1,
      -1,   207,    -1,   209,    -1,   211,    -1,   213,   214,   215,
      -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,
      -1,    -1,    42,    -1,    -1,   177,    -1,   179,    -1,   181,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,   190,    -1,
     192,     1,   194,    -1,    -1,    -1,     6,    -1,   200,    -1,
      10,   203,   204,    -1,    -1,   207,    -1,   209,    -1,   211,
      -1,   213,   214,   215,    24,    25,    -1,     8,    -1,    29,
      -1,    -1,    32,    33,    15,    16,    -1,    -1,    19,    -1,
      21,    22,    23,    -1,    25,    -1,    -1,    -1,    29,    30,
      -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    45,    46,    -1,    -1,    -1,    -1,
      -1,    71,    -1,     4,     5,     6,     7,    -1,    59,    -1,
      -1,    81,    82,    83,    84,    -1,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   163,   104,    36,    37,    -1,    -1,    -1,
     170,    -1,    43,    44,    -1,   115,   116,   177,     4,    50,
      51,    52,     8,     9,    10,    -1,    -1,   127,    -1,    -1,
     190,   131,   192,    -1,    20,    -1,    22,    -1,    -1,    -1,
      26,    27,    -1,    -1,   204,    -1,    32,   207,    -1,   209,
      -1,    -1,    38,    -1,   214,   215,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,    -1,    -1,    -1,     8,    54,    10,
      -1,    57,    -1,    59,    60,    61,    -1,    -1,    -1,    20,
     180,    22,   182,    -1,    -1,    26,    27,   187,    -1,    -1,
      -1,    32,    -1,    -1,    -1,   195,    -1,    38,    -1,    -1,
      -1,   201,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    54,    16,    -1,    57,    -1,    59,    60,
      61,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,     4,
       5,     6,     7,    -1,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,     4,     5,     6,     7,    -1,
       9,    36,    37,    -1,    -1,    14,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    28,
       4,     5,     6,     7,    -1,    -1,    -1,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,     4,     5,
       6,     7,    36,    37,    -1,    -1,    -1,    -1,    14,    43,
      44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    -1,    28,    -1,    -1,    31,     4,     5,     6,     7,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,     4,     5,     6,     7,    -1,    -1,    -1,    36,    37,
      -1,    14,    -1,    16,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    50,    51,    52,    28,    54,    -1,    -1,    -1,
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
      52,    53,    54,    55,    56,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    50,    51,    52,    53,    54,    55,    56,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    -1,    -1,
      -1,    -1,    21,    22,    23,    -1,    25,    -1,    -1,    -1,
      29,    30,    -1,    -1,    33,    34,    35,    -1,    -1,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    45,    46,    15,    16,
      -1,    18,    -1,    -1,    21,    22,    23,    -1,    25,    -1,
      59,    -1,    29,    30,    -1,    -1,    -1,    34,    35,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    45,    46,
      19,    -1,    21,    22,    23,    -1,    25,    -1,    -1,    -1,
      29,    30,    59,    -1,    -1,    34,    35,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    16,    45,    46,    19,    -1,
      21,    22,    23,    -1,    25,    -1,    -1,    -1,    29,    30,
      59,    -1,    -1,    34,    35,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    16,    45,    46,    19,    -1,    21,    22,
      23,    -1,    25,    -1,    -1,    -1,    29,    30,    59,    -1,
      -1,    34,    35,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    16,    45,    46,    19,    -1,    21,    22,    23,    -1,
      25,    -1,    -1,    -1,    29,    30,    59,    -1,    -1,    34,
      35,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,
      45,    46,    19,    -1,    21,    22,    23,    -1,    25,    -1,
      -1,    -1,    29,    30,    59,    -1,    -1,    34,    35,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    45,    46,
      19,    -1,    21,    22,    23,    -1,    25,    -1,    -1,    -1,
      29,    30,    59,    -1,    -1,    34,    35,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    16,    45,    46,    19,    -1,
      21,    22,    23,    -1,    25,    -1,    -1,    -1,    29,    30,
      59,    -1,    -1,    34,    35,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    16,    45,    46,    -1,    -1,    21,    22,
      23,    -1,    25,    -1,    -1,    -1,    29,    30,    59,    -1,
      -1,    34,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59
  };

  const signed char
  parser::yystos_[] =
  {
       0,     8,    15,    16,    21,    22,    23,    25,    29,    30,
      34,    35,    45,    46,    59,    63,    64,    65,    68,    69,
      72,    73,    77,    78,     4,    10,    20,    22,    26,    27,
      32,    38,    54,    57,    60,    61,    73,    76,    77,    78,
      80,    83,    64,    59,    74,    59,    70,    71,    76,    22,
      59,    66,    64,    75,    76,    76,    59,    59,     0,    65,
       3,    48,     8,    12,    47,    49,    60,    79,    83,    76,
      11,    12,    59,    76,    84,    85,     8,    81,    76,    76,
      76,     4,     5,     6,     7,     9,    14,    28,    36,    37,
      39,    40,    41,    42,    43,    44,    50,    51,    52,    53,
      54,    55,    56,    19,     3,    24,    48,    47,    49,    81,
      31,    59,    56,     3,    48,    33,    48,    16,    45,    75,
      73,     9,    75,    76,    59,    59,    76,     3,    11,    46,
      48,    86,     9,    38,    59,    74,    82,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    75,
      59,    59,    59,    64,    81,    59,    75,    59,    76,    76,
      64,     9,    13,    79,    13,    76,    85,    64,    48,     9,
      48,    16,    18,    67,    55,    56,    19,     3,    19,    38,
      64,    76,    64,    76,    17,    18,    19,    59,    76,    19,
      16,    48,    19,    31,    64,    76,    55,    64,    76,    64,
      19,    31,    19,    16,    64,    64,    19
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    62,    63,    64,    64,    64,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    66,    66,    66,    66,    67,
      67,    67,    68,    68,    69,    70,    70,    71,    71,    72,
      72,    73,    73,    73,    74,    74,    75,    75,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    77,    77,    77,    78,    78,    79,    79,    79,
      79,    80,    81,    81,    82,    82,    82,    83,    83,    84,
      84,    85,    85,    85,    86,    86
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     1,     2,     1,     1,     3,     1,
       1,     1,     2,     3,     5,     4,     8,     6,     9,    11,
       7,     3,     4,     2,     4,     1,     4,     3,     6,     0,
       4,     5,     1,     2,     3,     1,     3,     1,     3,     1,
       3,     1,     4,     3,     1,     3,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     1,     1,     3,     2,     4,     3,     2,     1,
       1,     2,     5,     4,     1,     3,     1,     3,     2,     1,
       3,     5,     3,     1,     1,     1
  };




#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   155,   155,   163,   169,   181,   200,   206,   212,   231,
     237,   243,   249,   257,   263,   282,   301,   332,   357,   383,
     415,   440,   459,   473,   486,   507,   516,   525,   538,   553,
     559,   578,   603,   611,   626,   636,   644,   658,   671,   687,
     700,   719,   728,   748,   765,   773,   787,   800,   819,   827,
     835,   843,   852,   861,   869,   883,   897,   911,   934,   957,
     980,  1003,  1026,  1049,  1072,  1095,  1118,  1141,  1164,  1187,
    1210,  1233,  1256,  1279,  1302,  1325,  1348,  1371,  1394,  1411,
    1428,  1445,  1464,  1478,  1492,  1507,  1526,  1548,  1562,  1570,
    1584,  1598,  1613,  1632,  1647,  1660,  1674,  1684,  1697,  1706,
    1719,  1738,  1758,  1773,  1789,  1795
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
#line 3738 "parser.cpp"

#line 1802 "parser.y"


void
yy::parser::error (const location_type& l, const std::string& m)
{
    std::stringstream ss;
    ss << l;
    fakelua::throw_fakelua_exception(std::format("{}: {}", ss.str(), m));
}
