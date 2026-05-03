#pragma once

#include "bison/location.hh"
#include "util/common.h"

namespace fakelua {

enum InferredType {
    T_UNKNOWN = 0,
    T_INT,
    T_FLOAT,
    T_DYNAMIC
};

// 语法树节点类型
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

// 将语法树类型转换为字符串描述
inline std::string SyntaxTreeTypeToString(SyntaxTreeType type) {
    switch (type) {
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

// 语法树节点位置类型
using SyntaxTreeLocation = yy::location;

// 语法树节点基类接口
class SyntaxTreeInterface {
public:
    explicit SyntaxTreeInterface(const SyntaxTreeLocation &loc) : loc_(loc) {
    }

    virtual ~SyntaxTreeInterface() = default;

    // 获取节点类型
    [[nodiscard]] virtual SyntaxTreeType Type() const = 0;

    // 将语法树转储为字符串以便调试
    [[nodiscard]] virtual std::string Dump(int tab) const = 0;

    // 获取节点在源码中的位置
    [[nodiscard]] const SyntaxTreeLocation &Loc() const {
        return loc_;
    }

    void SetLoc(const SyntaxTreeLocation &loc) {
        loc_ = loc;
    }

    void SetEvalType(const InferredType type) {
        eval_type_ = type;
    }

    [[nodiscard]] InferredType EvalType() const {
        return eval_type_;
    }

protected:
    // 生成缩进字符串
    [[nodiscard]] std::string GenTab(int tab) const {
        std::string str;
        for (int i = 0; i < tab; ++i) {
            str += "  ";
        }
        return str;
    }

    // 生成位置信息字符串（格式：行:列）
    [[nodiscard]] std::string LocStr() const {
        std::string str;
        str += std::to_string(loc_.begin.line);
        str += ":";
        str += std::to_string(loc_.begin.column);
        return str;
    }

private:
    // 节点位置信息
    SyntaxTreeLocation loc_;
    InferredType eval_type_ = T_UNKNOWN;
};

// 语法树智能指针类型
using SyntaxTreeInterfacePtr = std::shared_ptr<SyntaxTreeInterface>;

// 空节点
class SyntaxTreeEmpty final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeEmpty(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Empty;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;
};

// 代码块节点（由多个语句组成）
class SyntaxTreeBlock final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeBlock(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeBlock() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Block;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 向块中添加一条语句
    void AddStmt(const SyntaxTreeInterfacePtr &stmt) {
        stmts_.push_back(stmt);
    }

    // 获取所有语句
    [[nodiscard]] const std::vector<SyntaxTreeInterfacePtr> &Stmts() const {
        return stmts_;
    }

    // 批量设置语句
    void SetStmts(const std::vector<SyntaxTreeInterfacePtr> &stmts) {
        stmts_ = stmts;
    }

private:
    std::vector<SyntaxTreeInterfacePtr> stmts_;
};

// 标签节点（用于 goto 目标）
class SyntaxTreeLabel final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeLabel(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeLabel() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Label;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置标签名称
    void SetName(const std::string &name) {
        name_ = name;
    }

private:
    std::string name_;
};

// 返回语句节点
class SyntaxTreeReturn final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeReturn(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeReturn() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Return;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置返回表达式列表
    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    // 获取返回表达式列表
    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

private:
    SyntaxTreeInterfacePtr explist_;
};

// 赋值语句节点
class SyntaxTreeAssign final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeAssign(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeAssign() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Assign;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置变量列表
    void SetVarlist(const SyntaxTreeInterfacePtr &varlist) {
        varlist_ = varlist;
    }

    // 设置值列表
    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    // 获取变量列表
    [[nodiscard]] SyntaxTreeInterfacePtr Varlist() const {
        return varlist_;
    }

    // 获取值列表
    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

private:
    SyntaxTreeInterfacePtr varlist_;
    SyntaxTreeInterfacePtr explist_;
};

// 变量列表节点
class SyntaxTreeVarlist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeVarlist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeVarlist() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::VarList;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 添加一个变量
    void AddVar(const SyntaxTreeInterfacePtr &var) {
        vars_.push_back(var);
    }

    // 获取所有变量列表
    std::vector<SyntaxTreeInterfacePtr> &Vars() {
        return vars_;
    }

private:
    std::vector<SyntaxTreeInterfacePtr> vars_;
};

// 表达式列表节点
class SyntaxTreeExplist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeExplist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeExplist() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ExpList;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 添加一个表达式
    void AddExp(const SyntaxTreeInterfacePtr &exp) {
        exps_.push_back(exp);
    }

    // 获取所有表达式列表
    std::vector<SyntaxTreeInterfacePtr> &Exps() {
        return exps_;
    }

private:
    std::vector<SyntaxTreeInterfacePtr> exps_;
};

// 变量引用节点
class SyntaxTreeVar final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeVar(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeVar() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Var;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置变量名
    void SetName(const std::string &name) {
        name_ = name;
    }

    // 设置前缀表达式（如 a.b 中的 a）
    void SetPrefixexp(const SyntaxTreeInterfacePtr &prefixexp) {
        prefixexp_ = prefixexp;
    }

    // 获取前缀表达式
    SyntaxTreeInterfacePtr GetPrefixexp() {
        return prefixexp_;
    }

    // 设置键表达式（如 a[b] 中的 b）
    void SetExp(const SyntaxTreeInterfacePtr &exp) {
        exp_ = exp;
    }

    // 获取键表达式
    SyntaxTreeInterfacePtr GetExp() {
        return exp_;
    }

    // 设置变量访问类型
    void SetType(const std::string &type) {
        type_ = type;
    }

    // 获取变量名
    [[nodiscard]] std::string GetName() const {
        return name_;
    }

    // 获取前缀表达式（只读）
    [[nodiscard]] SyntaxTreeInterfacePtr GetPrefixexp() const {
        return prefixexp_;
    }

    // 获取键表达式（只读）
    [[nodiscard]] SyntaxTreeInterfacePtr GetExp() const {
        return exp_;
    }

    // 获取变量访问类型
    [[nodiscard]] std::string GetType() const {
        return type_;
    }

private:
    std::string name_;
    SyntaxTreeInterfacePtr exp_;
    SyntaxTreeInterfacePtr prefixexp_;
    std::string type_;
};

// 函数调用节点
class SyntaxTreeFunctioncall final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFunctioncall(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFunctioncall() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FunctionCall;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置函数调用的前缀表达式
    void SetPrefixexp(const SyntaxTreeInterfacePtr &prefixexp) {
        prefixexp_ = prefixexp;
    }

    // 设置函数参数
    void SetArgs(const SyntaxTreeInterfacePtr &args) {
        args_ = args;
    }

    // 设置函数名
    void SetName(const std::string &name) {
        name_ = name;
    }

    // 获取前缀表达式
    [[nodiscard]] SyntaxTreeInterfacePtr prefixexp() const {
        return prefixexp_;
    }

    // 获取函数参数
    [[nodiscard]] SyntaxTreeInterfacePtr Args() const {
        return args_;
    }

    // 获取函数名
    [[nodiscard]] std::string Name() const {
        return name_;
    }

private:
    SyntaxTreeInterfacePtr prefixexp_;
    SyntaxTreeInterfacePtr args_;
    std::string name_;
};

// 表构造节点
class SyntaxTreeTableconstructor final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeTableconstructor(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeTableconstructor() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::TableConstructor;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置表字段列表
    void SetFieldlist(const SyntaxTreeInterfacePtr &fieldlist) {
        fieldlist_ = fieldlist;
    }

    // 获取表字段列表
    [[nodiscard]] SyntaxTreeInterfacePtr Fieldlist() const {
        return fieldlist_;
    }

private:
    SyntaxTreeInterfacePtr fieldlist_;
};

// 表字段列表节点
class SyntaxTreeFieldlist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFieldlist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFieldlist() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FieldList;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 添加一个字段定义
    void AddField(const SyntaxTreeInterfacePtr &field) {
        fields_.push_back(field);
    }

