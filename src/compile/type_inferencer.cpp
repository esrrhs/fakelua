#include "compile/type_inferencer.h"

#include <algorithm>
#include <fstream>
#include <ranges>

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "util/common.h"
#include "util/file_util.h"

namespace fakelua {

void TypeInferencer::DumpASTWithTypes(const SyntaxTreeInterfacePtr &node, const EvalTypeSnapshot &snapshot, int tab, std::ostream &os) const {
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
            auto prefixexp = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
            DumpASTWithTypes(prefixexp->GetValue(), snapshot, tab + 1, os);
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

void TypeInferencer::TypeEnvironment::Define(const std::string &name, const InferredType type, SyntaxTreeInterface* init_node) {
    scopes_.back()[name] = EnvEntry{type, init_node};
}

bool TypeInferencer::TypeEnvironment::Update(const std::string &name, const InferredType type, EvalTypeSnapshot &current_map) {
    for (auto &scope: std::views::reverse(scopes_)) {
        if (const auto found = scope.find(name); found != scope.end()) {
            const InferredType old_type = found->second.type;
            const InferredType merged = MergeType(old_type, type);
            found->second.type = merged;

            // 逆向传播：当变量类型退化为 T_DYNAMIC（如后续赋了非数值类型）时，
            // 将初始声明/赋值表达式的类型也退化为 T_DYNAMIC，防止 CGen 将其声明为 C 的原生强类型。
            if (merged == T_DYNAMIC && old_type != T_DYNAMIC && found->second.init_node != nullptr) {
                current_map[found->second.init_node] = T_DYNAMIC;
            }
            return true;
        }
    }
    return false;
}

InferredType TypeInferencer::TypeEnvironment::Lookup(const std::string &name) const {
    for (const auto &scope: std::views::reverse(scopes_)) {
        if (const auto found = scope.find(name); found != scope.end()) {
            return found->second.type;
        }
    }
    return T_DYNAMIC;
}

const SyntaxTreeInterface* TypeInferencer::TypeEnvironment::LookupInitNode(const std::string &name) const {
    for (const auto &scope: std::views::reverse(scopes_)) {
        if (const auto found = scope.find(name); found != scope.end()) {
            return found->second.init_node;
        }
    }
    return nullptr;
}

InferredType TypeInferencer::TypeEnvironment::MergeType(const InferredType old_type, const InferredType new_type) {
    DEBUG_ASSERT(old_type != T_UNKNOWN);
    DEBUG_ASSERT(new_type != T_UNKNOWN);
    if (old_type == T_DYNAMIC || new_type == T_DYNAMIC) {
        return T_DYNAMIC;
    }
    if (old_type == new_type) {
        return old_type;
    }
    if ((old_type == T_INT && new_type == T_FLOAT) || (old_type == T_FLOAT && new_type == T_INT)) {
        return T_FLOAT;
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
    TraversalContext tctx{current_map, env, nullptr, ir.var_define_nodes};

    InferNode(pr.chunk, tctx);
    // 将当前推断结果复制为全局主快照，供 CGen 在非特化路径下查询节点类型。
    ir.main_eval_types = current_map;

    CollectGlobalConstVars(pr, current_map, ir);

    // 在正常推断之后，通过三个阶段发现数学参数并生成特化信息：
    // IdentifyMathParams：多轮迭代识别数学参数
    const auto math_func_info = IdentifyMathParams(pr, ir);
    if (!math_func_info.empty()) {
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

    // 分析 table 形状，填充 table_spec_infos（流不敏感字段并集 + optional 标记）
    AnalyzeTableShapes(pr.chunk, ir);

    // 预计算 per-spec-type 字段布局元数据（按 spec 类型名去重）。
    // CGen 据此发射 typedef / getter / setter 以及字段名/C 字段名/索引/类型查询，
    // 不再自行计算字段布局。
    ComputeSpecTypeMetadata(ir);

    // 预计算数学参数特化上下文（per func+bitmask）：snapshot 指针 + param_types + param_names。
    // CGen::CompileFuncBody 据此初始化发射上下文，不再自行做 MathParamKindOf 推导、snapshot 选择
    // 或 param_types / param_names 初始填充。
    ComputeSpecFuncContext(ir, math_func_info);

    // 流敏感前向分析：为每处 Var 字段引用节点标注「该程序点上该变量的 spec 类型名」。
    // CGen 在 CompileVar（kSquare/kDot）里通过 var_spec_annotations[node] 读取，
    // 不再自行维护 table_spec_types_ / global_table_spec_types_ 等流敏感状态。
    ComputeVarSpecAnnotations(pr, ir);

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
        default:
            ThrowFakeluaException(std::format("InferNode: unexpected SyntaxTreeType: {}", SyntaxTreeTypeToString(node->Type())));
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
        SyntaxTreeInterface* init_node = (i < exps.size()) ? exps[i].get() : nullptr;
        tctx.env.Define(names[i], type, init_node);
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

    const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(varlist->Vars()[0]);
    DEBUG_ASSERT(var && var->GetVarKind() == VarKind::kSimple);

    auto &current_map = tctx.current_map;
    const std::string name = var->GetName();
    InferredType current = T_DYNAMIC;

    const InferredType rhs_type = InferNode(explist->Exps()[0], tctx);

    if (tctx.IsPinnedVar(name)) {
        current = tctx.env.Lookup(name);
    } else if (tctx.env.Update(name, rhs_type, current_map)) {
        current = tctx.env.Lookup(name);
    }

    if (const auto* init = tctx.env.LookupInitNode(name)) {
        tctx.var_define_nodes[var.get()] = init;
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
    const bool all_int = begin_valid && end_valid && begin_type == T_INT && end_type == T_INT && (!for_loop->ExpStep() || step_type == T_INT);
    const bool all_numeric = !all_int && begin_valid && end_valid && (begin_type == T_INT || begin_type == T_FLOAT) && (end_type == T_INT || end_type == T_FLOAT) && step_numeric;
    const InferredType loop_var_type = all_int ? T_INT : (all_numeric ? T_FLOAT : T_DYNAMIC);

    auto &current_map = tctx.current_map;
    tctx.env.EnterScope();
    tctx.env.Define(for_loop->Name(), loop_var_type, for_loop.get());
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
                    if (const auto left_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(exp->Left()); left_exp && left_exp->GetExpKind() == ExpKind::kBinop) {
                        if (const auto left_op = std::dynamic_pointer_cast<SyntaxTreeBinop>(left_exp->Op()); left_op && left_op->GetOpKind() == BinOpKind::kAnd) {
                            const auto it = current_map.find(left_exp->Right().get());
                            if (const auto val1_type = (it != current_map.end()) ? it->second : T_DYNAMIC; IsNumericInferredType(val1_type) && IsNumericInferredType(right_type)) {
                                return RecordType(current_map, exp.get(), (val1_type == right_type) ? val1_type : T_FLOAT);
                            }
                        }
                    }
                }
                return RecordType(current_map, exp.get(), IsNumericInferredType(left_type) ? left_type : T_DYNAMIC);
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
            InferNode(exp->Right(), tctx);
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
            if (const auto* init = tctx.env.LookupInitNode(var->GetName())) {
                tctx.var_define_nodes[var.get()] = init;
            }
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
            const auto baseline = RunTrialInference(info.block, info.params, MakeAssumedParamTypes(info.params, "", T_DYNAMIC, T_DYNAMIC), nullptr, nullptr, true, ir.var_define_nodes);
            const auto all_int = RunTrialInference(info.block, info.params, MakeAssumedParamTypes(info.params, "", T_INT, T_INT), nullptr, nullptr, true, ir.var_define_nodes);
            const auto math_indices = FindMathParamIndices(info, baseline, all_int, ir.math_param_positions, ir.var_define_nodes);

            if (math_indices.empty()) {
                continue;
            }
            if (math_indices.size() > kMaxMathSpecializedParams) {
                LOG_INFO("TypeInferencer: {} math params for {} exceeds limit {}, treating all as dynamic", math_indices.size(), info.name, kMaxMathSpecializedParams);
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
                                                      const std::unordered_map<std::string, std::vector<int>> &known_math_positions,
                                                      std::unordered_map<const SyntaxTreeInterface*, const SyntaxTreeInterface*> &var_define_nodes) {
    std::vector<int> math_indices;
    // 快速剪枝：若全 T_INT 与 baseline 无改善，函数不具备特化价值。
    if (!CheckArithmeticTypeChanges(all_int, baseline, info.block, true, known_math_positions)) {
        return math_indices;
    }

    for (int i = 0; i < static_cast<int>(info.params.size()); ++i) {
        // without_p：除 p_i 为 T_DYNAMIC 外，其余参数均为 T_INT。
        const auto without_p_assumed = MakeAssumedParamTypes(info.params, info.params[i], T_DYNAMIC, T_INT);
        // 若去掉 p_i 后算术/比较/for-loop 退化，则 p_i 是数学参数。
        if (const auto without_p_map = RunTrialInference(info.block, info.params, without_p_assumed, nullptr, nullptr, true, var_define_nodes);
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
            snapshots[static_cast<size_t>(bitmask)] = RunTrialInference(func_block, func_params, assumed, nullptr, nullptr, false, ir.var_define_nodes);
        }
    }
}

TypeInferencer::EvalTypeMap TypeInferencer::RunTrialInference(const SyntaxTreeInterfacePtr &func_block, const std::vector<std::string> &params,
                                                              const std::unordered_map<std::string, InferredType> &assumed_types,
                                                              const std::unordered_map<std::string, std::vector<int>> *math_positions,
                                                              const std::unordered_map<std::string, std::vector<InferredType>> *assumed_ret, bool skip_post_processing,
                                                              std::unordered_map<const SyntaxTreeInterface*, const SyntaxTreeInterface*> &var_define_nodes) {
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
        TraversalContext tctx{current_map, env, &ctx, var_define_nodes};
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

std::unordered_map<std::string, TypeInferencer::FuncRetInfo> TypeInferencer::BuildFunctionReturnCache(const MathFuncInfoMap &math_func_info) const {
    std::unordered_map<std::string, FuncRetInfo> func_ret_cache;
    for (const auto &[func_name, info]: math_func_info) {
        FuncRetInfo ret_info;
        ret_info.ends_with_return = CollectReturnExps(info.block, ret_info.ret_exps);
        func_ret_cache[func_name] = std::move(ret_info);
    }
    return func_ret_cache;
}

void TypeInferencer::InferSpecializationReturnTypes(InferResult &ir, const MathFuncInfoMap &math_func_info, const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache) {
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
                auto new_snapshot = RunTrialInference(func_block, func_params, assumed, &ir.math_param_positions, &ir.specialization_return_types, false, ir.var_define_nodes);

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

std::unordered_map<std::string, InferredType> TypeInferencer::MakeAssumedParamTypes(const std::vector<std::string> &params, const std::string &special_param, const InferredType special_type,
                                                                                    const InferredType default_type) const {
    std::unordered_map<std::string, InferredType> assumed;
    for (const auto &param: params) {
        assumed[param] = (!special_param.empty() && param == special_param) ? special_type : default_type;
    }
    return assumed;
}

std::unordered_map<std::string, InferredType> TypeInferencer::MakeSpecializedParamTypes(const std::vector<std::string> &params, const std::vector<int> &math_indices, const int bitmask) const {
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
        return k == BinOpKind::kPlus || k == BinOpKind::kMinus || k == BinOpKind::kStar || k == BinOpKind::kSlash || k == BinOpKind::kDoubleSlash || k == BinOpKind::kPow || k == BinOpKind::kMod ||
               k == BinOpKind::kBitAnd || k == BinOpKind::kXor || k == BinOpKind::kBitOr || k == BinOpKind::kLeftShift || k == BinOpKind::kRightShift;
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

bool CheckNodeChangeCommon(const SyntaxTreeInterfacePtr &node, const EvalTypeSnapshot &typed_map, const EvalTypeSnapshot &compare_map, const bool improvement_mode) {
    const auto it_typed = typed_map.find(node.get());
    const auto it_compare = compare_map.find(node.get());
    DEBUG_ASSERT(it_typed != typed_map.end() && it_compare != compare_map.end());
    return IsNumericInferredType(it_typed->second) && (improvement_mode ? (it_compare->second == T_DYNAMIC) : (it_compare->second != it_typed->second));
}

}// namespace

bool TypeInferencer::CheckArithmeticNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map, const EvalTypeMap &compare_map, const bool improvement_mode) const {
    return IsArithmeticExpr(node) && CheckNodeChangeCommon(node, typed_map, compare_map, improvement_mode);
}

bool TypeInferencer::CheckComparisonNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map, const EvalTypeMap &compare_map, const bool improvement_mode) const {
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
    DEBUG_ASSERT(lt_typed != typed_map.end() && rt_typed != typed_map.end() && lt_compare != compare_map.end() && rt_compare != compare_map.end());
    if (IsNumericInferredType(lt_typed->second) && IsNumericInferredType(rt_typed->second)) {
        if (improvement_mode) {
            return (lt_compare->second == T_DYNAMIC || rt_compare->second == T_DYNAMIC);
        } else {
            return (lt_compare->second != lt_typed->second || rt_compare->second != rt_typed->second);
        }
    }
    return false;
}

bool TypeInferencer::CheckForLoopNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map, const EvalTypeMap &compare_map, const bool improvement_mode) const {
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

bool TypeInferencer::CheckArithmeticTypeChanges(const EvalTypeMap &typed_map, const EvalTypeMap &compare_map, const SyntaxTreeInterfacePtr &func_block, const bool improvement_mode,
                                                const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found) {
            return;
        }
        if (CheckArithmeticNodeChange(node, typed_map, compare_map, improvement_mode) || CheckComparisonNodeChange(node, typed_map, compare_map, improvement_mode) ||
            CheckForLoopNodeChange(node, typed_map, compare_map, improvement_mode) || CheckCallNodeChange(node, typed_map, compare_map, math_param_positions)) {
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

void TypeInferencer::CollectGlobalConstVars(const ParseResult &pr, const EvalTypeMap &current_map, InferResult &ir) {
    DEBUG_ASSERT(pr.chunk->Type() == SyntaxTreeType::Block);
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);
    for (const auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto namelist = local_var->Namelist();
            const auto explist = local_var->Explist();
            const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
            const auto &names = namelist_ptr->Names();
            std::vector<SyntaxTreeInterfacePtr> exps;
            if (explist) {
                exps = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist)->Exps();
            }
            for (size_t i = 0; i < names.size(); ++i) {
                InferredType type = T_DYNAMIC;
                if (i < exps.size()) {
                    if (auto it = current_map.find(exps[i].get()); it != current_map.end()) {
                        type = it->second;
                    }
                }
                ir.global_const_vars[names[i]] = type;
            }
        }
    }
}

std::string TypeInferencer::FieldKeyDescriptor(const TableFieldInfo &f) {
    // 实现已上移到 compile_common.h 的共享自由函数 TableFieldDescriptor，
    // 确保 spec 类型名哈希与去重/并集逻辑使用完全相同的描述符。
    return fakelua::TableFieldDescriptor(f);
}

void TypeInferencer::MergeFieldsInto(std::vector<TableFieldInfo> &dst, const std::vector<TableFieldInfo> &src) {
    // 按 key 描述符去重并集：已存在则保留 dst 条目（不覆盖），否则追加。
    std::unordered_set<std::string> dst_descs;
    for (const auto &f: dst) {
        dst_descs.insert(FieldKeyDescriptor(f));
    }
    for (const auto &f: src) {
        const auto desc = FieldKeyDescriptor(f);
        if (!dst_descs.contains(desc)) {
            dst.push_back(f);
            dst_descs.insert(desc);
        }
    }
}

bool TypeInferencer::BuildCtorFields(const SyntaxTreeInterfacePtr &tc, std::vector<TableFieldInfo> &out) {
    out.clear();
    if (!tc || tc->Type() != SyntaxTreeType::TableConstructor) return false;
    const auto tc_ptr = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(tc);
    if (!tc_ptr->Fieldlist()) return false;
    const auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(tc_ptr->Fieldlist());
    if (!fieldlist) return false;

    struct Entry {
        TableFieldInfo info;
        std::string desc;
    };
    std::unordered_map<std::string, Entry> unique;
    std::vector<std::string> order;
    int array_idx = 1;

    for (const auto &field: fieldlist->Fields()) {
        const auto fp = std::dynamic_pointer_cast<SyntaxTreeField>(field);
        if (!fp) return false;

        TableFieldInfo f;
        f.type = T_DYNAMIC; // 类型由 CGen 在 emit 时重新推导，这里只关心 key 布局
        std::string desc;

        if (fp->GetFieldKind() == FieldKind::kObject) {
            f.key = fp->Name();
            f.key_kind = TableKeyKind::kString;
            f.c_field_name = fp->Name();
            desc = "S_" + f.key;
        } else {
            // FieldKind::kArray
            if (fp->Key() == nullptr) {
                // 隐式索引：值不能是函数调用或 varargs（可能多返回）
                auto val_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(fp->Value());
                if (val_exp) {
                    if (val_exp->GetExpKind() == ExpKind::kPrefixExp) {
                        auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(val_exp->Right());
                        if (pe && pe->GetPrefixKind() == PrefixExpKind::kFunctionCall) return false;
                        if (pe && pe->GetPrefixKind() == PrefixExpKind::kVar) {
                            auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
                            if (var && var->GetVarKind() == VarKind::kSimple && var->GetName().rfind("__fakelua_vararg_", 0) == 0) return false;
                        }
                    } else if (val_exp->GetExpKind() == ExpKind::kVarParams) {
                        return false;
                    }
                }
                f.key = std::to_string(array_idx);
                f.key_kind = TableKeyKind::kInt;
                f.c_field_name = "_int_" + f.key;
                f.int_value = array_idx;
                desc = "I_" + f.key;
                array_idx++;
            } else {
                const auto key_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(fp->Key());
                if (!key_exp) return false;
                auto kind = key_exp->GetExpKind();
                if (kind == ExpKind::kString) {
                    f.key = key_exp->ExpValue();
                    f.key_kind = TableKeyKind::kString;
                    f.c_field_name = f.key;
                    desc = "S_" + f.key;
                } else if (kind == ExpKind::kNumber) {
                    std::string num_str = key_exp->ExpValue();
                    if (num_str.find('.') == std::string::npos && num_str.find('e') == std::string::npos && num_str.find('E') == std::string::npos) {
                        f.key = num_str;
                        f.key_kind = TableKeyKind::kInt;
                        f.int_value = std::stoll(num_str);
                        std::string sanitized = f.key;
                        std::replace(sanitized.begin(), sanitized.end(), '-', '_');
                        f.c_field_name = "_int_" + sanitized;
                        desc = "I_" + f.key;
                        array_idx = std::max(array_idx, static_cast<int>(f.int_value + 1));
                    } else {
                        f.key = num_str;
                        f.key_kind = TableKeyKind::kFloat;
                        std::string sanitized = num_str;
                        std::replace(sanitized.begin(), sanitized.end(), '.', '_');
                        std::replace(sanitized.begin(), sanitized.end(), '-', '_');
                        std::replace(sanitized.begin(), sanitized.end(), '+', '_');
                        f.c_field_name = "_float_" + sanitized;
                        f.float_value = std::stod(num_str);
                        desc = "F_" + f.key;
                    }
                } else if (kind == ExpKind::kTrue) {
                    f.key = "true";
                    f.key_kind = TableKeyKind::kBool;
                    f.c_field_name = "_bool_true";
                    f.bool_value = true;
                    desc = "B_true";
                } else if (kind == ExpKind::kFalse) {
                    f.key = "false";
                    f.key_kind = TableKeyKind::kBool;
                    f.c_field_name = "_bool_false";
                    f.bool_value = false;
                    desc = "B_false";
                } else {
                    return false; // 非静态 key
                }
            }
        }

        if (!unique.contains(desc)) {
            unique[desc] = {f, desc};
            order.push_back(desc);
        } else {
            ThrowFakeluaException(std::format("duplicate key in table constructor: {}", desc.substr(2)));
        }
    }

    out.reserve(order.size());
    for (const auto &d: order) {
        out.push_back(unique[d].info);
    }
    return true;
}

void TypeInferencer::AnalyzeTableShapes(const SyntaxTreeInterfacePtr &chunk, InferResult &ir) {
    // 流不敏感的 per-variable 字段并集分析（函数级隔离）。
    //
    // 原理：为让 if-else 两分支构造的不同 shape table（如 {x} 和 {y}）能统一到同一
    // 结构体布局，每个 constructor 必须按其目标变量的「所有赋值字段并集」来 emit。
    // 这样两分支的 constructor 产生相同的 spec-type-name（由字段签名决定），
    // CGen 侧 Phase 1 的字符串比较 join 自然判定为一致并保留，字段访问走 FL_SPEC。
    //
    // optional 标记：某字段在当前 constructor 字面量中不存在（由兄弟分支贡献），
    // CGen emit 时需显式 nil 初始化该字段，避免 temp allocator 不清零导致的未定义读。
    //
    // 函数级隔离：不同函数的局部变量即使同名也独立分析，避免跨函数并集导致
    // shape 膨胀（如 test_table_basic 的 {1..5} 和 test_table_string_keys 的
    // {name,age,city} 被错误合并成 15 字段结构体）。嵌套函数体进入时压栈、退出时清栈。

    std::unordered_map<const SyntaxTreeInterface *, std::vector<TableFieldInfo>> ctor_own_fields;

    struct FuncFrame {
        std::unordered_map<const SyntaxTreeInterface *, std::string> ctor_target_vars;
        std::unordered_map<std::string, std::vector<TableFieldInfo>> var_fields;
    };
    std::vector<std::shared_ptr<FuncFrame>> frames;
    auto push_frame = [&]() { frames.push_back(std::make_shared<FuncFrame>()); };
    // 注意：不 pop frame。所有 frame 保留到 stamping 阶段供查询，
    // 因为 ctor_target_vars / var_fields 在函数退出后仍需被 stamp 循环访问。
    // 函数级隔离由 push_frame 实现（每个 function body 进入时新开 frame）。
    push_frame(); // 顶层 frame（文件级 / __fakelua_init）

    // 辅助：从 table constructor 节点构建 own_fields 并登记
    auto record_ctor_node = [&](const SyntaxTreeInterfacePtr &tc) -> const SyntaxTreeInterface * {
        if (!tc || tc->Type() != SyntaxTreeType::TableConstructor) return nullptr;
        std::vector<TableFieldInfo> own;
        if (!BuildCtorFields(tc, own) || own.empty()) return nullptr;
        const auto *key = tc.get();
        ctor_own_fields[key] = std::move(own);
        return key;
    };
    // 辅助：若 exp 是 table constructor 表达式，提取其 tc 节点并登记
    auto record_ctor_exp = [&](const SyntaxTreeInterfacePtr &exp) -> const SyntaxTreeInterface * {
        if (!exp || exp->Type() != SyntaxTreeType::Exp) return nullptr;
        auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
        if (!e || e->GetExpKind() != ExpKind::kTableConstructor) return nullptr;
        // SyntaxTreeExp 将 table constructor 存于 Right()
        return record_ctor_node(e->Right());
    };

    std::function<void(const SyntaxTreeInterfacePtr &)> walk = [&](const SyntaxTreeInterfacePtr &node) {
        if (!node) return;
        switch (node->Type()) {
            case SyntaxTreeType::Block: {
                auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
                for (const auto &stmt: blk->Stmts()) { walk(stmt); }
                return;
            }
            case SyntaxTreeType::LocalVar: {
                auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
                const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
                std::vector<SyntaxTreeInterfacePtr> exps;
                if (auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist())) {
                    exps = el->Exps();
                    for (const auto &exp: exps) { walk(exp); }
                }
                if (namelist) {
                    const auto &names = namelist->Names();
                    for (size_t i = 0; i < names.size() && i < exps.size(); ++i) {
                        if (const auto *tc_key = record_ctor_exp(exps[i])) {
                            auto &frame = *frames.back();
                            frame.ctor_target_vars[tc_key] = names[i];
                            MergeFieldsInto(frame.var_fields[names[i]], ctor_own_fields[tc_key]);
                        }
                    }
                }
                return;
            }
            case SyntaxTreeType::Assign: {
                auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
                const auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
                const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
                if (explist) {
                    for (const auto &exp: explist->Exps()) { walk(exp); }
                }
                // 预处理保证 1 var / 1 exp
                if (varlist && explist && !explist->Exps().empty()) {
                    const auto v = varlist->Vars().empty() ? nullptr : std::dynamic_pointer_cast<SyntaxTreeVar>(varlist->Vars()[0]);
                    if (v && v->GetVarKind() == VarKind::kSimple) {
                        if (const auto *tc_key = record_ctor_exp(explist->Exps()[0])) {
                            auto &frame = *frames.back();
                            frame.ctor_target_vars[tc_key] = v->GetName();
                            MergeFieldsInto(frame.var_fields[v->GetName()], ctor_own_fields[tc_key]);
                        }
                    }
                }
                return;
            }
            case SyntaxTreeType::Exp: {
                auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
                // 表达式本身的 table constructor 子节点也要登记（无目标变量的 ctor）
                if (exp->GetExpKind() == ExpKind::kTableConstructor) {
                    record_ctor_node(exp->Right());
                }
                walk(exp->Left());
                walk(exp->Right());
                return;
            }
            case SyntaxTreeType::PrefixExp: {
                auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
                walk(pe->GetValue());
                return;
            }
            case SyntaxTreeType::Var: {
                auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
                if (var->GetVarKind() != VarKind::kSimple) {
                    walk(var->GetPrefixexp());
                }
                return;
            }
            case SyntaxTreeType::TableConstructor:
                record_ctor_node(node);
                // Recurse into nested constructors so they are also recorded in
                // table_spec_infos (e.g. { outer = { inner = 5 } }).
                if (const auto tc_ptr = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node); tc_ptr->Fieldlist()) {
                    walk(tc_ptr->Fieldlist());
                }
                return;
            case SyntaxTreeType::While: {
                auto while_stmt = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
                walk(while_stmt->Exp());
                walk(while_stmt->Block());
                return;
            }
            case SyntaxTreeType::Repeat: {
                auto repeat_stmt = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
                walk(repeat_stmt->Block());
                walk(repeat_stmt->Exp());
                return;
            }
            case SyntaxTreeType::If: {
                auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
                walk(if_stmt->Exp());
                walk(if_stmt->Block());
                if (const auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs())) {
                    for (size_t i = 0; i < elseifs->ElseifSize(); ++i) {
                        walk(elseifs->ElseifExp(i));
                        walk(elseifs->ElseifBlock(i));
                    }
                }
                if (auto else_block = if_stmt->ElseBlock()) { walk(else_block); }
                return;
            }
            case SyntaxTreeType::ForLoop: {
                auto for_stmt = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
                if (for_stmt->ExpBegin()) walk(for_stmt->ExpBegin());
                if (for_stmt->ExpEnd()) walk(for_stmt->ExpEnd());
                if (for_stmt->ExpStep()) walk(for_stmt->ExpStep());
                walk(for_stmt->Block());
                return;
            }
            case SyntaxTreeType::ForIn: {
                auto forin_stmt = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
                walk(forin_stmt->Explist());
                walk(forin_stmt->Block());
                return;
            }
            case SyntaxTreeType::Function: {
                auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
                // 函数体是变量作用域边界，进入时压栈（不 pop，见 frame 定义注释），
                // 隔离不同函数的同名局部变量（如各函数的 local t）。
                push_frame();
                walk(func->Funcbody());
                return;
            }
            case SyntaxTreeType::LocalFunction: {
                auto lf = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node);
                push_frame();
                walk(lf->Funcbody());
                return;
            }
            case SyntaxTreeType::FuncBody: {
                auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(node);
                if (fb->Parlist()) walk(fb->Parlist());
                walk(fb->Block());
                return;
            }
            case SyntaxTreeType::Return: {
                auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
                walk(ret->Explist());
                return;
            }
            case SyntaxTreeType::ExpList: {
                auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
                for (const auto &exp: el->Exps()) { walk(exp); }
                return;
            }
            case SyntaxTreeType::FunctionCall: {
                auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
                walk(fc->prefixexp());
                walk(fc->Args());
                return;
            }
            case SyntaxTreeType::Args: {
                auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
                if (auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(args->Explist())) {
                    for (const auto &exp: el->Exps()) { walk(exp); }
                } else if (args->GetArgsKind() == ArgsKind::kTableConstructor) {
                    walk(args->Tableconstructor());
                }
                return;
            }
            case SyntaxTreeType::None:
            case SyntaxTreeType::Empty:
            case SyntaxTreeType::Label:
            case SyntaxTreeType::VarList:
            case SyntaxTreeType::Break:
            case SyntaxTreeType::Goto:
            case SyntaxTreeType::ElseIfList:
            case SyntaxTreeType::NameList:
            case SyntaxTreeType::FunctionDef:
            case SyntaxTreeType::FuncNameList:
            case SyntaxTreeType::FuncName:
            case SyntaxTreeType::ParList:
            case SyntaxTreeType::Binop:
            case SyntaxTreeType::Unop:
                return;
            case SyntaxTreeType::FieldList: {
                const auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
                for (const auto &field: fieldlist->Fields()) { walk(field); }
                return;
            }
            case SyntaxTreeType::Field: {
                const auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
                if (field->Key()) walk(field->Key());
                walk(field->Value());
                return;
            }
        }
        ThrowFakeluaException("AnalyzeTableShapes: unhandled SyntaxTreeType");
    };

    walk(chunk);

    // 为每个登记过的 constructor 节点计算其应 emit 的合并字段布局
    for (const auto &[tc_key, own]: ctor_own_fields) {
        TableSpecInfo info;
        info.can_specialize = true;

        std::unordered_set<std::string> own_descs;
        for (const auto &f: own) {
            own_descs.insert(FieldKeyDescriptor(f));
        }

        // 在所有函数 frame 中查找该 ctor 的目标变量（ctor 节点唯一，只可能在一个 frame 中）
        const std::vector<TableFieldInfo> *merged_ptr = nullptr;
        for (const auto &frame_ptr: frames) {
            const auto &frame = *frame_ptr;
            const auto tv_it = frame.ctor_target_vars.find(tc_key);
            if (tv_it != frame.ctor_target_vars.end()) {
                merged_ptr = &frame.var_fields.at(tv_it->second);
                break;
            }
        }

        if (merged_ptr != nullptr) {
            // 有目标变量：用变量的字段并集作为布局，标记 optional
            info.fields = *merged_ptr;
            for (auto &f: info.fields) {
                if (!own_descs.contains(FieldKeyDescriptor(f))) {
                    f.optional = true;
                } else {
                    f.optional = false;
                }
            }
        } else {
            // 无目标变量（如函数参数、返回值）：用自身字段，全部 non-optional
            info.fields = own;
        }
        ir.table_spec_infos[tc_key] = std::move(info);
    }
}

