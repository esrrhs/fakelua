#include "compile/type_inferencer.h"

#include <algorithm>
#include <fstream>
#include <ranges>

#include "compile/syntax_tree.h"
#include "util/common.h"
#include "util/file_util.h"

namespace fakelua {

void TypeInferencer::DumpASTWithTypes(const SyntaxTreeInterfacePtr &node, const EvalTypeSnapshot &snapshot, int tab,
                                      std::ostream &os) const {
    if (!node) {
        return;
    }

    for (int i = 0; i < tab; ++i) {
        os << "  ";
    }

    std::string type_str = "T_UNKNOWN";
    if (auto it = snapshot.find(node.get()); it != snapshot.end()) {
        type_str = InferredTypeToString(it->second);
    }

    std::string node_name = SyntaxTreeTypeToString(node->Type());
    std::string extra_info = "";

    if (node->Type() == SyntaxTreeType::Exp) {
        if (auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node); exp->GetExpKind() == ExpKind::kNumber) {
            extra_info = std::format(" (number: {})", exp->ExpValue());
        } else if (exp->GetExpKind() == ExpKind::kString) {
            extra_info = std::format(" (string: \"{}\")", exp->ExpValue());
        }
    } else if (node->Type() == SyntaxTreeType::Var) {
        if (auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node); var->GetVarKind() == VarKind::kSimple) {
            extra_info = std::format(" (var: {})", var->GetName());
        }
    } else if (node->Type() == SyntaxTreeType::Label) {
        auto label = std::dynamic_pointer_cast<SyntaxTreeLabel>(node);
        extra_info = std::format(" (label: {})", label->GetName());
    }

    os << std::format("{}{} <{}>\n", node_name, extra_info, type_str);

    switch (node->Type()) {
        case SyntaxTreeType::Empty:
        case SyntaxTreeType::Label:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::Binop:
        case SyntaxTreeType::Unop:
        case SyntaxTreeType::FuncNameList:
        case SyntaxTreeType::NameList:
            break;
        case SyntaxTreeType::Block: {
            auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (auto &stmt: block->Stmts()) {
                DumpASTWithTypes(stmt, snapshot, tab + 1, os);
            }
            break;
        }
        case SyntaxTreeType::Return: {
            auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            DumpASTWithTypes(ret->Explist(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::Assign: {
            auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            DumpASTWithTypes(assign->Varlist(), snapshot, tab + 1, os);
            DumpASTWithTypes(assign->Explist(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::VarList: {
            auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(node);
            for (auto &var: varlist->Vars()) {
                DumpASTWithTypes(var, snapshot, tab + 1, os);
            }
            break;
        }
        case SyntaxTreeType::ExpList: {
            auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            for (auto &exp: explist->Exps()) {
                DumpASTWithTypes(exp, snapshot, tab + 1, os);
            }
            break;
        }
        case SyntaxTreeType::Var: {
            if (auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node); var->GetVarKind() == VarKind::kSquare) {
                DumpASTWithTypes(var->GetPrefixexp(), snapshot, tab + 1, os);
                DumpASTWithTypes(var->GetExp(), snapshot, tab + 1, os);
            } else if (var->GetVarKind() == VarKind::kDot) {
                DumpASTWithTypes(var->GetPrefixexp(), snapshot, tab + 1, os);
            }
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            auto functioncall = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            DumpASTWithTypes(functioncall->prefixexp(), snapshot, tab + 1, os);
            DumpASTWithTypes(functioncall->Args(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::TableConstructor: {
            auto tableconstructor = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
            DumpASTWithTypes(tableconstructor->Fieldlist(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::FieldList: {
            auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            for (auto &field: fieldlist->Fields()) {
                DumpASTWithTypes(field, snapshot, tab + 1, os);
            }
            break;
        }
        case SyntaxTreeType::Field: {
            auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            DumpASTWithTypes(field->Key(), snapshot, tab + 1, os);
            DumpASTWithTypes(field->Value(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::While: {
            auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            DumpASTWithTypes(while_node->Exp(), snapshot, tab + 1, os);
            DumpASTWithTypes(while_node->Block(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::Repeat: {
            auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            DumpASTWithTypes(rep->Block(), snapshot, tab + 1, os);
            DumpASTWithTypes(rep->Exp(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::If: {
            auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            DumpASTWithTypes(if_node->Exp(), snapshot, tab + 1, os);
            DumpASTWithTypes(if_node->Block(), snapshot, tab + 1, os);
            DumpASTWithTypes(if_node->ElseIfs(), snapshot, tab + 1, os);
            DumpASTWithTypes(if_node->ElseBlock(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::ElseIfList: {
            auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
            for (size_t i = 0; i < elseifs->ElseifSize(); ++i) {
                DumpASTWithTypes(elseifs->ElseifExp(i), snapshot, tab + 1, os);
                DumpASTWithTypes(elseifs->ElseifBlock(i), snapshot, tab + 1, os);
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            DumpASTWithTypes(for_loop->ExpBegin(), snapshot, tab + 1, os);
            DumpASTWithTypes(for_loop->ExpEnd(), snapshot, tab + 1, os);
            DumpASTWithTypes(for_loop->ExpStep(), snapshot, tab + 1, os);
            DumpASTWithTypes(for_loop->Block(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::ForIn: {
            auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            DumpASTWithTypes(for_in->Namelist(), snapshot, tab + 1, os);
            DumpASTWithTypes(for_in->Explist(), snapshot, tab + 1, os);
            DumpASTWithTypes(for_in->Block(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::Function: {
            auto func_node = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
            DumpASTWithTypes(func_node->Funcname(), snapshot, tab + 1, os);
            DumpASTWithTypes(func_node->Funcbody(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::FuncName: {
            auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(node);
            DumpASTWithTypes(funcname->FuncNameList(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::FuncBody: {
            auto funcbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(node);
            DumpASTWithTypes(funcbody->Parlist(), snapshot, tab + 1, os);
            DumpASTWithTypes(funcbody->Block(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::FunctionDef: {
            auto functiondef = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(node);
            DumpASTWithTypes(functiondef->Funcbody(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::ParList: {
            auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(node);
            DumpASTWithTypes(parlist->Namelist(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::LocalFunction: {
            auto local_function = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node);
            DumpASTWithTypes(local_function->Funcbody(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::LocalVar: {
            auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            DumpASTWithTypes(local_var->Namelist(), snapshot, tab + 1, os);
            DumpASTWithTypes(local_var->Explist(), snapshot, tab + 1, os);
            break;
        }
        case SyntaxTreeType::Exp: {
            if (auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node); exp->GetExpKind() == ExpKind::kBinop) {
                DumpASTWithTypes(exp->Left(), snapshot, tab + 1, os);
                DumpASTWithTypes(exp->Op(), snapshot, tab + 1, os);
                DumpASTWithTypes(exp->Right(), snapshot, tab + 1, os);
            } else if (exp->GetExpKind() == ExpKind::kUnop) {
                DumpASTWithTypes(exp->Op(), snapshot, tab + 1, os);
                DumpASTWithTypes(exp->Right(), snapshot, tab + 1, os);
            } else if (exp->GetExpKind() == ExpKind::kPrefixExp) {
                DumpASTWithTypes(exp->Right(), snapshot, tab + 1, os);
            }
            break;
        }
        case SyntaxTreeType::Args: {
            if (auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node); args->GetArgsKind() == ArgsKind::kExpList) {
                DumpASTWithTypes(args->Explist(), snapshot, tab + 1, os);
            } else if (args->GetArgsKind() == ArgsKind::kTableConstructor) {
                DumpASTWithTypes(args->Tableconstructor(), snapshot, tab + 1, os);
            }
            break;
        }
        case SyntaxTreeType::PrefixExp: {
            if (auto prefixexp = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node); prefixexp->GetPrefixKind() == PrefixExpKind::kVar) {
                DumpASTWithTypes(prefixexp->GetValue(), snapshot, tab + 1, os);
            } else if (prefixexp->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
                DumpASTWithTypes(prefixexp->GetValue(), snapshot, tab + 1, os);
            } else if (prefixexp->GetPrefixKind() == PrefixExpKind::kExp) {
                DumpASTWithTypes(prefixexp->GetValue(), snapshot, tab + 1, os);
            }
            break;
        }
        default:
            ThrowFakeluaException(std::format("DumpASTWithTypes: unexpected SyntaxTreeType: {}", SyntaxTreeTypeToString(node->Type())));
    }
}

// ===========================================================================
// 第一部分：TypeEnvironment 实现
// ===========================================================================

TypeInferencer::TypeEnvironment::TypeEnvironment() {
    EnterScope();
}

void TypeInferencer::TypeEnvironment::EnterScope() {
    scopes_.emplace_back();
}

void TypeInferencer::TypeEnvironment::ExitScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

void TypeInferencer::TypeEnvironment::Define(const std::string &name, const InferredType type) {
    scopes_.back()[name] = type;
}

bool TypeInferencer::TypeEnvironment::Update(const std::string &name, const InferredType type) {
    for (auto &scope: std::views::reverse(scopes_)) {
        if (const auto found = scope.find(name); found != scope.end()) {
            found->second = MergeType(found->second, type);
            return true;
        }
    }
    return false;
}

InferredType TypeInferencer::TypeEnvironment::Lookup(const std::string &name) const {
    for (const auto &scope: std::views::reverse(scopes_)) {
        if (const auto found = scope.find(name); found != scope.end()) {
            return found->second;
        }
    }
    return T_DYNAMIC;
}

InferredType TypeInferencer::TypeEnvironment::MergeType(const InferredType old_type, const InferredType new_type) {
    if (old_type == T_DYNAMIC || new_type == T_DYNAMIC) {
        return T_DYNAMIC;
    }
    DEBUG_ASSERT(old_type != T_UNKNOWN);
    DEBUG_ASSERT(new_type != T_UNKNOWN);
    if (old_type == new_type) {
        return old_type;
    }
    return T_DYNAMIC;
}

// ===========================================================================
// 第二部分：核心 AST 类型推断引擎
// ===========================================================================

InferResult TypeInferencer::InferTypes(const ParseResult &pr, const CompileConfig &cfg) {
    file_level_types_.clear();
    InferResult ir;
    EvalTypeMap current_map;
    TypeEnvironment env;
    TraversalContext tctx{current_map, env};

    InferNode(pr.chunk, tctx);
    // 将当前推断结果复制为全局主快照，供 CGen 在非特化路径下查询节点类型。
    ir.main_eval_types = current_map;

    // 在正常推断之后，通过三个阶段发现数学参数并生成特化信息：
    // IdentifyMathParams：多轮迭代识别数学参数
    if (const auto math_func_info = IdentifyMathParams(pr, ir); !math_func_info.empty()) {
        // GenerateInitialSnapshots：生成各特化版本的初始类型快照
        GenerateInitialSnapshots(ir, math_func_info);
        // InferSpecializationReturnTypes：不动点迭代精化返回类型
        const auto func_ret_cache = BuildFunctionReturnCache(math_func_info);
        InferSpecializationReturnTypes(ir, math_func_info, func_ret_cache);
    }

    if (cfg.debug_mode) {
        const auto dumpfile = GenerateTmpFilename("fakelua_infer_", ".txt");
        if (std::ofstream ofs(dumpfile); ofs.is_open()) {
            ofs << "=== Main Evaluation Types ===\n";
            DumpASTWithTypes(pr.chunk, ir.main_eval_types, 0, ofs);

            if (!ir.specialization_snapshots.empty()) {
                ofs << "\n=== Specialization Snapshots ===\n";
                for (const auto &[func_name, snapshots]: ir.specialization_snapshots) {
                    ofs << std::format("Function: {}\n", func_name);
                    for (size_t bitmask = 0; bitmask < snapshots.size(); ++bitmask) {
                        ofs << std::format("  Spec bitmask: {}\n", bitmask);
                        DumpASTWithTypes(pr.chunk, snapshots[bitmask], 2, ofs);
                    }
                }
            }
            ofs.close();
            std::cerr << "TypeInferencer: Type inference results dumped to " << dumpfile << std::endl;
            LOG_INFO("Type inference results generated: {}", dumpfile);
        } else {
            LOG_ERROR("Failed to open output file: {}", dumpfile);
        }
    }

    return ir;
}

InferredType TypeInferencer::InferNode(const SyntaxTreeInterfacePtr &node, TraversalContext &tctx) {
    if (!node) {
        return T_UNKNOWN;
    }

    auto &current_map = tctx.current_map;

    switch (node->Type()) {
        case SyntaxTreeType::Block: {
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            // 独立的 do...end 块必须引入自己的作用域，使内部的
            // 局部声明不会污染（或覆盖）外围作用域。
            InferBlock(block, true, tctx);
            return RecordType(current_map, block.get(), T_UNKNOWN);
        }
        case SyntaxTreeType::LocalVar: {
            return InferLocalVar(std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node), tctx);
        }
        case SyntaxTreeType::Assign: {
            return InferAssign(std::dynamic_pointer_cast<SyntaxTreeAssign>(node), tctx);
        }
        case SyntaxTreeType::ForLoop: {
            return InferForLoop(std::dynamic_pointer_cast<SyntaxTreeForLoop>(node), tctx);
        }
        case SyntaxTreeType::Function: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
            InferNode(func->Funcbody(), tctx);
            return RecordType(current_map, node.get(), T_UNKNOWN);
        }
        case SyntaxTreeType::LocalFunction: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node);
            InferNode(func->Funcbody(), tctx);
            return RecordType(current_map, node.get(), T_UNKNOWN);
        }
        case SyntaxTreeType::FuncBody: {
            const auto funcbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(node);
            tctx.env.EnterScope();
            if (const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(funcbody->Parlist())) {
                if (const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist())) {
                    for (const auto &name: namelist->Names()) {
                        tctx.env.Define(name, T_DYNAMIC);
                    }
                }
            }
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(funcbody->Block()), false, tctx);
            tctx.env.ExitScope();
            return RecordType(current_map, node.get(), T_UNKNOWN);
        }
        case SyntaxTreeType::Return: {
            const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            InferNode(ret->Explist(), tctx);
            return RecordType(current_map, node.get(), T_UNKNOWN);
        }
        case SyntaxTreeType::ExpList: {
            const auto exp_list = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            InferredType last = T_UNKNOWN;
            for (const auto &exp: exp_list->Exps()) {
                last = InferNode(exp, tctx);
            }
            return RecordType(current_map, node.get(), last);
        }
        case SyntaxTreeType::Exp: {
            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            return InferExp(exp, tctx);
        }
        case SyntaxTreeType::PrefixExp: {
            const auto prefix_exp = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
            return InferPrefixExp(prefix_exp, tctx);
        }
        case SyntaxTreeType::Var: {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
            return InferVar(var, tctx);
        }
        case SyntaxTreeType::FunctionCall: {
            const auto functioncall = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            InferNode(functioncall->prefixexp(), tctx);
            InferNode(functioncall->Args(), tctx);
            // 当正在运行携带被调函数返回类型提示的试推断时，尝试解析该函数调用的实际返回类型。
            // 主推断遍（ctx 为 null）保持原有的 T_DYNAMIC 行为。
            const auto ret_type = ResolveCallReturnType(functioncall, tctx);
            return RecordType(current_map, node.get(), ret_type);
        }
        case SyntaxTreeType::TableConstructor: {
            const auto tableconstructor = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
            InferNode(tableconstructor->Fieldlist(), tctx);
            return RecordType(current_map, node.get(), T_DYNAMIC);
        }
        case SyntaxTreeType::Args: {
            const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            InferNode(args->Explist(), tctx);
            InferNode(args->Tableconstructor(), tctx);
            InferNode(args->String(), tctx);
            return RecordType(current_map, node.get(), T_DYNAMIC);
        }
        case SyntaxTreeType::FieldList: {
            const auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            for (const auto &field: fieldlist->Fields()) {
                InferNode(field, tctx);
            }
            return RecordType(current_map, node.get(), T_DYNAMIC);
        }
        case SyntaxTreeType::Field: {
            const auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            InferNode(field->Key(), tctx);
            InferNode(field->Value(), tctx);
            return RecordType(current_map, node.get(), T_DYNAMIC);
        }
        case SyntaxTreeType::If: {
            return InferIf(std::dynamic_pointer_cast<SyntaxTreeIf>(node), tctx);
        }
        case SyntaxTreeType::While: {
            return InferWhile(std::dynamic_pointer_cast<SyntaxTreeWhile>(node), tctx);
        }
        case SyntaxTreeType::Repeat: {
            return InferRepeat(std::dynamic_pointer_cast<SyntaxTreeRepeat>(node), tctx);
        }
        case SyntaxTreeType::ForIn: {
            return InferForIn(std::dynamic_pointer_cast<SyntaxTreeForIn>(node), tctx);
        }
        case SyntaxTreeType::Empty:
        case SyntaxTreeType::Label:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::NameList: {
            // 这些节点是语句或辅助结构，没有表达式类型
            return RecordType(current_map, node.get(), T_UNKNOWN);
        }
        default: {
            ThrowFakeluaException(std::format("InferNode: unexpected SyntaxTreeType: {}", SyntaxTreeTypeToString(node->Type())));
        }
    }
}

InferredType TypeInferencer::InferLocalVar(const std::shared_ptr<SyntaxTreeLocalVar> &local_var, TraversalContext &tctx) {
    const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
    std::vector<SyntaxTreeInterfacePtr> exps;
    if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist())) {
        exps = explist->Exps();
    }

    DEBUG_ASSERT(namelist);
    auto &current_map = tctx.current_map;

    const auto &names = namelist->Names();
    for (size_t i = 0; i < names.size(); ++i) {
        InferredType type = T_DYNAMIC;
        if (i < exps.size()) {
            type = InferNode(exps[i], tctx);
        }
        tctx.env.Define(names[i], type);
        // 文件顶层数值类型局部变量（非试推断且作用域深度 <= 2）：
        // 将其记录 to file_level_types，供 RunTrialInference
        // 在重置 env_ 后重新注入，使函数特化试推断能看到正确类型。
        if (!tctx.IsTrialInference() && tctx.env.GetScopeDepth() <= 2 && IsNumericInferredType(type)) {
            file_level_types_[names[i]] = type;
        }
    }

    return RecordType(current_map, local_var.get(), T_UNKNOWN);
}

InferredType TypeInferencer::InferAssign(const std::shared_ptr<SyntaxTreeAssign> &assign, TraversalContext &tctx) {
    const auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
    const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
    DEBUG_ASSERT(varlist && explist && !varlist->Vars().empty() && !explist->Exps().empty());

    DEBUG_ASSERT(varlist->Vars().size() == 1 && explist->Exps().size() == 1);// 预处理阶段已将多赋值拆分成单赋值

    const InferredType rhs_type = InferNode(explist->Exps()[0], tctx);
    const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(varlist->Vars()[0]);
    DEBUG_ASSERT(var && var->GetVarKind() == VarKind::kSimple);

    auto &current_map = tctx.current_map;
    const std::string name = var->GetName();
    InferredType current = T_DYNAMIC;
    if (tctx.IsPinnedVar(name)) {
        current = tctx.env.Lookup(name);
    } else if (tctx.env.Update(name, rhs_type)) {
        current = tctx.env.Lookup(name);
    }

    current_map[var.get()] = current;
    return RecordType(current_map, assign.get(), current);
}

InferredType TypeInferencer::InferForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_loop, TraversalContext &tctx) {
    const InferredType begin_type = InferNode(for_loop->ExpBegin(), tctx);
    const InferredType end_type = InferNode(for_loop->ExpEnd(), tctx);
    const InferredType step_type = InferNode(for_loop->ExpStep(), tctx);

    // 仅当所有边界都是 T_INT 时才将循环变量标记为 T_INT，
    // 这与 CGen 使用的整型特化路径相匹配。当所有边界均为数值（T_INT 或 T_FLOAT）
    // 但并非全为 T_INT 时，标记为 T_FLOAT 以启用 double 快路径。
    // 当任何边界为 T_DYNAMIC 时，CGen 会生成 CVar 循环控制变量，
    // 因此循环变量也必须是 T_DYNAMIC 以保持类型一致。
    const bool begin_valid = for_loop->ExpBegin() != nullptr;
    const bool end_valid = for_loop->ExpEnd() != nullptr;
    const bool step_numeric = !for_loop->ExpStep() || step_type == T_INT || step_type == T_FLOAT;
    const bool all_int =
            begin_valid && end_valid && begin_type == T_INT && end_type == T_INT && (!for_loop->ExpStep() || step_type == T_INT);
    const bool all_numeric = !all_int && begin_valid && end_valid && (begin_type == T_INT || begin_type == T_FLOAT) &&
                             (end_type == T_INT || end_type == T_FLOAT) && step_numeric;
    const InferredType loop_var_type = all_int ? T_INT : (all_numeric ? T_FLOAT : T_DYNAMIC);

    auto &current_map = tctx.current_map;
    tctx.env.EnterScope();
    tctx.env.Define(for_loop->Name(), loop_var_type);
    InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(for_loop->Block()), false, tctx);
    // 循环体内可能对循环变量重新赋值（例如 `a = "test"`），导致其类型
    // 从初始 of T_INT/T_FLOAT 拓宽为 T_DYNAMIC。此处重新查询循环变量 of
    // 最终类型，以便 CGen 决定生成原生整型/浮点快路径还是 CVar 动态路径。
    const InferredType final_loop_var_type = tctx.env.Lookup(for_loop->Name());
    tctx.env.ExitScope();

    return RecordType(current_map, for_loop.get(), final_loop_var_type);
}

InferredType TypeInferencer::InferForIn(const std::shared_ptr<SyntaxTreeForIn> &for_in, TraversalContext &tctx) {
    InferNode(for_in->Explist(), tctx);
    const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(for_in->Namelist());
    DEBUG_ASSERT(namelist);
    // For-in 循环变量是循环体作用域内的局部变量，必须先注入作用域。
    // 否则循环体中的赋值会错误地更新外层同名变量，导致类型污染。
    tctx.env.EnterScope();
    for (const auto &name: namelist->Names()) {
        tctx.env.Define(name, T_DYNAMIC);
    }
    InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(for_in->Block()), false, tctx);
    tctx.env.ExitScope();
    return RecordType(tctx.current_map, for_in.get(), T_UNKNOWN);
}

InferredType TypeInferencer::InferWhile(const std::shared_ptr<SyntaxTreeWhile> &while_stmt, TraversalContext &tctx) {
    InferNode(while_stmt->Exp(), tctx);
    InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(while_stmt->Block()), true, tctx);
    return RecordType(tctx.current_map, while_stmt.get(), T_UNKNOWN);
}

InferredType TypeInferencer::InferRepeat(const std::shared_ptr<SyntaxTreeRepeat> &repeat_stmt, TraversalContext &tctx) {
    // Lua 语义：until 条件可以访问 repeat 块内声明 of local 变量。
    // 因此必须在 until 条件推断完成之后再退出作用域，而不能在块结束时立即退出。
    tctx.env.EnterScope();
    InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(repeat_stmt->Block()), false, tctx);
    InferNode(repeat_stmt->Exp(), tctx);
    tctx.env.ExitScope();
    return RecordType(tctx.current_map, repeat_stmt.get(), T_UNKNOWN);
}

InferredType TypeInferencer::InferIf(const std::shared_ptr<SyntaxTreeIf> &if_stmt, TraversalContext &tctx) {
    InferNode(if_stmt->Exp(), tctx);
    InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(if_stmt->Block()), true, tctx);
    if (const auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs())) {
        for (size_t i = 0; i < elseifs->ElseifSize(); ++i) {
            InferNode(elseifs->ElseifExp(i), tctx);
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(elseifs->ElseifBlock(i)), true, tctx);
        }
    }
    InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(if_stmt->ElseBlock()), true, tctx);
    return RecordType(tctx.current_map, if_stmt.get(), T_UNKNOWN);
}

InferredType TypeInferencer::InferExp(const std::shared_ptr<SyntaxTreeExp> &exp, TraversalContext &tctx) {
    const auto exp_kind = exp->GetExpKind();
    auto &current_map = tctx.current_map;

    switch (exp_kind) {
        case ExpKind::kNumber: {
            const auto &value = exp->ExpValue();
            const auto ret = IsInteger(value) ? T_INT : T_FLOAT;
            return RecordType(current_map, exp.get(), ret);
        }
        case ExpKind::kPrefixExp: {
            const auto ret = InferNode(exp->Right(), tctx);
            return RecordType(current_map, exp.get(), ret);
        }
        case ExpKind::kBinop: {
            const auto left_type = InferNode(exp->Left(), tctx);
            const auto right_type = InferNode(exp->Right(), tctx);

            const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
            DEBUG_ASSERT(op);
            const auto op_kind = op->GetOpKind();

            if (op_kind == BinOpKind::kOr) {
                // Pattern match Lua ternary: (cond and val1) or val2
                if (exp->Left()->Type() == SyntaxTreeType::Exp) {
                    if (const auto left_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(exp->Left());
                        left_exp && left_exp->GetExpKind() == ExpKind::kBinop) {
                        if (const auto left_op = std::dynamic_pointer_cast<SyntaxTreeBinop>(left_exp->Op());
                            left_op && left_op->GetOpKind() == BinOpKind::kAnd) {
                            const auto it = current_map.find(left_exp->Right().get());
                            if (const auto val1_type = (it != current_map.end()) ? it->second : T_DYNAMIC;
                                (val1_type == T_INT || val1_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                                const auto merged = (val1_type == right_type) ? val1_type : T_FLOAT;
                                return RecordType(current_map, exp.get(), merged);
                            }
                        }
                    }
                }
                if (left_type == T_INT || left_type == T_FLOAT) {
                    return RecordType(current_map, exp.get(), left_type);
                }
                return RecordType(current_map, exp.get(), T_DYNAMIC);
            }

            if (left_type == T_DYNAMIC || right_type == T_DYNAMIC) {
                return RecordType(current_map, exp.get(), T_DYNAMIC);
            }

            switch (op_kind) {
                // 保持 INT+INT=INT、混合→FLOAT 语义的算术运算
                case BinOpKind::kPlus:
                case BinOpKind::kMinus:
                case BinOpKind::kStar:
                case BinOpKind::kDoubleSlash:
                case BinOpKind::kMod: {
                    if (left_type == T_INT && right_type == T_INT) {
                        return RecordType(current_map, exp.get(), T_INT);
                    }
                    DEBUG_ASSERT((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT));
                    return RecordType(current_map, exp.get(), T_FLOAT);
                }

                // 结果始终为 FLOAT 的运算
                case BinOpKind::kSlash:
                case BinOpKind::kPow: {
                    DEBUG_ASSERT((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT));
                    return RecordType(current_map, exp.get(), T_FLOAT);
                }

                // 位运算：Lua 5.4 会将整数浮点（如3.0）自动转为 int，
                // 结果始终为 T_INT。
                case BinOpKind::kBitAnd:
                case BinOpKind::kBitOr:
                case BinOpKind::kXor:
                case BinOpKind::kLeftShift:
                case BinOpKind::kRightShift: {
                    DEBUG_ASSERT(IsNumericInferredType(left_type) && IsNumericInferredType(right_type));
                    return RecordType(current_map, exp.get(), T_INT);
                }

                // AND/OR：Lua 中整数 and 浮点数始终为真值（包括 0），因此：
                //   a and b（a 为 T_INT/T_FLOAT）：a 始终为真，结果为 b → 类型为 right_type
                case BinOpKind::kAnd: {
                    DEBUG_ASSERT((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT));
                    return RecordType(current_map, exp.get(), right_type);
                }

                // 比较运算与字符串连接：结果为 T_DYNAMIC
                case BinOpKind::kLess:
                case BinOpKind::kLessEqual:
                case BinOpKind::kMore:
                case BinOpKind::kMoreEqual:
                case BinOpKind::kEqual:
                case BinOpKind::kNotEqual:
                case BinOpKind::kConcat:
                    break;
                default:
                    ThrowFakeluaException("unexpected binary operator kind: " + std::to_string(static_cast<int>(op_kind)));
            }

            return RecordType(current_map, exp.get(), T_DYNAMIC);
        }
        case ExpKind::kUnop: {
            const auto operand_type = InferNode(exp->Right(), tctx);
            const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(exp->Op());
            DEBUG_ASSERT(op);
            switch (const auto op_kind = op->GetOpKind()) {
                case UnOpKind::kMinus: {
                    if (operand_type == T_INT) {
                        return RecordType(current_map, exp.get(), T_INT);
                    }
                    if (operand_type == T_FLOAT) {
                        return RecordType(current_map, exp.get(), T_FLOAT);
                    }
                    break;
                }
                case UnOpKind::kBitNot: {
                    // Lua 5.4 会将整数浮点自动转为 int，因此 ~T_FLOAT 也是合法的。
                    if (operand_type == T_INT || operand_type == T_FLOAT) {
                        return RecordType(current_map, exp.get(), T_INT);
                    }
                    break;
                }
                case UnOpKind::kNumberSign: {
                    // # 运算符始终返回整数（字符串字节数或表元素数）。
                    // 无论操作数是字符串还是表（均为 T_DYNAMIC），结果类型始终为 T_INT。
                    return RecordType(current_map, exp.get(), T_INT);
                }
                case UnOpKind::kNot: {
                    // not 运算符始终返回布尔值，对于类型推断视为 T_DYNAMIC。
                    break;
                }
                default:
                    ThrowFakeluaException("unexpected unary operator kind: " + std::to_string(static_cast<int>(op_kind)));
            }
            return RecordType(current_map, exp.get(), T_DYNAMIC);
        }
        case ExpKind::kNil:
        case ExpKind::kTrue:
        case ExpKind::kFalse:
        case ExpKind::kString:
        case ExpKind::kVarParams:
        case ExpKind::kFunctionDef:
        case ExpKind::kTableConstructor: {
            return RecordType(current_map, exp.get(), T_DYNAMIC);
        }
        default:
            ThrowFakeluaException("unexpected expression kind: " + std::to_string(static_cast<int>(exp_kind)));
    }
}

InferredType TypeInferencer::InferPrefixExp(const std::shared_ptr<SyntaxTreePrefixexp> &prefix_exp, TraversalContext &tctx) {
    const auto prefix_kind = prefix_exp->GetPrefixKind();
    InferredType ret = T_DYNAMIC;

    switch (prefix_kind) {
        case PrefixExpKind::kVar:
        case PrefixExpKind::kExp:
        case PrefixExpKind::kFunctionCall:
            ret = InferNode(prefix_exp->GetValue(), tctx);
            break;
        default:
            ThrowFakeluaException("unexpected prefix expression kind: " + std::to_string(static_cast<int>(prefix_kind)));
    }

    return RecordType(tctx.current_map, prefix_exp.get(), ret);
}

InferredType TypeInferencer::InferVar(const std::shared_ptr<SyntaxTreeVar> &var, TraversalContext &tctx) {
    auto &current_map = tctx.current_map;
    switch (var->GetVarKind()) {
        case VarKind::kSimple: {
            const auto ret = tctx.env.Lookup(var->GetName());
            return RecordType(current_map, var.get(), ret);
        }
        case VarKind::kSquare: {
            // 对于"方括号"变量，处理子表达式以便内部变量
            //（例如用作表索引的整型循环变量）被记录到 current_map，
            // 从而使 CGen 在生成变量引用时能通过 LookupNodeType 查到其原生类型，
            // 而不会在需要 CVar 的地方错误地发出原始 int64_t 变量名。
            if (const auto pe = var->GetPrefixexp()) {
                InferNode(pe, tctx);
            }
            if (const auto exp = var->GetExp()) {
                InferNode(exp, tctx);
            }
            return RecordType(current_map, var.get(), T_DYNAMIC);
        }
        case VarKind::kDot: {
            // 对于"点号"变量，处理前缀表达式子节点
            if (const auto pe = var->GetPrefixexp()) {
                InferNode(pe, tctx);
            }
            return RecordType(current_map, var.get(), T_DYNAMIC);
        }
        default:
            ThrowFakeluaException("unexpected variable kind: " + std::to_string(static_cast<int>(var->GetVarKind())));
    }
}

void TypeInferencer::InferBlock(const std::shared_ptr<SyntaxTreeBlock> &block, const bool new_scope, TraversalContext &tctx) {
    if (!block) {
        return;
    }

    if (new_scope) {
        tctx.env.EnterScope();
    }

    for (const auto &stmt: block->Stmts()) {
        InferNode(stmt, tctx);
    }

    auto &current_map = tctx.current_map;

    if (!tctx.SkipPostProcessing()) {
        for (const auto &stmt: block->Stmts()) {
            if (stmt->Type() == SyntaxTreeType::LocalVar) {
                const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
                const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
                DEBUG_ASSERT(namelist);
                std::vector<SyntaxTreeInterfacePtr> exps;
                if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist())) {
                    exps = explist->Exps();
                }
                const auto &names = namelist->Names();
                for (size_t i = 0; i < names.size(); ++i) {
                    if (i < exps.size()) {
                        current_map[exps[i].get()] = tctx.env.Lookup(names[i]);
                    }
                }
            }
        }
    }

    current_map[block.get()] = T_UNKNOWN;
    if (new_scope) {
        tctx.env.ExitScope();
    }
}

// ===========================================================================
// 第三部分：特化与数学参数识别逻辑
// ===========================================================================

TypeInferencer::MathFuncInfoMap TypeInferencer::IdentifyMathParams(const ParseResult &pr, InferResult &ir) {
    MathFuncInfoMap math_func_info;
    const auto function_infos = CollectFunctionSpecInfos(pr);

    // 多轮迭代直到无新的数学参数被发现（解决函数处理顺序依赖问题）。
    // 发现阶段跳过 InferBlock 后处理，保留算术表达式节点的真实推断类型，
    // 使 CheckArithmeticTypeChanges 能正确检测改善/退化。
    for (int pass = 0; pass < kMaxSpecIterations; ++pass) {
        bool new_discovery = false;
        for (const auto &info: function_infos) {
            // 已知数学函数，跳过重复发现。
            if (ir.math_param_positions.contains(info.name)) {
                continue;
            }
            // baseline：所有参数均假设为 T_DYNAMIC；all_int：所有参数均假设为 T_INT。
            // 两次推断的对比用于判断该函数的算术表达式是否能因参数类型已知而改善。
            const auto baseline = RunTrialInference(info.block, info.params, MakeAssumedParamTypes(info.params, "", T_DYNAMIC, T_DYNAMIC),
                                                    nullptr, nullptr, true);
            const auto all_int = RunTrialInference(info.block, info.params, MakeAssumedParamTypes(info.params, "", T_INT, T_INT), nullptr,
                                                   nullptr, true);
            const auto math_indices = FindMathParamIndices(info, baseline, all_int, ir.math_param_positions);

            if (math_indices.empty()) {
                continue;
            }
            ir.math_param_positions[info.name] = math_indices;
            math_func_info[info.name] = {info.block, info.params};
            new_discovery = true;
            LOG_INFO("TypeInferencer: {} math params for {} (pass {})", math_indices.size(), info.name, pass);
        }
        if (!new_discovery) {
            break;
        }
    }

    return math_func_info;
}

std::vector<int> TypeInferencer::FindMathParamIndices(const FunctionSpecInfo &info, const EvalTypeMap &baseline, const EvalTypeMap &all_int,
                                                      const std::unordered_map<std::string, std::vector<int>> &known_math_positions) {
    std::vector<int> math_indices;
    // 快速剪枝：若全 T_INT 与 baseline 无改善，函数不具备特化价值。
    if (!CheckArithmeticTypeChanges(all_int, baseline, info.block, true, known_math_positions)) {
        return math_indices;
    }

    for (int i = 0; i < static_cast<int>(info.params.size()); ++i) {
        // without_p：除 p_i 为 T_DYNAMIC 外，其余参数均为 T_INT。
        const auto without_p_assumed = MakeAssumedParamTypes(info.params, info.params[i], T_DYNAMIC, T_INT);
        // 若去掉 p_i 后算术/比较/for-loop 退化，则 p_i 是数学参数。
        if (const auto without_p_map = RunTrialInference(info.block, info.params, without_p_assumed, nullptr, nullptr, true);
            CheckArithmeticTypeChanges(all_int, without_p_map, info.block, false, known_math_positions)) {
            math_indices.push_back(i);
        }
    }
    return math_indices;
}

void TypeInferencer::GenerateInitialSnapshots(InferResult &ir, const MathFuncInfoMap &math_func_info) {
    for (const auto &[func_name, func_data]: math_func_info) {
        const auto &[func_block, func_params] = func_data;
        const auto &math_indices = ir.math_param_positions.at(func_name);
        const int num_specs = 1 << static_cast<int>(math_indices.size());

        auto &snapshots = ir.specialization_snapshots[func_name];
        snapshots.resize(static_cast<size_t>(num_specs));

        for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
            // 构造参数类型假设：非数学参数为 T_DYNAMIC，数学参数按 bitmask 分配
            auto assumed = MakeSpecializedParamTypes(func_params, math_indices, bitmask);
            snapshots[static_cast<size_t>(bitmask)] = RunTrialInference(func_block, func_params, assumed);
        }
    }
}

TypeInferencer::EvalTypeMap TypeInferencer::RunTrialInference(const SyntaxTreeInterfacePtr &func_block,
                                                              const std::vector<std::string> &params,
                                                              const std::unordered_map<std::string, InferredType> &assumed_types,
                                                              const std::unordered_map<std::string, std::vector<int>> *math_positions,
                                                              const std::unordered_map<std::string, std::vector<InferredType>> *assumed_ret,
                                                              bool skip_post_processing) {
    std::unordered_set<std::string> pinned_vars;
    for (const auto &[name, t]: assumed_types) {
        if (t == T_INT || t == T_FLOAT) {
            pinned_vars.insert(name);
        }
    }

    // 构造试推断上下文，沿调用链传递给 ResolveCallReturnType。
    TrialInferenceContext ctx;
    ctx.math_positions = math_positions;
    ctx.assumed_ret = assumed_ret;
    ctx.pinned_vars = &pinned_vars;
    ctx.skip_post_processing = skip_post_processing;

    EvalTypeMap prev_map;
    EvalTypeMap current_map;

    for (int round = 0; round < kMaxSpecIterations; ++round) {
        // 每轮清除 func_block 节点在 current_map 中的旧条目，保证推断从干净状态开始。
        current_map.clear();

        // 以假定的参数类型初始化 trial 环境；同时注入文件级数值常量，
        // 使函数体能看到正确的文件级局部变量类型（T_INT/T_FLOAT）。
        TypeEnvironment env;
        for (const auto &[fname, ftype]: file_level_types_) {
            env.Define(fname, ftype);
        }
        for (const auto &p: params) {
            const auto it = assumed_types.find(p);
            env.Define(p, it != assumed_types.end() ? it->second : T_DYNAMIC);
        }

        // 运行函数体类型推断（不新开作用域，参数已在当前作用域中定义）。
        TraversalContext tctx{current_map, env, &ctx};
        InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(func_block), false, tctx);

        // 快照本轮推断结果（仅 func_block 节点）。
        EvalTypeMap curr_map;
        WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &n) {
            const auto it = current_map.find(n.get());
            curr_map[n.get()] = (it != current_map.end()) ? it->second : T_UNKNOWN;
        });

        if (curr_map == prev_map) {
            // 已达到不动点，提前退出。
            break;
        }
        prev_map = std::move(curr_map);
    }

    return prev_map;
}

std::unordered_map<std::string, TypeInferencer::FuncRetInfo>
TypeInferencer::BuildFunctionReturnCache(const MathFuncInfoMap &math_func_info) const {
    std::unordered_map<std::string, FuncRetInfo> func_ret_cache;
    for (const auto &[func_name, info]: math_func_info) {
        FuncRetInfo ret_info;
        ret_info.ends_with_return = CollectReturnExps(info.block, ret_info.ret_exps);
        func_ret_cache[func_name] = std::move(ret_info);
    }
    return func_ret_cache;
}

void TypeInferencer::InferSpecializationReturnTypes(InferResult &ir, const MathFuncInfoMap &math_func_info,
                                                    const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache) {
    // 乐观初始值：所有特化版本均假设返回 T_INT。
    for (const auto &[func_name, math_params]: ir.math_param_positions) {
        ir.specialization_return_types[func_name].assign(static_cast<size_t>(1 << static_cast<int>(math_params.size())), T_INT);
    }

    for (int round = 0; round < kMaxSpecIterations; ++round) {
        bool changed = false;
        for (const auto &[func_name, info]: math_func_info) {
            const auto &[func_block, func_params] = info;
            const auto &math_indices = ir.math_param_positions.at(func_name);
            const auto &ret_info = func_ret_cache.at(func_name);
            const int num_specs = 1 << static_cast<int>(math_indices.size());

            for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
                // 以当前 assumed_ret 为提示重新运行试推断，生成精确快照。
                auto assumed = MakeSpecializedParamTypes(func_params, math_indices, bitmask);
                auto new_snapshot =
                        RunTrialInference(func_block, func_params, assumed, &ir.math_param_positions, &ir.specialization_return_types);

                // 从新快照中直接读取 return 表达式节点的类型。
                const auto new_ret = ComputeReturnTypeFromSnapshot(new_snapshot, ret_info);

                // 若快照或返回类型有变化，则更新并标记 changed。
                const auto bitmask_sz = static_cast<size_t>(bitmask);
                auto &cur_ret = ir.specialization_return_types[func_name][bitmask_sz];
                if (auto &cur_snap = ir.specialization_snapshots[func_name][bitmask_sz]; new_ret != cur_ret || new_snapshot != cur_snap) {
                    cur_ret = new_ret;
                    cur_snap = std::move(new_snapshot);
                    changed = true;
                }
            }
        }
        if (!changed) {
            break;
        }
    }
}

// ===========================================================================
// 第四部分：AST 分析与辅助工具函数
// ===========================================================================

std::unordered_map<std::string, InferredType> TypeInferencer::MakeAssumedParamTypes(const std::vector<std::string> &params,
                                                                                    const std::string &special_param,
                                                                                    const InferredType special_type,
                                                                                    const InferredType default_type) const {
    std::unordered_map<std::string, InferredType> assumed;
    for (const auto &param: params) {
        assumed[param] = (!special_param.empty() && param == special_param) ? special_type : default_type;
    }
    return assumed;
}

std::unordered_map<std::string, InferredType> TypeInferencer::MakeSpecializedParamTypes(const std::vector<std::string> &params,
                                                                                        const std::vector<int> &math_indices,
                                                                                        const int bitmask) const {
    std::unordered_map<std::string, InferredType> assumed;
    for (const auto &p: params) {
        assumed[p] = T_DYNAMIC;
    }
    for (int i = 0; i < static_cast<int>(math_indices.size()); ++i) {
        assumed[params[static_cast<size_t>(math_indices[i])]] = (MathParamKindOf(bitmask, i) == kMathParamFloat) ? T_FLOAT : T_INT;
    }
    return assumed;
}

bool TypeInferencer::IsArithmeticExpr(const SyntaxTreeInterfacePtr &node) const {
    if (node->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    if (exp->GetExpKind() == ExpKind::kBinop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
        DEBUG_ASSERT(op);
        const auto k = op->GetOpKind();
        return k == BinOpKind::kPlus || k == BinOpKind::kMinus || k == BinOpKind::kStar || k == BinOpKind::kSlash ||
               k == BinOpKind::kDoubleSlash || k == BinOpKind::kPow || k == BinOpKind::kMod || k == BinOpKind::kBitAnd ||
               k == BinOpKind::kXor || k == BinOpKind::kBitOr || k == BinOpKind::kLeftShift || k == BinOpKind::kRightShift;
    }
    if (exp->GetExpKind() == ExpKind::kUnop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(exp->Op());
        DEBUG_ASSERT(op);
        // 一元负号：-T_INT=T_INT，-T_FLOAT=T_FLOAT，随参数类型改变。
        // 按位取反：~T_INT=T_INT，仅对整数参数有意义。
        return op->GetOpKind() == UnOpKind::kMinus || op->GetOpKind() == UnOpKind::kBitNot;
    }
    return false;
}

bool TypeInferencer::IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const {
    if (node->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    if (exp->GetExpKind() != ExpKind::kBinop) {
        return false;
    }
    const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
    DEBUG_ASSERT(op);
    const auto k = op->GetOpKind();
    return k == BinOpKind::kLess || k == BinOpKind::kLessEqual || k == BinOpKind::kMore || k == BinOpKind::kMoreEqual;
}

namespace {

bool CheckNodeChangeCommon(const SyntaxTreeInterfacePtr &node, const EvalTypeSnapshot &typed_map,
                           const EvalTypeSnapshot &compare_map, const bool improvement_mode) {
    const auto it_typed = typed_map.find(node.get());
    const auto it_compare = compare_map.find(node.get());
    DEBUG_ASSERT(it_typed != typed_map.end() && it_compare != compare_map.end());
    return IsNumericInferredType(it_typed->second) &&
           (improvement_mode ? (it_compare->second == T_DYNAMIC) : (it_compare->second != it_typed->second));
}

} // namespace

bool TypeInferencer::CheckArithmeticNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map,
                                               const EvalTypeMap &compare_map, const bool improvement_mode) const {
    return IsArithmeticExpr(node) && CheckNodeChangeCommon(node, typed_map, compare_map, improvement_mode);
}

bool TypeInferencer::CheckComparisonNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map,
                                               const EvalTypeMap &compare_map, const bool improvement_mode) const {
    if (!IsNativeComparisonExpr(node)) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    const auto left = exp->Left();
    const auto right = exp->Right();
    DEBUG_ASSERT(left && right);
    const auto lt_typed = typed_map.find(left.get());
    const auto rt_typed = typed_map.find(right.get());
    const auto lt_compare = compare_map.find(left.get());
    const auto rt_compare = compare_map.find(right.get());
    DEBUG_ASSERT(lt_typed != typed_map.end() && rt_typed != typed_map.end() && lt_compare != compare_map.end() &&
                 rt_compare != compare_map.end());
    if (IsNumericInferredType(lt_typed->second) && IsNumericInferredType(rt_typed->second)) {
        if (improvement_mode) {
            return (lt_compare->second == T_DYNAMIC || rt_compare->second == T_DYNAMIC);
        } else {
            return (lt_compare->second != lt_typed->second || rt_compare->second != rt_typed->second);
        }
    }
    return false;
}

bool TypeInferencer::CheckForLoopNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map,
                                            const EvalTypeMap &compare_map, const bool improvement_mode) const {
    return (node->Type() == SyntaxTreeType::ForLoop) && CheckNodeChangeCommon(node, typed_map, compare_map, improvement_mode);
}

bool TypeInferencer::CheckCallNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map, const EvalTypeMap &compare_map,
                                         const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    if (node->Type() != SyntaxTreeType::FunctionCall) {
        return false;
    }
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
    DEBUG_ASSERT(fc);
    const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
    DEBUG_ASSERT(callee_pe && callee_pe->GetPrefixKind() == PrefixExpKind::kVar);
    const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
    DEBUG_ASSERT(callee_var && callee_var->GetVarKind() == VarKind::kSimple);
    const auto &callee_name = callee_var->GetName();
    if (const auto math_it = math_param_positions.find(callee_name); math_it != math_param_positions.end()) {
        const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
        DEBUG_ASSERT(args_ptr);
        const auto raw_args = ExtractCallRawArgs(args_ptr);
        for (const int param_pos: math_it->second) {
            if (param_pos >= static_cast<int>(raw_args.size())) {
                return false;
            }
            const auto &arg = raw_args[static_cast<size_t>(param_pos)];
            const auto it_typed = typed_map.find(arg.get());
            const auto it_comp = compare_map.find(arg.get());
            DEBUG_ASSERT(it_typed != typed_map.end() && it_comp != compare_map.end());
            if ((it_typed->second == T_INT || it_typed->second == T_FLOAT) && it_comp->second != it_typed->second) {
                return true;
            }
        }
        return false;
    }
    return false;
}

bool TypeInferencer::CheckArithmeticTypeChanges(const EvalTypeMap &typed_map, const EvalTypeMap &compare_map,
                                                const SyntaxTreeInterfacePtr &func_block, const bool improvement_mode,
                                                const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found) {
            return;
        }
        if (CheckArithmeticNodeChange(node, typed_map, compare_map, improvement_mode) ||
            CheckComparisonNodeChange(node, typed_map, compare_map, improvement_mode) ||
            CheckForLoopNodeChange(node, typed_map, compare_map, improvement_mode) ||
            CheckCallNodeChange(node, typed_map, compare_map, math_param_positions)) {
            found = true;
        }
    });
    return found;
}

std::vector<TypeInferencer::FunctionSpecInfo> TypeInferencer::CollectFunctionSpecInfos(const ParseResult &pr) const {
    std::vector<FunctionSpecInfo> infos;
    const auto chunk = pr.chunk;
    DEBUG_ASSERT(chunk && chunk->Type() == SyntaxTreeType::Block);

    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    for (const auto &stmt: top_block->Stmts()) {
        std::string name;
        SyntaxTreeInterfacePtr funcbody;
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            const auto fnlist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcname->FuncNameList());
            name = fnlist->Funcnames()[0];
            funcbody = func->Funcbody();
        } else if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
            name = func->Name();
            funcbody = func->Funcbody();
        }
        if (name.empty() || !funcbody) {
            continue;
        }
        const auto funcbody_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
        const auto parlist_node = funcbody_ptr->Parlist();
        if (!parlist_node) {
            continue;
        }
        const auto parlist_ptr = std::dynamic_pointer_cast<SyntaxTreeParlist>(parlist_node);
        const auto namelist_node = parlist_ptr->Namelist();
        DEBUG_ASSERT(namelist_node);
        const auto params = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist_node)->Names();
        DEBUG_ASSERT(!params.empty());
        FunctionSpecInfo info;
        info.name = name;
        info.block = funcbody_ptr->Block();
        info.params = params;
        infos.emplace_back(std::move(info));
    }
    return infos;
}

bool TypeInferencer::AllPathsReturn(const SyntaxTreeInterfacePtr &block_node) const {
    DEBUG_ASSERT(block_node);
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(block_node);
    if (!block || block->Stmts().empty()) {
        return false;
    }
    switch (const auto &last = block->Stmts().back(); last->Type()) {
        case SyntaxTreeType::Return:
            return true;
        case SyntaxTreeType::Block:
            // do...end 块：无条件执行（不同于 if/while 可能跳过），内部可含任意控制流。
            // 若其所有路径均以 return 结束，则外层函数视为已返回。
            return AllPathsReturn(last);
        case SyntaxTreeType::If: {
            const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(last);
            // if 分支必须返回。
            if (!AllPathsReturn(if_node->Block())) {
                return false;
            }
            // 所有 elseif 分支必须返回。
            if (const auto elseifs = if_node->ElseIfs()) {
                const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs);
                for (const auto &blk: el->ElseifBlocks()) {
                    if (!AllPathsReturn(blk)) {
                        return false;
                    }
                }
            }
            // 必须有 else 分支且它也返回，否则无法保证所有路径均返回。
            if (!if_node->ElseBlock()) {
                return false;
            }
            return AllPathsReturn(if_node->ElseBlock());
        }
        case SyntaxTreeType::Assign:
        case SyntaxTreeType::LocalVar:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::Function:
        case SyntaxTreeType::FunctionCall:
        case SyntaxTreeType::While:
        case SyntaxTreeType::Repeat:
        case SyntaxTreeType::ForLoop:
        case SyntaxTreeType::ForIn:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::Label:
        case SyntaxTreeType::Empty:
            return false;
        default:
            ThrowFakeluaException(std::format("AllPathsReturn: unexpected statement type {}", SyntaxTreeTypeToString(last->Type())));
    }
}

bool TypeInferencer::CollectReturnExps(const SyntaxTreeInterfacePtr &block_node, std::vector<SyntaxTreeInterfacePtr> &ret_exps) const {
    if (!block_node) {
        return false;
    }
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(block_node);
    DEBUG_ASSERT(block);
    const auto &stmts = block->Stmts();
    if (stmts.empty()) {
        return false;
    }
    const bool ends_with_return = AllPathsReturn(block_node);
    for (const auto &stmt: stmts) {
        switch (stmt->Type()) {
            case SyntaxTreeType::Return: {
                const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);
                const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(ret->Explist());
                ret_exps.push_back((el && !el->Exps().empty()) ? el->Exps()[0] : nullptr);
                break;
            }
            case SyntaxTreeType::If: {
                const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
                CollectReturnExps(if_node->Block(), ret_exps);
                if (const auto elseifs = if_node->ElseIfs()) {
                    const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs);
                    for (const auto &blk: el->ElseifBlocks()) {
                        CollectReturnExps(blk, ret_exps);
                    }
                }
                CollectReturnExps(if_node->ElseBlock(), ret_exps);
                break;
            }
            case SyntaxTreeType::While: {
                const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);
                CollectReturnExps(while_node->Block(), ret_exps);
                break;
            }
            case SyntaxTreeType::Repeat: {
                const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);
                CollectReturnExps(rep->Block(), ret_exps);
                break;
            }
            case SyntaxTreeType::ForLoop: {
                const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
                CollectReturnExps(for_loop->Block(), ret_exps);
                break;
            }
            case SyntaxTreeType::ForIn: {
                const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
                CollectReturnExps(for_in->Block(), ret_exps);
                break;
            }
            case SyntaxTreeType::Block:
                // do...end 块：递归收集其内部的 return 表达式。
                CollectReturnExps(stmt, ret_exps);
                break;
            case SyntaxTreeType::Assign:
            case SyntaxTreeType::LocalVar:
            case SyntaxTreeType::LocalFunction:
            case SyntaxTreeType::Function:
            case SyntaxTreeType::FunctionCall:
            case SyntaxTreeType::Break:
            case SyntaxTreeType::Goto:
            case SyntaxTreeType::Label:
            case SyntaxTreeType::Empty:
                // 不含 return，无需递归。
                break;
            default:
                ThrowFakeluaException(std::format("CollectReturnExps: unexpected statement type {}", SyntaxTreeTypeToString(stmt->Type())));
        }
    }
    return ends_with_return;
}

InferredType TypeInferencer::ResolveCallReturnType(const std::shared_ptr<SyntaxTreeFunctioncall> &fc, const TraversalContext &tctx) const {
    if (!tctx.IsTrialInference() || !tctx.ctx->math_positions || !tctx.ctx->assumed_ret) {
        return T_DYNAMIC;
    }
    // 提取被调函数名（仅支持简单形式 callee(...)，方法调用形式不处理）。
    const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
    DEBUG_ASSERT(callee_pe && callee_pe->GetPrefixKind() == PrefixExpKind::kVar);
    const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
    DEBUG_ASSERT(callee_var && callee_var->GetVarKind() == VarKind::kSimple);
    const auto &callee_name = callee_var->GetName();

    const auto math_it = tctx.ctx->math_positions->find(callee_name);
    if (math_it == tctx.ctx->math_positions->end()) {
        return T_DYNAMIC;
    }
    const auto ret_it = tctx.ctx->assumed_ret->find(callee_name);
    DEBUG_ASSERT(ret_it != tctx.ctx->assumed_ret->end());

    // 从已推断的实参类型（tctx.current_map 中已由 InferNode(Args) 填充）构造 bitmask。
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
    DEBUG_ASSERT(args_ptr);
    const auto raw_args = ExtractCallRawArgs(args_ptr);
    const auto &math_params = math_it->second;

    int bitmask = 0;
    for (int i = 0; i < static_cast<int>(math_params.size()); ++i) {
        const int param_pos = math_params[static_cast<size_t>(i)];
        if (param_pos >= static_cast<int>(raw_args.size())) {
            return T_DYNAMIC;
        }
        const auto arg_it = tctx.current_map.find(raw_args[static_cast<size_t>(param_pos)].get());
        DEBUG_ASSERT(arg_it != tctx.current_map.end());
        if (arg_it->second == T_FLOAT) {
            bitmask |= (1 << i);
        } else if (arg_it->second != T_INT) {
            // 实参不是数值类型，无法确定特化 bitmask。
            return T_DYNAMIC;
        }
    }

    const auto &ret_types = ret_it->second;
    DEBUG_ASSERT(bitmask < static_cast<int>(ret_types.size()));
    return ret_types[static_cast<size_t>(bitmask)];
}

InferredType TypeInferencer::ComputeReturnTypeFromSnapshot(const EvalTypeSnapshot &snapshot, const FuncRetInfo &ret_info) const {
    if (!ret_info.ends_with_return || ret_info.ret_exps.empty()) {
        return T_DYNAMIC;
    }
    InferredType actual_ret = T_INT;// 乐观初值
    for (const auto &ret_exp: ret_info.ret_exps) {
        if (!ret_exp) {
            // nullptr 代表显式的 nil return（return 无表达式）。
            return T_DYNAMIC;
        }
        const auto inferred = [&]() {
            if (const auto it = snapshot.find(ret_exp.get()); it != snapshot.end()) {
                return it->second;
            }
            return T_DYNAMIC;
        }();
        if (inferred == T_FLOAT) {
            if (actual_ret == T_INT) {
                actual_ret = T_FLOAT;
            }
        } else if (inferred != T_INT) {
            return T_DYNAMIC;
        }
    }
    return actual_ret;
}

}// namespace fakelua