    // 获取所有字段列表
    std::vector<SyntaxTreeInterfacePtr> &Fields() {
        return fields_;
    }

private:
    std::vector<SyntaxTreeInterfacePtr> fields_;
};

// 表字段定义节点
class SyntaxTreeField final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeField(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeField() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Field;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置字段键表达式
    void SetKey(const SyntaxTreeInterfacePtr &key) {
        key_ = key;
    }

    // 设置字段值表达式
    void SetValue(const SyntaxTreeInterfacePtr &value) {
        value_ = value;
    }

    // 设置字段名
    void SetName(const std::string &name) {
        name_ = name;
    }

    // 设置字段类型
    void SetType(const std::string &type) {
        type_ = type;
    }

    // 获取字段键表达式
    [[nodiscard]] SyntaxTreeInterfacePtr Key() const {
        return key_;
    }

    // 获取字段值表达式
    [[nodiscard]] SyntaxTreeInterfacePtr Value() const {
        return value_;
    }

    // 获取字段名
    [[nodiscard]] std::string Name() const {
        return name_;
    }

    // 获取字段类型
    [[nodiscard]] std::string GetType() const {
        return type_;
    }

private:
    SyntaxTreeInterfacePtr key_;
    SyntaxTreeInterfacePtr value_;
    std::string name_;
    std::string type_;
};