// ===========================================================================
// 流敏感 table 特化前向分析
// ===========================================================================
//
// 为目标：为每个 Var(kSquare)/Var(kDot)「读」引用节点标注"在该程序点上，该变量的 spec
// 类型名（空串 = dynamic）"。CGen 在 CompileVar 中通过 var_spec_annotations[node] 读取，
// 不再自行维护 table_spec_types_ / global_table_spec_types_。
//
// 状态 (FlowState)：
//   local : 变量名 → spec 类型名。每函数清空；if-else 汇合时各分支一致才保留。
//   global: 顶层 chunk（即 __fakelua_init 函数）的局部变量 → spec 类型名，跨函数持久。
//           跨函数写入条件等价于 CGen "cur_spec_func_name_.empty() || ==__fakelua_init"：
//           即任意 __fakelua_init 内赋值（顶层 chunk 编译）写入 global；其它函数仅写 local。
//           每个函数内同样在 if-else 汇合时按 local 规则合并。
//
// 读取优先级 local -> global 对应 GetSpecTypeForVar 的两级 map 查找。
//
// 语义关键：对同一被赋值变量，CGen 为字面量构造器引用的是合并布局（ir.table_spec_infos），
// 这与 AnalyzeTableShapes 的跨分支字段并集一致 → if 两分支即使构造字面量字段不同也产出同一
// spec 类型名，汇合后自然保留。fcn-call / varargs 多返回值一律降为空（不设 spec）。

