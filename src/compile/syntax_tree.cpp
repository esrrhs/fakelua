#include "syntax_tree.h"

namespace fakelua {

// 转储空节点信息
std::string SyntaxTreeEmpty::Dump(int tab) const {
    return GenTab(tab) + "(empty)[" + LocStr() + "]\n";
}

// 转储代码块及其包含的所有语句
std::string SyntaxTreeBlock::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(block)[" + LocStr() + "]\n";
    for (auto &stmt: stmts_) {
        str += stmt->Dump(tab + 1);
    }
    return str;
}

// 转储标签名
std::string SyntaxTreeLabel::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + name_ + "(label)[" + LocStr() + "]\n";
    return str;
}

// 转储返回语句及其表达式列表
std::string SyntaxTreeReturn::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(return)[" + LocStr() + "]\n";
    if (explist_) {
        str += explist_->Dump(tab + 1);
    }
    return str;
}

// 转储赋值语句，包括变量列表和表达式列表
std::string SyntaxTreeAssign::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(assign)[" + LocStr() + "]\n";
    str += varlist_->Dump(tab + 1);
    str += explist_->Dump(tab + 1);
    return str;
}

// 转储变量列表中的所有变量
std::string SyntaxTreeVarlist::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(varlist)[" + LocStr() + "]\n";
    for (auto &var: vars_) {
        str += var->Dump(tab + 1);
    }
    return str;
}

// 转储表达式列表中的所有表达式
std::string SyntaxTreeExplist::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(explist)[" + LocStr() + "]\n";
    for (auto &exp: exps_) {
        str += exp->Dump(tab + 1);
    }
    return str;
}

// 转储变量引用信息（简单变量、下标访问或成员访问）
std::string SyntaxTreeVar::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(var)[" + LocStr() + "]\n";
    if (var_kind_ == VarKind::kSimple) {
        str += GenTab(tab + 1) + "type: simple\n";
        str += GenTab(tab + 1) + "name: " + name_ + "\n";
    } else if (var_kind_ == VarKind::kSquare) {
        str += GenTab(tab + 1) + "type: square\n";
        str += prefixexp_->Dump(tab + 1);
        str += exp_->Dump(tab + 1);
    } else if (var_kind_ == VarKind::kDot) {
        str += GenTab(tab + 1) + "type: dot\n";
        str += prefixexp_->Dump(tab + 1);
        str += GenTab(tab + 1) + "name: " + name_ + "\n";
    } else {
        DEBUG_ASSERT(false && "unknown var type");
    }
    return str;
}

// 转储函数调用信息
std::string SyntaxTreeFunctioncall::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(functioncall)[" + LocStr() + "]\n";
    str += prefixexp_->Dump(tab + 1);
    if (!name_.empty()) {
        str += GenTab(tab + 1) + "name: " + name_ + "\n";
    }
    str += args_->Dump(tab + 1);
    return str;
}

// 转储表构造器信息
std::string SyntaxTreeTableconstructor::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(tableconstructor)[" + LocStr() + "]\n";
    if (fieldlist_) {
        str += fieldlist_->Dump(tab + 1);
    }
    return str;
}

// 转储表字段列表
std::string SyntaxTreeFieldlist::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(fieldlist)[" + LocStr() + "]\n";
    for (auto &field: fields_) {
        str += field->Dump(tab + 1);
    }
    return str;
}

// 转储表字段定义信息（数组部分或对象部分）
std::string SyntaxTreeField::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(field)[" + LocStr() + "]\n";
    if (field_kind_ == FieldKind::kArray) {
        str += GenTab(tab + 1) + "type: array\n";
        if (key_) {
            str += key_->Dump(tab + 1);
        }
        str += value_->Dump(tab + 1);
    } else if (field_kind_ == FieldKind::kObject) {
        str += GenTab(tab + 1) + "type: object\n";
        str += GenTab(tab + 1) + "name: " + name_ + "\n";
        str += value_->Dump(tab + 1);
    } else {
        DEBUG_ASSERT(false && "unknown field type");
    }
    return str;
}

// 转储 break 语句
std::string SyntaxTreeBreak::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(break)[" + LocStr() + "]\n";
    return str;
}

// 转储 goto 语句及目标标签
std::string SyntaxTreeGoto::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(goto)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "label: " + label_ + "\n";
    return str;
}

// 转储 while 循环
std::string SyntaxTreeWhile::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(while)[" + LocStr() + "]\n";
    str += exp_->Dump(tab + 1);
    str += block_->Dump(tab + 1);
    return str;
}

