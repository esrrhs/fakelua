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

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_label: // label
        value.YY_MOVE_OR_COPY< syntax_tree_interface_ptr > (YY_MOVE (that.value));
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

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_label: // label
        value.move< syntax_tree_interface_ptr > (YY_MOVE (that.value));
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

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_label: // label
        value.copy< syntax_tree_interface_ptr > (that.value);
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

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_label: // label
        value.move< syntax_tree_interface_ptr > (that.value);
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
#line 114 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 367 "parser.cpp"
        break;

      case symbol_kind::S_STRING: // "string"
#line 114 "parser.y"
                 { yyo << yysym.value.template as < std::string > (); }
#line 373 "parser.cpp"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 114 "parser.y"
                 { yyo << yysym.value.template as < double > (); }
#line 379 "parser.cpp"
        break;

      case symbol_kind::S_chunk: // chunk
#line 114 "parser.y"
                 { yyo << yysym.value.template as < syntax_tree_interface_ptr > (); }
#line 385 "parser.cpp"
        break;

      case symbol_kind::S_block: // block
#line 114 "parser.y"
                 { yyo << yysym.value.template as < syntax_tree_interface_ptr > (); }
#line 391 "parser.cpp"
        break;

      case symbol_kind::S_stmt: // stmt
#line 114 "parser.y"
                 { yyo << yysym.value.template as < syntax_tree_interface_ptr > (); }
#line 397 "parser.cpp"
        break;

      case symbol_kind::S_label: // label
#line 114 "parser.y"
                 { yyo << yysym.value.template as < syntax_tree_interface_ptr > (); }
#line 403 "parser.cpp"
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

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_chunk: // chunk
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_label: // label
        yylhs.value.emplace< syntax_tree_interface_ptr > ();
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
#line 121 "parser.y"
        {
  		LOG(INFO) << "[bison]: chunk: " << "block";
  		l->set_chunk(yystack_[0].value.as < syntax_tree_interface_ptr > ());
	}
#line 685 "parser.cpp"
    break;

  case 3: // block: %empty
#line 129 "parser.y"
        {
  	}
#line 692 "parser.cpp"
    break;

  case 4: // block: stmt
#line 133 "parser.y"
        {
		LOG(INFO) << "[bison]: block: " << "stmt";
		auto block = std::make_shared<syntax_tree_block>(yystack_[0].location);
		if (yystack_[0].value.as < syntax_tree_interface_ptr > () != nullptr) {
			block->add_stmt(yystack_[0].value.as < syntax_tree_interface_ptr > ());
		}
  		yylhs.value.as < syntax_tree_interface_ptr > () = block;
  	}
#line 705 "parser.cpp"
    break;

  case 5: // block: block stmt
#line 143 "parser.y"
        {
		LOG(INFO) << "[bison]: block: " << "block stmt";
		auto block = std::dynamic_pointer_cast<syntax_tree_block>(yystack_[1].value.as < syntax_tree_interface_ptr > ());
		if (block == nullptr) {
			LOG(ERROR) << "[bison]: block: " << "block is not a block";
			throw std::runtime_error("block is not a block");
		}
		if (yystack_[0].value.as < syntax_tree_interface_ptr > () != nullptr) {
			block->add_stmt(yystack_[0].value.as < syntax_tree_interface_ptr > ());
		}
  		yylhs.value.as < syntax_tree_interface_ptr > () = block;
	}
#line 722 "parser.cpp"
    break;

  case 6: // stmt: retstat
#line 159 "parser.y"
        {
        	LOG(INFO) << "[bison]: stmt: " << "retstat";
        	yylhs.value.as < syntax_tree_interface_ptr > () = nullptr;
        }
#line 731 "parser.cpp"
    break;

  case 7: // stmt: ";"
#line 165 "parser.y"
        {
		LOG(INFO) << "[bison]: stmt: " << "SEMICOLON";
		yylhs.value.as < syntax_tree_interface_ptr > () = nullptr;
	}
#line 740 "parser.cpp"
    break;

  case 8: // stmt: varlist "=" explist
#line 171 "parser.y"
        {
		LOG(INFO) << "[bison]: stmt: " << "varlist ASSIGN explist";
		yylhs.value.as < syntax_tree_interface_ptr > () = nullptr;
	}
#line 749 "parser.cpp"
    break;

  case 9: // stmt: label