std::string TypeInferencer::LookupSpec(const FlowState &st, const std::string &name) {
    if (name.empty()) return "";
    if (auto it = st.local.find(name); it != st.local.end()) return it->second;
    if (auto git = st.global.find(name); git != st.global.end()) return git->second;
    return "";
}

std::string TypeInferencer::SpecFromRhs(const SyntaxTreeInterfacePtr &exp, const FlowState &st, const InferResult &ir) {
    // 只有 Exp 能是构造器 / 前缀引用 / 字面量。
    if (!exp || exp->Type() != SyntaxTreeType::Exp) return "";
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto kind = e->GetExpKind();

    // 情形 1：字面量构造器。用合并布局（若可特化且登记过）。
    if (kind == ExpKind::kTableConstructor) {
        const auto tc = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(e->Right());
        if (!tc) return "";
        const auto it = ir.table_spec_infos.find(tc.get());
        if (it == ir.table_spec_infos.end()) return "";
        if (it->second.fields.empty()) return "";
        return ComputeTableSpecName(it->second.fields);
    }

    // 情形 2：简单变量拷贝 — 与 CGen 的 table_spec_types_.find(copy) 等价。
    const auto name = GetSimpleVarName(exp);
    if (!name.empty()) return LookupSpec(st, name);

    // 情形 3：函数调用 / varargs / 二元运算… 一律 dynamic。
    return "";
}

