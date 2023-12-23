%require "3.2"
%language "c++"
%header

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
#include "fakelua.h"
#include "util/common.h"
#include "util/exception.h"
#include "compile/syntax_tree.h"

namespace fakelua {
    class myflexer;
}

// https://www.gnu.org/software/bison/manual/html_node/A-Simple-C_002b_002b-Example.html

}

// The parsing context.
%param { fakelua::myflexer* l }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
#include "compile/myflexer.h"

yy::parser::symbol_type yylex(fakelua::myflexer* l) {
    auto ret = l->my_yylex();
    std::stringstream ss;
    ss << ret.location;
    LOG_INFO("[bison]: bison get token: {} loc: {}", ret.name(), ss.str());
    return ret;
}

int yyFlexLexer::yylex() { return -1; }

}

%define api.token.prefix {TOK_}
%token
  ASSIGN        "="
  MINUS         "-"
  PLUS          "+"
  STAR          "*"
  SLASH         "/"
  LPAREN        "("
  RPAREN        ")"
  LCURLY        "{"
  RCURLY        "}"
  LSQUARE       "["
  RSQUARE       "]"
  AND           "and"
  BREAK         "break"
  DO            "do"
  ELSE          "else"
  ELSEIF        "elseif"
  END           "end"
  FALSES        "false"
  FOR           "for"
  FUNCTION      "function"
  IF            "if"
  IN            "in"
  LOCAL         "local"
  NIL           "nil"
  NOT           "not"
  OR            "or"
  REPEAT        "repeat"
  RETURN        "return"
  THEN          "then"
  TRUE          "true"
  UNTIL         "until"
  WHILE         "while"
  GOTO          "goto"
  DOUBLE_SLASH  "//"
  CONCAT        ".."
  VAR_PARAMS    "..."
  EQUAL         "=="
  MORE_EQUAL    ">="
  LESS_EQUAL    "<="
  NOT_EQUAL     "~="
  LEFT_SHIFT    "<<"
  RIGHT_SHIFT   ">>"
  GOTO_TAG      "::"
  SEMICOLON     ";"
  COLON         ":"
  COMMA         ","
  DOT           "."
  XOR           "^"
  MOD           "%"
  BITAND        "&"
  BITOR         "|"
  BITNOT        "~"
  MORE          ">"
  LESS          "<"
  NUMBER_SIGN   "#"
;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> STRING "string"
%token <std::string> NUMBER "number"

%type <fakelua::syntax_tree_interface_ptr> label
%type <fakelua::syntax_tree_interface_ptr> stmt
%type <fakelua::syntax_tree_interface_ptr> block
%type <fakelua::syntax_tree_interface_ptr> chunk
%type <fakelua::syntax_tree_interface_ptr> retstat
%type <fakelua::syntax_tree_interface_ptr> varlist
%type <fakelua::syntax_tree_interface_ptr> explist
%type <fakelua::syntax_tree_interface_ptr> var
%type <fakelua::syntax_tree_interface_ptr> exp
%type <fakelua::syntax_tree_interface_ptr> prefixexp
%type <fakelua::syntax_tree_interface_ptr> functioncall
%type <fakelua::syntax_tree_interface_ptr> args
%type <fakelua::syntax_tree_interface_ptr> tableconstructor
%type <fakelua::syntax_tree_interface_ptr> fieldlist
%type <fakelua::syntax_tree_interface_ptr> field
%type <fakelua::syntax_tree_interface_ptr> elseifs
%type <fakelua::syntax_tree_interface_ptr> namelist
%type <fakelua::syntax_tree_interface_ptr> parlist
%type <fakelua::syntax_tree_interface_ptr> funcbody
%type <fakelua::syntax_tree_interface_ptr> funcname
%type <fakelua::syntax_tree_interface_ptr> funcnamelist
%type <fakelua::syntax_tree_interface_ptr> functiondef
%type <fakelua::syntax_tree_interface_ptr> binop
%type <fakelua::syntax_tree_interface_ptr> unop
%type <fakelua::syntax_tree_interface_ptr> attnamelist

%printer { yyo << $$; } <*>;

%%
%start chunk;

chunk:
    block
    {
    LOG_INFO("[bison]: chunk: block");
    l->set_chunk($1);
    }
    ;

