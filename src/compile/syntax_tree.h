#pragma once

#include "bison/location.hh"
#include "util/common.h"
#include "util/exception.h"

namespace fakelua {

// syntax tree type
enum class SyntaxTreeType {
    None = 0,
    Empty,
    Block,
    Label,
    Return,
    Assign,
    VarList,
    ExpList,
    Var,
    FunctionCall,
    TableConstructor,
    FieldList,
    Field,
    Break,
    Goto,
    While,
    Repeat,
    If,
    ElseIfList,
    ForLoop,
    ForIn,
    NameList,
    Function,
    FuncNameList,
    FuncName,
    FuncBody,
    FunctionDef,
    ParList,
    LocalFunction,
    LocalVar,
    Exp,
    Binop,
    Unop,
    Args,
    PrefixExp,
};

inline std::string SyntaxTreeTypeToString(SyntaxTreeType t) {
    switch (t) {
        case SyntaxTreeType::None:
            return "None";
        case SyntaxTreeType::Empty:
            return "Empty";
        case SyntaxTreeType::Block:
            return "Block";
        case SyntaxTreeType::Label:
            return "Label";
        case SyntaxTreeType::Return:
            return "Return";
        case SyntaxTreeType::Assign:
            return "Assign";
        case SyntaxTreeType::VarList:
            return "VarList";
        case SyntaxTreeType::ExpList:
            return "ExpList";
        case SyntaxTreeType::Var:
            return "Var";
        case SyntaxTreeType::FunctionCall:
            return "FunctionCall";
        case SyntaxTreeType::TableConstructor:
            return "TableConstructor";
        case SyntaxTreeType::FieldList:
            return "FieldList";
        case SyntaxTreeType::Field:
            return "Field";
        case SyntaxTreeType::Break:
            return "Break";
        case SyntaxTreeType::Goto:
            return "Goto";
        case SyntaxTreeType::While:
            return "While";
        case SyntaxTreeType::Repeat:
            return "Repeat";
        case SyntaxTreeType::If:
            return "If";
        case SyntaxTreeType::ElseIfList:
            return "ElseIfList";
        case SyntaxTreeType::ForLoop:
            return "ForLoop";
        case SyntaxTreeType::ForIn:
            return "ForIn";
        case SyntaxTreeType::NameList:
            return "NameList";
        case SyntaxTreeType::Function:
            return "Function";
        case SyntaxTreeType::FuncNameList:
            return "FuncNameList";
        case SyntaxTreeType::FuncName:
            return "FuncName";
        case SyntaxTreeType::FuncBody:
            return "FuncBody";
        case SyntaxTreeType::FunctionDef:
            return "FunctionDef";
        case SyntaxTreeType::ParList:
            return "ParList";
        case SyntaxTreeType::LocalFunction:
            return "LocalFunction";
        case SyntaxTreeType::LocalVar:
            return "LocalVar";
        case SyntaxTreeType::Exp:
            return "Exp";
        case SyntaxTreeType::Binop:
            return "Binop";
        case SyntaxTreeType::Unop:
            return "Unop";
        case SyntaxTreeType::Args:
            return "Args";
        case SyntaxTreeType::PrefixExp:
            return "PrefixExp";
        default:
            return "UNKNOWN";
    }
}

// syntax tree location type
typedef yy::location SyntaxTreeLocation;

// syntax tree interface
class SyntaxTreeInterface {
public:
    explicit SyntaxTreeInterface(const SyntaxTreeLocation &loc) : loc_(loc) {
    }

    virtual ~SyntaxTreeInterface() = default;

    // get syntax tree type
    [[nodiscard]] virtual SyntaxTreeType Type() const = 0;

    // dump a syntax tree to string
    [[nodiscard]] virtual std::string Dump(int tab) const = 0;

    // get syntax tree location
    [[nodiscard]] const SyntaxTreeLocation &Loc() const {
        return loc_;
    }

protected:
    // generate tab string
    [[nodiscard]] std::string GenTab(int tab) const {
        std::string str;
        for (int i = 0; i < tab; ++i) {
            str += "  ";
        }
        return str;
    }