void TypeInferencer::AnnotateExprs(const SyntaxTreeInterfacePtr &node, FlowState &st, InferResult &ir) {
    if (!node) return;

    switch (node->Type()) {
    case SyntaxTreeType::Var: {
        const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
        const auto var_kind = var->GetVarKind();
        if (var_kind == VarKind::kSquare || var_kind == VarKind::kDot) {
            // 字段读：receiver 指代的变量在其程序点上应取到的 spec 类型名。
            // 以 prefixexp 节点为 key，与 CGen 在 GetSpecTypeForVar(pe) 处的查找保持一致。
            const SyntaxTreeInterfacePtr receiver = var->GetPrefixexp();
            if (receiver) {
                const auto receiver_name = GetSimpleVarName(receiver);
                const auto spec = LookupSpec(st, receiver_name);
                if (!spec.empty()) {
                    ir.var_spec_annotations[receiver.get()] = spec;
                }
                // CGen 会通过 CompileExp 继续编译 prefixexp 子表达式：若它本身是 Var 字段读，
                // 则会被单独再调一次 CompileVar + GetSpecTypeForVar，故这里也递归前缀以逐层标注。
                AnnotateExprs(receiver, st, ir);
            }
        }
        if (var_kind == VarKind::kSquare) {
            // 下标表达式里可能含字段读。
            AnnotateExprs(var->GetExp(), st, ir);
        }
        return;
    }
    case SyntaxTreeType::FunctionCall: {
        const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
        // FAKELUA_SET_TABLE(t, key, val)：receiver 是 args[0] 里的 prefixExp。
        if (const auto pf = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp())) {
            if (pf->GetPrefixKind() == PrefixExpKind::kVar) {
                const auto fn_var = std::dynamic_pointer_cast<SyntaxTreeVar>(pf->GetValue());
                if (fn_var && fn_var->GetVarKind() == VarKind::kSimple && fn_var->GetName() == "FAKELUA_SET_TABLE") {
                    const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
                    if (args && args->GetArgsKind() == ArgsKind::kExpList) {
                        const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(args->Explist());
                        if (explist && !explist->Exps().empty()) {
                            const auto first = explist->Exps()[0];
                            const auto recv_name = GetSimpleVarName(first);
                            const auto spec = LookupSpec(st, recv_name);
                            if (!spec.empty()) {
                                ir.var_spec_annotations[first.get()] = spec;
                            }
                        }
                    }
                    return;
                }
            }
        }
        // 普通调用：遍历前缀与参数，捕获其内的字段读。
        AnnotateExprs(fc->prefixexp(), st, ir);
        AnnotateExprs(fc->Args(), st, ir);
        return;
    }
    case SyntaxTreeType::Args: {
        const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
        AnnotateExprs(args->Explist(), st, ir);
        AnnotateExprs(args->Tableconstructor(), st, ir);
        AnnotateExprs(args->String(), st, ir);
        return;
    }
    case SyntaxTreeType::ExpList: {
        const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
        for (const auto &exp: el->Exps()) AnnotateExprs(exp, st, ir);
        return;
    }
    case SyntaxTreeType::Exp: {
        const auto exp_node = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
        const auto k = exp_node->GetExpKind();
        if (k == ExpKind::kPrefixExp) {
            AnnotateExprs(exp_node->Right(), st, ir);
        } else if (k == ExpKind::kBinop) {
            AnnotateExprs(exp_node->Left(), st, ir);
            AnnotateExprs(exp_node->Right(), st, ir);
        } else if (k == ExpKind::kUnop) {
            AnnotateExprs(exp_node->Right(), st, ir);
        } else if (k == ExpKind::kTableConstructor) {
            // 构造器字段值里可能包含字段读。
            const auto tc = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(exp_node->Right());
            if (tc) AnnotateExprs(tc->Fieldlist(), st, ir);
        }
        return;
    }
    case SyntaxTreeType::FieldList: {
        const auto fl = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
        for (const auto &field: fl->Fields()) AnnotateExprs(field, st, ir);
        return;
    }
    case SyntaxTreeType::Field: {
        const auto field_node = std::dynamic_pointer_cast<SyntaxTreeField>(node);
        AnnotateExprs(field_node->Key(), st, ir);
        AnnotateExprs(field_node->Value(), st, ir);
        return;
    }
    case SyntaxTreeType::PrefixExp: {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
        AnnotateExprs(pe->GetValue(), st, ir);
        return;
    }
    case SyntaxTreeType::TableConstructor: {
        const auto tc = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
        AnnotateExprs(tc->Fieldlist(), st, ir);
        return;
    }
    case SyntaxTreeType::Return: {
        const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
        AnnotateExprs(ret->Explist(), st, ir);
        return;
    }
    default:
        return;
    }
}