block:
    %empty
    {
        LOG_INFO("[bison]: block: empty");
        $$ = std::make_shared<fakelua::syntax_tree_block>(@0);
    }
    |
    stmt
    {
        LOG_INFO("[bison]: block: stmt");
        auto block = std::make_shared<fakelua::syntax_tree_block>(@1);
        auto stmt = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($1);
        if (stmt == nullptr) {
            LOG_ERROR("[bison]: block: stmt is nullptr");
        }
        block->add_stmt(stmt);
        $$ = block;
    }
    |
    block stmt
    {
        LOG_INFO("[bison]: block: block stmt");
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($1);
        if (block == nullptr) {
            LOG_ERROR("[bison]: block: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        auto stmt = std::dynamic_pointer_cast<fakelua::syntax_tree_interface>($2);
        if (stmt == nullptr) {
            LOG_ERROR("[bison]: block: stmt is not a stmt");
            fakelua::throw_fakelua_exception("stmt is not a stmt");
        }
        block->add_stmt(stmt);
        $$ = block;
    }
    ;

stmt:
    retstat
    {
        LOG_INFO("[bison]: stmt: retstat");
        $$ = $1;
    }
    |
    SEMICOLON
    {
        LOG_INFO("[bison]: stmt: SEMICOLON");
        $$ = std::make_shared<fakelua::syntax_tree_empty>(@1);
    }
    |
    varlist ASSIGN explist
    {
        LOG_INFO("[bison]: stmt: varlist ASSIGN explist");
        auto varlist = std::dynamic_pointer_cast<fakelua::syntax_tree_varlist>($1);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>($3);
        if (varlist == nullptr) {
            LOG_ERROR("[bison]: stmt: varlist is not a varlist");
            fakelua::throw_fakelua_exception("varlist is not a varlist");
        }
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        auto assign = std::make_shared<fakelua::syntax_tree_assign>(@2);
        assign->set_varlist(varlist);
        assign->set_explist(explist);
        $$ = assign;
    }
    |
    functioncall
    {
        LOG_INFO("[bison]: stmt: functioncall");
        $$ = $1;
    }
    |
    label
    {
        LOG_INFO("[bison]: stmt: label");
        $$ = $1;
    }
    |
    BREAK
    {
        LOG_INFO("[bison]: stmt: BREAK");
        $$ = std::make_shared<fakelua::syntax_tree_break>(@1);
    }
    |
    GOTO IDENTIFIER
    {
        LOG_INFO("[bison]: stmt: GOTO IDENTIFIER");
        auto go = std::make_shared<fakelua::syntax_tree_goto>(@2);
        go->set_label($2);
        $$ = go;
    }
    |
    DO block END
    {
        LOG_INFO("[bison]: stmt: DO block END");
        $$ = $2;
    }
    |
    WHILE exp DO block END
    {
        LOG_INFO("[bison]: stmt: WHILE exp DO block END");
        auto while_stmt = std::make_shared<fakelua::syntax_tree_while>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        while_stmt->set_exp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        while_stmt->set_block(block);
        $$ = while_stmt;
    }
    |
    REPEAT block UNTIL exp
    {
        LOG_INFO("[bison]: stmt: REPEAT block UNTIL exp");
        auto repeat = std::make_shared<fakelua::syntax_tree_repeat>(@1);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($2);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        repeat->set_block(block);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($4);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        repeat->set_exp(exp);
        $$ = repeat;
    }
    |
    IF exp THEN block elseifs ELSE block END
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs ELSE block END");
        auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        if_stmt->set_exp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        if_stmt->set_block(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>($5);
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::throw_fakelua_exception("elseiflist is not a elseiflist");
        }
        if_stmt->set_elseiflist(elseifs);
        auto else_block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($7);
        if (else_block == nullptr) {
            LOG_ERROR("[bison]: stmt: else_block is not a block");
            fakelua::throw_fakelua_exception("else_block is not a block");
        }
        if_stmt->set_else_block(else_block);
        $$ = if_stmt;
    }
    |
    IF exp THEN block elseifs END
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs END");
        auto if_stmt = std::make_shared<fakelua::syntax_tree_if>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        if_stmt->set_exp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        if_stmt->set_block(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>($5);
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::throw_fakelua_exception("elseiflist is not a elseiflist");
        }
        if_stmt->set_elseiflist(elseifs);
        $$ = if_stmt;
    }
    |
    FOR IDENTIFIER ASSIGN exp COMMA exp DO block END
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(@1);
        for_loop_stmt->set_name($2);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($4);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        for_loop_stmt->set_exp_begin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($6);
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::throw_fakelua_exception("end_exp is not a exp");
        }
        for_loop_stmt->set_exp_end(end_exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($8);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        for_loop_stmt->set_block(block);
        $$ = for_loop_stmt;
    }
    |
    FOR IDENTIFIER ASSIGN exp COMMA exp COMMA exp DO block END
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::syntax_tree_for_loop>(@1);
        for_loop_stmt->set_name($2);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($4);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        for_loop_stmt->set_exp_begin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($6);
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::throw_fakelua_exception("end_exp is not a exp");
        }
        for_loop_stmt->set_exp_end(end_exp);
        auto step_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($8);
        if (step_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: step_exp is not a exp");
            fakelua::throw_fakelua_exception("step_exp is not a exp");
        }
        for_loop_stmt->set_exp_step(step_exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($10);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        for_loop_stmt->set_block(block);
        $$ = for_loop_stmt;
    }
    |
    FOR namelist IN explist DO block END
    {
        LOG_INFO("[bison]: stmt: for namelist in explist do block end");
        auto for_in_stmt = std::make_shared<fakelua::syntax_tree_for_in>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>($2);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        for_in_stmt->set_namelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>($4);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        for_in_stmt->set_explist(explist);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($6);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        for_in_stmt->set_block(block);
        $$ = for_in_stmt;
    }
    |
    FUNCTION funcname funcbody
    {
        LOG_INFO("[bison]: stmt: function funcname funcbody");
        auto func_stmt = std::make_shared<fakelua::syntax_tree_function>(@1);
        auto funcname = std::dynamic_pointer_cast<fakelua::syntax_tree_funcname>($2);
        if (funcname == nullptr) {
            LOG_ERROR("[bison]: stmt: funcname is not a funcname");
            fakelua::throw_fakelua_exception("funcname is not a funcname");
        }
        func_stmt->set_funcname(funcname);
        auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>($3);
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::throw_fakelua_exception("funcbody is not a funcbody");
        }
        func_stmt->set_funcbody(funcbody);
        $$ = func_stmt;
    }
    |
    LOCAL FUNCTION IDENTIFIER funcbody
    {
        LOG_INFO("[bison]: stmt: local function IDENTIFIER funcbody");
        auto local_func_stmt = std::make_shared<fakelua::syntax_tree_local_function>(@1);
        local_func_stmt->set_name($3);
        auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>($4);
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::throw_fakelua_exception("funcbody is not a funcbody");
        }
        local_func_stmt->set_funcbody(funcbody);
        $$ = local_func_stmt;
    }
    |
    LOCAL attnamelist
    {
        LOG_INFO("[bison]: stmt: local attnamelist");
        auto local_stmt = std::make_shared<fakelua::syntax_tree_local_var>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>($2);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        local_stmt->set_namelist(namelist);
        $$ = local_stmt;
    }
    |
    LOCAL attnamelist ASSIGN explist
    {
        LOG_INFO("[bison]: stmt: local attnamelist assign explist");
        auto local_stmt = std::make_shared<fakelua::syntax_tree_local_var>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>($2);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        local_stmt->set_namelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>($4);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        local_stmt->set_explist(explist);
        $$ = local_stmt;
    }
    ;

