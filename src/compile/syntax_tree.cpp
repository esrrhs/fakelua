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
    if (type_ == "simple") {
        str += GenTab(tab + 1) + "type: simple\n";
        str += GenTab(tab + 1) + "name: " + name_ + "\n";
    } else if (type_ == "square") {
        str += GenTab(tab + 1) + "type: square\n";
        str += prefixexp_->Dump(tab + 1);
        str += exp_->Dump(tab + 1);
    } else if (type_ == "dot") {
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
    if (type_ == "array") {
        str += GenTab(tab + 1) + "type: array\n";
        if (key_) {
            str += key_->Dump(tab + 1);
        }
        str += value_->Dump(tab + 1);
    } else if (type_ == "object") {
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
    std::string str;
    str += GenTab(tab) + "(exp)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "type: " + type_ + "\n";
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
    std::string str;
    str += GenTab(tab) + "(binop)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "op: " + op_ + "\n";
    return str;
}

// 转储一元运算符
std::string SyntaxTreeUnop::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(unop)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "op: " + op_ + "\n";
    return str;
}

// 转储函数参数列表
std::string SyntaxTreeArgs::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(args)[" + LocStr() + "]\n";
    if (type_ == "explist") {
        str += explist_->Dump(tab + 1);
    } else if (type_ == "tableconstructor") {
        str += tableconstructor_->Dump(tab + 1);
    } else if (type_ == "string") {
        str += string_->Dump(tab + 1);
    } else if (type_ == "empty") {
        str += GenTab(tab + 1) + "empty" + "\n";
    } else {
        DEBUG_ASSERT(false && "unknown args type");
    }
    return str;
}

// 转储前缀表达式
std::string SyntaxTreePrefixexp::Dump(int tab) const {
    std::string str;
    str += GenTab(tab) + "(prefixexp)[" + LocStr() + "]\n";
    str += GenTab(tab + 1) + "type: " + type_ + "\n";
    str += value_->Dump(tab + 1);
    return str;
}