#line 177 "parser.y"
        {
  		LOG(INFO) << "[bison]: stmt: " << "label";
  		yylhs.value.as < syntax_tree_interface_ptr > () = yystack_[0].value.as < syntax_tree_interface_ptr > ();
  	}
#line 758 "parser.cpp"
    break;

  case 10: // retstat: "return"
#line 185 "parser.y"
        {
		LOG(INFO) << "[bison]: retstat: " << "RETURN";
	}
#line 766 "parser.cpp"
    break;

  case 11: // retstat: "return" explist
#line 190 "parser.y"
        {
		LOG(INFO) << "[bison]: retstat: " << "RETURN explist";
	}
#line 774 "parser.cpp"
    break;

  case 12: // label: "::" "identifier" "::"
#line 197 "parser.y"
        {
		LOG(INFO) << "[bison]: bison get label: " << yystack_[1].value.as < std::string > () << " loc: " << yystack_[1].location;
	}
#line 782 "parser.cpp"
    break;

  case 13: // varlist: var
#line 228 "parser.y"
        {
		LOG(INFO) << "[bison]: varlist: " << "var";
	}
#line 790 "parser.cpp"
    break;

  case 14: // varlist: varlist "," var
#line 233 "parser.y"
        {
		LOG(INFO) << "[bison]: varlist: " << "varlist COMMA var";
	}
#line 798 "parser.cpp"
    break;

  case 15: // var: "identifier"
#line 240 "parser.y"
        {
		LOG(INFO) << "[bison]: var: " << "IDENTIFIER";
	}
#line 806 "parser.cpp"
    break;

  case 16: // var: prefixexp "[" exp "]"
#line 245 "parser.y"
        {
		LOG(INFO) << "[bison]: var: " << "prefixexp LSQUARE exp RSQUARE";
	}
#line 814 "parser.cpp"
    break;

  case 17: // var: prefixexp "." "identifier"
#line 250 "parser.y"
        {
		LOG(INFO) << "[bison]: var: " << "prefixexp DOT IDENTIFIER";
	}
#line 822 "parser.cpp"
    break;

  case 18: // namelist: "identifier"
#line 257 "parser.y"
        {
		LOG(INFO) << "[bison]: namelist: " << "IDENTIFIER";
	}
#line 830 "parser.cpp"
    break;

  case 19: // namelist: namelist "," "identifier"
#line 262 "parser.y"
        {
		LOG(INFO) << "[bison]: namelist: " << "namelist COMMA IDENTIFIER";
	}
#line 838 "parser.cpp"
    break;

  case 20: // explist: exp
#line 269 "parser.y"
        {
		LOG(INFO) << "[bison]: explist: " << "exp";
	}
#line 846 "parser.cpp"
    break;

  case 21: // explist: explist "," exp
#line 274 "parser.y"
        {
		LOG(INFO) << "[bison]: explist: " << "explist COMMA exp";
	}
#line 854 "parser.cpp"
    break;

  case 22: // exp: "nil"
#line 281 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "NIL";
	}
#line 862 "parser.cpp"
    break;

  case 23: // exp: "true"
#line 286 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "TRUE";
	}
#line 870 "parser.cpp"
    break;

  case 24: // exp: "false"
#line 291 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "FALSE";
	}
#line 878 "parser.cpp"
    break;

  case 25: // exp: "number"
#line 296 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "NUMBER";
	}
#line 886 "parser.cpp"
    break;

  case 26: // exp: "string"
#line 301 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "STRING";
	}
#line 894 "parser.cpp"
    break;

  case 27: // exp: "..."
#line 306 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "VAR_PARAMS";
	}
#line 902 "parser.cpp"
    break;

  case 28: // exp: functiondef
#line 311 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "functiondef";
	}
#line 910 "parser.cpp"
    break;

  case 29: // exp: prefixexp
#line 316 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "prefixexp";
	}
#line 918 "parser.cpp"
    break;

  case 30: // exp: tableconstructor
#line 321 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "tableconstructor";
	}
#line 926 "parser.cpp"
    break;

  case 31: // exp: exp binop exp
#line 326 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "exp binop exp";
	}
#line 934 "parser.cpp"
    break;

  case 32: // exp: unop exp
#line 331 "parser.y"
        {
		LOG(INFO) << "[bison]: exp: " << "unop exp";
	}
#line 942 "parser.cpp"
    break;

  case 33: // prefixexp: var