attnamelist:
    IDENTIFIER
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(@1);
        namelist->add_name($1);
        namelist->add_attrib("");
        $$ = namelist;
    }
    |
    IDENTIFIER LESS IDENTIFIER MORE
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(@1);
        namelist->add_name($1);
        namelist->add_attrib($3);
        $$ = namelist;
    }
    |
    attnamelist COMMA IDENTIFIER
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        namelist->add_name($3);
        namelist->add_attrib("");
        $$ = namelist;
    }
    |
    attnamelist COMMA IDENTIFIER LESS IDENTIFIER MORE
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        namelist->add_name($3);
        namelist->add_attrib($5);
        $$ = namelist;
    }
    ;

elseifs:
    %empty
    {
        LOG_INFO("[bison]: elseifs: empty");
        $$ = std::make_shared<fakelua::syntax_tree_elseiflist>(@0);
    }
    |
    ELSEIF exp THEN block
    {
        LOG_INFO("[bison]: elseifs: elseif exp then block");
        auto elseifs = std::make_shared<fakelua::syntax_tree_elseiflist>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        elseifs->add_elseif_expr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        elseifs->add_elseif_block(block);
        $$ = elseifs;
    }
    |
    elseifs ELSEIF exp THEN block
    {
        LOG_INFO("[bison]: elseifs: elseifs elseif exp then block");
        auto elseifs = std::dynamic_pointer_cast<fakelua::syntax_tree_elseiflist>($1);
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: elseifs: elseifs is not a elseifs");
            fakelua::throw_fakelua_exception("elseifs is not a elseifs");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($3);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        elseifs->add_elseif_expr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($5);
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        elseifs->add_elseif_block(block);
        $$ = elseifs;
    }
    ;