// break 语句节点
class SyntaxTreeBreak final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeBreak(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeBreak() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Break;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;
};

// goto 语句节点
class SyntaxTreeGoto final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeGoto(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeGoto() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Goto;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置跳转目标标签名
    void SetLabel(const std::string &label) {
        label_ = label;
    }

private:
    std::string label_;
};

// while 循环节点
class SyntaxTreeWhile final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeWhile(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeWhile() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::While;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置循环条件
    void SetExp(const SyntaxTreeInterfacePtr &exp) {
        exp_ = exp;
    }

    // 设置循环体
    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    // 获取循环条件
    [[nodiscard]] SyntaxTreeInterfacePtr Exp() const {
        return exp_;
    }

    // 获取循环体
    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

private:
    SyntaxTreeInterfacePtr exp_;
    SyntaxTreeInterfacePtr block_;
};

// repeat-until 循环节点
class SyntaxTreeRepeat final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeRepeat(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeRepeat() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Repeat;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置循环结束条件
    void SetExp(const SyntaxTreeInterfacePtr &exp) {
        exp_ = exp;
    }

    // 设置循环体
    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    // 获取循环结束条件
    [[nodiscard]] SyntaxTreeInterfacePtr Exp() const {
        return exp_;
    }

    // 获取循环体
    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

private:
    SyntaxTreeInterfacePtr exp_;
    SyntaxTreeInterfacePtr block_;
};

// if 条件分支节点
class SyntaxTreeIf final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeIf(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeIf() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::If;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置 if 条件表达式
    void SetExp(const SyntaxTreeInterfacePtr &exp) {
        exp_ = exp;
    }

    // 设置 if 代码块
    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    // 设置 elseif 列表
    void SetElseiflist(const SyntaxTreeInterfacePtr &elseifs) {
        elseifs_ = elseifs;
    }

    // 设置 else 代码块
    void SetElseBlock(const SyntaxTreeInterfacePtr &elseblock) {
        elseblock_ = elseblock;
    }

    // 获取 if 条件表达式
    [[nodiscard]] SyntaxTreeInterfacePtr Exp() const {
        return exp_;
    }

    // 获取 if 代码块
    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

    // 获取 elseif 列表
    [[nodiscard]] SyntaxTreeInterfacePtr ElseIfs() const {
        return elseifs_;
    }

    // 获取 else 代码块
    [[nodiscard]] SyntaxTreeInterfacePtr ElseBlock() const {
        return elseblock_;
    }

private:
    SyntaxTreeInterfacePtr exp_;
    SyntaxTreeInterfacePtr block_;
    SyntaxTreeInterfacePtr elseifs_;
    SyntaxTreeInterfacePtr elseblock_;
};

// elseif 列表节点
class SyntaxTreeElseiflist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeElseiflist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeElseiflist() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ElseIfList;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 添加 elseif 条件
    void AddElseifExpr(const SyntaxTreeInterfacePtr &exp) {
        exps_.push_back(exp);
    }

    // 添加 elseif 代码块
    void AddElseifBlock(const SyntaxTreeInterfacePtr &block) {
        blocks_.push_back(block);
    }

    // 获取 elseif 数量
    [[nodiscard]] size_t ElseifSize() const {
        return exps_.size();
    }

    // 获取所有 elseif 条件表达式列表
    [[nodiscard]] const std::vector<SyntaxTreeInterfacePtr> &ElseifExps() const {
        return exps_;
    }

    // 获取所有 elseif 代码块列表
    [[nodiscard]] const std::vector<SyntaxTreeInterfacePtr> &ElseifBlocks() const {
        return blocks_;
    }

    // 获取指定索引的 elseif 条件
    [[nodiscard]] SyntaxTreeInterfacePtr ElseifExp(size_t idx) const {
        return exps_[idx];
    }

    // 获取指定索引的 elseif 代码块
    [[nodiscard]] SyntaxTreeInterfacePtr ElseifBlock(size_t idx) const {
        return blocks_[idx];
    }