    // generate location string
    [[nodiscard]] std::string LocStr() const {
        // maybe the loc_'s filename ptr is invalid now, so ignore it
        std::string str;
        str += std::to_string(loc_.begin.line);
        str += ":";
        str += std::to_string(loc_.begin.column);
        return str;
    }

private:
    // syntax tree location
    SyntaxTreeLocation loc_;
};

// syntax tree shared pointer
typedef std::shared_ptr<SyntaxTreeInterface> SyntaxTreeInterfacePtr;

// empty
class SyntaxTreeEmpty final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeEmpty(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Empty;
    }

    [[nodiscard]] std::string Dump(int tab) const override;
};

// block
class SyntaxTreeBlock final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeBlock(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeBlock() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Block;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void AddStmt(const SyntaxTreeInterfacePtr &stmt) {
        stmts_.push_back(stmt);
    }

    [[nodiscard]] const std::vector<SyntaxTreeInterfacePtr> &Stmts() const {
        return stmts_;
    }

    void SetStmts(const std::vector<SyntaxTreeInterfacePtr> &stmts) {
        stmts_ = stmts;
    }

private:
    std::vector<SyntaxTreeInterfacePtr> stmts_;
};

// label
class SyntaxTreeLabel final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeLabel(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeLabel() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Label;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetName(const std::string &name) {
        name_ = name;
    }

private:
    std::string name_;
};

// return
class SyntaxTreeReturn final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeReturn(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeReturn() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Return;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

private:
    SyntaxTreeInterfacePtr explist_;
};

// assign
class SyntaxTreeAssign final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeAssign(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeAssign() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Assign;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetVarlist(const SyntaxTreeInterfacePtr &varlist) {
        varlist_ = varlist;
    }

    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Varlist() const {
        return varlist_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

private:
    SyntaxTreeInterfacePtr varlist_;
    SyntaxTreeInterfacePtr explist_;
};

// var list
class SyntaxTreeVarlist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeVarlist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeVarlist() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::VarList;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void AddVar(const SyntaxTreeInterfacePtr &var) {
        vars_.push_back(var);
    }

    std::vector<SyntaxTreeInterfacePtr> &Vars() {
        return vars_;
    }

private:
    std::vector<SyntaxTreeInterfacePtr> vars_;
};

// exp list
class SyntaxTreeExplist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeExplist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeExplist() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ExpList;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void AddExp(const SyntaxTreeInterfacePtr &exp) {
        exps_.push_back(exp);
    }

    std::vector<SyntaxTreeInterfacePtr> &Exps() {
        return exps_;
    }

private:
    std::vector<SyntaxTreeInterfacePtr> exps_;
};