#line 338 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "var";
	}
#line 950 "parser.cpp"
    break;

  case 34: // prefixexp: functioncall
#line 343 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "functioncall";
	}
#line 958 "parser.cpp"
    break;

  case 35: // prefixexp: "(" exp ")"
#line 348 "parser.y"
        {
		LOG(INFO) << "[bison]: prefixexp: " << "LPAREN exp RPAREN";
	}
#line 966 "parser.cpp"
    break;

  case 36: // functioncall: prefixexp args
#line 354 "parser.y"
        {
		LOG(INFO) << "[bison]: functioncall: " << "prefixexp args";
	}
#line 974 "parser.cpp"
    break;

  case 37: // functioncall: prefixexp ":" "identifier" args
#line 359 "parser.y"
        {
		LOG(INFO) << "[bison]: functioncall: " << "prefixexp COLON IDENTIFIER args";
	}
#line 982 "parser.cpp"
    break;

  case 38: // args: "(" explist ")"
#line 366 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "LPAREN explist RPAREN";
	}
#line 990 "parser.cpp"
    break;

  case 39: // args: "(" ")"
#line 371 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "LPAREN RPAREN";
	}
#line 998 "parser.cpp"
    break;

  case 40: // args: tableconstructor
#line 376 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "tableconstructor";
	}
#line 1006 "parser.cpp"
    break;

  case 41: // args: "string"
#line 381 "parser.y"
        {
		LOG(INFO) << "[bison]: args: " << "STRING";
	}
#line 1014 "parser.cpp"
    break;

  case 42: // functiondef: "function" funcbody
#line 388 "parser.y"
        {
		LOG(INFO) << "[bison]: functiondef: " << "FUNCTION funcbody";
	}
#line 1022 "parser.cpp"
    break;

  case 43: // funcbody: "(" parlist ")" block "end"
#line 395 "parser.y"
        {
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN parlist RPAREN block END";
	}
#line 1030 "parser.cpp"
    break;

  case 44: // funcbody: "(" ")" block "end"
#line 400 "parser.y"
        {
		LOG(INFO) << "[bison]: funcbody: " << "LPAREN RPAREN block END";
	}
#line 1038 "parser.cpp"
    break;

  case 45: // parlist: namelist
#line 407 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "namelist";
	}
#line 1046 "parser.cpp"
    break;

  case 46: // parlist: namelist "," "..."
#line 412 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "namelist COMMA VAR_PARAMS";
	}
#line 1054 "parser.cpp"
    break;

  case 47: // parlist: "..."
#line 417 "parser.y"
        {
		LOG(INFO) << "[bison]: parlist: " << "VAR_PARAMS";
	}
#line 1062 "parser.cpp"
    break;

  case 48: // tableconstructor: "{" fieldlist "}"
#line 424 "parser.y"
        {
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY fieldlist RCURLY";
	}
#line 1070 "parser.cpp"
    break;

  case 49: // tableconstructor: "{" "}"
#line 429 "parser.y"
        {
		LOG(INFO) << "[bison]: tableconstructor: " << "LCURLY RCURLY";
	}
#line 1078 "parser.cpp"
    break;

  case 50: // fieldlist: field
#line 436 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldlist: " << "field";
	}
#line 1086 "parser.cpp"
    break;

  case 51: // fieldlist: fieldlist fieldsep field
#line 441 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldlist: " << "fieldlist fieldsep field";
	}
#line 1094 "parser.cpp"
    break;

  case 52: // field: "[" exp "]" "=" exp
#line 449 "parser.y"
        {
		LOG(INFO) << "[bison]: field: " << "LSQUARE exp RSQUARE ASSIGN exp";
	}
#line 1102 "parser.cpp"
    break;

  case 53: // field: "identifier" "=" exp
#line 454 "parser.y"
        {
		LOG(INFO) << "[bison]: field: " << "IDENTIFIER ASSIGN exp";
	}
#line 1110 "parser.cpp"
    break;

  case 54: // field: exp
#line 459 "parser.y"
        {
		LOG(INFO) << "[bison]: field: " << "exp";
	}
#line 1118 "parser.cpp"
    break;

  case 55: // fieldsep: ","
#line 467 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldsep: " << "COMMA";
	}
#line 1126 "parser.cpp"
    break;

  case 56: // fieldsep: ";"