// 转储 repeat-until 循环
std::string SyntaxTreeRepeat::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(repeat)[" + LocStr() + "]\n";
    str += block_->Dump(tab + 1);
    str += exp_->Dump(tab + 1);
    return str;
}

// 转储 if 条件分支语句
std::string SyntaxTreeIf::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(if)[" + LocStr() + "]\n";
    str += exp_->Dump(tab + 1);
    str += block_->Dump(tab + 1);
    str += elseifs_->Dump(tab + 1);
    if (elseblock_) {
        str += elseblock_->Dump(tab + 1);
    }
    return str;
}

// 转储 elseif 列表信息
std::string SyntaxTreeElseiflist::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(elseif)[" + LocStr() + "]\n";
    for (int i = 0; i < static_cast<int>(exps_.size()); ++i) {
        str += exps_[i]->Dump(tab + 1);
        str += blocks_[i]->Dump(tab + 1);
    }
    return str;
}

// 转储数值型 for 循环
std::string SyntaxTreeForLoop::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(for_loop)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "name: " + name_ + "\n";
    str += exp_begin_->Dump(tab + 1);
    str += exp_end_->Dump(tab + 1);
    if (exp_step_) {
        str += exp_step_->Dump(tab + 1);
    }
    str += block_->Dump(tab + 1);
    return str;
}

// 转储泛型 for 循环
std::string SyntaxTreeForIn::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(for_in)[" + LocStr() + "]\n";
    str += namelist_->Dump(tab + 1);
    str += explist_->Dump(tab + 1);
    str += block_->Dump(tab + 1);
    return str;
}

// 转储变量名及属性列表
std::string SyntaxTreeNamelist::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(namelist)[" + LocStr() + "]\n";
    size_t index = 0;
    for (auto &name: names_) {
        str += GenTab(tab + 1) + "name: " + name + "\n";
        if (index < attrib_.size() && !attrib_[index].empty()) {
            str += GenTab(tab + 1) + "attrib: " + attrib_[index] + "\n";
        }
        ++index;
    }
    return str;
}

// 转储全局函数定义
std::string SyntaxTreeFunction::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(function)[" + LocStr() + "]\n";
    str += funcname_->Dump(tab + 1);
    str += funcbody_->Dump(tab + 1);
    return str;
}

// 转储函数名组件列表
std::string SyntaxTreeFuncnamelist::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(funcnamelist)[" + LocStr() + "]\n";
    for (auto &name: funcnames_) {
        str += GenTab(tab + 1) + "name: " + name + "\n";
    }
    return str;
}

// 转储函数全名，包括冒号后的方法名
std::string SyntaxTreeFuncname::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(funcname)[" + LocStr() + "]\n";
    str += funcnamelist_->Dump(tab + 1);
    str += GenTab(tab + 1) + "ColonName: " + colon_name_ + "\n";
    return str;
}

// 转储函数体，包括形参列表和代码块
std::string SyntaxTreeFuncbody::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(funcbody)[" + LocStr() + "]\n";
    if (parlist_) {
        str += parlist_->Dump(tab + 1);
    }
    str += block_->Dump(tab + 1);
    return str;
}

// 转储匿名函数定义
std::string SyntaxTreeFunctiondef::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(functiondef)[" + LocStr() + "]\n";
    str += funcbody_->Dump(tab + 1);
    return str;
}

// 转储形参列表及变长参数信息
std::string SyntaxTreeParlist::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(parlist)[" + LocStr() + "]\n";
    if (namelist_) {
        str += namelist_->Dump(tab + 1);
    }
    if (var_params_) {
        str += GenTab(tab + 1) + "VarParams: " + std::to_string(var_params_) + "\n";
    }
    return str;
}

// 转储局部函数定义
std::string SyntaxTreeLocalFunction::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(local_function)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "name: " + name_ + "\n";
    str += funcbody_->Dump(tab + 1);
    return str;
}

// 转储局部变量定义及其初始化列表
std::string SyntaxTreeLocalVar::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(local_var)[" + LocStr() + "]\n";
    str += namelist_->Dump(tab + 1);
    if (explist_) {
        str += explist_->Dump(tab + 1);
    }
    return str;
}

// 转储表达式及其各操作数
std::string SyntaxTreeExp::Dump(int tab) const {
    static constexpr const char *kExpKindNames[] = {"nil", "true", "false", "number", "string", "VarParams", "functiondef", "prefixexp", "tableconstructor", "binop", "unop"};
    std::string str;
    str += GenTab(tab) + "(exp)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "type: " + kExpKindNames[static_cast<int>(exp_kind_)] + "\n";
    str += GenTab(tab + 1) + "value: " + value_ + "\n";
    if (left_) {
        str += left_->Dump(tab + 1);
    }
    if (op_) {
        str += op_->Dump(tab + 1);
    }
    if (right_) {
        str += right_->Dump(tab + 1);
    }
    return str;
}

