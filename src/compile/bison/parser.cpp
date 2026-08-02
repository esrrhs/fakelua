// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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


// First part of user declarations.

#line 37 "parser.cpp" // lalr1.cc:404

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "parser.h"

// User implementation prologue.

#line 51 "parser.cpp" // lalr1.cc:412
// Unqualified %code blocks.
#line 30 "parser.y" // lalr1.cc:413

#include "compile/my_flexer.h"

using namespace fakelua;

yy::parser::symbol_type yylex(fakelua::MyFlexer* l) {
    auto ret = l->MyYylex();
    std::stringstream ss;
    ss << ret.location;
    LOG_INFO("[bison]: bison get token loc: {}", ss.str());
    return ret;
}

int yyFlexLexer::yylex() { return -1; }


#line 70 "parser.cpp" // lalr1.cc:413


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
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

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
      *yycdebug_ << std::endl;                  \
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
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE(Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {
#line 156 "parser.cpp" // lalr1.cc:479

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              // Fall through.
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  parser::parser (fakelua::MyFlexer* l_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      l (l_yyarg)
  {}

  parser::~parser ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  inline
  parser::by_state::by_state ()
    : state (empty_state)
  {}

  inline
  parser::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
  parser::by_state::clear ()
  {
    state = empty_state;
  }

  inline
  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  inline
  parser::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
  parser::symbol_number_type
  parser::by_state::type_get () const
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
  }

  inline
  parser::stack_symbol_type::stack_symbol_type ()
  {}


  inline
  parser::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
      switch (that.type_get ())
    {
      case 64: // chunk
      case 65: // block
      case 66: // stmt
      case 67: // attnamelist
      case 68: // elseifs
      case 69: // retstat
      case 70: // label
      case 71: // funcnamelist
      case 72: // funcname
      case 73: // varlist
      case 74: // var
      case 75: // namelist
      case 76: // explist
      case 77: // exp
      case 78: // prefixexp
      case 79: // functioncall
      case 80: // args
      case 81: // functiondef
      case 82: // funcbody
      case 83: // parlist
      case 84: // tableconstructor
      case 85: // fieldlist
      case 86: // field
        value.move< fakelua::SyntaxTreeInterfacePtr > (that.value);
        break;

      case 60: // "identifier"
      case 61: // "string"
      case 62: // "number"
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

  inline
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
      switch (that.type_get ())
    {
      case 64: // chunk
      case 65: // block
      case 66: // stmt
      case 67: // attnamelist
      case 68: // elseifs
      case 69: // retstat
      case 70: // label
      case 71: // funcnamelist
      case 72: // funcname
      case 73: // varlist
      case 74: // var
      case 75: // namelist
      case 76: // explist
      case 77: // exp
      case 78: // prefixexp
      case 79: // functioncall
      case 80: // args
      case 81: // functiondef
      case 82: // funcbody
      case 83: // parlist
      case 84: // tableconstructor
      case 85: // fieldlist
      case 86: // field
        value.copy< fakelua::SyntaxTreeInterfacePtr > (that.value);
        break;

      case 60: // "identifier"
      case 61: // "string"
      case 62: // "number"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }


  template <typename Base>
  inline
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    switch (yytype)
    {
            case 60: // "identifier"

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< std::string > (); }
#line 387 "parser.cpp" // lalr1.cc:636
        break;

      case 61: // "string"

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< std::string > (); }
#line 394 "parser.cpp" // lalr1.cc:636
        break;

      case 62: // "number"

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< std::string > (); }
#line 401 "parser.cpp" // lalr1.cc:636
        break;

      case 64: // chunk

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 408 "parser.cpp" // lalr1.cc:636
        break;

      case 65: // block

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 415 "parser.cpp" // lalr1.cc:636
        break;

      case 66: // stmt

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 422 "parser.cpp" // lalr1.cc:636
        break;

      case 67: // attnamelist

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 429 "parser.cpp" // lalr1.cc:636
        break;

      case 68: // elseifs

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 436 "parser.cpp" // lalr1.cc:636
        break;

      case 69: // retstat

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 443 "parser.cpp" // lalr1.cc:636
        break;

      case 70: // label

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 450 "parser.cpp" // lalr1.cc:636
        break;

      case 71: // funcnamelist

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 457 "parser.cpp" // lalr1.cc:636
        break;

      case 72: // funcname

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 464 "parser.cpp" // lalr1.cc:636
        break;

      case 73: // varlist

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 471 "parser.cpp" // lalr1.cc:636
        break;

      case 74: // var

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 478 "parser.cpp" // lalr1.cc:636
        break;

      case 75: // namelist

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 485 "parser.cpp" // lalr1.cc:636
        break;

      case 76: // explist

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 492 "parser.cpp" // lalr1.cc:636
        break;

      case 77: // exp

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 499 "parser.cpp" // lalr1.cc:636
        break;

      case 78: // prefixexp

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 506 "parser.cpp" // lalr1.cc:636
        break;

      case 79: // functioncall

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 513 "parser.cpp" // lalr1.cc:636
        break;

      case 80: // args

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 520 "parser.cpp" // lalr1.cc:636
        break;

      case 81: // functiondef

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 527 "parser.cpp" // lalr1.cc:636
        break;

      case 82: // funcbody

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 534 "parser.cpp" // lalr1.cc:636
        break;

      case 83: // parlist

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 541 "parser.cpp" // lalr1.cc:636
        break;

      case 84: // tableconstructor

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 548 "parser.cpp" // lalr1.cc:636
        break;

      case 85: // fieldlist

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 555 "parser.cpp" // lalr1.cc:636
        break;

      case 86: // field

#line 156 "parser.y" // lalr1.cc:636
        { yyo << yysym.value.template as< fakelua::SyntaxTreeInterfacePtr > (); }
#line 562 "parser.cpp" // lalr1.cc:636
        break;


      default:
        break;
    }
    yyo << ')';
  }
#endif

  inline
  void
  parser::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  inline
  void
  parser::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  inline
  void
  parser::yypop_ (unsigned int n)
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

  inline parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  inline bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::parse ()
  {
    // State.
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

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            symbol_type yylookahead (yylex (l));
            yyla.move (yylookahead);
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
        switch (yyr1_[yyn])
    {
      case 64: // chunk
      case 65: // block
      case 66: // stmt
      case 67: // attnamelist
      case 68: // elseifs
      case 69: // retstat
      case 70: // label
      case 71: // funcnamelist
      case 72: // funcname
      case 73: // varlist
      case 74: // var
      case 75: // namelist
      case 76: // explist
      case 77: // exp
      case 78: // prefixexp
      case 79: // functioncall
      case 80: // args
      case 81: // functiondef
      case 82: // funcbody
      case 83: // parlist
      case 84: // tableconstructor
      case 85: // fieldlist
      case 86: // field
        yylhs.value.build< fakelua::SyntaxTreeInterfacePtr > ();
        break;

      case 60: // "identifier"
      case 61: // "string"
      case 62: // "number"
        yylhs.value.build< std::string > ();
        break;

      default:
        break;
    }


      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 2:
#line 163 "parser.y" // lalr1.cc:859
    {
    LOG_INFO("[bison]: chunk: block");
    l->SetChunk(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
    }
#line 817 "parser.cpp" // lalr1.cc:859
    break;

  case 3:
#line 171 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: block: empty");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = std::make_shared<fakelua::SyntaxTreeBlock>(yystack_[0].location);
    }
#line 826 "parser.cpp" // lalr1.cc:859
    break;

  case 4:
#line 177 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: block: block stmt");
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: block: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        auto stmt = std::dynamic_pointer_cast<fakelua::SyntaxTreeInterface>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (stmt == nullptr) {
            LOG_ERROR("[bison]: block: stmt is not a stmt");
            fakelua::ThrowFakeluaException("stmt is not a stmt");
        }
        if (block->Stmts().empty()) {
            block->SetLoc(yystack_[0].location);
        }
        block->AddStmt(stmt);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = block;
    }