retstat:
    RETURN
    {
        LOG_INFO("[bison]: retstat: RETURN");
        auto ret = std::make_shared<fakelua::syntax_tree_return>(@1);
        ret->set_explist(nullptr);
        $$ = ret;
    }
    |
    RETURN explist
    {
        LOG_INFO("[bison]: retstat: RETURN explist");
        auto ret = std::make_shared<fakelua::syntax_tree_return>(@1);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>($2);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: retstat: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        ret->set_explist(explist);
        $$ = ret;
    }
    ;

label:
    GOTO_TAG IDENTIFIER GOTO_TAG
    {
    	LOG_INFO("[bison]: label: GOTO_TAG IDENTIFIER GOTO_TAG");
        auto ret = std::make_shared<fakelua::syntax_tree_label>(@2);
        ret->set_name($2);
        $$ = ret;
    }
    ;

funcnamelist:
    IDENTIFIER
    {
        LOG_INFO("[bison]: funcnamelist: IDENTIFIER");
        auto funcnamelist = std::make_shared<fakelua::syntax_tree_funcnamelist>(@1);
        funcnamelist->add_name($1);
        $$ = funcnamelist;
    }
    |
    funcnamelist DOT IDENTIFIER
    {
        LOG_INFO("[bison]: funcnamelist: funcnamelist DOT IDENTIFIER");
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>($1);
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcnamelist: funcnamelist is not a funcnamelist");
            fakelua::throw_fakelua_exception("funcnamelist is not a funcnamelist");
        }
        funcnamelist->add_name($3);
        $$ = funcnamelist;
    }
    ;

funcname:
    funcnamelist
    {
        LOG_INFO("[bison]: funcname: funcnamelist");
        auto funcname = std::make_shared<fakelua::syntax_tree_funcname>(@1);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>($1);
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::throw_fakelua_exception("funcnamelist is not a funcnamelist");
        }
        funcname->set_funcnamelist(funcnamelist);
        $$ = funcname;
    }
    |
    funcnamelist COLON IDENTIFIER
    {
        LOG_INFO("[bison]: funcname: funcnamelist COLON IDENTIFIER");
        auto funcname = std::make_shared<fakelua::syntax_tree_funcname>(@1);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::syntax_tree_funcnamelist>($1);
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::throw_fakelua_exception("funcnamelist is not a funcnamelist");
        }
        funcname->set_funcnamelist(funcnamelist);
        funcname->set_colon_name($3);
        $$ = funcname;
    }
    ;

varlist:
    var
    {
        LOG_INFO("[bison]: varlist: var");
        auto varlist = std::make_shared<fakelua::syntax_tree_varlist>(@1);
        auto var = std::dynamic_pointer_cast<fakelua::syntax_tree_var>($1);
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::throw_fakelua_exception("var is not a var");
        }
        varlist->add_var(var);
        $$ = varlist;
    }
    |
    varlist COMMA var
    {
        LOG_INFO("[bison]: varlist: varlist COMMA var");
        auto varlist = std::dynamic_pointer_cast<fakelua::syntax_tree_varlist>($1);
        if (varlist == nullptr) {
            LOG_ERROR("[bison]: varlist: varlist is not a varlist");
            fakelua::throw_fakelua_exception("varlist is not a varlist");
        }
        auto var = std::dynamic_pointer_cast<fakelua::syntax_tree_var>($3);
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::throw_fakelua_exception("var is not a var");
        }
        varlist->add_var(var);
        $$ = varlist;
    }
    ;

var:
    IDENTIFIER
    {
        LOG_INFO("[bison]: var: IDENTIFIER");
        auto var = std::make_shared<fakelua::syntax_tree_var>(@1);
        var->set_name($1);
        var->set_type("simple");
        $$ = var;
    }
    |
    prefixexp LSQUARE exp RSQUARE
    {
        LOG_INFO("[bison]: var: prefixexp LSQUARE exp RSQUARE");
        auto var = std::make_shared<fakelua::syntax_tree_var>(@2);
        var->set_type("square");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        var->set_prefixexp(prefixexp);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($3);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: var: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        var->set_exp(exp);
        $$ = var;
    }
    |
    prefixexp DOT IDENTIFIER
    {
        LOG_INFO("[bison]: var: prefixexp DOT IDENTIFIER");
        auto var = std::make_shared<fakelua::syntax_tree_var>(@2);
        var->set_type("dot");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        var->set_prefixexp(prefixexp);
        var->set_name($3);
        $$ = var;
    }
    ;