// 转储二元运算符
std::string SyntaxTreeBinop::Dump(int tab) const {
    static constexpr const char *kBinOpNames[] = {"PLUS",       "MINUS",  "STAR", "SLASH",      "DOUBLE_SLASH", "POW",        "MOD",   "BITAND",    "XOR", "BITOR", "RIGHT_SHIFT",
                                                  "LEFT_SHIFT", "CONCAT", "LESS", "LESS_EQUAL", "MORE",         "MORE_EQUAL", "EQUAL", "NOT_EQUAL", "AND", "OR"};
    std::string str;
    str += GenTab(tab) + "(binop)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "op: " + kBinOpNames[static_cast<int>(op_kind_)] + "\n";
    return str;
}

// 转储一元运算符
std::string SyntaxTreeUnop::Dump(int tab) const {
    static constexpr const char *kUnOpNames[] = {"MINUS", "NOT", "NUMBER_SIGN", "BITNOT"};
    std::string str;
    str += GenTab(tab) + "(unop)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "op: " + kUnOpNames[static_cast<int>(op_kind_)] + "\n";
    return str;
}

// 转储函数参数列表
std::string SyntaxTreeArgs::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(args)[" + LocStr() + "]\n";
    if (args_kind_ == ArgsKind::kExpList) {
        str += explist_->Dump(tab + 1);
    } else if (args_kind_ == ArgsKind::kTableConstructor) {
        str += tableconstructor_->Dump(tab + 1);
    } else if (args_kind_ == ArgsKind::kString) {
        str += string_->Dump(tab + 1);
    } else if (args_kind_ == ArgsKind::kEmpty) {
        str += GenTab(tab + 1) + "empty" + "\n";
    } else {
        DEBUG_ASSERT(false && "unknown args type");
    }
    return str;
}

// 转储前缀表达式
std::string SyntaxTreePrefixexp::Dump(int tab) const {
    static constexpr const char *kPrefixKindNames[] = {"var", "functioncall", "exp"};
    std::string str;
    str += GenTab(tab) + "(prefixexp)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "type: " + kPrefixKindNames[static_cast<int>(prefix_kind_)] + "\n";
    str += value_->Dump(tab + 1);
    return str;
}

// WalkSyntaxTree delegates to WalkSyntaxTreePruned — the canonical tree walker.
// Use a never-pruning callback that always returns true, so no subtree is skipped.
void WalkSyntaxTree(const SyntaxTreeInterfacePtr &node, const WalkSyntaxTreeFunc &func) {
    WalkSyntaxTreePruned(node, [&func](const SyntaxTreeInterfacePtr &n) {
        func(n);
        return true;
    });
}