void TypeInferencer::FlowBlock(const SyntaxTreeInterfacePtr &block, FlowState &st, InferResult &ir, const bool is_top_level) {
    if (!block || block->Type() != SyntaxTreeType::Block) return;
    const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);
    for (const auto &stmt: block_ptr->Stmts()) {
        FlowStmt(stmt, st, ir, is_top_level);
    }
}

void TypeInferencer::FlowStmt(const SyntaxTreeInterfacePtr &stmt, FlowState &st, InferResult &ir, const bool is_top_level) {
    if (!stmt) return;

    switch (stmt->Type()) {
    case SyntaxTreeType::LocalVar: {
        // 先标注所有读（右侧表达式里的字段引用）。
        const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
        if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist())) {
            for (const auto &exp: explist->Exps()) AnnotateExprs(exp, st, ir);
        }
        // 再按语义更新 local / global。
        const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
        if (!namelist) return;
        const auto exps = [&]() -> std::vector<SyntaxTreeInterfacePtr> {
            if (const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist())) return el->Exps();
            return {};
        }();
        const auto &names = namelist->Names();
        // 多返回值（函数调用 / varargs 最后一位 + 名字多于表达式）：尾部全部降 dynamic。
        const bool multi_return = names.size() > exps.size();
        for (size_t i = 0; i < names.size(); ++i) {
            const auto &name = names[i];
            std::string spec;
            if (i < exps.size()) {
                spec = SpecFromRhs(exps[i], st, ir);
            }
            // 多返回：从 exps.size()-1 位起全部 dynamic (i >= exps.size()-1 即尾部)。
            if (multi_return && static_cast<int>(i) >= static_cast<int>(exps.size()) - 1) {
                spec.clear();
            }
            if (spec.empty()) {
                st.local.erase(name);
                st.global.erase(name);
            } else {
                st.local[name] = spec;
                if (is_top_level) st.global[name] = spec;
            }
        }
        return;
    }
    case SyntaxTreeType::Assign: {
        const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
        // 标注右侧字段读。
        if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist())) {
            for (const auto &exp: explist->Exps()) AnnotateExprs(exp, st, ir);
        }
        const auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
        const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
        // 预处理保证单变量、单表达式。
        if (!varlist || !explist || varlist->Vars().empty() || explist->Exps().empty()) return;
        const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(varlist->Vars()[0]);
        if (!var) return;
        const auto name = var->GetName();
        const auto spec = SpecFromRhs(explist->Exps()[0], st, ir);
        if (spec.empty()) {
            st.local.erase(name);
            st.global.erase(name);
        } else {
            st.local[name] = spec;
            if (is_top_level) st.global[name] = spec;
        }
        return;
    }
    case SyntaxTreeType::If: {
        const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
        // 条件里的字段读在汇合前求值：用 if 之前的状态。
        AnnotateExprs(if_node->Exp(), st, ir);

        const auto snapshot_local = st.local;
        const auto snapshot_global = st.global;

        std::vector<FlowState> branch_states;
        // then 分支
        {
            FlowState branch{st};
            FlowBlock(if_node->Block(), branch, ir, is_top_level);
            branch_states.push_back(std::move(branch));
        }
        // elseif 分支
        if (const auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_node->ElseIfs())) {
            for (size_t i = 0; i < elseifs->ElseifSize(); ++i) {
                // 条件读使用 if 前状态。
                AnnotateExprs(elseifs->ElseifExp(i), st, ir);
                FlowState branch{snapshot_local, snapshot_global};
                FlowBlock(elseifs->ElseifBlock(i), branch, ir, is_top_level);
                branch_states.push_back(std::move(branch));
            }
        }
        // else 分支
        if (const auto else_block = if_node->ElseBlock()) {
            FlowState branch{snapshot_local, snapshot_global};
            FlowBlock(else_block, branch, ir, is_top_level);
            branch_states.push_back(std::move(branch));
        } else {
            // 无 else：隐式 else 路径保持 if 前状态（等同 s0），纳入汇合。
            branch_states.push_back(FlowState{snapshot_local, snapshot_global});
        }

        // 汇合：各分支同 key 值全一致才保留。
        JoinFlowStates(branch_states, st);
        return;
    }
    case SyntaxTreeType::While: {
        const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);
        AnnotateExprs(while_node->Exp(), st, ir);
        // CGen 不快照循环：精确复现直接贯穿（虽理论上多轮可能降级，但保持与原 emitter 行为一致）。
        FlowBlock(while_node->Block(), st, ir, is_top_level);
        return;
    }
    case SyntaxTreeType::Repeat: {
        const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);
        FlowBlock(rep->Block(), st, ir, is_top_level);
        AnnotateExprs(rep->Exp(), st, ir);
        return;
    }
    case SyntaxTreeType::ForLoop: {
        const auto for_node = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
        AnnotateExprs(for_node->ExpBegin(), st, ir);
        AnnotateExprs(for_node->ExpEnd(), st, ir);
        if (for_node->ExpStep()) AnnotateExprs(for_node->ExpStep(), st, ir);
        FlowBlock(for_node->Block(), st, ir, is_top_level);
        return;
    }
    case SyntaxTreeType::ForIn: {
        const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
        if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(for_in->Explist())) {
            for (const auto &exp: explist->Exps()) AnnotateExprs(exp, st, ir);
        }
        FlowBlock(for_in->Block(), st, ir, is_top_level);
        return;
    }
    default:
        // Break / Label / Goto / Return / ExprStmt 等无赋值语义，仅标注可能含的字段读。
        AnnotateExprs(stmt, st, ir);
        return;
    }
}

