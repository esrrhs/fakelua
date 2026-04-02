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
    class MyFlexer;
}

// https://www.gnu.org/software/bison/manual/html_node/A-Simple-C_002b_002b-Example.html

}

// The parsing context.
%param { fakelua::MyFlexer* l }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
#include "compile/my_flexer.h"

yy::parser::symbol_type yylex(fakelua::MyFlexer* l) {
    auto ret = l->MyYylex();
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
  POW           "^"
  MOD           "%"
  BITAND        "&"
  BITOR         "|"
  BITNOT        "~"
  MORE          ">"
  LESS          "<"
  NUMBER_SIGN   "#"
;

%left "or"
%left "and"
%left "<"     ">"     "<="    ">="    "~="    "=="
%left "|"
%left "~"
%left "&"
%left "<<"    ">>"
%right ".."
%left "+"     "-"
%left "*"     "/"     "//"    "%"
%precedence UNARY
%right "^"

%token <std::string> IDENTIFIER "identifier"
%token <std::string> STRING "string"
%token <std::string> NUMBER "number"

%type <fakelua::SyntaxTreeInterfacePtr> label
%type <fakelua::SyntaxTreeInterfacePtr> stmt
%type <fakelua::SyntaxTreeInterfacePtr> block
%type <fakelua::SyntaxTreeInterfacePtr> chunk
%type <fakelua::SyntaxTreeInterfacePtr> retstat
%type <fakelua::SyntaxTreeInterfacePtr> varlist
%type <fakelua::SyntaxTreeInterfacePtr> explist
%type <fakelua::SyntaxTreeInterfacePtr> var
%type <fakelua::SyntaxTreeInterfacePtr> exp
%type <fakelua::SyntaxTreeInterfacePtr> prefixexp
%type <fakelua::SyntaxTreeInterfacePtr> functioncall
%type <fakelua::SyntaxTreeInterfacePtr> args
%type <fakelua::SyntaxTreeInterfacePtr> tableconstructor
%type <fakelua::SyntaxTreeInterfacePtr> fieldlist
%type <fakelua::SyntaxTreeInterfacePtr> field
%type <fakelua::SyntaxTreeInterfacePtr> elseifs
%type <fakelua::SyntaxTreeInterfacePtr> namelist
%type <fakelua::SyntaxTreeInterfacePtr> parlist
%type <fakelua::SyntaxTreeInterfacePtr> funcbody
%type <fakelua::SyntaxTreeInterfacePtr> funcname
%type <fakelua::SyntaxTreeInterfacePtr> funcnamelist
%type <fakelua::SyntaxTreeInterfacePtr> functiondef
%type <fakelua::SyntaxTreeInterfacePtr> attnamelist

%printer { yyo << $$; } <*>;

%%
%start chunk;

chunk:
    block
    {
    LOG_INFO("[bison]: chunk: block");
    l->SetChunk($1);
    }
    ;

block:
    %empty
    {
        LOG_INFO("[bison]: block: empty");
        $$ = std::make_shared<fakelua::SyntaxTreeBlock>(@0);
    }
    |
    stmt
    {
        LOG_INFO("[bison]: block: stmt");
        auto block = std::make_shared<fakelua::SyntaxTreeBlock>(@1);
        auto stmt = std::dynamic_pointer_cast<fakelua::SyntaxTreeInterface>($1);
        if (stmt == nullptr) {
            LOG_ERROR("[bison]: block: stmt is nullptr");
        }
        block->AddStmt(stmt);
        $$ = block;
    }
    |
    block stmt
    {
        LOG_INFO("[bison]: block: block stmt");
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($1);
        if (block == nullptr) {
            LOG_ERROR("[bison]: block: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        auto stmt = std::dynamic_pointer_cast<fakelua::SyntaxTreeInterface>($2);
        if (stmt == nullptr) {
            LOG_ERROR("[bison]: block: stmt is not a stmt");
            fakelua::ThrowFakeluaException("stmt is not a stmt");
        }
        block->AddStmt(stmt);
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
        $$ = std::make_shared<fakelua::SyntaxTreeEmpty>(@1);
    }
    |
    varlist ASSIGN explist
    {
        LOG_INFO("[bison]: stmt: varlist ASSIGN explist");
        auto varlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeVarlist>($1);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>($3);
        if (varlist == nullptr) {
            LOG_ERROR("[bison]: stmt: varlist is not a varlist");
            fakelua::ThrowFakeluaException("varlist is not a varlist");
        }
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        auto assign = std::make_shared<fakelua::SyntaxTreeAssign>(@2);
        assign->SetVarlist(varlist);
        assign->SetExplist(explist);
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
        $$ = std::make_shared<fakelua::SyntaxTreeBreak>(@1);
    }
    |
    GOTO IDENTIFIER
    {
        LOG_INFO("[bison]: stmt: GOTO IDENTIFIER");
        auto go = std::make_shared<fakelua::SyntaxTreeGoto>(@2);
        go->SetLabel($2);
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
        auto while_stmt = std::make_shared<fakelua::SyntaxTreeWhile>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        while_stmt->SetExp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        while_stmt->SetBlock(block);
        $$ = while_stmt;
    }
    |
    REPEAT block UNTIL exp
    {
        LOG_INFO("[bison]: stmt: REPEAT block UNTIL exp");
        auto repeat = std::make_shared<fakelua::SyntaxTreeRepeat>(@1);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($2);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        repeat->SetBlock(block);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($4);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        repeat->SetExp(exp);
        $$ = repeat;
    }
    |
    IF exp THEN block elseifs ELSE block END
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs ELSE block END");
        auto if_stmt = std::make_shared<fakelua::SyntaxTreeIf>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        if_stmt->SetExp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        if_stmt->SetBlock(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>($5);
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::ThrowFakeluaException("elseiflist is not a elseiflist");
        }
        if_stmt->SetElseiflist(elseifs);
        auto else_block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($7);
        if (else_block == nullptr) {
            LOG_ERROR("[bison]: stmt: else_block is not a block");
            fakelua::ThrowFakeluaException("else_block is not a block");
        }
        if_stmt->SetElseBlock(else_block);
        $$ = if_stmt;
    }
    |
    IF exp THEN block elseifs END
    {
        LOG_INFO("[bison]: stmt: IF exp THEN block elseifs END");
        auto if_stmt = std::make_shared<fakelua::SyntaxTreeIf>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        if_stmt->SetExp(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        if_stmt->SetBlock(block);
        auto elseifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>($5);
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: stmt: elseiflist is not a elseiflist");
            fakelua::ThrowFakeluaException("elseiflist is not a elseiflist");
        }
        if_stmt->SetElseiflist(elseifs);
        $$ = if_stmt;
    }
    |
    FOR IDENTIFIER ASSIGN exp COMMA exp DO block END
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::SyntaxTreeForLoop>(@1);
        for_loop_stmt->SetName($2);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($4);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        for_loop_stmt->SetExpBegin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($6);
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::ThrowFakeluaException("end_exp is not a exp");
        }
        for_loop_stmt->SetExpEnd(end_exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($8);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        for_loop_stmt->SetBlock(block);
        $$ = for_loop_stmt;
    }
    |
    FOR IDENTIFIER ASSIGN exp COMMA exp COMMA exp DO block END
    {
        LOG_INFO("[bison]: stmt: for IDENTIFIER assign exp COMMA exp COMMA exp do block end");
        auto for_loop_stmt = std::make_shared<fakelua::SyntaxTreeForLoop>(@1);
        for_loop_stmt->SetName($2);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($4);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: stmt: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        for_loop_stmt->SetExpBegin(exp);
        auto end_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($6);
        if (end_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: end_exp is not a exp");
            fakelua::ThrowFakeluaException("end_exp is not a exp");
        }
        for_loop_stmt->SetExpEnd(end_exp);
        auto step_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($8);
        if (step_exp == nullptr) {
            LOG_ERROR("[bison]: stmt: step_exp is not a exp");
            fakelua::ThrowFakeluaException("step_exp is not a exp");
        }
        for_loop_stmt->SetExpStep(step_exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($10);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        for_loop_stmt->SetBlock(block);
        $$ = for_loop_stmt;
    }
    |
    FOR namelist IN explist DO block END
    {
        LOG_INFO("[bison]: stmt: for namelist in explist do block end");
        auto for_in_stmt = std::make_shared<fakelua::SyntaxTreeForIn>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>($2);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        for_in_stmt->SetNamelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>($4);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        for_in_stmt->SetExplist(explist);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($6);
        if (block == nullptr) {
            LOG_ERROR("[bison]: stmt: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        for_in_stmt->SetBlock(block);
        $$ = for_in_stmt;
    }
    |
    FUNCTION funcname funcbody
    {
        LOG_INFO("[bison]: stmt: function funcname funcbody");
        auto func_stmt = std::make_shared<fakelua::SyntaxTreeFunction>(@1);
        auto funcname = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncname>($2);
        if (funcname == nullptr) {
            LOG_ERROR("[bison]: stmt: funcname is not a funcname");
            fakelua::ThrowFakeluaException("funcname is not a funcname");
        }
        func_stmt->SetFuncname(funcname);
        auto funcbody = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>($3);
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::ThrowFakeluaException("funcbody is not a funcbody");
        }
        func_stmt->SetFuncbody(funcbody);
        $$ = func_stmt;
    }
    |
    LOCAL FUNCTION IDENTIFIER funcbody
    {
        LOG_INFO("[bison]: stmt: local function IDENTIFIER funcbody");
        auto local_func_stmt = std::make_shared<fakelua::SyntaxTreeLocalFunction>(@1);
        local_func_stmt->SetName($3);
        auto funcbody = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>($4);
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: stmt: funcbody is not a funcbody");
            fakelua::ThrowFakeluaException("funcbody is not a funcbody");
        }
        local_func_stmt->SetFuncbody(funcbody);
        $$ = local_func_stmt;
    }
    |
    LOCAL attnamelist
    {
        LOG_INFO("[bison]: stmt: local attnamelist");
        auto local_stmt = std::make_shared<fakelua::SyntaxTreeLocalVar>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>($2);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        local_stmt->SetNamelist(namelist);
        $$ = local_stmt;
    }
    |
    LOCAL attnamelist ASSIGN explist
    {
        LOG_INFO("[bison]: stmt: local attnamelist assign explist");
        auto local_stmt = std::make_shared<fakelua::SyntaxTreeLocalVar>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>($2);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: stmt: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        local_stmt->SetNamelist(namelist);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>($4);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: stmt: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        local_stmt->SetExplist(explist);
        $$ = local_stmt;
    }
    ;

attnamelist:
    IDENTIFIER
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::SyntaxTreeNamelist>(@1);
        namelist->AddName($1);
        namelist->AddAttrib("");
        $$ = namelist;
    }
    |
    IDENTIFIER LESS IDENTIFIER MORE
    {
        LOG_INFO("[bison]: attnamelist: IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::make_shared<fakelua::SyntaxTreeNamelist>(@1);
        namelist->AddName($1);
        namelist->AddAttrib($3);
        $$ = namelist;
    }
    |
    attnamelist COMMA IDENTIFIER
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        namelist->AddName($3);
        namelist->AddAttrib("");
        $$ = namelist;
    }
    |
    attnamelist COMMA IDENTIFIER LESS IDENTIFIER MORE
    {
        LOG_INFO("[bison]: attnamelist: attnamelist COMMA IDENTIFIER LESS IDENTIFIER MORE");
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        namelist->AddName($3);
        namelist->AddAttrib($5);
        $$ = namelist;
    }
    ;

elseifs:
    %empty
    {
        LOG_INFO("[bison]: elseifs: empty");
        $$ = std::make_shared<fakelua::SyntaxTreeElseiflist>(@0);
    }
    |
    ELSEIF exp THEN block
    {
        LOG_INFO("[bison]: elseifs: elseif exp then block");
        auto elseifs = std::make_shared<fakelua::SyntaxTreeElseiflist>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        elseifs->AddElseifExpr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        elseifs->AddElseifBlock(block);
        $$ = elseifs;
    }
    |
    elseifs ELSEIF exp THEN block
    {
        LOG_INFO("[bison]: elseifs: elseifs elseif exp then block");
        auto elseifs = std::dynamic_pointer_cast<fakelua::SyntaxTreeElseiflist>($1);
        if (elseifs == nullptr) {
            LOG_ERROR("[bison]: elseifs: elseifs is not a elseifs");
            fakelua::ThrowFakeluaException("elseifs is not a elseifs");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: elseifs: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        elseifs->AddElseifExpr(exp);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($5);
        if (block == nullptr) {
            LOG_ERROR("[bison]: elseifs: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        elseifs->AddElseifBlock(block);
        $$ = elseifs;
    }
    ;

retstat:
    RETURN
    {
        LOG_INFO("[bison]: retstat: RETURN");
        auto ret = std::make_shared<fakelua::SyntaxTreeReturn>(@1);
        ret->SetExplist(nullptr);
        $$ = ret;
    }
    |
    RETURN explist
    {
        LOG_INFO("[bison]: retstat: RETURN explist");
        auto ret = std::make_shared<fakelua::SyntaxTreeReturn>(@1);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>($2);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: retstat: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        ret->SetExplist(explist);
        $$ = ret;
    }
    ;

label:
    GOTO_TAG IDENTIFIER GOTO_TAG
    {
            LOG_INFO("[bison]: label: GOTO_TAG IDENTIFIER GOTO_TAG");
        auto ret = std::make_shared<fakelua::SyntaxTreeLabel>(@2);
        ret->SetName($2);
        $$ = ret;
    }
    ;

funcnamelist:
    IDENTIFIER
    {
        LOG_INFO("[bison]: funcnamelist: IDENTIFIER");
        auto funcnamelist = std::make_shared<fakelua::SyntaxTreeFuncnamelist>(@1);
        funcnamelist->AddName($1);
        $$ = funcnamelist;
    }
    |
    funcnamelist DOT IDENTIFIER
    {
        LOG_INFO("[bison]: funcnamelist: funcnamelist DOT IDENTIFIER");
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncnamelist>($1);
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcnamelist: funcnamelist is not a funcnamelist");
            fakelua::ThrowFakeluaException("funcnamelist is not a funcnamelist");
        }
        funcnamelist->AddName($3);
        $$ = funcnamelist;
    }
    ;

funcname:
    funcnamelist
    {
        LOG_INFO("[bison]: funcname: funcnamelist");
        auto funcname = std::make_shared<fakelua::SyntaxTreeFuncname>(@1);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncnamelist>($1);
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::ThrowFakeluaException("funcnamelist is not a funcnamelist");
        }
        funcname->SetFuncNameList(funcnamelist);
        $$ = funcname;
    }
    |
    funcnamelist COLON IDENTIFIER
    {
        LOG_INFO("[bison]: funcname: funcnamelist COLON IDENTIFIER");
        auto funcname = std::make_shared<fakelua::SyntaxTreeFuncname>(@1);
        auto funcnamelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncnamelist>($1);
        if (funcnamelist == nullptr) {
            LOG_ERROR("[bison]: funcname: funcnamelist is not a funcnamelist");
            fakelua::ThrowFakeluaException("funcnamelist is not a funcnamelist");
        }
        funcname->SetFuncNameList(funcnamelist);
        funcname->SetColonName($3);
        $$ = funcname;
    }
    ;

varlist:
    var
    {
        LOG_INFO("[bison]: varlist: var");
        auto varlist = std::make_shared<fakelua::SyntaxTreeVarlist>(@1);
        auto var = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>($1);
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::ThrowFakeluaException("var is not a var");
        }
        varlist->AddVar(var);
        $$ = varlist;
    }
    |
    varlist COMMA var
    {
        LOG_INFO("[bison]: varlist: varlist COMMA var");
        auto varlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeVarlist>($1);
        if (varlist == nullptr) {
            LOG_ERROR("[bison]: varlist: varlist is not a varlist");
            fakelua::ThrowFakeluaException("varlist is not a varlist");
        }
        auto var = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>($3);
        if (var == nullptr) {
            LOG_ERROR("[bison]: varlist: var is not a var");
            fakelua::ThrowFakeluaException("var is not a var");
        }
        varlist->AddVar(var);
        $$ = varlist;
    }
    ;

var:
    IDENTIFIER
    {
        LOG_INFO("[bison]: var: IDENTIFIER");
        auto var = std::make_shared<fakelua::SyntaxTreeVar>(@1);
        var->SetName($1);
        var->SetType("simple");
        $$ = var;
    }
    |
    prefixexp LSQUARE exp RSQUARE
    {
        LOG_INFO("[bison]: var: prefixexp LSQUARE exp RSQUARE");
        auto var = std::make_shared<fakelua::SyntaxTreeVar>(@2);
        var->SetType("square");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        var->SetPrefixexp(prefixexp);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: var: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        var->SetExp(exp);
        $$ = var;
    }
    |
    prefixexp DOT IDENTIFIER
    {
        LOG_INFO("[bison]: var: prefixexp DOT IDENTIFIER");
        auto var = std::make_shared<fakelua::SyntaxTreeVar>(@2);
        var->SetType("dot");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: var: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        var->SetPrefixexp(prefixexp);
        var->SetName($3);
        $$ = var;
    }
    ;

namelist:
    IDENTIFIER
    {
        LOG_INFO("[bison]: namelist: IDENTIFIER");
        auto namelist = std::make_shared<fakelua::SyntaxTreeNamelist>(@1);
        namelist->AddName($1);
        $$ = namelist;
    }
    |
    namelist COMMA IDENTIFIER
    {
        LOG_INFO("[bison]: namelist: namelist COMMA IDENTIFIER");
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: namelist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        namelist->AddName($3);
        $$ = namelist;
    }
    ;

explist:
    exp
    {
        LOG_INFO("[bison]: explist: exp");
        auto explist = std::make_shared<fakelua::SyntaxTreeExplist>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        explist->AddExp(exp);
        $$ = explist;
    }
    |
    explist COMMA exp
    {
        LOG_INFO("[bison]: explist: explist COMMA exp");
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>($1);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: explist: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: explist: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        explist->AddExp(exp);
        $$ = explist;
    }
    ;

exp:
    NIL
    {
        LOG_INFO("[bison]: exp: NIL");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("nil");
        $$ = exp;
    }
    |
    TRUE
    {
        LOG_INFO("[bison]: exp: TRUE");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("true");
        $$ = exp;
    }
    |
    FALSES
    {
        LOG_INFO("[bison]: exp: FALSES");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("false");
        $$ = exp;
    }
    |
    NUMBER
    {
        LOG_INFO("[bison]: exp: NUMBER");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("number");
        exp->SetValue($1);
        $$ = exp;
    }
    |
    STRING
    {
        LOG_INFO("[bison]: exp: STRING");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("string");
        exp->SetValue(l->RemoveQuotes($1));
        $$ = exp;
    }
    |
    VAR_PARAMS
    {
        LOG_INFO("[bison]: exp: VAR_PARAMS");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("VarParams");
        $$ = exp;
    }
    |
    functiondef
    {
        LOG_INFO("[bison]: exp: functiondef");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("functiondef");
        auto functiondef = std::dynamic_pointer_cast<fakelua::SyntaxTreeFunctiondef>($1);
        if (functiondef == nullptr) {
            LOG_ERROR("[bison]: exp: functiondef is not a functiondef");
            fakelua::ThrowFakeluaException("functiondef is not a functiondef");
        }
        exp->SetRight(functiondef);
        $$ = exp;
    }
    |
    prefixexp
    {
        LOG_INFO("[bison]: exp: prefixexp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("prefixexp");
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: exp: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        exp->SetRight(prefixexp);
        $$ = exp;
    }
    |
    tableconstructor
    {
        LOG_INFO("[bison]: exp: tableconstructor");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("tableconstructor");
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::SyntaxTreeTableconstructor>($1);
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: exp: tableconstructor is not a tableconstructor");
            fakelua::ThrowFakeluaException("tableconstructor is not a tableconstructor");
        }
        exp->SetRight(tableconstructor);
        $$ = exp;
    }
    |
    exp PLUS exp
    {
        LOG_INFO("[bison]: exp: exp PLUS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("PLUS");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp MINUS exp
    {
        LOG_INFO("[bison]: exp: exp MINUS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("MINUS");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp STAR exp
    {
        LOG_INFO("[bison]: exp: exp STAR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("STAR");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp SLASH exp
    {
        LOG_INFO("[bison]: exp: exp SLASH exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("SLASH");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp DOUBLE_SLASH exp
    {
        LOG_INFO("[bison]: exp: exp DOUBLE_SLASH exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("DOUBLE_SLASH");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp POW exp
    {
        LOG_INFO("[bison]: exp: exp POW exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("POW");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp MOD exp
    {
        LOG_INFO("[bison]: exp: exp MOD exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("MOD");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp BITAND exp
    {
        LOG_INFO("[bison]: exp: exp BITAND exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("BITAND");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp BITNOT exp
    {
        LOG_INFO("[bison]: exp: exp XOR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("XOR");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp BITOR exp
    {
        LOG_INFO("[bison]: exp: exp BITOR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("BITOR");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp RIGHT_SHIFT exp
    {
        LOG_INFO("[bison]: exp: exp RIGHT_SHIFT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("RIGHT_SHIFT");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp LEFT_SHIFT exp
    {
        LOG_INFO("[bison]: exp: exp LEFT_SHIFT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("LEFT_SHIFT");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp CONCAT exp
    {
        LOG_INFO("[bison]: exp: exp CONCAT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("CONCAT");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp LESS exp
    {
        LOG_INFO("[bison]: exp: exp LESS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("LESS");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp LESS_EQUAL exp
    {
        LOG_INFO("[bison]: exp: exp LESS_EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("LESS_EQUAL");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp MORE exp
    {
        LOG_INFO("[bison]: exp: exp MORE exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("MORE");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp MORE_EQUAL exp
    {
        LOG_INFO("[bison]: exp: exp MORE_EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("MORE_EQUAL");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp EQUAL exp
    {
        LOG_INFO("[bison]: exp: exp EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("EQUAL");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp NOT_EQUAL exp
    {
        LOG_INFO("[bison]: exp: exp NOT_EQUAL exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("NOT_EQUAL");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp AND exp
    {
        LOG_INFO("[bison]: exp: exp AND exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("AND");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    exp OR exp
    {
        LOG_INFO("[bison]: exp: exp OR exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("binop");
        auto left_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (left_exp == nullptr) {
            LOG_ERROR("[bison]: exp: left_exp is not a exp");
            fakelua::ThrowFakeluaException("left_exp is not a exp");
        }
        exp->SetLeft(left_exp);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        auto binop = std::make_shared<fakelua::SyntaxTreeBinop>(@2);
        binop->SetOp("OR");
        exp->SetOp(binop);
        $$ = exp;
    }
    |
    MINUS exp %prec UNARY
    {
        LOG_INFO("[bison]: exp: MINUS exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("unop");
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(@1);
        unop->SetOp("MINUS");
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        $$ = exp;
    }
    |
    NOT exp %prec UNARY
    {
        LOG_INFO("[bison]: exp: NOT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("unop");
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(@1);
        unop->SetOp("NOT");
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        $$ = exp;
    }
    |
    NUMBER_SIGN exp %prec UNARY
    {
        LOG_INFO("[bison]: exp: NUMBER_SIGN exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("unop");
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(@1);
        unop->SetOp("NUMBER_SIGN");
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        $$ = exp;
    }
    |
    BITNOT exp %prec UNARY
    {
        LOG_INFO("[bison]: exp: BITNOT exp");
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("unop");
        auto unop = std::make_shared<fakelua::SyntaxTreeUnop>(@1);
        unop->SetOp("BITNOT");
        exp->SetOp(unop);
        auto right_exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (right_exp == nullptr) {
            LOG_ERROR("[bison]: exp: right_exp is not a exp");
            fakelua::ThrowFakeluaException("right_exp is not a exp");
        }
        exp->SetRight(right_exp);
        $$ = exp;
    }
    ;

prefixexp:
    var
    {
        LOG_INFO("[bison]: prefixexp: var");
        auto prefixexp = std::make_shared<fakelua::SyntaxTreePrefixexp>(@1);
        prefixexp->SetType("var");
        auto var = std::dynamic_pointer_cast<fakelua::SyntaxTreeVar>($1);
        if (var == nullptr) {
            LOG_ERROR("[bison]: prefixexp: var is not a var");
            fakelua::ThrowFakeluaException("var is not a var");
        }
        prefixexp->SetValue(var);
        $$ = prefixexp;
    }
    |
    functioncall
    {
        LOG_INFO("[bison]: prefixexp: functioncall");
        auto prefixexp = std::make_shared<fakelua::SyntaxTreePrefixexp>(@1);
        prefixexp->SetType("functioncall");
        auto functioncall = std::dynamic_pointer_cast<fakelua::SyntaxTreeFunctioncall>($1);
        if (functioncall == nullptr) {
            LOG_ERROR("[bison]: prefixexp: functioncall is not a functioncall");
            fakelua::ThrowFakeluaException("functioncall is not a functioncall");
        }
        prefixexp->SetValue(functioncall);
        $$ = prefixexp;
    }
    |
    LPAREN exp RPAREN
    {
        LOG_INFO("[bison]: prefixexp: LPAREN exp RPAREN");
        auto prefixexp = std::make_shared<fakelua::SyntaxTreePrefixexp>(@1);
        prefixexp->SetType("exp");
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: prefixexp: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        prefixexp->SetValue(exp);
        $$ = prefixexp;
    }

functioncall:
    prefixexp args
    {
        LOG_INFO("[bison]: functioncall: prefixexp args");
        auto functioncall = std::make_shared<fakelua::SyntaxTreeFunctioncall>(@1);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        functioncall->SetPrefixexp(prefixexp);
        auto args = std::dynamic_pointer_cast<fakelua::SyntaxTreeArgs>($2);
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::ThrowFakeluaException("args is not a args");
        }
        functioncall->SetArgs(args);
        $$ = functioncall;
    }
    |
    prefixexp COLON IDENTIFIER args
    {
        LOG_INFO("[bison]: functioncall: prefixexp COLON IDENTIFIER args");
        auto functioncall = std::make_shared<fakelua::SyntaxTreeFunctioncall>(@1);
        auto prefixexp = std::dynamic_pointer_cast<fakelua::SyntaxTreePrefixexp>($1);
        if (prefixexp == nullptr) {
            LOG_ERROR("[bison]: functioncall: prefixexp is not a prefixexp");
            fakelua::ThrowFakeluaException("prefixexp is not a prefixexp");
        }
        functioncall->SetPrefixexp(prefixexp);
        functioncall->SetName($3);
        auto args = std::dynamic_pointer_cast<fakelua::SyntaxTreeArgs>($4);
        if (args == nullptr) {
            LOG_ERROR("[bison]: functioncall: args is not a args");
            fakelua::ThrowFakeluaException("args is not a args");
        }
        functioncall->SetArgs(args);
        $$ = functioncall;
    }
    ;

args:
    LPAREN explist RPAREN
    {
        LOG_INFO("[bison]: args: LPAREN explist RPAREN");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(@1);
        auto explist = std::dynamic_pointer_cast<fakelua::SyntaxTreeExplist>($2);
        if (explist == nullptr) {
            LOG_ERROR("[bison]: args: explist is not a explist");
            fakelua::ThrowFakeluaException("explist is not a explist");
        }
        args->SetExplist(explist);
        args->SetType("explist");
        $$ = args;
    }
    |
    LPAREN RPAREN
    {
        LOG_INFO("[bison]: args: LPAREN RPAREN");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(@1);
        args->SetType("empty");
        $$ = args;
    }
    |
    tableconstructor
    {
        LOG_INFO("[bison]: args: tableconstructor");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(@1);
        auto tableconstructor = std::dynamic_pointer_cast<fakelua::SyntaxTreeTableconstructor>($1);
        if (tableconstructor == nullptr) {
            LOG_ERROR("[bison]: args: tableconstructor is not a tableconstructor");
            fakelua::ThrowFakeluaException("tableconstructor is not a tableconstructor");
        }
        args->SetTableconstructor(tableconstructor);
        args->SetType("tableconstructor");
        $$ = args;
    }
    |
    STRING
    {
        LOG_INFO("[bison]: args: STRING");
        auto args = std::make_shared<fakelua::SyntaxTreeArgs>(@1);
        auto exp = std::make_shared<fakelua::SyntaxTreeExp>(@1);
        exp->SetType("string");
        exp->SetValue(l->RemoveQuotes($1));
        args->SetString(exp);
        args->SetType("string");
        $$ = args;
    }
    ;

functiondef:
    FUNCTION funcbody
    {
        LOG_INFO("[bison]: functiondef: FUNCTION funcbody");
        auto functiondef = std::make_shared<fakelua::SyntaxTreeFunctiondef>(@1);
        auto funcbody = std::dynamic_pointer_cast<fakelua::SyntaxTreeFuncbody>($2);
        if (funcbody == nullptr) {
            LOG_ERROR("[bison]: functiondef: funcbody is not a funcbody");
            fakelua::ThrowFakeluaException("funcbody is not a funcbody");
        }
        functiondef->SetFuncbody(funcbody);
        $$ = functiondef;
    }
    ;

funcbody:
    LPAREN parlist RPAREN block END
    {
        LOG_INFO("[bison]: funcbody: LPAREN parlist RPAREN block END");
        auto funcbody = std::make_shared<fakelua::SyntaxTreeFuncbody>(@1);
        auto parlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeParlist>($2);
        if (parlist == nullptr) {
            LOG_ERROR("[bison]: funcbody: parlist is not a parlist");
            fakelua::ThrowFakeluaException("parlist is not a parlist");
        }
        funcbody->SetParlist(parlist);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($4);
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        funcbody->SetBlock(block);
        $$ = funcbody;
    }
    |
    LPAREN RPAREN block END
    {
        LOG_INFO("[bison]: funcbody: LPAREN RPAREN block END");
        auto funcbody = std::make_shared<fakelua::SyntaxTreeFuncbody>(@1);
        auto block = std::dynamic_pointer_cast<fakelua::SyntaxTreeBlock>($3);
        if (block == nullptr) {
            LOG_ERROR("[bison]: funcbody: block is not a block");
            fakelua::ThrowFakeluaException("block is not a block");
        }
        funcbody->SetBlock(block);
        $$ = funcbody;
    }
    ;

parlist:
    namelist
    {
        LOG_INFO("[bison]: parlist: namelist");
        auto parlist = std::make_shared<fakelua::SyntaxTreeParlist>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        parlist->SetNamelist(namelist);
        $$ = parlist;
    }
    |
    namelist COMMA VAR_PARAMS
    {
        LOG_INFO("[bison]: parlist: namelist COMMA VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::SyntaxTreeParlist>(@1);
        auto namelist = std::dynamic_pointer_cast<fakelua::SyntaxTreeNamelist>($1);
        if (namelist == nullptr) {
            LOG_ERROR("[bison]: parlist: namelist is not a namelist");
            fakelua::ThrowFakeluaException("namelist is not a namelist");
        }
        parlist->SetNamelist(namelist);
        parlist->SetVarParams(true);
        $$ = parlist;
    }
    |
    VAR_PARAMS
    {
        LOG_INFO("[bison]: parlist: VAR_PARAMS");
        auto parlist = std::make_shared<fakelua::SyntaxTreeParlist>(@1);
        parlist->SetVarParams(true);
        $$ = parlist;
    }
    ;

tableconstructor:
    LCURLY fieldlist RCURLY
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY fieldlist RCURLY");
        auto tableconstructor = std::make_shared<fakelua::SyntaxTreeTableconstructor>(@1);
        auto fieldlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFieldlist>($2);
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: tableconstructor: fieldlist is not a fieldlist");
            fakelua::ThrowFakeluaException("fieldlist is not a fieldlist");
        }
        tableconstructor->SetFieldlist(fieldlist);
        $$ = tableconstructor;
    }
    |
    LCURLY RCURLY
    {
        LOG_INFO("[bison]: tableconstructor: LCURLY RCURLY");
        auto tableconstructor = std::make_shared<fakelua::SyntaxTreeTableconstructor>(@1);
        $$ = tableconstructor;
    }
    ;

fieldlist:
    field
    {
        LOG_INFO("[bison]: fieldlist: field");
        auto fieldlist = std::make_shared<fakelua::SyntaxTreeFieldlist>(@1);
        auto field = std::dynamic_pointer_cast<fakelua::SyntaxTreeField>($1);
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::ThrowFakeluaException("field is not a field");
        }
        fieldlist->AddField(field);
        $$ = fieldlist;
    }
    |
    fieldlist fieldsep field
    {
        LOG_INFO("[bison]: fieldlist: fieldlist fieldsep field");
        auto fieldlist = std::dynamic_pointer_cast<fakelua::SyntaxTreeFieldlist>($1);
        if (fieldlist == nullptr) {
            LOG_ERROR("[bison]: fieldlist: fieldlist is not a fieldlist");
            fakelua::ThrowFakeluaException("fieldlist is not a fieldlist");
        }
        auto field = std::dynamic_pointer_cast<fakelua::SyntaxTreeField>($3);
        if (field == nullptr) {
            LOG_ERROR("[bison]: fieldlist: field is not a field");
            fakelua::ThrowFakeluaException("field is not a field");
        }
        fieldlist->AddField(field);
        $$ = fieldlist;
    }
    ;

field:
    LSQUARE exp RSQUARE ASSIGN exp
    {
        LOG_INFO("[bison]: field: LSQUARE exp RSQUARE ASSIGN exp");
        auto field = std::make_shared<fakelua::SyntaxTreeField>(@1);
        auto key = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($2);
        if (key == nullptr) {
            LOG_ERROR("[bison]: key: key is not a exp");
            fakelua::ThrowFakeluaException("key is not a exp");
        }
        field->SetKey(key);
        auto value = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($5);
        if (value == nullptr) {
            LOG_ERROR("[bison]: field: value is not a exp");
            fakelua::ThrowFakeluaException("value is not a exp");
        }
        field->SetValue(value);
        field->SetType("array");
        $$ = field;
    }
    |
    IDENTIFIER ASSIGN exp
    {
        LOG_INFO("[bison]: field: IDENTIFIER ASSIGN exp");
        auto field = std::make_shared<fakelua::SyntaxTreeField>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($3);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        field->SetName($1);
        field->SetValue(exp);
        field->SetType("object");
        $$ = field;
    }
    |
    exp
    {
        LOG_INFO("[bison]: field: exp");
        auto field = std::make_shared<fakelua::SyntaxTreeField>(@1);
        auto exp = std::dynamic_pointer_cast<fakelua::SyntaxTreeExp>($1);
        if (exp == nullptr) {
            LOG_ERROR("[bison]: field: exp is not a exp");
            fakelua::ThrowFakeluaException("exp is not a exp");
        }
        field->SetValue(exp);
        field->SetType("array");
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

%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
    std::stringstream ss;
    ss << l;
    fakelua::ThrowFakeluaException(std::format("{}: {}", ss.str(), m));
}