void WalkSyntaxTreePruned(const SyntaxTreeInterfacePtr &node, const WalkSyntaxTreePrunedFunc &func) {
    if (!node) {
        return;
    }
    if (!func(node)) {
        return;
    }
    switch (node->Type()) {
        case SyntaxTreeType::Empty:
        case SyntaxTreeType::Label:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::NameList:
        case SyntaxTreeType::FuncNameList:
        case SyntaxTreeType::Binop:
        case SyntaxTreeType::Unop:
            break;
        case SyntaxTreeType::Block: {
            auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (auto &stmt: block->Stmts()) {
                WalkSyntaxTreePruned(stmt, func);
            }
            break;
        }
        case SyntaxTreeType::Return:
            WalkSyntaxTreePruned(std::dynamic_pointer_cast<SyntaxTreeReturn>(node)->Explist(), func);
            break;
        case SyntaxTreeType::Assign: {
            auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            WalkSyntaxTreePruned(assign->Varlist(), func);
            WalkSyntaxTreePruned(assign->Explist(), func);
            break;
        }
        case SyntaxTreeType::VarList: {
            auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(node);
            for (auto &var: varlist->Vars()) {
                WalkSyntaxTreePruned(var, func);
            }
            break;
        }
        case SyntaxTreeType::ExpList: {
            auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            for (auto &exp: explist->Exps()) {
                WalkSyntaxTreePruned(exp, func);
            }
            break;
        }
        case SyntaxTreeType::Var: {
            auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
            WalkSyntaxTreePruned(var->GetExp(), func);
            WalkSyntaxTreePruned(var->GetPrefixexp(), func);
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            auto functioncall = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            WalkSyntaxTreePruned(functioncall->prefixexp(), func);
            WalkSyntaxTreePruned(functioncall->Args(), func);
            break;
        }
        case SyntaxTreeType::TableConstructor:
            WalkSyntaxTreePruned(std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node)->Fieldlist(), func);
            break;
        case SyntaxTreeType::FieldList: {
            auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            for (auto &field: fieldlist->Fields()) {
                WalkSyntaxTreePruned(field, func);
            }
            break;
        }
        case SyntaxTreeType::Field: {
            auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            WalkSyntaxTreePruned(field->Key(), func);
            WalkSyntaxTreePruned(field->Value(), func);
            break;
        }
        case SyntaxTreeType::While: {
            auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            WalkSyntaxTreePruned(while_node->Exp(), func);
            WalkSyntaxTreePruned(while_node->Block(), func);
            break;
        }
        case SyntaxTreeType::Repeat: {
            auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            WalkSyntaxTreePruned(rep->Block(), func);
            WalkSyntaxTreePruned(rep->Exp(), func);
            break;
        }
        case SyntaxTreeType::If: {
            auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            WalkSyntaxTreePruned(if_node->Exp(), func);
            WalkSyntaxTreePruned(if_node->Block(), func);
            WalkSyntaxTreePruned(if_node->ElseIfs(), func);
            WalkSyntaxTreePruned(if_node->ElseBlock(), func);
            break;
        }
        case SyntaxTreeType::ElseIfList: {
            auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
            for (auto &exp: elseifs->ElseifExps()) {
                WalkSyntaxTreePruned(exp, func);
            }
            for (auto &block: elseifs->ElseifBlocks()) {
                WalkSyntaxTreePruned(block, func);
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            WalkSyntaxTreePruned(for_loop->ExpBegin(), func);
            WalkSyntaxTreePruned(for_loop->ExpEnd(), func);
            WalkSyntaxTreePruned(for_loop->ExpStep(), func);
            WalkSyntaxTreePruned(for_loop->Block(), func);
            break;
        }
        case SyntaxTreeType::ForIn: {
            auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            WalkSyntaxTreePruned(for_in->Namelist(), func);
            WalkSyntaxTreePruned(for_in->Explist(), func);
            WalkSyntaxTreePruned(for_in->Block(), func);
            break;
        }
        case SyntaxTreeType::Function: {
            auto func_node = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
            WalkSyntaxTreePruned(func_node->Funcname(), func);
            WalkSyntaxTreePruned(func_node->Funcbody(), func);
            break;
        }
        case SyntaxTreeType::FuncName:
            WalkSyntaxTreePruned(std::dynamic_pointer_cast<SyntaxTreeFuncname>(node)->FuncNameList(), func);
            break;
        case SyntaxTreeType::FuncBody: {
            auto funcbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(node);
            WalkSyntaxTreePruned(funcbody->Parlist(), func);
            WalkSyntaxTreePruned(funcbody->Block(), func);
            break;
        }
        case SyntaxTreeType::FunctionDef:
            WalkSyntaxTreePruned(std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(node)->Funcbody(), func);
            break;
        case SyntaxTreeType::ParList:
            WalkSyntaxTreePruned(std::dynamic_pointer_cast<SyntaxTreeParlist>(node)->Namelist(), func);
            break;
        case SyntaxTreeType::LocalFunction:
            WalkSyntaxTreePruned(std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node)->Funcbody(), func);
            break;
        case SyntaxTreeType::LocalVar: {
            auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            WalkSyntaxTreePruned(local_var->Namelist(), func);
            WalkSyntaxTreePruned(local_var->Explist(), func);
            break;
        }
        case SyntaxTreeType::Exp: {
            auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            WalkSyntaxTreePruned(exp->Left(), func);
            WalkSyntaxTreePruned(exp->Op(), func);
            WalkSyntaxTreePruned(exp->Right(), func);
            break;
        }
        case SyntaxTreeType::Args: {
            auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            WalkSyntaxTreePruned(args->Explist(), func);
            WalkSyntaxTreePruned(args->Tableconstructor(), func);
            WalkSyntaxTreePruned(args->String(), func);
            break;
        }
        case SyntaxTreeType::PrefixExp:
            WalkSyntaxTreePruned(std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node)->GetValue(), func);
            break;
        default:
            ThrowFakeluaException(std::format("WalkSyntaxTreePruned: unknown syntax tree type {}", static_cast<int>(node->Type())));
            break;
    }
}

}// namespace fakelua