void TypeInferencer::JoinFlowStates(const std::vector<FlowState> &branch_states, FlowState &out) {
    // 分别汇合 local 与 global；任一分支缺失该 key 或不一致 → 不写入 out（即 dynamic）。
    auto join_one = [](const std::vector<FlowState> &branches, const auto &selector, auto &out_map) {
        out_map.clear();
        std::unordered_set<std::string> keys;
        for (const auto &bs: branches) {
            const auto &m = selector(bs);
            for (const auto &[k, v]: m) keys.insert(k);
        }
        for (const auto &k: keys) {
            std::string consistent;
            bool conflict = false;
            for (const auto &bs: branches) {
                const auto &m = selector(bs);
                const auto it = m.find(k);
                if (it == m.end()) {
                    conflict = true;
                    break;
                }
                if (consistent.empty()) {
                    consistent = it->second;
                } else if (consistent != it->second) {
                    conflict = true;
                    break;
                }
            }
            if (!conflict) out_map[k] = consistent;
        }
    };
    join_one(branch_states, [](const FlowState &s) -> const auto & { return s.local; }, out.local);
    join_one(branch_states, [](const FlowState &s) -> const auto & { return s.global; }, out.global);
}

void TypeInferencer::ComputeVarSpecAnnotations(const ParseResult &pr, InferResult &ir) {
    FlowState st;

    // 顶层 chunk：CGen 把非函数语句塞进 __fakelua_init，先处理顶层（写 global），再处理各函数。
    if (!pr.chunk || pr.chunk->Type() != SyntaxTreeType::Block) return;
    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);

    // 收集函数定义（顶层函数 + 嵌套函数均独立作用域）。
    // __fakelua_init 对应 CGen 里"优先编译"的顶层 chunk 初始化函数：其内赋值写 global，
    // 必须先于其它函数处理，才能使全局特化状态在后续函数读引用中可见。
    std::vector<SyntaxTreeInterfacePtr> funcs;
    std::vector<SyntaxTreeInterfacePtr> top_stmts;
    SyntaxTreeInterfacePtr init_func_stmt;
    const auto is_init = [](const SyntaxTreeInterfacePtr &stmt) -> bool {
        // 与 CGen::CompileFuncName 等价：取函数名链表的唯一定段。
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto f = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto fn = std::dynamic_pointer_cast<SyntaxTreeFuncname>(f->Funcname());
            const auto fnl = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(fn->FuncNameList());
            const auto &names = fnl->Funcnames();
            return names.size() == 1 && names[0] == kInitFunctionName;
        }
        if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            const auto lf = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
            return lf->Name() == kInitFunctionName;
        }
        return false;
    };
    for (const auto &stmt: top_block->Stmts()) {
        if (is_init(stmt)) {
            init_func_stmt = stmt;
        } else if (stmt->Type() == SyntaxTreeType::Function || stmt->Type() == SyntaxTreeType::LocalFunction) {
            funcs.push_back(stmt);
        } else {
            top_stmts.push_back(stmt);
        }
    }

    // 顶层语句（top_level=true → 赋值写 global）。
    for (const auto &stmt: top_stmts) {
        FlowStmt(stmt, st, ir, true);
    }

    // 先处理 __fakelua_init，使其对全局变量的 spec 赋值流入后续函数。
    if (init_func_stmt) {
        FlowState init_state{st.global};
        if (auto bb = FuncBodyBlock(init_func_stmt)) FlowBlock(bb, init_state, ir, true);
        // init 内的赋值写入 global（is_top_level=true），但本身也是函数作用域：
        // 将 init 执行结束后的 global 作为后续函数的 global 基线。
        st.global = std::move(init_state.global);
    }

    // 各函数体：每函数清空 local（global 保持顶层/init 写入）。
    for (const auto &func: funcs) {
        auto body_block = FuncBodyBlock(func);
        if (!body_block) continue;
        // 函数入口清空 local 作用域（对应 CGen 每函数清空 table_spec_types_）。
        FlowState func_state{st.global};
        FlowBlock(body_block, func_state, ir, false);
    }
}