#line 849 "parser.cpp" // lalr1.cc:859
    break;

  case 5:
#line 199 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: retstat");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ();
    }
#line 858 "parser.cpp" // lalr1.cc:859
    break;

  case 6:
#line 205 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: SEMICOLON");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = std::make_shared<fakelua::SyntaxTreeEmpty>(yystack_[0].location);
    }
#line 867 "parser.cpp" // lalr1.cc:859
    break;

  case 7:
#line 211 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: varlist ASSIGN explist");
        auto varlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeVarlist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
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
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = assign;
    }
#line 889 "parser.cpp" // lalr1.cc:859
    break;

  case 8:
#line 230 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: functioncall");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ();
    }
#line 898 "parser.cpp" // lalr1.cc:859
    break;

  case 9:
#line 236 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: label");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ();
    }
#line 907 "parser.cpp" // lalr1.cc:859
    break;

  case 10:
#line 242 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: BREAK");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = std::make_shared<fakelua::SyntaxTreeBreak>(yystack_[0].location);
    }
#line 916 "parser.cpp" // lalr1.cc:859
    break;

  case 11:
#line 248 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: CONTINUE");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = std::make_shared<fakelua::SyntaxTreeContinue>(yystack_[0].location);
    }
#line 925 "parser.cpp" // lalr1.cc:859
    break;

  case 12:
#line 254 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: GOTO IDENTIFIER");
        auto go = std::make_shared<fakelua::SyntaxTreeGoto>(yystack_[0].location);
        go->SetLabel(yystack_[0].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = go;
    }
#line 936 "parser.cpp" // lalr1.cc:859
    break;

  case 13:
#line 262 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: DO block END");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ();
    }
#line 945 "parser.cpp" // lalr1.cc:859
    break;

  case 14:
#line 268 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: WHILE exp DO block END");
        auto while_stmt = std::make_shared<fakelua::SyntaxTreeWhile>(yystack_[4].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[3].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        while_stmt->SetExp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        while_stmt->SetBlock(block);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = while_stmt;
    }
#line 967 "parser.cpp" // lalr1.cc:859
    break;

  case 15:
#line 287 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: REPEAT block UNTIL exp");
        auto repeat = std::make_shared<fakelua::SyntaxTreeRepeat>(yystack_[3].location);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        repeat->SetBlock(block);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        repeat->SetExp(exp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = repeat;
    }
#line 989 "parser.cpp" // lalr1.cc:859
    break;

  case 16:
#line 306 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs ELSE block END");
        auto if_stmt = std::make_shared<fakelua::SyntaxTreeIf>(yystack_[7].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[6].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        if_stmt->SetExp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[4].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        if_stmt->SetBlock(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>(yystack_[3].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::ThrowFakeluaException("elseiflist is not a elseiflist");
        }
        if_stmt->SetElseiflist(elseifs);
        auto else_block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (else_block == nullptr) {
            LOG_ERROR("[bison]: stmt: else_block is not a block");
            fakelua::ThrowFakeluaException("else_block is not a block");
        }
        if_stmt->SetElseBlock(else_block);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = if_stmt;
    }
#line 1023 "parser.cpp" // lalr1.cc:859
    break;

  case 17:
#line 337 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs END");
        auto if_stmt = std::make_shared<fakelua::SyntaxTreeIf>(yystack_[5].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[4].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        if_stmt->SetExp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        if_stmt->SetBlock(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::ThrowFakeluaException("elseiflist is not a elseiflist");
        }
        if_stmt->SetElseiflist(elseifs);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = if_stmt;
    }
#line 1051 "parser.cpp" // lalr1.cc:859
    break;

  case 18:
#line 362 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::SyntaxTreeForLoop>(yystack_[8].location);
        for_loop_stmt->SetName(yystack_[7].value.as< std::string > ());
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[5].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        for_loop_stmt->SetExpBegin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[3].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::ThrowFakeluaException("end_exp is not a exp");
        }
        for_loop_stmt->SetExpEnd(end_exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        for_loop_stmt->SetBlock(block);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = for_loop_stmt;
    }
#line 1080 "parser.cpp" // lalr1.cc:859
    break;

  case 19:
#line 388 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::SyntaxTreeForLoop>(yystack_[10].location);
        for_loop_stmt->SetName(yystack_[9].value.as< std::string > ());
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[7].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        for_loop_stmt->SetExpBegin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[5].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::ThrowFakeluaException("end_exp is not a exp");
        }
        for_loop_stmt->SetExpEnd(end_exp);
        auto step_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[3].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (step_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: step_exp is not a exp");
            fakelua::ThrowFakeluaException("step_exp is not a exp");
        }
        for_loop_stmt->SetExpStep(step_exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        for_loop_stmt->SetBlock(block);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = for_loop_stmt;
    }
#line 1115 "parser.cpp" // lalr1.cc:859
    break;

  case 20:
#line 420 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: for namelist in explist do block end");
        auto for_in_stmt = std::make_shared<fakelua::SyntaxTreeForIn>(yystack_[6].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[5].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        for_in_stmt->SetNamelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[3].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        for_in_stmt->SetExplist(explist);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        for_in_stmt->SetBlock(block);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = for_in_stmt;
    }
#line 1143 "parser.cpp" // lalr1.cc:859
    break;

  case 21:
#line 445 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: function funcname funcbody");
        auto func_stmt = std::make_shared<fakelua::SyntaxTreeFunction>(yystack_[2].location);
        auto funcname = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncname>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (funcname == nullptr) {
            LOG_ERROR("[bison]: stmt: funcname is not a funcname");
            fakelua::ThrowFakeluaException("funcname is not a funcname");
        }
        func_stmt->SetFuncname(funcname);
        auto funcbody = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::ThrowFakeluaException("funcbody is not a funcbody");
        }
        func_stmt->SetFuncbody(funcbody);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = func_stmt;
    }
#line 1165 "parser.cpp" // lalr1.cc:859
    break;

  case 22:
#line 464 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: local function IDENTIFIER funcbody");
        auto local_func_stmt = std::make_shared<fakelua::SyntaxTreeLocalFunction>(yystack_[3].location);
        local_func_stmt->SetName(yystack_[1].value.as< std::string > ());
        auto funcbody = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::ThrowFakeluaException("funcbody is not a funcbody");
        }
        local_func_stmt->SetFuncbody(funcbody);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = local_func_stmt;
    }
#line 1182 "parser.cpp" // lalr1.cc:859
    break;

  case 23:
#line 478 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: local attnamelist");
        auto local_stmt = std::make_shared<fakelua::SyntaxTreeLocalVar>(yystack_[1].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        local_stmt->SetNamelist(namelist);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = local_stmt;
    }
#line 1198 "parser.cpp" // lalr1.cc:859
    break;

  case 24:
#line 491 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: stmt: local attnamelist assign explist");
        auto local_stmt = std::make_shared<fakelua::SyntaxTreeLocalVar>(yystack_[3].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        local_stmt->SetNamelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        local_stmt->SetExplist(explist);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = local_stmt;
    }
#line 1220 "parser.cpp" // lalr1.cc:859
    break;

  case 25:
#line 512 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::SyntaxTreeNamelist>(yystack_[0].location);
        namelist->AddName(yystack_[0].value.as< std::string > ());
        namelist->AddAttrib("");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1232 "parser.cpp" // lalr1.cc:859
    break;

  case 26:
#line 521 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::make_shared<fakelua::SyntaxTreeNamelist>(yystack_[3].location);
        namelist->AddName(yystack_[3].value.as< std::string > ());
        namelist->AddAttrib(yystack_[1].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1244 "parser.cpp" // lalr1.cc:859
    break;

  case 27:
#line 530 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        namelist->AddName(yystack_[0].value.as< std::string > ());
        namelist->AddAttrib("");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1260 "parser.cpp" // lalr1.cc:859
    break;

  case 28:
#line 543 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[5].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        namelist->AddName(yystack_[3].value.as< std::string > ());
        namelist->AddAttrib(yystack_[1].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1276 "parser.cpp" // lalr1.cc:859
    break;

  case 29:
#line 558 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: elseifs: empty");
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = std::make_shared<fakelua::SyntaxTreeElseiflist>(yystack_[0].location);
    }
#line 1285 "parser.cpp" // lalr1.cc:859
    break;

  case 30:
#line 564 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: elseifs: elseif exp then block");
        auto elseifs = std::make_shared<fakelua::SyntaxTreeElseiflist>(yystack_[3].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        elseifs->AddElseifExpr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        elseifs->AddElseifBlock(block);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = elseifs;
    }
#line 1307 "parser.cpp" // lalr1.cc:859
    break;

  case 31:
#line 583 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: elseifs: elseifs elseif exp then block");
        auto elseifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>(yystack_[4].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: elseifs: elseifs is not a elseifs");
            fakelua::ThrowFakeluaException("elseifs is not a elseifs");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        elseifs->AddElseifExpr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        elseifs->AddElseifBlock(block);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = elseifs;
    }
#line 1333 "parser.cpp" // lalr1.cc:859
    break;

  case 32:
#line 608 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: retstat: RETURN");
        auto ret = std::make_shared<fakelua::SyntaxTreeReturn>(yystack_[0].location);
        ret->SetExplist(nullptr);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = ret;
    }
#line 1344 "parser.cpp" // lalr1.cc:859
    break;

  case 33:
#line 616 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: retstat: RETURN explist");
        auto ret = std::make_shared<fakelua::SyntaxTreeReturn>(yystack_[1].location);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: retstat: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        ret->SetExplist(explist);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = ret;
    }
#line 1360 "parser.cpp" // lalr1.cc:859
    break;

  case 34:
#line 631 "parser.y" // lalr1.cc:859
    {
            LOG_INFO("[bison]: label: GOTO_TAG IDENTIFIER GOTO_TAG");
        auto ret = std::make_shared<fakelua::SyntaxTreeLabel>(yystack_[1].location);
        ret->SetName(yystack_[1].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = ret;
    }
#line 1371 "parser.cpp" // lalr1.cc:859
    break;

  case 35:
#line 641 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: funcnamelist: IDENTIFIER");
        auto funcnamelist = std::make_shared<fakelua::SyntaxTreeFuncnamelist>(yystack_[0].location);
        funcnamelist->AddName(yystack_[0].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = funcnamelist;
    }
#line 1382 "parser.cpp" // lalr1.cc:859
    break;

  case 36:
#line 649 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: funcnamelist: funcnamelist DOT IDENTIFIER");
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncnamelist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcnamelist: funcnamelist is not a funcnamelist");
            fakelua::ThrowFakeluaException("funcnamelist is not a funcnamelist");
        }
        funcnamelist->AddName(yystack_[0].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = funcnamelist;
    }
#line 1397 "parser.cpp" // lalr1.cc:859
    break;

  case 37:
#line 663 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: funcname: funcnamelist");
        auto funcname = std::make_shared<fakelua::SyntaxTreeFuncname>(yystack_[0].location);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncnamelist>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::ThrowFakeluaException("funcnamelist is not a funcnamelist");
        }
        funcname->SetFuncNameList(funcnamelist);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = funcname;
    }
#line 1413 "parser.cpp" // lalr1.cc:859
    break;

  case 38:
#line 676 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: funcname: funcnamelist COLON IDENTIFIER");
        auto funcname = std::make_shared<fakelua::SyntaxTreeFuncname>(yystack_[2].location);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncnamelist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::ThrowFakeluaException("funcnamelist is not a funcnamelist");
        }
        funcname->SetFuncNameList(funcnamelist);
        funcname->SetColonName(yystack_[0].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = funcname;
    }
#line 1430 "parser.cpp" // lalr1.cc:859
    break;

  case 39:
#line 692 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: varlist: var");
        auto varlist = std::make_shared<fakelua::SyntaxTreeVarlist>(yystack_[0].location);
        auto var = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::ThrowFakeluaException("var is not a var");
        }
        varlist->AddVar(var);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = varlist;
    }
#line 1446 "parser.cpp" // lalr1.cc:859
    break;

  case 40:
#line 705 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: varlist: varlist COMMA var");
        auto varlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeVarlist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (varlist == nullptr) {
            LOG_ERROR("[bison]: varlist: varlist is not a varlist");
            fakelua::ThrowFakeluaException("varlist is not a varlist");
        }
        auto var = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::ThrowFakeluaException("var is not a var");
        }
        varlist->AddVar(var);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = varlist;
    }
#line 1466 "parser.cpp" // lalr1.cc:859
    break;

  case 41:
#line 724 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: var: IDENTIFIER");
        auto var = std::make_shared<fakelua::SyntaxTreeVar>(yystack_[0].location);
        var->SetName(yystack_[0].value.as< std::string > ());
        var->SetVarKind(VarKind::kSimple);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = var;
    }
#line 1478 "parser.cpp" // lalr1.cc:859
    break;

  case 42:
#line 733 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: var: prefixexp LSQUARE exp RSQUARE");
        auto var = std::make_shared<fakelua::SyntaxTreeVar>(yystack_[2].location);
        var->SetVarKind(VarKind::kSquare);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[3].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        var->SetPrefixexp(prefixexp);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: var: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        var->SetExp(exp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = var;
    }
#line 1501 "parser.cpp" // lalr1.cc:859
    break;

  case 43:
#line 753 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: var: prefixexp DOT IDENTIFIER");
        auto var = std::make_shared<fakelua::SyntaxTreeVar>(yystack_[1].location);
        var->SetVarKind(VarKind::kDot);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        var->SetPrefixexp(prefixexp);
        var->SetName(yystack_[0].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = var;
    }
#line 1519 "parser.cpp" // lalr1.cc:859
    break;

  case 44:
#line 770 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: namelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::SyntaxTreeNamelist>(yystack_[0].location);
        namelist->AddName(yystack_[0].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1530 "parser.cpp" // lalr1.cc:859
    break;

  case 45:
#line 778 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: namelist: namelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        namelist->AddName(yystack_[0].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = namelist;
    }
#line 1545 "parser.cpp" // lalr1.cc:859
    break;

  case 46:
#line 792 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: explist: exp");
        auto explist = std::make_shared<fakelua::SyntaxTreeExplist>(yystack_[0].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        explist->AddExp(exp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = explist;
    }
#line 1561 "parser.cpp" // lalr1.cc:859
    break;

  case 47:
#line 805 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: explist: explist COMMA exp");
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: explist: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        explist->AddExp(exp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = explist;
    }
#line 1581 "parser.cpp" // lalr1.cc:859
    break;

  case 48:
#line 824 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: NIL");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kNil);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1592 "parser.cpp" // lalr1.cc:859
    break;

  case 49:
#line 832 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: TRUE");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kTrue);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1603 "parser.cpp" // lalr1.cc:859
    break;

  case 50:
#line 840 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: FALSES");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kFalse);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1614 "parser.cpp" // lalr1.cc:859
    break;

  case 51:
#line 848 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: NUMBER");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kNumber);
        exp->SetValue(yystack_[0].value.as< std::string > ());
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1626 "parser.cpp" // lalr1.cc:859
    break;

  case 52:
#line 857 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: STRING");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kString);
        exp->SetValue(l->RemoveQuotes(yystack_[0].value.as< std::string > ()));
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1638 "parser.cpp" // lalr1.cc:859
    break;

  case 53:
#line 866 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: VAR_PARAMS");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kVarParams);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1649 "parser.cpp" // lalr1.cc:859
    break;

  case 54:
#line 874 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: functiondef");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kFunctionDef);
        auto functiondef = std::dynamic_pointer_cast<fakelua::SyntaxTreeFunctiondef>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (functiondef == nullptr) {
            LOG_ERROR("[bison]: exp: functiondef is not a functiondef");
            fakelua::ThrowFakeluaException("functiondef is not a functiondef");
        }
        exp->SetRight(functiondef);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1666 "parser.cpp" // lalr1.cc:859
    break;

  case 55:
#line 888 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: prefixexp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kPrefixExp);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: exp: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        exp->SetRight(prefixexp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1683 "parser.cpp" // lalr1.cc:859
    break;

  case 56:
#line 902 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: tableconstructor");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kTableConstructor);
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::SyntaxTreeTableconstructor>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: exp: tableconstructor is not a tableconstructor");
            fakelua::ThrowFakeluaException("tableconstructor is not a tableconstructor");
        }
        exp->SetRight(tableconstructor);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1700 "parser.cpp" // lalr1.cc:859
    break;

  case 57:
#line 916 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp PLUS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kPlus);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1726 "parser.cpp" // lalr1.cc:859
    break;

  case 58:
#line 939 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp MINUS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kMinus);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1752 "parser.cpp" // lalr1.cc:859
    break;

  case 59:
#line 962 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp STAR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kStar);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1778 "parser.cpp" // lalr1.cc:859
    break;

  case 60:
#line 985 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp SLASH exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kSlash);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1804 "parser.cpp" // lalr1.cc:859
    break;

  case 61:
#line 1008 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp DOUBLE_SLASH exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kDoubleSlash);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1830 "parser.cpp" // lalr1.cc:859
    break;

  case 62:
#line 1031 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp POW exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kPow);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1856 "parser.cpp" // lalr1.cc:859
    break;

  case 63:
#line 1054 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp MOD exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kMod);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1882 "parser.cpp" // lalr1.cc:859
    break;

  case 64:
#line 1077 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp BITAND exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kBitAnd);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1908 "parser.cpp" // lalr1.cc:859
    break;

  case 65:
#line 1100 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp XOR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kXor);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1934 "parser.cpp" // lalr1.cc:859
    break;

  case 66:
#line 1123 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp BITOR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kBitOr);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1960 "parser.cpp" // lalr1.cc:859
    break;

  case 67:
#line 1146 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp RIGHT_SHIFT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kRightShift);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 1986 "parser.cpp" // lalr1.cc:859
    break;

  case 68:
#line 1169 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp LEFT_SHIFT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kLeftShift);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2012 "parser.cpp" // lalr1.cc:859
    break;

  case 69:
#line 1192 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp CONCAT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kConcat);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2038 "parser.cpp" // lalr1.cc:859
    break;

  case 70:
#line 1215 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp LESS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kLess);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2064 "parser.cpp" // lalr1.cc:859
    break;

  case 71:
#line 1238 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp LESS_EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kLessEqual);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2090 "parser.cpp" // lalr1.cc:859
    break;

  case 72:
#line 1261 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp MORE exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kMore);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2116 "parser.cpp" // lalr1.cc:859
    break;

  case 73:
#line 1284 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp MORE_EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kMoreEqual);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2142 "parser.cpp" // lalr1.cc:859
    break;

  case 74:
#line 1307 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kEqual);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2168 "parser.cpp" // lalr1.cc:859
    break;

  case 75:
#line 1330 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp NOT_EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kNotEqual);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2194 "parser.cpp" // lalr1.cc:859
    break;

  case 76:
#line 1353 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp AND exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kAnd);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2220 "parser.cpp" // lalr1.cc:859
    break;

  case 77:
#line 1376 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: exp OR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[2].location);
        exp->SetExpKind(ExpKind::kBinop);
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(yystack_[1].location);
        binop->SetOpKind(BinOpKind::kOr);
        exp->SetOp(binop);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2246 "parser.cpp" // lalr1.cc:859
    break;

  case 78:
#line 1399 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: MINUS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[1].location);
        exp->SetExpKind(ExpKind::kUnop);
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(yystack_[1].location);
        unop->SetOpKind(UnOpKind::kMinus);
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2266 "parser.cpp" // lalr1.cc:859
    break;

  case 79:
#line 1416 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: NOT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[1].location);
        exp->SetExpKind(ExpKind::kUnop);
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(yystack_[1].location);
        unop->SetOpKind(UnOpKind::kNot);
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2286 "parser.cpp" // lalr1.cc:859
    break;

  case 80:
#line 1433 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: NUMBER_SIGN exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[1].location);
        exp->SetExpKind(ExpKind::kUnop);
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(yystack_[1].location);
        unop->SetOpKind(UnOpKind::kNumberSign);
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2306 "parser.cpp" // lalr1.cc:859
    break;

  case 81:
#line 1450 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: exp: BITNOT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[1].location);
        exp->SetExpKind(ExpKind::kUnop);
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(yystack_[1].location);
        unop->SetOpKind(UnOpKind::kBitNot);
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = exp;
    }
#line 2326 "parser.cpp" // lalr1.cc:859
    break;

  case 82:
#line 1469 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: prefixexp: var");
        auto prefixexp = std::make_shared<fakelua::SyntaxTreePrefixexp>(yystack_[0].location);
        prefixexp->SetPrefixKind(PrefixExpKind::kVar);
        auto var = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (var == nullptr) {
            LOG_ERROR("[bison]: prefixexp: var is not a var");
            fakelua::ThrowFakeluaException("var is not a var");
        }
        prefixexp->SetValue(var);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = prefixexp;
    }
#line 2343 "parser.cpp" // lalr1.cc:859
    break;

  case 83:
#line 1483 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: prefixexp: functioncall");
        auto prefixexp = std::make_shared<fakelua::SyntaxTreePrefixexp>(yystack_[0].location);
        prefixexp->SetPrefixKind(PrefixExpKind::kFunctionCall);
        auto functioncall = std::dynamic_pointer_cast<fakelua::SyntaxTreeFunctioncall>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (functioncall == nullptr) {
            LOG_ERROR("[bison]: prefixexp: functioncall is not a functioncall");
            fakelua::ThrowFakeluaException("functioncall is not a functioncall");
        }
        prefixexp->SetValue(functioncall);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = prefixexp;
    }
#line 2360 "parser.cpp" // lalr1.cc:859
    break;

  case 84:
#line 1497 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: prefixexp: LPAREN exp RPAREN");
        auto prefixexp = std::make_shared<fakelua::SyntaxTreePrefixexp>(yystack_[2].location);
        prefixexp->SetPrefixKind(PrefixExpKind::kExp);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: prefixexp: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        prefixexp->SetValue(exp);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = prefixexp;
    }
#line 2377 "parser.cpp" // lalr1.cc:859
    break;

  case 85:
#line 1512 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: functioncall: prefixexp args");
        auto functioncall = std::make_shared<fakelua::SyntaxTreeFunctioncall>(yystack_[1].location);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        functioncall->SetPrefixexp(prefixexp);
        auto args = std::dynamic_pointer_cast<fakelua::SyntaxTreeArgs>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::ThrowFakeluaException("args is not a args");
        }
        functioncall->SetArgs(args);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = functioncall;
    }
#line 2399 "parser.cpp" // lalr1.cc:859
    break;

  case 86:
#line 1531 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: functioncall: prefixexp COLON IDENTIFIER args");
        auto functioncall = std::make_shared<fakelua::SyntaxTreeFunctioncall>(yystack_[3].location);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>(yystack_[3].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        functioncall->SetPrefixexp(prefixexp);
        functioncall->SetName(yystack_[1].value.as< std::string > ());
        auto args = std::dynamic_pointer_cast<fakelua::SyntaxTreeArgs>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::ThrowFakeluaException("args is not a args");
        }
        functioncall->SetArgs(args);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = functioncall;
    }
#line 2422 "parser.cpp" // lalr1.cc:859
    break;

  case 87:
#line 1553 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: args: LPAREN explist RPAREN");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(yystack_[2].location);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (explist == nullptr) {
            LOG_ERROR("[bison]: args: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        args->SetExplist(explist);
        args->SetArgsKind(ArgsKind::kExpList);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = args;
    }