private:
    std::vector<SyntaxTreeInterfacePtr> exps_;
    std::vector<SyntaxTreeInterfacePtr> blocks_;
};

// 数值型 for 循环节点
class SyntaxTreeForLoop final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeForLoop(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeForLoop() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ForLoop;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置循环变量名
    void SetName(const std::string &name) {
        name_ = name;
    }

    // 设置循环体
    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    // 设置循环起始值
    void SetExpBegin(const SyntaxTreeInterfacePtr &exp) {
        exp_begin_ = exp;
    }

    // 设置循环终止值
    void SetExpEnd(const SyntaxTreeInterfacePtr &exp) {
        exp_end_ = exp;
    }

    // 设置步长表达式
    void SetExpStep(const SyntaxTreeInterfacePtr &exp) {
        exp_step_ = exp;
    }

    // 获取循环变量名
    [[nodiscard]] std::string Name() const {
        return name_;
    }

    // 获取循环体
    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

    // 获取循环起始值
    [[nodiscard]] SyntaxTreeInterfacePtr ExpBegin() const {
        return exp_begin_;
    }

    // 获取循环终止值
    [[nodiscard]] SyntaxTreeInterfacePtr ExpEnd() const {
        return exp_end_;
    }

    // 获取步长表达式
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

// 泛型 for 循环节点
class SyntaxTreeForIn final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeForIn(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeForIn() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ForIn;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置循环变量列表
    void SetNamelist(const SyntaxTreeInterfacePtr &namelist) {
        namelist_ = namelist;
    }

    // 设置迭代器表达式列表
    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    // 设置循环体
    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    // 获取循环变量列表
    [[nodiscard]] SyntaxTreeInterfacePtr Namelist() const {
        return namelist_;
    }

    // 获取迭代器表达式列表
    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

    // 获取循环体
    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

private:
    SyntaxTreeInterfacePtr namelist_;
    SyntaxTreeInterfacePtr explist_;
    SyntaxTreeInterfacePtr block_;
};

// 名称列表节点
class SyntaxTreeNamelist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeNamelist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeNamelist() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::NameList;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 添加名称
    void AddName(const std::string &name) {
        names_.push_back(name);
    }

    // 添加名称属性（如 <const>）
    void AddAttrib(const std::string &attrib) {
        attrib_.push_back(attrib);
    }

    // 获取所有名称列表
    [[nodiscard]] const std::vector<std::string> &Names() const {
        return names_;
    }

private:
    std::vector<std::string> names_;
    std::vector<std::string> attrib_;
};

// 全局函数定义节点
class SyntaxTreeFunction final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFunction(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFunction() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Function;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置函数全名
    void SetFuncname(const SyntaxTreeInterfacePtr &funcname) {
        funcname_ = funcname;
    }

    // 设置函数体
    void SetFuncbody(const SyntaxTreeInterfacePtr &funcbody) {
        funcbody_ = funcbody;
    }

    // 获取函数全名
    [[nodiscard]] SyntaxTreeInterfacePtr Funcname() const {
        return funcname_;
    }

    // 获取函数体
    [[nodiscard]] SyntaxTreeInterfacePtr Funcbody() const {
        return funcbody_;
    }

private:
    SyntaxTreeInterfacePtr funcname_;
    SyntaxTreeInterfacePtr funcbody_;
};

// 函数全名列表节点（如 a.b.c）
class SyntaxTreeFuncnamelist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFuncnamelist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFuncnamelist() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FuncNameList;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 添加函数名组件
    void AddName(const std::string &funcname) {
        funcnames_.push_back(funcname);
    }

    // 获取函数名组件列表
    [[nodiscard]] const std::vector<std::string> &Funcnames() const {
        return funcnames_;
    }

private:
    std::vector<std::string> funcnames_;
};