namelist:
    IDENTIFIER
    {
        LOG_INFO("[bison]: namelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::syntax_tree_namelist>(@1);
        namelist->add_name($1);
        $$ = namelist;
    }
    |
    namelist COMMA IDENTIFIER
    {
        LOG_INFO("[bison]: namelist: namelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        namelist->add_name($3);
        $$ = namelist;
    }
    ;

explist:
    exp
    {
        LOG_INFO("[bison]: explist: exp");
        auto explist = std::make_shared<fakelua::syntax_tree_explist>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($1);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        explist->add_exp(exp);
        $$ = explist;
    }
    |
    explist COMMA exp
    {
        LOG_INFO("[bison]: explist: explist COMMA exp");
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>($1);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: explist: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($3);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        explist->add_exp(exp);
        $$ = explist;
    }
    ;

exp:
    NIL
    {
        LOG_INFO("[bison]: exp: NIL");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("nil");
        $$ = exp;
    }
    |
    TRUE
    {
        LOG_INFO("[bison]: exp: TRUE");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("true");
        $$ = exp;
    }
    |
    FALSES
    {
        LOG_INFO("[bison]: exp: FALSES");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("false");
        $$ = exp;
    }
    |
    NUMBER
    {
        LOG_INFO("[bison]: exp: NUMBER");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("number");
        exp->set_value($1);
        $$ = exp;
    }
    |
    STRING
    {
        LOG_INFO("[bison]: exp: STRING");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("string");
        exp->set_value(l->remove_quotes($1));
        $$ = exp;
    }
    |
    VAR_PARAMS
    {
        LOG_INFO("[bison]: exp: VAR_PARAMS");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("var_params");
        $$ = exp;
    }
    |
    functiondef
    {
        LOG_INFO("[bison]: exp: functiondef");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("functiondef");
        auto functiondef = std::dynamic_pointer_cast<fakelua::syntax_tree_functiondef>($1);
        if (functiondef == nullptr) {
            LOG_ERROR("[bison]: exp: functiondef is not a functiondef");
            fakelua::throw_fakelua_exception("functiondef is not a functiondef");
        }
        exp->set_right(functiondef);
        $$ = exp;
    }
    |
    prefixexp
    {
        LOG_INFO("[bison]: exp: prefixexp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("prefixexp");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: exp: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        exp->set_right(prefixexp);
        $$ = exp;
    }
    |
    tableconstructor
    {
        LOG_INFO("[bison]: exp: tableconstructor");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("tableconstructor");
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::syntax_tree_tableconstructor>($1);
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: exp: tableconstructor is not a tableconstructor");
            fakelua::throw_fakelua_exception("tableconstructor is not a tableconstructor");
        }
        exp->set_right(tableconstructor);
        $$ = exp;
    }
    |
    exp binop exp
    {
        LOG_INFO("[bison]: exp: exp binop exp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::throw_fakelua_exception("left_exp is not a exp");
        }
        exp->set_left(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::throw_fakelua_exception("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        auto binop = std::dynamic_pointer_cast<fakelua::syntax_tree_binop>($2);
        if (binop == nullptr) {
            LOG_ERROR("[bison]: exp: binop is not a binop");
            fakelua::throw_fakelua_exception("binop is not a binop");
        }
        exp->set_op(binop);
        $$ = exp;
    }
    |
    unop exp
    {
        LOG_INFO("[bison]: exp: unop exp");
        auto exp = std::make_shared<fakelua::syntax_tree_exp>(@1);
        exp->set_type("unop");
        auto unop = std::dynamic_pointer_cast<fakelua::syntax_tree_unop>($1);
        if (unop == nullptr) {
            LOG_ERROR("[bison]: exp: unop is not a unop");
            fakelua::throw_fakelua_exception("unop is not a unop");
        }
        exp->set_op(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($2);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::throw_fakelua_exception("right_exp is not a exp");
        }
        exp->set_right(right_exp);
        $$ = exp;
    }
    ;

prefixexp:
    var
    {
        LOG_INFO("[bison]: prefixexp: var");
        auto prefixexp = std::make_shared<fakelua::syntax_tree_prefixexp>(@1);
        prefixexp->set_type("var");
        auto var = std::dynamic_pointer_cast<fakelua::syntax_tree_var>($1);
        if (var == nullptr) {
            LOG_ERROR("[bison]: prefixexp: var is not a var");
            fakelua::throw_fakelua_exception("var is not a var");
        }
        prefixexp->set_value(var);
        $$ = prefixexp;
    }
    |
    functioncall
    {
        LOG_INFO("[bison]: prefixexp: functioncall");
        auto prefixexp = std::make_shared<fakelua::syntax_tree_prefixexp>(@1);
        prefixexp->set_type("functioncall");
        auto functioncall = std::dynamic_pointer_cast<fakelua::syntax_tree_functioncall>($1);
        if (functioncall == nullptr) {
            LOG_ERROR("[bison]: prefixexp: functioncall is not a functioncall");
            fakelua::throw_fakelua_exception("functioncall is not a functioncall");
        }
        prefixexp->set_value(functioncall);
        $$ = prefixexp;
    }
    |
    LPAREN exp RPAREN
    {
        LOG_INFO("[bison]: prefixexp: LPAREN exp RPAREN");
        auto prefixexp = std::make_shared<fakelua::syntax_tree_prefixexp>(@1);
        prefixexp->set_type("exp");
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: prefixexp: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        prefixexp->set_value(exp);
        $$ = prefixexp;
    }

functioncall:
    prefixexp args
    {
        LOG_INFO("[bison]: functioncall: prefixexp args");
        auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(@1);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        functioncall->set_prefixexp(prefixexp);
        auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_args>($2);
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::throw_fakelua_exception("args is not a args");
        }
        functioncall->set_args(args);
        $$ = functioncall;
    }
    |
    prefixexp COLON IDENTIFIER args
    {
        LOG_INFO("[bison]: functioncall: prefixexp COLON IDENTIFIER args");
        auto functioncall = std::make_shared<fakelua::syntax_tree_functioncall>(@1);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::syntax_tree_prefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::throw_fakelua_exception("prefixexp is not a prefixexp");
        }
        functioncall->set_prefixexp(prefixexp);
        functioncall->set_name($3);
        auto args = std::dynamic_pointer_cast<fakelua::syntax_tree_args>($4);
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::throw_fakelua_exception("args is not a args");
        }
        functioncall->set_args(args);
        $$ = functioncall;
    }
    ;

args:
    LPAREN explist RPAREN
    {
        LOG_INFO("[bison]: args: LPAREN explist RPAREN");
        auto args = std::make_shared<fakelua::syntax_tree_args>(@1);
        auto explist = std::dynamic_pointer_cast<fakelua::syntax_tree_explist>($2);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: args: explist is not a explist");
            fakelua::throw_fakelua_exception("explist is not a explist");
        }
        args->set_explist(explist);
        args->set_type("explist");
        $$ = args;
    }
    |
    LPAREN RPAREN
    {
        LOG_INFO("[bison]: args: LPAREN RPAREN");
        auto args = std::make_shared<fakelua::syntax_tree_args>(@1);
        args->set_type("empty");
        $$ = args;
    }
    |
    tableconstructor
    {
        LOG_INFO("[bison]: args: tableconstructor");
        auto args = std::make_shared<fakelua::syntax_tree_args>(@1);
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::syntax_tree_tableconstructor>($1);
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: args: tableconstructor is not a tableconstructor");
            fakelua::throw_fakelua_exception("tableconstructor is not a tableconstructor");
        }
        args->set_tableconstructor(tableconstructor);
        args->set_type("tableconstructor");
        $$ = args;
    }
    |
    STRING
    {
        LOG_INFO("[bison]: args: STRING");
        auto args = std::make_shared<fakelua::syntax_tree_args>(@1);
        args->set_string(l->remove_quotes($1));
        args->set_type("string");
        $$ = args;
    }
    ;