#line 2439 "parser.cpp" // lalr1.cc:859
    break;

  case 88:
#line 1567 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: args: LPAREN RPAREN");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(yystack_[1].location);
        args->SetArgsKind(ArgsKind::kEmpty);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = args;
    }
#line 2450 "parser.cpp" // lalr1.cc:859
    break;

  case 89:
#line 1575 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: args: tableconstructor");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(yystack_[0].location);
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::SyntaxTreeTableconstructor>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: args: tableconstructor is not a tableconstructor");
            fakelua::ThrowFakeluaException("tableconstructor is not a tableconstructor");
        }
        args->SetTableconstructor(tableconstructor);
        args->SetArgsKind(ArgsKind::kTableConstructor);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = args;
    }
#line 2467 "parser.cpp" // lalr1.cc:859
    break;

  case 90:
#line 1589 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: args: STRING");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(yystack_[0].location);
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(yystack_[0].location);
        exp->SetExpKind(ExpKind::kString);
        exp->SetValue(l->RemoveQuotes(yystack_[0].value.as< std::string > ()));
        args->SetString(exp);
        args->SetArgsKind(ArgsKind::kString);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = args;
    }
#line 2482 "parser.cpp" // lalr1.cc:859
    break;

  case 91:
#line 1603 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: functiondef: FUNCTION funcbody");
        auto functiondef = std::make_shared<fakelua::SyntaxTreeFunctiondef>(yystack_[1].location);
        auto funcbody = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: functiondef: funcbody is not a funcbody");
            fakelua::ThrowFakeluaException("funcbody is not a funcbody");
        }
        functiondef->SetFuncbody(funcbody);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = functiondef;
    }
#line 2498 "parser.cpp" // lalr1.cc:859
    break;

  case 92:
#line 1618 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: funcbody: LPAREN parlist RPAREN block END");
        auto funcbody = std::make_shared<fakelua::SyntaxTreeFuncbody>(yystack_[4].location);
        auto parlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeParlist>(yystack_[3].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (parlist == nullptr) {
            LOG_ERROR("[bison]: funcbody: parlist is not a parlist");
            fakelua::ThrowFakeluaException("parlist is not a parlist");
        }
        funcbody->SetParlist(parlist);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        funcbody->SetBlock(block);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = funcbody;
    }
#line 2520 "parser.cpp" // lalr1.cc:859
    break;

  case 93:
#line 1637 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: funcbody: LPAREN RPAREN block END");
        auto funcbody = std::make_shared<fakelua::SyntaxTreeFuncbody>(yystack_[3].location);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        funcbody->SetBlock(block);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = funcbody;
    }
#line 2536 "parser.cpp" // lalr1.cc:859
    break;

  case 94:
#line 1652 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: parlist: namelist");
        auto parlist = std::make_shared<fakelua::SyntaxTreeParlist>(yystack_[0].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        parlist->SetNamelist(namelist);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = parlist;
    }
#line 2552 "parser.cpp" // lalr1.cc:859
    break;

  case 95:
#line 1665 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: parlist: namelist COMMA VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::SyntaxTreeParlist>(yystack_[2].location);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        parlist->SetNamelist(namelist);
        parlist->SetVarParams(true);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = parlist;
    }
#line 2569 "parser.cpp" // lalr1.cc:859
    break;

  case 96:
#line 1679 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: parlist: VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::SyntaxTreeParlist>(yystack_[0].location);
        parlist->SetVarParams(true);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = parlist;
    }
#line 2580 "parser.cpp" // lalr1.cc:859
    break;

  case 97:
#line 1689 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY fieldlist RCURLY");
        auto tableconstructor = std::make_shared<fakelua::SyntaxTreeTableconstructor>(yystack_[2].location);
        auto fieldlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFieldlist>(yystack_[1].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: tableconstructor: fieldlist is not a fieldlist");
            fakelua::ThrowFakeluaException("fieldlist is not a fieldlist");
        }
        tableconstructor->SetFieldlist(fieldlist);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = tableconstructor;
    }
#line 2596 "parser.cpp" // lalr1.cc:859
    break;

  case 98:
#line 1702 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY RCURLY");
        auto tableconstructor = std::make_shared<fakelua::SyntaxTreeTableconstructor>(yystack_[1].location);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = tableconstructor;
    }
#line 2606 "parser.cpp" // lalr1.cc:859
    break;

  case 99:
#line 1711 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: fieldlist: field");
        auto fieldlist = std::make_shared<fakelua::SyntaxTreeFieldlist>(yystack_[0].location);
        auto field = std::dynamic_pointer_cast<fakelua::SyntaxTreeField>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::ThrowFakeluaException("field is not a field");
        }
        fieldlist->AddField(field);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = fieldlist;
    }
#line 2622 "parser.cpp" // lalr1.cc:859
    break;

  case 100:
#line 1724 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: fieldlist: fieldlist fieldsep field");
        auto fieldlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFieldlist>(yystack_[2].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: fieldlist: fieldlist is not a fieldlist");
            fakelua::ThrowFakeluaException("fieldlist is not a fieldlist");
        }
        auto field = std::dynamic_pointer_cast<fakelua::SyntaxTreeField>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::ThrowFakeluaException("field is not a field");
        }
        fieldlist->AddField(field);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = fieldlist;
    }
#line 2642 "parser.cpp" // lalr1.cc:859
    break;

  case 101:
#line 1743 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: field: LSQUARE exp RSQUARE ASSIGN exp");
        auto field = std::make_shared<fakelua::SyntaxTreeField>(yystack_[4].location);
        auto key = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[3].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (key == nullptr) {
            LOG_ERROR("[bison]: key: key is not a exp");
            fakelua::ThrowFakeluaException("key is not a exp");
        }
        field->SetKey(key);
        auto value = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (value == nullptr) {
            LOG_ERROR("[bison]: field: value is not a exp");
            fakelua::ThrowFakeluaException("value is not a exp");
        }
        field->SetValue(value);
        field->SetFieldKind(FieldKind::kArray);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = field;
    }
#line 2665 "parser.cpp" // lalr1.cc:859
    break;

  case 102:
#line 1763 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: field: IDENTIFIER ASSIGN exp");
        auto field = std::make_shared<fakelua::SyntaxTreeField>(yystack_[2].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        field->SetName(yystack_[2].value.as< std::string > ());
        field->SetValue(exp);
        field->SetFieldKind(FieldKind::kObject);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = field;
    }
#line 2683 "parser.cpp" // lalr1.cc:859
    break;

  case 103:
#line 1778 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: field: exp");
        auto field = std::make_shared<fakelua::SyntaxTreeField>(yystack_[0].location);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>(yystack_[0].value.as< fakelua::SyntaxTreeInterfacePtr > ());
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        field->SetValue(exp);
        field->SetFieldKind(FieldKind::kArray);
        yylhs.value.as< fakelua::SyntaxTreeInterfacePtr > () = field;
    }
#line 2700 "parser.cpp" // lalr1.cc:859
    break;

  case 104:
#line 1794 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: fieldsep: COMMA");
        // nothing to do
    }
#line 2709 "parser.cpp" // lalr1.cc:859
    break;

  case 105:
#line 1800 "parser.y" // lalr1.cc:859
    {
        LOG_INFO("[bison]: fieldsep: SEMICOLON");
        // nothing to do
    }
#line 2718 "parser.cpp" // lalr1.cc:859
    break;