#line 472 "parser.y"
        {
		LOG(INFO) << "[bison]: fieldsep: " << "SEMICOLON";
	}
#line 1134 "parser.cpp"
    break;

  case 57: // binop: "+"
#line 479 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "PLUS";
	}
#line 1142 "parser.cpp"
    break;

  case 58: // binop: "-"
#line 484 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MINUS";
	}
#line 1150 "parser.cpp"
    break;

  case 59: // binop: "*"
#line 489 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "STAR";
	}
#line 1158 "parser.cpp"
    break;

  case 60: // binop: "/"
#line 494 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "SLASH";
	}
#line 1166 "parser.cpp"
    break;

  case 61: // binop: "//"
#line 499 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "DOUBLE_SLASH";
	}
#line 1174 "parser.cpp"
    break;

  case 62: // binop: "^"
#line 504 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "XOR";
	}
#line 1182 "parser.cpp"
    break;

  case 63: // binop: "%"
#line 509 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MOD";
	}
#line 1190 "parser.cpp"
    break;

  case 64: // binop: "&"
#line 514 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITAND";
	}
#line 1198 "parser.cpp"
    break;

  case 65: // binop: "~"
#line 519 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITNOT";
	}
#line 1206 "parser.cpp"
    break;

  case 66: // binop: "|"
#line 524 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "BITOR";
	}
#line 1214 "parser.cpp"
    break;

  case 67: // binop: ">>"
#line 529 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "RIGHT_SHIFT";
	}
#line 1222 "parser.cpp"
    break;

  case 68: // binop: "<<"
#line 534 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LEFT_SHIFT";
	}
#line 1230 "parser.cpp"
    break;

  case 69: // binop: ".."
#line 539 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "CONCAT";
	}
#line 1238 "parser.cpp"
    break;

  case 70: // binop: "<"
#line 544 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LESS";
	}
#line 1246 "parser.cpp"
    break;

  case 71: // binop: "<="
#line 549 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "LESS_EQUAL";
	}
#line 1254 "parser.cpp"
    break;

  case 72: // binop: ">"
#line 554 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MORE";
	}
#line 1262 "parser.cpp"
    break;

  case 73: // binop: ">="
#line 559 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "MORE_EQUAL";
	}
#line 1270 "parser.cpp"
    break;

  case 74: // binop: "=="
#line 564 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "EQUAL";
	}
#line 1278 "parser.cpp"
    break;

  case 75: // binop: "~="
#line 569 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "NOT_EQUAL";
	}
#line 1286 "parser.cpp"
    break;

  case 76: // binop: "and"
#line 574 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "AND";
	}
#line 1294 "parser.cpp"
    break;

  case 77: // binop: "or"
#line 579 "parser.y"
        {
		LOG(INFO) << "[bison]: binop: " << "OR";
	}
#line 1302 "parser.cpp"
    break;

  case 78: // unop: "-"
#line 586 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "MINUS";
	}
#line 1310 "parser.cpp"
    break;

  case 79: // unop: "not"
#line 591 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "NOT";
	}
#line 1318 "parser.cpp"
    break;

  case 80: // unop: "#"
#line 596 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "NUMBER_SIGN";
	}
#line 1326 "parser.cpp"
    break;

  case 81: // unop: "~"
#line 601 "parser.y"
        {
		LOG(INFO) << "[bison]: unop: " << "BITNOT";
	}
#line 1334 "parser.cpp"
    break;