functiondef:
    FUNCTION funcbody
    {
        LOG_INFO("[bison]: functiondef: FUNCTION funcbody");
        auto functiondef = std::make_shared<fakelua::syntax_tree_functiondef>(@1);
        auto funcbody = std::dynamic_pointer_cast<fakelua::syntax_tree_funcbody>($2);
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: functiondef: funcbody is not a funcbody");
            fakelua::throw_fakelua_exception("funcbody is not a funcbody");
        }
        functiondef->set_funcbody(funcbody);
        $$ = functiondef;
    }
    ;

funcbody:
    LPAREN parlist RPAREN block END
    {
        LOG_INFO("[bison]: funcbody: LPAREN parlist RPAREN block END");
        auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(@1);
        auto parlist = std::dynamic_pointer_cast<fakelua::syntax_tree_parlist>($2);
        if (parlist == nullptr) {
            LOG_ERROR("[bison]: funcbody: parlist is not a parlist");
            fakelua::throw_fakelua_exception("parlist is not a parlist");
        }
        funcbody->set_parlist(parlist);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        funcbody->set_block(block);
        $$ = funcbody;
    }
    |
    LPAREN RPAREN block END
    {
        LOG_INFO("[bison]: funcbody: LPAREN RPAREN block END");
        auto funcbody = std::make_shared<fakelua::syntax_tree_funcbody>(@1);
        auto block = std::dynamic_pointer_cast<fakelua::syntax_tree_block>($3);
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::throw_fakelua_exception("block is not a block");
        }
        funcbody->set_block(block);
        $$ = funcbody;
    }
    ;