#line 2722 "parser.cpp" // lalr1.cc:859
            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
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
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
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

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
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

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what());
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

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
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -48;

  const signed char parser::yytable_ninf_ = -84;

  const short int
  parser::yypact_[] =
  {
     -48,    19,  1221,   -48,   325,   -48,   -48,   -48,   -32,   -31,
     325,   -20,   -48,   325,   325,   -28,   -26,   -48,   -48,   -48,
     -48,   -48,    13,    36,    -3,    68,   325,    10,   -48,    46,
     -48,   325,   -48,   -48,   325,   325,   -48,   -48,   -48,   440,
      -3,   -48,   -48,   -48,   898,    38,   -13,   -48,   -37,    46,
     494,    22,    26,    47,   931,    30,   827,   548,   -48,    43,
     325,     7,    65,   325,    24,    31,   -48,   -48,   -48,    39,
     -48,   325,    91,   827,    84,   -48,    21,   -48,    39,    39,
      39,   325,   325,   325,   325,   -48,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   -48,   325,   325,    42,    45,    49,   -48,
     -48,    46,    50,   325,    51,   325,   325,   -48,   -48,    30,
      48,   -48,    17,   569,     0,   -48,   623,   325,   -48,   -48,
     -48,   109,   -48,   -48,   -48,    54,    90,   108,   108,    39,
      39,   848,   225,    39,   148,   466,   466,   466,   466,   148,
     148,    39,    39,   293,   727,  1254,   466,   466,   644,    27,
     -48,   -48,   -48,   964,   -48,    56,    30,    67,   827,   827,
     997,   -48,   -48,   -48,   103,   827,   -48,  1030,   -33,   -48,
     325,   -48,   325,     5,   -48,    62,   -48,   325,   -48,   -48,
    1063,   386,  1096,   698,   -48,   325,   -48,    72,   827,   -48,
     -48,   325,   -48,   -48,  1129,   752,   -48,  1162,   806,  1221,
     -48,   -48,   -48,   -48,  1221,  1195,   -48
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     2,     1,     0,    10,    11,     3,     0,     0,
       0,     0,     3,    32,     0,     0,     0,     6,    41,     4,
       5,     9,     0,    82,     0,     8,     0,     0,    50,     0,
      48,     0,    49,    53,     0,     0,    52,    51,    82,     0,
      55,    83,    54,    56,     0,    44,     0,    35,    37,     0,
       0,     0,    25,    23,     0,    33,    46,     0,    12,     0,
       0,     0,     0,     0,     0,     0,    90,    85,    89,    78,
      98,     0,    41,   103,     0,    99,     0,    91,    79,    81,
      80,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,     0,     0,     0,     0,     0,    21,
       3,     0,     0,     0,     0,     0,     0,     3,    34,     7,
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

  const short int
  parser::yypgoto_[] =
  {
     -48,   -48,    41,   -48,   -48,   -48,   -48,   -48,   -48,   -48,
     -48,    -2,    58,    -5,   224,     2,    33,   -17,   -48,   -47,
     -48,   -23,   -48,     4,   -48
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     1,     2,    19,    53,   183,    20,    21,    48,    49,
      22,    38,    46,    55,    56,    40,    41,    67,    42,    77,
     136,    43,    74,    75,   131
  };

  const short int
  parser::yytable_[] =
  {
      23,    68,   109,    51,    24,    62,   189,    27,    62,    63,
      27,   107,   105,   108,    26,     4,    60,    68,     4,     3,
      27,    70,    71,   194,   195,   196,   171,   160,    45,    47,
     132,    28,    58,    29,    59,    25,   106,    30,    31,   -39,
      52,   104,    23,    32,   181,    64,    24,    65,    44,    33,
     113,   -40,    23,    54,    76,   119,    24,   122,    66,   120,
     133,    66,    61,    24,   164,    34,   116,    18,    35,    26,
      72,    36,    37,     4,   121,    27,   116,    25,   -83,   116,
     -83,   134,   111,   112,   124,   -39,    28,    25,    29,   118,
      96,   125,    30,    31,   127,   128,   114,   -40,    32,   179,
     159,    68,   160,   178,    33,   161,   187,   173,   166,   162,
     165,   167,   184,    26,    83,    84,   -83,     4,   -83,    27,
      34,    71,   197,    35,   185,    18,    36,    37,   206,   -83,
      28,   129,    29,   130,   135,   176,    30,    31,     0,     0,
       0,     0,    32,     0,     0,    88,     0,     0,    33,     0,
       0,   163,    81,    82,    83,    84,     0,     0,   170,    96,
      97,    23,     0,     0,    34,    24,     0,    35,    23,    72,
      36,    37,    24,   177,     0,    23,     0,     0,     0,    24,
       0,     0,     0,     0,     0,    88,    89,     0,    23,     0,
      23,     0,    24,     0,    24,     0,    25,     0,     0,    96,
      97,     0,    23,    25,     0,    23,    24,    23,     0,    24,
      25,    24,    23,    23,     0,     0,    24,    24,     0,     0,
     190,     0,   192,    25,     0,    25,     0,     0,    39,    81,
      82,    83,    84,     0,    50,   204,     0,    25,    57,    86,
      25,   207,    25,     0,   209,     0,     0,    25,    25,     0,
      69,    73,   214,     0,   215,    78,     0,     0,    79,    80,
       0,     0,    88,    89,     0,    90,    91,    92,    93,    94,
      95,     0,     0,     0,     0,     0,    96,    97,    98,    99,
     100,   101,   102,     0,     0,     0,     0,   123,     0,     0,
       0,     0,     0,     0,     0,   126,     0,    81,    82,    83,
      84,     0,     0,     0,     0,   137,   138,   139,   140,     0,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,     0,   158,    26,
      88,    89,     0,     4,     0,    27,     0,    94,    95,   168,
     169,     0,     0,     0,    96,    97,    28,     0,    29,     0,
       0,   175,    30,    31,     0,    73,     0,     0,    32,     0,
       0,     0,     0,     0,    33,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      34,     0,     0,    35,     0,    18,    36,    37,     0,     0,
      81,    82,    83,    84,     0,     0,     0,     0,     0,     0,
      86,     0,     0,   200,   191,     0,   193,     0,     0,     0,
       0,   198,     0,     0,     0,    87,     0,     0,     0,   205,
       0,     0,     0,    88,    89,   208,    90,    91,    92,    93,
      94,    95,     0,     0,     0,   201,     0,    96,    97,    98,
      99,   100,   101,   102,    81,    82,    83,    84,     0,    85,
       0,     0,     0,     0,    86,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    87,
      81,    82,    83,    84,     0,     0,     0,    88,    89,     0,
      90,    91,    92,    93,    94,    95,     0,     0,     0,     0,
       0,    96,    97,    98,    99,   100,   101,   102,    81,    82,
      83,    84,     0,    88,    89,     0,     0,     0,    86,     0,
      94,    95,     0,     0,     0,     0,     0,    96,    97,    98,
      99,   100,     0,    87,     0,     0,   110,     0,     0,     0,
       0,    88,    89,     0,    90,    91,    92,    93,    94,    95,
       0,     0,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,    81,    82,    83,    84,     0,     0,     0,     0,
       0,     0,    86,     0,     0,   117,     0,     0,     0,     0,
       0,     0,     0,    81,    82,    83,    84,    87,     0,     0,
       0,     0,   172,    86,     0,    88,    89,     0,    90,    91,
      92,    93,    94,    95,     0,     0,     0,     0,    87,    96,
      97,    98,    99,   100,   101,   102,    88,    89,     0,    90,
      91,    92,    93,    94,    95,     0,     0,     0,     0,     0,
      96,    97,    98,    99,   100,   101,   102,    81,    82,    83,
      84,     0,     0,     0,     0,     0,   174,    86,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    81,    82,
      83,    84,    87,     0,     0,     0,     0,     0,    86,     0,
      88,    89,     0,    90,    91,    92,    93,    94,    95,     0,
       0,     0,     0,    87,    96,    97,    98,    99,   100,   101,
     102,    88,    89,     0,    90,    91,    92,    93,    94,    95,
       0,     0,     0,   180,     0,    96,    97,    98,    99,   100,
     101,   102,    81,    82,    83,    84,     0,     0,     0,     0,
       0,     0,    86,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    87,     0,     0,
     203,    81,    82,    83,    84,    88,    89,     0,    90,    91,
      92,    93,    94,    95,     0,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,    81,    82,    83,    84,
       0,     0,     0,     0,    88,    89,    86,     0,     0,     0,
       0,    94,    95,     0,     0,     0,     0,     0,    96,    97,
      98,    87,   100,     0,   211,     0,     0,     0,     0,    88,
      89,     0,    90,    91,    92,    93,    94,    95,     0,     0,
       0,     0,     0,    96,    97,    98,    99,   100,   101,   102,
      81,    82,    83,    84,     0,     0,     0,     0,     0,     0,
      86,     0,     0,   213,     0,     0,     0,     0,     0,     0,
       0,    81,    82,    83,    84,    87,     0,     0,     0,     0,
       0,    86,     0,    88,    89,     0,    90,    91,    92,    93,
      94,    95,    81,    82,    83,    84,    87,    96,    97,    98,
      99,   100,   101,   102,    88,    89,     0,    90,    91,    92,
      93,    94,    95,     0,     0,     0,     0,     0,    96,    97,
      98,    99,   100,   101,   102,    88,    89,     0,    90,    91,
      92,    93,    94,    95,     0,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,     4,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     0,     0,   103,     0,
       8,     9,    10,     0,    11,     0,     0,     0,    12,    13,
       0,     0,     0,    14,    15,     0,     0,     0,     0,     4,
       0,     0,     0,     0,    16,    17,     5,     6,     7,     0,
       0,     0,     0,     8,     9,    10,     0,    11,    18,     0,
       0,    12,    13,     0,     0,   115,    14,    15,     0,     0,
       0,     0,     4,     0,     0,     0,     0,    16,    17,     5,
       6,     7,     0,   182,     0,     0,     8,     9,    10,     0,
      11,    18,     0,     0,    12,    13,     0,     0,     0,    14,
      15,     0,     0,     0,     0,     4,     0,     0,     0,     0,
      16,    17,     5,     6,     7,     0,     0,   186,     0,     8,
       9,    10,     0,    11,    18,     0,     0,    12,    13,     0,
       0,     0,    14,    15,     0,     0,     0,     0,     4,     0,
       0,     0,     0,    16,    17,     5,     6,     7,     0,     0,
     188,     0,     8,     9,    10,     0,    11,    18,     0,     0,
      12,    13,     0,     0,     0,    14,    15,     0,     0,     0,
       0,     4,     0,     0,     0,     0,    16,    17,     5,     6,
       7,     0,     0,   199,     0,     8,     9,    10,     0,    11,
      18,     0,     0,    12,    13,     0,     0,     0,    14,    15,
       0,     0,     0,     0,     4,     0,     0,     0,     0,    16,
      17,     5,     6,     7,     0,     0,   202,     0,     8,     9,
      10,     0,    11,    18,     0,     0,    12,    13,     0,     0,
       0,    14,    15,     0,     0,     0,     0,     4,     0,     0,
       0,     0,    16,    17,     5,     6,     7,     0,     0,   210,
       0,     8,     9,    10,     0,    11,    18,     0,     0,    12,
      13,     0,     0,     0,    14,    15,     0,     0,     0,     0,
       4,     0,     0,     0,     0,    16,    17,     5,     6,     7,
       0,     0,   212,     0,     8,     9,    10,     0,    11,    18,
       0,     0,    12,    13,     0,     0,     0,    14,    15,     0,
       0,     0,     0,     4,     0,     0,     0,     0,    16,    17,
       5,     6,     7,     0,     0,   216,     0,     8,     9,    10,
       0,    11,    18,     0,     0,    12,    13,     0,     0,     4,
      14,    15,     0,     0,     0,     0,     5,     6,     7,     0,
       0,    16,    17,     8,     9,    10,     0,    11,     0,     0,
       0,    12,    13,     0,     0,    18,    14,    15,    81,    82,
      83,    84,     0,     0,     0,     0,     0,    16,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    88,    89,     0,     0,     0,     0,     0,    94,    95,
       0,     0,     0,     0,     0,    96,    97,    98
  };

  const short int
  parser::yycheck_[] =
  {
       2,    24,    49,    23,     2,     8,    39,    10,     8,    12,
      10,    48,    25,    50,     4,     8,     3,    40,     8,     0,
      10,    11,    12,    18,    19,    20,     9,    60,    60,    60,
       9,    21,    60,    23,    60,     2,    49,    27,    28,     3,
      60,     3,    44,    33,    17,    48,    44,    50,     7,    39,
       3,     3,    54,    12,     8,    60,    54,    62,    61,    61,
      39,    61,    49,    61,   111,    55,    49,    60,    58,     4,
      60,    61,    62,     8,     9,    10,    49,    44,    10,    49,
      12,    60,    60,    57,    60,    49,    21,    54,    23,    46,
      51,    60,    27,    28,     3,    11,    49,    49,    33,     9,
     105,   124,    60,    49,    39,    60,     3,   124,   113,    60,
      60,    60,    56,     4,     6,     7,    48,     8,    50,    10,
      55,    12,    60,    58,    57,    60,    61,    62,    56,    61,
      21,    47,    23,    49,    76,   131,    27,    28,    -1,    -1,
      -1,    -1,    33,    -1,    -1,    37,    -1,    -1,    39,    -1,
      -1,   110,     4,     5,     6,     7,    -1,    -1,   117,    51,
      52,   163,    -1,    -1,    55,   163,    -1,    58,   170,    60,
      61,    62,   170,   132,    -1,   177,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,    37,    38,    -1,   190,    -1,
     192,    -1,   190,    -1,   192,    -1,   163,    -1,    -1,    51,
      52,    -1,   204,   170,    -1,   207,   204,   209,    -1,   207,
     177,   209,   214,   215,    -1,    -1,   214,   215,    -1,    -1,
     179,    -1,   181,   190,    -1,   192,    -1,    -1,     4,     4,
       5,     6,     7,    -1,    10,   194,    -1,   204,    14,    14,
     207,   200,   209,    -1,   203,    -1,    -1,   214,   215,    -1,
      26,    27,   211,    -1,   213,    31,    -1,    -1,    34,    35,
      -1,    -1,    37,    38,    -1,    40,    41,    42,    43,    44,
      45,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    81,    82,    83,    84,    -1,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,    -1,   104,     4,
      37,    38,    -1,     8,    -1,    10,    -1,    44,    45,   115,
     116,    -1,    -1,    -1,    51,    52,    21,    -1,    23,    -1,
      -1,   127,    27,    28,    -1,   131,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    58,    -1,    60,    61,    62,    -1,    -1,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    17,   180,    -1,   182,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    29,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    37,    38,   201,    40,    41,    42,    43,
      44,    45,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,     4,     5,     6,     7,    -1,     9,
      -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
       4,     5,     6,     7,    -1,    -1,    -1,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    -1,    -1,    -1,    -1,
      -1,    51,    52,    53,    54,    55,    56,    57,     4,     5,
       6,     7,    -1,    37,    38,    -1,    -1,    -1,    14,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    37,    38,    -1,    40,    41,    42,    43,    44,    45,
      -1,    -1,    -1,    -1,    -1,    51,    52,    53,    54,    55,
      56,    57,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    -1,    17,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,     5,     6,     7,    29,    -1,    -1,
      -1,    -1,    13,    14,    -1,    37,    38,    -1,    40,    41,
      42,    43,    44,    45,    -1,    -1,    -1,    -1,    29,    51,
      52,    53,    54,    55,    56,    57,    37,    38,    -1,    40,
      41,    42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,
      51,    52,    53,    54,    55,    56,    57,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,
       6,     7,    29,    -1,    -1,    -1,    -1,    -1,    14,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    -1,
      -1,    -1,    -1,    29,    51,    52,    53,    54,    55,    56,
      57,    37,    38,    -1,    40,    41,    42,    43,    44,    45,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,
      32,     4,     5,     6,     7,    37,    38,    -1,    40,    41,
      42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,    51,
      52,    53,    54,    55,    56,    57,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    37,    38,    14,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    51,    52,
      53,    29,    55,    -1,    32,    -1,    -1,    -1,    -1,    37,
      38,    -1,    40,    41,    42,    43,    44,    45,    -1,    -1,
      -1,    -1,    -1,    51,    52,    53,    54,    55,    56,    57,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    17,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     4,     5,     6,     7,    29,    -1,    -1,    -1,    -1,
      -1,    14,    -1,    37,    38,    -1,    40,    41,    42,    43,
      44,    45,     4,     5,     6,     7,    29,    51,    52,    53,
      54,    55,    56,    57,    37,    38,    -1,    40,    41,    42,
      43,    44,    45,    -1,    -1,    -1,    -1,    -1,    51,    52,
      53,    54,    55,    56,    57,    37,    38,    -1,    40,    41,
      42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,    51,
      52,    53,    54,    55,    56,    57,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    17,    -1,    -1,    20,    -1,
      22,    23,    24,    -1,    26,    -1,    -1,    -1,    30,    31,
      -1,    -1,    -1,    35,    36,    -1,    -1,    -1,    -1,     8,
      -1,    -1,    -1,    -1,    46,    47,    15,    16,    17,    -1,
      -1,    -1,    -1,    22,    23,    24,    -1,    26,    60,    -1,
      -1,    30,    31,    -1,    -1,    34,    35,    36,    -1,    -1,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    46,    47,    15,
      16,    17,    -1,    19,    -1,    -1,    22,    23,    24,    -1,
      26,    60,    -1,    -1,    30,    31,    -1,    -1,    -1,    35,
      36,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      46,    47,    15,    16,    17,    -1,    -1,    20,    -1,    22,
      23,    24,    -1,    26,    60,    -1,    -1,    30,    31,    -1,
      -1,    -1,    35,    36,    -1,    -1,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    46,    47,    15,    16,    17,    -1,    -1,
      20,    -1,    22,    23,    24,    -1,    26,    60,    -1,    -1,
      30,    31,    -1,    -1,    -1,    35,    36,    -1,    -1,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    46,    47,    15,    16,
      17,    -1,    -1,    20,    -1,    22,    23,    24,    -1,    26,
      60,    -1,    -1,    30,    31,    -1,    -1,    -1,    35,    36,
      -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    46,
      47,    15,    16,    17,    -1,    -1,    20,    -1,    22,    23,
      24,    -1,    26,    60,    -1,    -1,    30,    31,    -1,    -1,
      -1,    35,    36,    -1,    -1,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    46,    47,    15,    16,    17,    -1,    -1,    20,
      -1,    22,    23,    24,    -1,    26,    60,    -1,    -1,    30,
      31,    -1,    -1,    -1,    35,    36,    -1,    -1,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    46,    47,    15,    16,    17,
      -1,    -1,    20,    -1,    22,    23,    24,    -1,    26,    60,
      -1,    -1,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    46,    47,
      15,    16,    17,    -1,    -1,    20,    -1,    22,    23,    24,
      -1,    26,    60,    -1,    -1,    30,    31,    -1,    -1,     8,
      35,    36,    -1,    -1,    -1,    -1,    15,    16,    17,    -1,
      -1,    46,    47,    22,    23,    24,    -1,    26,    -1,    -1,
      -1,    30,    31,    -1,    -1,    60,    35,    36,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    51,    52,    53
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    64,    65,     0,     8,    15,    16,    17,    22,    23,
      24,    26,    30,    31,    35,    36,    46,    47,    60,    66,
      69,    70,    73,    74,    78,    79,     4,    10,    21,    23,
      27,    28,    33,    39,    55,    58,    61,    62,    74,    77,
      78,    79,    81,    84,    65,    60,    75,    60,    71,    72,
      77,    23,    60,    67,    65,    76,    77,    77,    60,    60,
       3,    49,     8,    12,    48,    50,    61,    80,    84,    77,
      11,    12,    60,    77,    85,    86,     8,    82,    77,    77,
      77,     4,     5,     6,     7,     9,    14,    29,    37,    38,
      40,    41,    42,    43,    44,    45,    51,    52,    53,    54,
      55,    56,    57,    20,     3,    25,    49,    48,    50,    82,
      32,    60,    57,     3,    49,    34,    49,    17,    46,    76,
      74,     9,    76,    77,    60,    60,    77,     3,    11,    47,
      49,    87,     9,    39,    60,    75,    83,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    76,
      60,    60,    60,    65,    82,    60,    76,    60,    77,    77,
      65,     9,    13,    80,    13,    77,    86,    65,    49,     9,
      49,    17,    19,    68,    56,    57,    20,     3,    20,    39,
      65,    77,    65,    77,    18,    19,    20,    60,    77,    20,
      17,    49,    20,    32,    65,    77,    56,    65,    77,    65,
      20,    32,    20,    17,    65,    65,    20
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    63,    64,    65,    65,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    67,    67,    67,    67,    68,
      68,    68,    69,    69,    70,    71,    71,    72,    72,    73,
      73,    74,    74,    74,    75,    75,    76,    76,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    78,    78,    78,    79,    79,    80,    80,    80,
      80,    81,    82,    82,    83,    83,    83,    84,    84,    85,
      85,    86,    86,    86,    87,    87
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     2,     1,     1,     3,     1,     1,
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



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "$end", "error", "$undefined", "\"=\"", "\"-\"", "\"+\"", "\"*\"",
  "\"/\"", "\"(\"", "\")\"", "\"{\"", "\"}\"", "\"[\"", "\"]\"", "\"and\"",
  "\"break\"", "\"continue\"", "\"do\"", "\"else\"", "\"elseif\"",
  "\"end\"", "\"false\"", "\"for\"", "\"function\"", "\"if\"", "\"in\"",
  "\"local\"", "\"nil\"", "\"not\"", "\"or\"", "\"repeat\"", "\"return\"",
  "\"then\"", "\"true\"", "\"until\"", "\"while\"", "\"goto\"", "\"//\"",
  "\"..\"", "\"...\"", "\"==\"", "\">=\"", "\"<=\"", "\"~=\"", "\"<<\"",
  "\">>\"", "\"::\"", "\";\"", "\":\"", "\",\"", "\".\"", "\"^\"", "\"%\"",
  "\"&\"", "\"|\"", "\"~\"", "\">\"", "\"<\"", "\"#\"", "UNARY",
  "\"identifier\"", "\"string\"", "\"number\"", "$accept", "chunk",
  "block", "stmt", "attnamelist", "elseifs", "retstat", "label",
  "funcnamelist", "funcname", "varlist", "var", "namelist", "explist",
  "exp", "prefixexp", "functioncall", "args", "functiondef", "funcbody",
  "parlist", "tableconstructor", "fieldlist", "field", "fieldsep", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,   162,   162,   170,   176,   198,   204,   210,   229,   235,
     241,   247,   253,   261,   267,   286,   305,   336,   361,   387,
     419,   444,   463,   477,   490,   511,   520,   529,   542,   557,
     563,   582,   607,   615,   630,   640,   648,   662,   675,   691,
     704,   723,   732,   752,   769,   777,   791,   804,   823,   831,
     839,   847,   856,   865,   873,   887,   901,   915,   938,   961,
     984,  1007,  1030,  1053,  1076,  1099,  1122,  1145,  1168,  1191,
    1214,  1237,  1260,  1283,  1306,  1329,  1352,  1375,  1398,  1415,
    1432,  1449,  1468,  1482,  1496,  1511,  1530,  1552,  1566,  1574,
    1588,  1602,  1617,  1636,  1651,  1664,  1678,  1688,  1701,  1710,
    1723,  1742,  1762,  1777,  1793,  1799
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG



} // yy
#line 3454 "parser.cpp" // lalr1.cc:1167
#line 1806 "parser.y" // lalr1.cc:1168


void
yy::parser::error (const location_type& l, const std::string& m)
{
    std::stringstream ss;
    ss << l;
    fakelua::ThrowFakeluaException(std::format("{}: {}", ss.str(), m));
}