// 辅助：取函数语句（Function/LocalFunction）的 body block。
SyntaxTreeInterfacePtr TypeInferencer::FuncBodyBlock(const SyntaxTreeInterfacePtr &func) {
    if (!func) return nullptr;
    if (func->Type() == SyntaxTreeType::Function) {
        const auto f = std::dynamic_pointer_cast<SyntaxTreeFunction>(func);
        const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(f->Funcbody());
        return fb ? fb->Block() : nullptr;
    }
    if (func->Type() == SyntaxTreeType::LocalFunction) {
        const auto lf = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(func);
        const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(lf->Funcbody());
        return fb ? fb->Block() : nullptr;
    }
    return nullptr;
}

void TypeInferencer::ComputeSpecTypeMetadata(InferResult &ir) {
    // 遍历所有已登记的 table constructor（ir.table_spec_infos），按 spec 类型名去重。
    // 同名类型必有相同字段布局（spec 类型名是字段签名的哈希），取任一即可。
    for (const auto &[tc, info]: ir.table_spec_infos) {
        if (!info.can_specialize || info.fields.empty()) continue;
        const auto spec_type = ComputeTableSpecName(info.fields);
        if (ir.spec_type_metadata.contains(spec_type)) continue; // 已登记，跳过

        SpecTypeMetadata meta;
        meta.name = spec_type;
        meta.fields = info.fields;
        for (const auto &f: info.fields) {
            switch (f.key_kind) {
                case TableKeyKind::kString: meta.has_string_keys = true; break;
                case TableKeyKind::kInt:    meta.has_int_keys = true;    break;
                case TableKeyKind::kFloat:  meta.has_float_keys = true;  break;
                case TableKeyKind::kBool:   meta.has_bool_keys = true;   break;
            }
            const auto desc = TableFieldDescriptor(f);
            meta.field_key_descs.insert(desc);
            meta.c_field_names[desc] = f.c_field_name;
            meta.field_types[desc] = f.type;
        }
        // 字段索引：按 emit 顺序（fields 已是排序后布局）编号，与 CGen 原行为一致。
        int idx = 0;
        for (const auto &f: info.fields) {
            meta.field_indices[TableFieldDescriptor(f)] = idx++;
        }
        ir.spec_type_metadata[spec_type] = std::move(meta);
    }
}