parlist:
    namelist
    {
        LOG_INFO("[bison]: parlist: namelist");
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        parlist->set_namelist(namelist);
        $$ = parlist;
    }
    |
    namelist COMMA VAR_PARAMS
    {
        LOG_INFO("[bison]: parlist: namelist COMMA VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::syntax_tree_namelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::throw_fakelua_exception("namelist is not a namelist");
        }
        parlist->set_namelist(namelist);
        parlist->set_var_params(true);
        $$ = parlist;
    }
    |
    VAR_PARAMS
    {
        LOG_INFO("[bison]: parlist: VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::syntax_tree_parlist>(@1);
        parlist->set_var_params(true);
        $$ = parlist;
    }
    ;

tableconstructor:
    LCURLY fieldlist RCURLY
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY fieldlist RCURLY");
        auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(@1);
        auto fieldlist = std::dynamic_pointer_cast<fakelua::syntax_tree_fieldlist>($2);
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: tableconstructor: fieldlist is not a fieldlist");
            fakelua::throw_fakelua_exception("fieldlist is not a fieldlist");
        }
        tableconstructor->set_fieldlist(fieldlist);
        $$ = tableconstructor;
    }
    |
    LCURLY RCURLY
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY RCURLY");
        auto tableconstructor = std::make_shared<fakelua::syntax_tree_tableconstructor>(@1);
        $$ = tableconstructor;
    }
    ;

fieldlist:
    field
    {
        LOG_INFO("[bison]: fieldlist: field");
        auto fieldlist = std::make_shared<fakelua::syntax_tree_fieldlist>(@1);
        auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_field>($1);
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::throw_fakelua_exception("field is not a field");
        }
        fieldlist->add_field(field);
        $$ = fieldlist;
    }
    |
    fieldlist fieldsep field
    {
        LOG_INFO("[bison]: fieldlist: fieldlist fieldsep field");
        auto fieldlist = std::dynamic_pointer_cast<fakelua::syntax_tree_fieldlist>($1);
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: fieldlist: fieldlist is not a fieldlist");
            fakelua::throw_fakelua_exception("fieldlist is not a fieldlist");
        }
        auto field = std::dynamic_pointer_cast<fakelua::syntax_tree_field>($3);
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::throw_fakelua_exception("field is not a field");
        }
        fieldlist->add_field(field);
        $$ = fieldlist;
    }
    ;

field:
    LSQUARE exp RSQUARE ASSIGN exp
    {
        LOG_INFO("[bison]: field: LSQUARE exp RSQUARE ASSIGN exp");
        auto field = std::make_shared<fakelua::syntax_tree_field>(@1);
        auto key = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($2);
        if (key == nullptr) {
            LOG_ERROR("[bison]: key: key is not a exp");
            fakelua::throw_fakelua_exception("key is not a exp");
        }
        field->set_key(key);
        auto value = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($5);
        if (value == nullptr) {
            LOG_ERROR("[bison]: field: value is not a exp");
            fakelua::throw_fakelua_exception("value is not a exp");
        }
        field->set_value(value);
        field->set_type("array");
        $$ = field;
    }
    |
    IDENTIFIER ASSIGN exp
    {
        LOG_INFO("[bison]: field: IDENTIFIER ASSIGN exp");
        auto field = std::make_shared<fakelua::syntax_tree_field>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($3);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        field->set_name($1);
        field->set_value(exp);
        field->set_type("object");
        $$ = field;
    }
    |
    exp
    {
        LOG_INFO("[bison]: field: exp");
        auto field = std::make_shared<fakelua::syntax_tree_field>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::syntax_tree_exp>($1);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::throw_fakelua_exception("exp is not a exp");
        }
        field->set_value(exp);
        field->set_type("array");
        $$ = field;
    }
    ;