// var
class SyntaxTreeVar final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeVar(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeVar() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Var;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetName(const std::string &name) {
        name_ = name;
    }

    void SetPrefixexp(const SyntaxTreeInterfacePtr &prefixexp) {
        prefixexp_ = prefixexp;
    }

    SyntaxTreeInterfacePtr GetPrefixexp() {
        return prefixexp_;
    }

    void SetExp(const SyntaxTreeInterfacePtr &exp) {
        exp_ = exp;
    }

    SyntaxTreeInterfacePtr GetExp() {
        return exp_;
    }

    void SetType(const std::string &type) {
        type_ = type;
    }

    [[nodiscard]] std::string GetName() const {
        return name_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr GetPrefixexp() const {
        return prefixexp_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr GetExp() const {
        return exp_;
    }

    [[nodiscard]] std::string GetType() const {
        return type_;
    }

private:
    std::string name_;
    SyntaxTreeInterfacePtr exp_;
    SyntaxTreeInterfacePtr prefixexp_;
    std::string type_;
};

// function call
class SyntaxTreeFunctioncall final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFunctioncall(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFunctioncall() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FunctionCall;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetPrefixexp(const SyntaxTreeInterfacePtr &prefixexp) {
        prefixexp_ = prefixexp;
    }

    void SetArgs(const SyntaxTreeInterfacePtr &args) {
        args_ = args;
    }

    void SetName(const std::string &name) {
        name_ = name;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr prefixexp() const {
        return prefixexp_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Args() const {
        return args_;
    }

    [[nodiscard]] std::string Name() const {
        return name_;
    }

private:
    SyntaxTreeInterfacePtr prefixexp_;
    SyntaxTreeInterfacePtr args_;
    std::string name_;
};

// table constructor
class SyntaxTreeTableconstructor final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeTableconstructor(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeTableconstructor() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::TableConstructor;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetFieldlist(const SyntaxTreeInterfacePtr &fieldlist) {
        fieldlist_ = fieldlist;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Fieldlist() const {
        return fieldlist_;
    }

private:
    SyntaxTreeInterfacePtr fieldlist_;
};

// field list
class SyntaxTreeFieldlist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFieldlist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFieldlist() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FieldList;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void AddField(const SyntaxTreeInterfacePtr &field) {
        fields_.push_back(field);
    }

    std::vector<SyntaxTreeInterfacePtr> &Fields() {
        return fields_;
    }

private:
    std::vector<SyntaxTreeInterfacePtr> fields_;
};

// field assignment
class SyntaxTreeField final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeField(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeField() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Field;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetKey(const SyntaxTreeInterfacePtr &key) {
        key_ = key;
    }

    void SetValue(const SyntaxTreeInterfacePtr &value) {
        value_ = value;
    }

    void SetName(const std::string &name) {
        name_ = name;
    }

    void SetType(const std::string &type) {
        type_ = type;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Key() const {
        return key_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Value() const {
        return value_;
    }

    [[nodiscard]] std::string Name() const {
        return name_;
    }

    [[nodiscard]] std::string GetType() const {
        return type_;
    }

private:
    SyntaxTreeInterfacePtr key_;
    SyntaxTreeInterfacePtr value_;
    std::string name_;
    std::string type_;
};

// break
class SyntaxTreeBreak final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeBreak(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeBreak() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Break;
    }

    [[nodiscard]] std::string Dump(int tab) const override;
};

// goto
class SyntaxTreeGoto final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeGoto(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeGoto() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Goto;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetLabel(const std::string &label) {
        label_ = label;
    }

private:
    std::string label_;
};

// while
class SyntaxTreeWhile final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeWhile(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeWhile() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::While;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetExp(const SyntaxTreeInterfacePtr &exp) {
        exp_ = exp;
    }

    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Exp() const {
        return exp_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

private:
    SyntaxTreeInterfacePtr exp_;
    SyntaxTreeInterfacePtr block_;
};

// repeat
class SyntaxTreeRepeat final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeRepeat(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeRepeat() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Repeat;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetExp(const SyntaxTreeInterfacePtr &exp) {
        exp_ = exp;
    }

    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Exp() const {
        return exp_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

private:
    SyntaxTreeInterfacePtr exp_;
    SyntaxTreeInterfacePtr block_;
};

// if
class SyntaxTreeIf final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeIf(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeIf() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::If;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetExp(const SyntaxTreeInterfacePtr &exp) {
        exp_ = exp;
    }

    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    void SetElseiflist(const SyntaxTreeInterfacePtr &elseifs) {
        elseifs_ = elseifs;
    }

    void SetElseBlock(const SyntaxTreeInterfacePtr &elseblock) {
        elseblock_ = elseblock;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Exp() const {
        return exp_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr ElseIfs() const {
        return elseifs_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr ElseBlock() const {
        return elseblock_;
    }

private:
    SyntaxTreeInterfacePtr exp_;
    SyntaxTreeInterfacePtr block_;
    SyntaxTreeInterfacePtr elseifs_;
    SyntaxTreeInterfacePtr elseblock_;
};

// elseif_list
class SyntaxTreeElseiflist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeElseiflist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeElseiflist() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ElseIfList;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void AddElseifExpr(const SyntaxTreeInterfacePtr &exp) {
        exps_.push_back(exp);
    }

    void AddElseifBlock(const SyntaxTreeInterfacePtr &block) {
        blocks_.push_back(block);
    }

    [[nodiscard]] size_t ElseifSize() const {
        return exps_.size();
    }

    [[nodiscard]] const std::vector<SyntaxTreeInterfacePtr> &ElseifExps() const {
        return exps_;
    }

    [[nodiscard]] const std::vector<SyntaxTreeInterfacePtr> &ElseifBlocks() const {
        return blocks_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr ElseifExp(size_t idx) const {
        return exps_[idx];
    }

    [[nodiscard]] SyntaxTreeInterfacePtr ElseifBlock(size_t idx) const {
        return blocks_[idx];
    }

private:
    std::vector<SyntaxTreeInterfacePtr> exps_;
    std::vector<SyntaxTreeInterfacePtr> blocks_;
};

// for loop
class SyntaxTreeForLoop final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeForLoop(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeForLoop() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ForLoop;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetName(const std::string &name) {
        name_ = name;
    }

    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    void SetExpBegin(const SyntaxTreeInterfacePtr &exp) {
        exp_begin_ = exp;
    }

    void SetExpEnd(const SyntaxTreeInterfacePtr &exp) {
        exp_end_ = exp;
    }

    void SetExpStep(const SyntaxTreeInterfacePtr &exp) {
        exp_step_ = exp;
    }

    [[nodiscard]] std::string Name() const {
        return name_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr ExpBegin() const {
        return exp_begin_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr ExpEnd() const {
        return exp_end_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr ExpStep() const {
        return exp_step_;
    }

private:
    std::string name_;
    SyntaxTreeInterfacePtr block_;
    SyntaxTreeInterfacePtr exp_begin_;
    SyntaxTreeInterfacePtr exp_end_;
    SyntaxTreeInterfacePtr exp_step_;
};

// for in
class SyntaxTreeForIn final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeForIn(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeForIn() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ForIn;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetNamelist(const SyntaxTreeInterfacePtr &namelist) {
        namelist_ = namelist;
    }

    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Namelist() const {
        return namelist_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

private:
    SyntaxTreeInterfacePtr namelist_;
    SyntaxTreeInterfacePtr explist_;
    SyntaxTreeInterfacePtr block_;
};

// name list
class SyntaxTreeNamelist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeNamelist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeNamelist() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::NameList;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void AddName(const std::string &name) {
        names_.push_back(name);
    }

    void AddAttrib(const std::string &attrib) {
        attrib_.push_back(attrib);
    }

    [[nodiscard]] const std::vector<std::string> &Names() const {
        return names_;
    }

private:
    std::vector<std::string> names_;
    std::vector<std::string> attrib_;
};

// function
class SyntaxTreeFunction final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFunction(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFunction() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Function;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetFuncname(const SyntaxTreeInterfacePtr &funcname) {
        funcname_ = funcname;
    }

    void SetFuncbody(const SyntaxTreeInterfacePtr &funcbody) {
        funcbody_ = funcbody;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Funcname() const {
        return funcname_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Funcbody() const {
        return funcbody_;
    }

private:
    SyntaxTreeInterfacePtr funcname_;
    SyntaxTreeInterfacePtr funcbody_;
};

// funcnamelist
class SyntaxTreeFuncnamelist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFuncnamelist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFuncnamelist() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FuncNameList;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void AddName(const std::string &funcname) {
        funcnames_.push_back(funcname);
    }

    [[nodiscard]] const std::vector<std::string> &Funcnames() const {
        return funcnames_;
    }

private:
    std::vector<std::string> funcnames_;
};

// funcname
class SyntaxTreeFuncname final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFuncname(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFuncname() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FuncName;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetFuncNameList(const SyntaxTreeInterfacePtr &funcnamelist) {
        funcnamelist_ = funcnamelist;
    }

    void SetColonName(const std::string &name) {
        colon_name_ = name;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr FuncNameList() const {
        return funcnamelist_;
    }

    [[nodiscard]] std::string ColonName() const {
        return colon_name_;
    }

private:
    SyntaxTreeInterfacePtr funcnamelist_;
    std::string colon_name_;
};

// funcbody
class SyntaxTreeFuncbody final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFuncbody(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFuncbody() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FuncBody;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetParlist(const SyntaxTreeInterfacePtr &parlist) {
        parlist_ = parlist;
    }

    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Parlist() const {
        return parlist_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

private:
    SyntaxTreeInterfacePtr parlist_;
    SyntaxTreeInterfacePtr block_;
};

// functiondef
class SyntaxTreeFunctiondef final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFunctiondef(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFunctiondef() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FunctionDef;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetFuncbody(const SyntaxTreeInterfacePtr &funcbody) {
        funcbody_ = funcbody;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Funcbody() const {
        return funcbody_;
    }

private:
    SyntaxTreeInterfacePtr funcbody_;
};

// parlist
class SyntaxTreeParlist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeParlist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeParlist() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ParList;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetNamelist(const SyntaxTreeInterfacePtr &namelist) {
        namelist_ = namelist;
    }

    void SetVarParams(bool VarParams) {
        var_params_ = VarParams;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Namelist() const {
        return namelist_;
    }

    [[nodiscard]] bool VarParams() const {
        return var_params_;
    }

private:
    SyntaxTreeInterfacePtr namelist_;
    bool var_params_ = false;
};

// local function
class SyntaxTreeLocalFunction final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeLocalFunction(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeLocalFunction() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::LocalFunction;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetName(const std::string &name) {
        name_ = name;
    }

    void SetFuncbody(const SyntaxTreeInterfacePtr &funcbody) {
        funcbody_ = funcbody;
    }

    [[nodiscard]] std::string Name() const {
        return name_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Funcbody() const {
        return funcbody_;
    }

private:
    std::string name_;
    SyntaxTreeInterfacePtr funcbody_;
};

// local var
class SyntaxTreeLocalVar final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeLocalVar(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeLocalVar() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::LocalVar;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetNamelist(const SyntaxTreeInterfacePtr &namelist) {
        namelist_ = namelist;
    }

    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Namelist() const {
        return namelist_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

private:
    SyntaxTreeInterfacePtr namelist_;
    SyntaxTreeInterfacePtr explist_;
};

// exp
class SyntaxTreeExp final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeExp(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeExp() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Exp;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetType(const std::string &type) {
        type_ = type;
    }

    void SetValue(const std::string &value) {
        value_ = value;
    }

    void SetLeft(const SyntaxTreeInterfacePtr &left) {
        left_ = left;
    }

    void SetOp(const SyntaxTreeInterfacePtr &op) {
        op_ = op;
    }

    void SetRight(const SyntaxTreeInterfacePtr &right) {
        right_ = right;
    }

    [[nodiscard]] std::string ExpType() const {
        return type_;
    }

    [[nodiscard]] std::string ExpValue() const {
        return value_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Left() const {
        return left_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Op() const {
        return op_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Right() const {
        return right_;
    }

private:
    std::string type_;
    std::string value_;
    SyntaxTreeInterfacePtr left_;
    SyntaxTreeInterfacePtr op_;
    SyntaxTreeInterfacePtr right_;
};

// binop
class SyntaxTreeBinop final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeBinop(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeBinop() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Binop;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetOp(const std::string &op) {
        op_ = op;
    }

    [[nodiscard]] std::string GetOp() const {
        return op_;
    }

private:
    std::string op_;
};

// unop
class SyntaxTreeUnop final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeUnop(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeUnop() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Unop;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetOp(const std::string &op) {
        op_ = op;
    }

    [[nodiscard]] std::string GetOp() const {
        return op_;
    }

private:
    std::string op_;
};

// args
class SyntaxTreeArgs final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeArgs(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeArgs() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Args;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    void SetTableconstructor(const SyntaxTreeInterfacePtr &tableconstructor) {
        tableconstructor_ = tableconstructor;
    }

    void SetString(const SyntaxTreeInterfacePtr &string) {
        string_ = string;
    }

    void SetType(const std::string &type) {
        type_ = type;
    }

    [[nodiscard]] std::string GetType() const {
        return type_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr Tableconstructor() const {
        return tableconstructor_;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr String() const {
        return string_;
    }

private:
    SyntaxTreeInterfacePtr explist_;
    SyntaxTreeInterfacePtr tableconstructor_;
    SyntaxTreeInterfacePtr string_;
    std::string type_;
};

// prefixexp
class SyntaxTreePrefixexp final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreePrefixexp(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreePrefixexp() override = default;

    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::PrefixExp;
    }

    [[nodiscard]] std::string Dump(int tab) const override;

    void SetValue(const SyntaxTreeInterfacePtr &value) {
        value_ = value;
    }

    void SetType(const std::string &type) {
        type_ = type;
    }

    [[nodiscard]] SyntaxTreeInterfacePtr GetValue() const {
        return value_;
    }

    [[nodiscard]] std::string GetType() const {
        return type_;
    }

private:
    SyntaxTreeInterfacePtr value_;
    std::string type_;
};

typedef std::function<void(const SyntaxTreeInterfacePtr &)> WalkSyntaxTreeFunc;
void WalkSyntaxTree(const SyntaxTreeInterfacePtr &node, const WalkSyntaxTreeFunc &func);

}// namespace fakelua