// 函数全名节点
class SyntaxTreeFuncname final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFuncname(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFuncname() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FuncName;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置函数全名列表
    void SetFuncNameList(const SyntaxTreeInterfacePtr &funcnamelist) {
        funcnamelist_ = funcnamelist;
    }

    // 设置冒号名（用于 a:b 形式）
    void SetColonName(const std::string &name) {
        colon_name_ = name;
    }

    // 获取函数全名列表
    [[nodiscard]] SyntaxTreeInterfacePtr FuncNameList() const {
        return funcnamelist_;
    }

    // 获取冒号名
    [[nodiscard]] std::string ColonName() const {
        return colon_name_;
    }

private:
    SyntaxTreeInterfacePtr funcnamelist_;
    std::string colon_name_;
};

// 函数体节点
class SyntaxTreeFuncbody final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFuncbody(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFuncbody() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FuncBody;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置函数形参列表
    void SetParlist(const SyntaxTreeInterfacePtr &parlist) {
        parlist_ = parlist;
    }

    // 设置函数体代码块
    void SetBlock(const SyntaxTreeInterfacePtr &block) {
        block_ = block;
    }

    // 获取函数形参列表
    [[nodiscard]] SyntaxTreeInterfacePtr Parlist() const {
        return parlist_;
    }

    // 获取函数体代码块
    [[nodiscard]] SyntaxTreeInterfacePtr Block() const {
        return block_;
    }

private:
    SyntaxTreeInterfacePtr parlist_;
    SyntaxTreeInterfacePtr block_;
};

// 匿名函数定义节点
class SyntaxTreeFunctiondef final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeFunctiondef(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeFunctiondef() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::FunctionDef;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置函数体内容
    void SetFuncbody(const SyntaxTreeInterfacePtr &funcbody) {
        funcbody_ = funcbody;
    }

    // 获取函数体内容
    [[nodiscard]] SyntaxTreeInterfacePtr Funcbody() const {
        return funcbody_;
    }

private:
    SyntaxTreeInterfacePtr funcbody_;
};

// 函数形参列表节点
class SyntaxTreeParlist final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeParlist(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeParlist() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::ParList;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置形参名称列表
    void SetNamelist(const SyntaxTreeInterfacePtr &namelist) {
        namelist_ = namelist;
    }

    // 设置是否支持变长参数
    void SetVarParams(bool VarParams) {
        var_params_ = VarParams;
    }

    // 获取形参名称列表
    [[nodiscard]] SyntaxTreeInterfacePtr Namelist() const {
        return namelist_;
    }

    // 获取是否支持变长参数
    [[nodiscard]] bool VarParams() const {
        return var_params_;
    }

private:
    SyntaxTreeInterfacePtr namelist_;
    bool var_params_ = false;
};

// 局部函数定义节点
class SyntaxTreeLocalFunction final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeLocalFunction(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeLocalFunction() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::LocalFunction;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置局部函数名
    void SetName(const std::string &name) {
        name_ = name;
    }

    // 设置函数体
    void SetFuncbody(const SyntaxTreeInterfacePtr &funcbody) {
        funcbody_ = funcbody;
    }

    // 获取局部函数名
    [[nodiscard]] std::string Name() const {
        return name_;
    }

    // 获取函数体
    [[nodiscard]] SyntaxTreeInterfacePtr Funcbody() const {
        return funcbody_;
    }

private:
    std::string name_;
    SyntaxTreeInterfacePtr funcbody_;
};

// 局部变量定义节点
class SyntaxTreeLocalVar final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeLocalVar(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeLocalVar() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::LocalVar;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置局部变量名列表
    void SetNamelist(const SyntaxTreeInterfacePtr &namelist) {
        namelist_ = namelist;
    }

    // 设置局部变量初始值列表
    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    // 获取局部变量名列表
    [[nodiscard]] SyntaxTreeInterfacePtr Namelist() const {
        return namelist_;
    }

    // 获取局部变量初始值列表
    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

private:
    SyntaxTreeInterfacePtr namelist_;
    SyntaxTreeInterfacePtr explist_;
};