fieldsep:
    COMMA
    {
        LOG_INFO("[bison]: fieldsep: COMMA");
        // nothing to do
    }
    |
    SEMICOLON
    {
        LOG_INFO("[bison]: fieldsep: SEMICOLON");
        // nothing to do
    }
    ;

binop:
    PLUS
    {
        LOG_INFO("[bison]: binop: PLUS");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("PLUS");
        $$ = binop;
    }
    |
    MINUS
    {
        LOG_INFO("[bison]: binop: MINUS");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("MINUS");
        $$ = binop;
    }
    |
    STAR
    {
        LOG_INFO("[bison]: binop: STAR");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("STAR");
        $$ = binop;
    }
    |
    SLASH
    {
        LOG_INFO("[bison]: binop: SLASH");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("SLASH");
        $$ = binop;
    }
    |
    DOUBLE_SLASH
    {
        LOG_INFO("[bison]: binop: DOUBLE_SLASH");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("DOUBLE_SLASH");
        $$ = binop;
    }
    |
    XOR
    {
        LOG_INFO("[bison]: binop: XOR");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("XOR");
        $$ = binop;
    }
    |
    MOD
    {
        LOG_INFO("[bison]: binop: MOD");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("MOD");
        $$ = binop;
    }
    |
    BITAND
    {
        LOG_INFO("[bison]: binop: BITAND");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("BITAND");
        $$ = binop;
    }
    |
    BITNOT
    {
        LOG_INFO("[bison]: binop: BITNOT");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("BITNOT");
        $$ = binop;
    }
    |
    BITOR
    {
        LOG_INFO("[bison]: binop: BITOR");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("BITOR");
        $$ = binop;
    }
    |
    RIGHT_SHIFT
    {
        LOG_INFO("[bison]: binop: RIGHT_SHIFT");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("RIGHT_SHIFT");
        $$ = binop;
    }
    |
    LEFT_SHIFT
    {
        LOG_INFO("[bison]: binop: LEFT_SHIFT");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("LEFT_SHIFT");
        $$ = binop;
    }
    |
    CONCAT
    {
        LOG_INFO("[bison]: binop: CONCAT");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("CONCAT");
        $$ = binop;
    }
    |
    LESS
    {
        LOG_INFO("[bison]: binop: LESS");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("LESS");
        $$ = binop;
    }
    |
    LESS_EQUAL
    {
        LOG_INFO("[bison]: binop: LESS_EQUAL");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("LESS_EQUAL");
        $$ = binop;
    }
    |
    MORE
    {
        LOG_INFO("[bison]: binop: MORE");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("MORE");
        $$ = binop;
    }
    |
    MORE_EQUAL
    {
        LOG_INFO("[bison]: binop: MORE_EQUAL");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("MORE_EQUAL");
        $$ = binop;
    }
    |
    EQUAL
    {
        LOG_INFO("[bison]: binop: EQUAL");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("EQUAL");
        $$ = binop;
    }
    |
    NOT_EQUAL
    {
        LOG_INFO("[bison]: binop: NOT_EQUAL");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("NOT_EQUAL");
        $$ = binop;
    }
    |
    AND
    {
        LOG_INFO("[bison]: binop: AND");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("AND");
        $$ = binop;
    }
    |
    OR
    {
        LOG_INFO("[bison]: binop: OR");
        auto binop = std::make_shared<fakelua::syntax_tree_binop>(@1);
        binop->set_op("OR");
        $$ = binop;
    }
    ;

unop:
    MINUS
    {
        LOG_INFO("[bison]: unop: MINUS");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(@1);
        unop->set_op("MINUS");
        $$ = unop;
    }
    |
    NOT
    {
        LOG_INFO("[bison]: unop: NOT");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(@1);
        unop->set_op("NOT");
        $$ = unop;
    }
    |
    NUMBER_SIGN
    {
        LOG_INFO("[bison]: unop: NUMBER_SIGN");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(@1);
        unop->set_op("NUMBER_SIGN");
        $$ = unop;
    }
    |
    BITNOT
    {
        LOG_INFO("[bison]: unop: BITNOT");
        auto unop = std::make_shared<fakelua::syntax_tree_unop>(@1);
        unop->set_op("BITNOT");
        $$ = unop;
    }
    ;
%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
    std::stringstream ss;
    ss << l;
    fakelua::throw_fakelua_exception(std::format("{}: {}", ss.str(), m));
}