// 递归遍历语法树的实现函数
void WalkSyntaxTree(const SyntaxTreeInterfacePtr &node, const WalkSyntaxTreeFunc &func) {
    if (!node) {
        return;
    }
    switch (node->Type()) {
        case SyntaxTreeType::Empty: {
            auto empty = std::dynamic_pointer_cast<SyntaxTreeEmpty>(node);
            func(empty);
            break;
        }
        case SyntaxTreeType::Block: {
            auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            func(block);
            for (auto &stmt: block->Stmts()) {
                WalkSyntaxTree(stmt, func);
            }
            break;
        }
        case SyntaxTreeType::Label: {
            auto label = std::dynamic_pointer_cast<SyntaxTreeLabel>(node);
            func(label);
            break;
        }
        case SyntaxTreeType::Return: {
            auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            func(ret);
            WalkSyntaxTree(ret->Explist(), func);
            break;
        }
        case SyntaxTreeType::Assign: {
            auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            func(assign);
            WalkSyntaxTree(assign->Varlist(), func);
            WalkSyntaxTree(assign->Explist(), func);
            break;
        }
        case SyntaxTreeType::VarList: {
            auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(node);
            func(varlist);
            for (auto &var: varlist->Vars()) {
                WalkSyntaxTree(var, func);
            }
            break;
        }
        case SyntaxTreeType::ExpList: {
            auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            func(explist);
            for (auto &exp: explist->Exps()) {
                WalkSyntaxTree(exp, func);
            }
            break;
        }
        case SyntaxTreeType::Var: {
            auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
            func(var);
            WalkSyntaxTree(var->GetExp(), func);
            WalkSyntaxTree(var->GetPrefixexp(), func);
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            auto functioncall = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            func(functioncall);
            WalkSyntaxTree(functioncall->prefixexp(), func);
            WalkSyntaxTree(functioncall->Args(), func);
            break;
        }
        case SyntaxTreeType::TableConstructor: {
            auto tableconstructor = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
            func(tableconstructor);
            WalkSyntaxTree(tableconstructor->Fieldlist(), func);
            break;
        }
        case SyntaxTreeType::FieldList: {
            auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            func(fieldlist);
            for (auto &field: fieldlist->Fields()) {
                WalkSyntaxTree(field, func);
            }
            break;
        }
        case SyntaxTreeType::Field: {
            auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            func(field);
            WalkSyntaxTree(field->Key(), func);
            WalkSyntaxTree(field->Value(), func);
            break;
        }
        case SyntaxTreeType::Break: {
            auto bre = std::dynamic_pointer_cast<SyntaxTreeBreak>(node);
            func(bre);
            break;
        }
        case SyntaxTreeType::Goto: {
            auto go = std::dynamic_pointer_cast<SyntaxTreeGoto>(node);
            func(go);
            break;
        }
        case SyntaxTreeType::While: {
            auto wh = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            func(wh);
            WalkSyntaxTree(wh->Exp(), func);
            WalkSyntaxTree(wh->Block(), func);
            break;
        }
        case SyntaxTreeType::Repeat: {
            auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            func(rep);
            WalkSyntaxTree(rep->Block(), func);
            WalkSyntaxTree(rep->Exp(), func);
            break;
        }
        case SyntaxTreeType::If: {
            auto i = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            func(i);
            WalkSyntaxTree(i->Exp(), func);
            WalkSyntaxTree(i->Block(), func);
            WalkSyntaxTree(i->ElseIfs(), func);
            WalkSyntaxTree(i->ElseBlock(), func);
            break;
        }
        case SyntaxTreeType::ElseIfList: {
            auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
            func(elseifs);
            for (auto &exp: elseifs->ElseifExps()) {
                WalkSyntaxTree(exp, func);
            }
            for (auto &block: elseifs->ElseifBlocks()) {
                WalkSyntaxTree(block, func);
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            func(for_loop);
            WalkSyntaxTree(for_loop->ExpBegin(), func);
            WalkSyntaxTree(for_loop->ExpEnd(), func);
            WalkSyntaxTree(for_loop->ExpStep(), func);
            WalkSyntaxTree(for_loop->Block(), func);
            break;
        }
        case SyntaxTreeType::ForIn: {
            auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            func(for_in);
            WalkSyntaxTree(for_in->Namelist(), func);
            WalkSyntaxTree(for_in->Explist(), func);
            WalkSyntaxTree(for_in->Block(), func);
            break;
        }
        case SyntaxTreeType::NameList: {
            auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(node);
            func(namelist);
            break;
        }
        case SyntaxTreeType::Function: {
            auto f = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
            func(f);
            WalkSyntaxTree(f->Funcname(), func);
            WalkSyntaxTree(f->Funcbody(), func);
            break;
        }
        case SyntaxTreeType::FuncNameList: {
            auto funcnamelist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(node);
            func(funcnamelist);
            break;
        }
        case SyntaxTreeType::FuncName: {
            auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(node);
            func(funcname);
            WalkSyntaxTree(funcname->FuncNameList(), func);
            break;
        }
        case SyntaxTreeType::FuncBody: {
            auto funcbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(node);
            func(funcbody);
            WalkSyntaxTree(funcbody->Parlist(), func);
            WalkSyntaxTree(funcbody->Block(), func);
            break;
        }
        case SyntaxTreeType::FunctionDef: {
            auto functiondef = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(node);
            func(functiondef);
            WalkSyntaxTree(functiondef->Funcbody(), func);
            break;
        }
        case SyntaxTreeType::ParList: {
            auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(node);
            func(parlist);
            WalkSyntaxTree(parlist->Namelist(), func);
            break;
        }
        case SyntaxTreeType::LocalFunction: {
            auto local_function = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node);
            func(local_function);
            WalkSyntaxTree(local_function->Funcbody(), func);
            break;
        }
        case SyntaxTreeType::LocalVar: {
            auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            func(local_var);
            WalkSyntaxTree(local_var->Namelist(), func);
            WalkSyntaxTree(local_var->Explist(), func);
            break;
        }
        case SyntaxTreeType::Exp: {
            auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            func(exp);
            WalkSyntaxTree(exp->Left(), func);
            WalkSyntaxTree(exp->Op(), func);
            WalkSyntaxTree(exp->Right(), func);
            break;
        }
        case SyntaxTreeType::Binop: {
            auto binop = std::dynamic_pointer_cast<SyntaxTreeBinop>(node);
            func(binop);
            break;
        }
        case SyntaxTreeType::Unop: {
            auto unop = std::dynamic_pointer_cast<SyntaxTreeUnop>(node);
            func(unop);
            break;
        }
        case SyntaxTreeType::Args: {
            auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            func(args);
            WalkSyntaxTree(args->Explist(), func);
            WalkSyntaxTree(args->Tableconstructor(), func);
            WalkSyntaxTree(args->String(), func);
            break;
        }
        case SyntaxTreeType::PrefixExp: {
            auto prefixexp = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
            func(prefixexp);
            WalkSyntaxTree(prefixexp->GetValue(), func);
            break;
        }
        default: {
            DEBUG_ASSERT(false && "unknown syntax tree type");
        }
    }
}

}// namespace fakelua