// 表达式节点
class SyntaxTreeExp final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeExp(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeExp() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Exp;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置表达式字面量类型
    void SetType(const std::string &type) {
        type_ = type;
    }

    // 设置表达式字面量值
    void SetValue(const std::string &value) {
        value_ = value;
    }

    // 设置二元运算左操作数
    void SetLeft(const SyntaxTreeInterfacePtr &left) {
        left_ = left;
    }

    // 设置运算符（一元或二元）
    void SetOp(const SyntaxTreeInterfacePtr &oper) {
        op_ = oper;
    }

    // 设置二元运算右操作数
    void SetRight(const SyntaxTreeInterfacePtr &right) {
        right_ = right;
    }

    // 获取表达式字面量类型
    [[nodiscard]] std::string ExpType() const {
        return type_;
    }

    // 获取表达式字面量值
    [[nodiscard]] std::string ExpValue() const {
        return value_;
    }

    // 获取二元运算左操作数
    [[nodiscard]] SyntaxTreeInterfacePtr Left() const {
        return left_;
    }

    // 获取运算符
    [[nodiscard]] SyntaxTreeInterfacePtr Op() const {
        return op_;
    }

    // 获取二元运算右操作数
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

// 二元运算符节点
class SyntaxTreeBinop final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeBinop(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeBinop() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Binop;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置二元运算符名
    void SetOp(const std::string &oper) {
        op_ = oper;
    }

    // 获取二元运算符名
    [[nodiscard]] std::string GetOp() const {
        return op_;
    }

private:
    std::string op_;
};

// 一元运算符节点
class SyntaxTreeUnop final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeUnop(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeUnop() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Unop;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置一元运算符名
    void SetOp(const std::string &oper) {
        op_ = oper;
    }

    // 获取一元运算符名
    [[nodiscard]] std::string GetOp() const {
        return op_;
    }

private:
    std::string op_;
};

// 函数参数节点
class SyntaxTreeArgs final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreeArgs(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreeArgs() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::Args;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置参数表达式列表（针对 (exp1, exp2)）
    void SetExplist(const SyntaxTreeInterfacePtr &explist) {
        explist_ = explist;
    }

    // 设置表参数（针对 {k=v}）
    void SetTableconstructor(const SyntaxTreeInterfacePtr &tableconstructor) {
        tableconstructor_ = tableconstructor;
    }

    // 设置字符串参数（针对 "string"）
    void SetString(const SyntaxTreeInterfacePtr &string) {
        string_ = string;
    }

    // 设置参数类型分类
    void SetType(const std::string &type) {
        type_ = type;
    }

    // 获取参数类型分类
    [[nodiscard]] std::string GetType() const {
        return type_;
    }

    // 获取参数表达式列表
    [[nodiscard]] SyntaxTreeInterfacePtr Explist() const {
        return explist_;
    }

    // 获取表参数内容
    [[nodiscard]] SyntaxTreeInterfacePtr Tableconstructor() const {
        return tableconstructor_;
    }

    // 获取字符串参数内容
    [[nodiscard]] SyntaxTreeInterfacePtr String() const {
        return string_;
    }

private:
    SyntaxTreeInterfacePtr explist_;
    SyntaxTreeInterfacePtr tableconstructor_;
    SyntaxTreeInterfacePtr string_;
    std::string type_;
};

// 前缀表达式节点
class SyntaxTreePrefixexp final : public SyntaxTreeInterface {
public:
    explicit SyntaxTreePrefixexp(const SyntaxTreeLocation &loc) : SyntaxTreeInterface(loc) {
    }

    ~SyntaxTreePrefixexp() override = default;

    // 获取节点类型
    [[nodiscard]] SyntaxTreeType Type() const override {
        return SyntaxTreeType::PrefixExp;
    }

    // 转储节点信息
    [[nodiscard]] std::string Dump(int tab) const override;

    // 设置内部表达式值
    void SetValue(const SyntaxTreeInterfacePtr &value) {
        value_ = value;
    }

    // 设置前缀表达式类型分类
    void SetType(const std::string &type) {
        type_ = type;
    }

    // 获取内部表达式值
    [[nodiscard]] SyntaxTreeInterfacePtr GetValue() const {
        return value_;
    }

    // 获取前缀表达式类型分类
    [[nodiscard]] std::string GetType() const {
        return type_;
    }

private:
    SyntaxTreeInterfacePtr value_;
    std::string type_;
};

// 语法树遍历函数定义
using WalkSyntaxTreeFunc = std::function<void(const SyntaxTreeInterfacePtr &)>;

// 深度优先遍历语法树
void WalkSyntaxTree(const SyntaxTreeInterfacePtr &node, const WalkSyntaxTreeFunc &func);

}// namespace fakelua