#line 1338 "parser.cpp"

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
  "false", "for", "function", "goto", "if", "in", "local", "nil", "not",
  "or", "repeat", "return", "then", "true", "until", "while", "//", "..",
  "...", "==", ">=", "<=", "~=", "<<", ">>", "::", ";", ":", ",", ".", "^",
  "%", "&", "|", "~", ">", "<", "#", "identifier", "string", "number",
  "$accept", "chunk", "block", "stmt", "retstat", "label", "varlist",
  "var", "namelist", "explist", "exp", "prefixexp", "functioncall", "args",
  "functiondef", "funcbody", "parlist", "tableconstructor", "fieldlist",
  "field", "fieldsep", "binop", "unop", YY_NULLPTR
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


  const signed char parser::yypact_ninf_ = -73;

  const signed char parser::yytable_ninf_ = -15;

  const short
  parser::yypact_[] =
  {
      21,   178,   178,   -44,   -73,   -73,    20,    21,   -73,   -73,
     -73,     8,    25,    38,   -73,   -73,     5,   -73,    15,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   213,    38,
     -73,   -73,   178,   -24,   372,   -15,   -73,   -73,   178,    -5,
     112,   178,   -22,    16,   -73,   -73,   -73,   -73,   178,    55,
     372,    78,   -73,    32,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   178,   372,   178,
     -73,   -24,    52,   -73,    12,   266,     2,   -73,   319,   178,
     -73,   -73,   -73,   121,    21,   -73,   -73,    28,    68,   372,
     372,   -73,   -73,   -73,    75,   372,   -73,    23,   -32,    21,
     178,   -73,   -73,   -73,    26,   372,   -73
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,    10,     0,     7,    15,     0,     2,     4,     6,
       9,     0,    33,     0,    34,    78,     0,    24,     0,    22,
      79,    23,    27,    81,    80,    26,    25,    33,     0,    29,
      28,    30,     0,    11,    20,     0,     1,     5,     0,     0,
       0,     0,     0,     0,    41,    36,    40,    49,     0,    15,
      54,     0,    50,     0,    42,    58,    57,    59,    60,    35,
      76,    77,    61,    69,    74,    73,    71,    75,    68,    67,
      62,    63,    64,    66,    65,    72,    70,     0,    32,     0,
      12,     8,    33,    39,     0,     0,     0,    17,     0,     0,
      48,    56,    55,     0,     3,    47,    18,    45,     0,    31,
      21,    38,    16,    37,     0,    53,    51,     0,     0,     3,
       0,    44,    46,    19,     0,    52,    43
  };

  const signed char
  parser::yypgoto_[] =
  {
     -73,   -73,   -72,    -2,   -73,   -73,   -73,     0,   -73,     9,
       3,     1,   -73,    -3,   -73,   -73,   -73,   -11,   -73,    -7,
     -73,   -73,   -73
  };

  const signed char
  parser::yydefgoto_[] =
  {
       0,     6,     7,     8,     9,    10,    11,    27,    97,    33,
      34,    29,    14,    45,    30,    54,    98,    31,    51,    52,
      93,    77,    32
  };

  const signed char
  parser::yytable_[] =
  {
      12,    13,    46,     1,    28,    37,   112,    12,    13,    15,
      40,    38,    16,     1,    35,    16,    47,    48,    46,    50,
      36,   101,   107,    53,    79,    17,   113,    18,   -13,     1,
      80,     1,    19,    20,     1,    78,    86,   114,    21,    82,
      13,    94,   111,    22,    85,   116,    40,    81,    16,    84,
      41,    88,     2,     5,     2,   -14,    39,     2,    89,    23,
      79,    44,    24,    49,    25,    26,     3,     4,     3,     4,
      95,     3,     4,   -13,    87,    46,   108,   109,   110,     5,
      99,     5,   100,   103,     5,    42,   106,    43,     0,    90,
      96,     0,   105,     0,    12,    13,    50,    44,     0,     0,
     -14,     0,     0,     0,     0,    37,     0,    12,    13,    12,
      13,     0,    37,   115,    12,    13,    15,     0,     0,     0,
       1,    83,    16,     0,    91,    15,    92,     0,     0,     1,
       0,    16,    17,    48,    18,     0,     0,     0,     0,    19,
      20,    17,     0,    18,     0,    21,     0,     0,    19,    20,
      22,     0,     0,     0,    21,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,    23,     0,     0,    24,
       5,    25,    26,     0,     0,    23,     0,     0,    24,    49,
      25,    26,    15,     0,     0,     0,     1,     0,    16,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,     0,
      18,     0,     0,     0,     0,    19,    20,     0,     0,     0,
       0,    21,     0,     0,     0,     0,    22,    55,    56,    57,
      58,     0,    59,     0,     0,     0,     0,    60,     0,     0,
       0,     0,    23,     0,     0,    24,     5,    25,    26,     0,
       0,     0,    61,     0,     0,     0,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,    70,    71,    72,    73,    74,    75,    76,
      55,    56,    57,    58,     0,     0,     0,     0,     0,   102,
      60,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    61,     0,     0,     0,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,    70,    71,    72,    73,
      74,    75,    76,    55,    56,    57,    58,     0,     0,     0,
       0,     0,   104,    60,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,     0,     0,     0,     0,     0,    70,
      71,    72,    73,    74,    75,    76,    55,    56,    57,    58,
       0,     0,     0,     0,     0,     0,    60,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    61,     0,     0,     0,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,    70,    71,    72,    73,    74,    75,    76
  };

  const signed char
  parser::yycheck_[] =
  {
       0,     0,    13,     8,     1,     7,    38,     7,     7,     4,
       8,     3,    10,     8,    58,    10,    11,    12,    29,    16,
       0,     9,    94,     8,    48,    20,    58,    22,     3,     8,
      45,     8,    27,    28,     8,    32,    58,   109,    33,    39,
      39,     9,    19,    38,    41,    19,     8,    38,    10,    40,
      12,    48,    31,    58,    31,     3,    48,    31,     3,    54,
      48,    59,    57,    58,    59,    60,    45,    46,    45,    46,
      38,    45,    46,    48,    58,    86,    48,     9,     3,    58,
      77,    58,    79,    86,    58,    47,    93,    49,    -1,    11,
      58,    -1,    89,    -1,    94,    94,    93,    59,    -1,    -1,
      48,    -1,    -1,    -1,    -1,   107,    -1,   107,   107,   109,
     109,    -1,   114,   110,   114,   114,     4,    -1,    -1,    -1,
       8,     9,    10,    -1,    46,     4,    48,    -1,    -1,     8,
      -1,    10,    20,    12,    22,    -1,    -1,    -1,    -1,    27,
      28,    20,    -1,    22,    -1,    33,    -1,    -1,    27,    28,
      38,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    57,
      58,    59,    60,    -1,    -1,    54,    -1,    -1,    57,    58,
      59,    60,     4,    -1,    -1,    -1,     8,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,
      22,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    38,     4,     5,     6,
       7,    -1,     9,    -1,    -1,    -1,    -1,    14,    -1,    -1,
      -1,    -1,    54,    -1,    -1,    57,    58,    59,    60,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    54,    55,    56,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,
      51,    52,    53,    54,    55,    56,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    50,    51,    52,    53,    54,    55,    56
  };

  const signed char
  parser::yystos_[] =
  {
       0,     8,    31,    45,    46,    58,    62,    63,    64,    65,
      66,    67,    68,    72,    73,     4,    10,    20,    22,    27,
      28,    33,    38,    54,    57,    59,    60,    68,    71,    72,
      75,    78,    83,    70,    71,    58,     0,    64,     3,    48,
       8,    12,    47,    49,    59,    74,    78,    11,    12,    58,
      71,    79,    80,     8,    76,     4,     5,     6,     7,     9,
      14,    29,    36,    37,    39,    40,    41,    42,    43,    44,
      50,    51,    52,    53,    54,    55,    56,    82,    71,    48,
      45,    70,    68,     9,    70,    71,    58,    58,    71,     3,
      11,    46,    48,    81,     9,    38,    58,    69,    77,    71,
      71,     9,    13,    74,    13,    71,    80,    63,    48,     9,
       3,    19,    38,    58,    63,    71,    19
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    61,    62,    63,    63,    63,    64,    64,    64,    64,
      65,    65,    66,    67,    67,    68,    68,    68,    69,    69,
      70,    70,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    72,    72,    72,    73,    73,    74,    74,
      74,    74,    75,    76,    76,    77,    77,    77,    78,    78,
      79,    79,    80,    80,    80,    81,    81,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    83,    83,
      83,    83
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     1,     2,     1,     1,     3,     1,
       1,     2,     3,     1,     3,     1,     4,     3,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     2,     1,     1,     3,     2,     4,     3,     2,
       1,     1,     2,     5,     4,     1,     3,     1,     3,     2,
       1,     3,     5,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
  };




#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   120,   120,   128,   132,   142,   158,   164,   170,   176,
     184,   189,   196,   227,   232,   239,   244,   249,   256,   261,
     268,   273,   280,   285,   290,   295,   300,   305,   310,   315,
     320,   325,   330,   337,   342,   347,   353,   358,   365,   370,
     375,   380,   387,   394,   399,   406,   411,   416,   423,   428,
     435,   440,   448,   453,   458,   466,   471,   478,   483,   488,
     493,   498,   503,   508,   513,   518,   523,   528,   533,   538,
     543,   548,   553,   558,   563,   568,   573,   578,   585,   590,
     595,   600
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
#line 2042 "parser.cpp"

#line 605 "parser.y"


void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