void TypeInferencer::ComputeSpecFuncContext(InferResult &ir, const MathFuncInfoMap &math_func_info) {
    // 为每个含数学参数的函数、每个特化 bitmask 预计算 SpecFuncContext。
    // 同时填充 param_names（全参数名列表）和 param_types（数学参数名→特化类型），
    // 使 CGen::CompileFuncBody 直接读取而无需手动拼接 bitmask + func_params。
    for (const auto &[func_name, snaps]: ir.specialization_snapshots) {
        auto &ctx_vec = ir.spec_func_context[func_name];
        ctx_vec.resize(snaps.size());

        // 从 math_func_info 中取该函数的参数名列表
        const std::vector<std::string> *func_params = nullptr;
        if (const auto it = math_func_info.find(func_name); it != math_func_info.end()) {
            func_params = &it->second.params;
        }

        const auto &math_indices = ir.math_param_positions.at(func_name);

        for (size_t bitmask = 0; bitmask < snaps.size(); ++bitmask) {
            SpecFuncContext &ctx = ctx_vec[bitmask];
            ctx.func_name = func_name;
            ctx.bitmask = static_cast<int>(bitmask);
            ctx.snapshot = &snaps[bitmask];

            if (func_params) {
                // 填充 param_names（所有参数按位置顺序）
                ctx.param_names = *func_params;

                // 填充 param_types（数学参数名 → 特化类型）
                for (int i = 0; i < static_cast<int>(math_indices.size()); ++i) {
                    const int param_pos = math_indices[i];
                    if (param_pos < static_cast<int>(func_params->size())) {
                        const auto &pname = (*func_params)[static_cast<size_t>(param_pos)];
                        ctx.param_types[pname] = (MathParamKindOf(static_cast<int>(bitmask), i) == kMathParamFloat) ? T_FLOAT : T_INT;
                    }
                }
            }
        }
    }
}

}// namespace fakelua
