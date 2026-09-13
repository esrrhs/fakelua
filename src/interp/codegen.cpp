#include "interp/codegen.h"

#include "compile/compile_common.h"
#include "interp/interpreter.h"
#include "interp/runtime.h"
#include "jit/vm.h"
#include "state/const_string.h"
#include "state/state.h"
#include "util/exception.h"
#include "util/string_util.h"
#include "var/var.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <format>
#include <limits>

namespace fakelua {

const std::unordered_set<std::string> InterpCodegen::kStringLibraryMethods = {"len", "sub", "rep", "reverse", "lower", "upper", "byte", "char", "format", "dump", "find", "match", "gmatch", "gsub"};

InterpCodegen::InterpCodegen(State *s) : s_(s) {
}

[[noreturn]] void InterpCodegen::ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) {
    ThrowFakeluaException(std::format("Code generate failed, {} at {}", msg, SyntaxTreeLocationStr(file_name_, ptr)));
}

int InterpCodegen::AllocReg() {
    if (stack_top_ < local_top_) {
        stack_top_ = local_top_;
    }
    const int r = stack_top_++;
    if (cur_proto_ && stack_top_ > cur_proto_->max_stack) {
        cur_proto_->max_stack = stack_top_;
    }
    return r;
}

int InterpCodegen::AllocLocal() {
    // 循环控制槽是 AllocReg 临时量，循环体里的 AllocLocal 不能踩上去。
    if (local_top_ < stack_top_) {
        local_top_ = stack_top_;
    }
    const int r = local_top_++;
    stack_top_ = local_top_;
    if (cur_proto_ && stack_top_ > cur_proto_->max_stack) {
        cur_proto_->max_stack = stack_top_;
    }
    return r;
}

void InterpCodegen::FreeTo(int top) {
    if (top < local_top_) {
        top = local_top_;
    }
    stack_top_ = top;
}

void InterpCodegen::EnsureStack(int n) {
    if (stack_top_ < n) {
        stack_top_ = n;
    }
    if (cur_proto_ && stack_top_ > cur_proto_->max_stack) {
        cur_proto_->max_stack = stack_top_;
    }
}

int InterpCodegen::AddConst(const CVar &v) {
    cur_proto_->constants.push_back(v);
    return static_cast<int>(cur_proto_->constants.size()) - 1;
}

int InterpCodegen::AddStringConst(const std::string &s) {
    CVar v{};
    v.type_ = static_cast<int>(VarType::StringId);
    v.data_.i = s_->GetConstString().Alloc(s);
    return AddConst(v);
}

int InterpCodegen::Emit(Op op, int a, int b, int c, int sbx) {
    Inst inst;
    inst.op = op;
    inst.a = static_cast<uint16_t>(a);
    inst.b = static_cast<uint16_t>(b);
    inst.c = static_cast<uint16_t>(c);
    inst.sbx = sbx;
    const int ip = static_cast<int>(cur_proto_->code.size());
    cur_proto_->code.push_back(inst);
    cur_proto_->lineinfo.push_back(0);
    return ip;
}

void InterpCodegen::PatchSbx(int ip, int dest) {
    cur_proto_->code[static_cast<size_t>(ip)].sbx = dest - ip - 1;
}

int InterpCodegen::LoadConstTo(int dest, const CVar &v) {
    if (v.type_ == static_cast<int>(VarType::Nil)) {
        Emit(Op::LOADNIL, dest);
        return dest;
    }
    if (v.type_ == static_cast<int>(VarType::Bool)) {
        Emit(Op::LOADBOOL, dest, v.data_.b ? 1 : 0);
        return dest;
    }
    const int k = AddConst(v);
    Emit(Op::LOADK, dest, 0, 0, k);
    return dest;
}

int InterpCodegen::MoveTo(int dest, int src) {
    if (dest != src) {
        Emit(Op::MOVE, dest, src);
    }
    return dest;
}

void InterpCodegen::StoreLocal(int dest, int src) {
    MoveTo(dest, src);
}

int InterpCodegen::LoadLocal(VarDef *def) {
    if (def->defining_func == cur_func_) {
        return def->reg;
    }
    if (def->defining_func != nullptr) {
        int idx = 0;
        if (cur_func_) {
            const auto it = std::find(cur_func_->captured_vars.begin(), cur_func_->captured_vars.end(), def);
            if (it != cur_func_->captured_vars.end()) {
                idx = static_cast<int>(it - cur_func_->captured_vars.begin());
            }
        }
        const int r = AllocReg();
        Emit(Op::GETUPVAL, r, idx);
        return r;
    }
    const int r = AllocReg();
    Emit(Op::GETGLOBAL, r, 0, 0, AddStringConst(def->name));
    return r;
}

int InterpCodegen::EmitThrow(const std::string &msg) {
    return Emit(Op::THROW, 0, 0, 0, AddStringConst(msg));
}

bool InterpCodegen::LastPreservesMulti(const SyntaxTreeInterfacePtr &exp) const {
    if (!ar_) return false;
    if (IsVarargExp(exp)) return true;
    if (!ar_->function_call_exps.contains(exp.get())) return false;
    const std::string callee = ar_->callee_names.contains(exp.get()) ? ar_->callee_names.at(exp.get()) : "";
    if (!callee.empty() && ar_->function_max_returns.contains(callee) && ar_->function_max_returns.at(callee) == 1) {
        return false;
    }
    return true;
}

bool InterpCodegen::IsPackageHeaderStmt(const SyntaxTreeInterfacePtr &stmt) const {
    if (cur_package_name_.empty()) return false;
    std::string ignored;
    return ExtractPackageName(stmt, ignored);
}

CVar InterpCodegen::LiteralFromExp(const SyntaxTreeInterfacePtr &exp) {
    CVar v = interp_rt::Nil();
    if (!exp || exp->Type() != SyntaxTreeType::Exp) return v;
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    switch (e->GetExpKind()) {
        case ExpKind::kNil:
            return interp_rt::Nil();
        case ExpKind::kTrue:
            return interp_rt::Bool(true);
        case ExpKind::kFalse:
            return interp_rt::Bool(false);
        case ExpKind::kNumber:
            if (IsInteger(e->ExpValue())) {
                return interp_rt::Int(ToInteger(e->ExpValue()));
            }
            return interp_rt::Float(ToFloat(e->ExpValue()));
        case ExpKind::kString: {
            CVar s{};
            s.type_ = static_cast<int>(VarType::StringId);
            s.data_.i = s_->GetConstString().Alloc(e->ExpValue());
            return s;
        }
        case ExpKind::kPrefixExp: {
            // `(1.5)` 这类括号字面量：预处理不当成复杂表达式，必须在这里拆开。
            const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
            if (pe && pe->GetPrefixKind() == PrefixExpKind::kExp) {
                return LiteralFromExp(pe->GetValue());
            }
            return interp_rt::Nil();
        }
        default:
            return interp_rt::Nil();
    }
}

int InterpCodegen::ResolveSimpleName(const SyntaxTreeVar *var, const std::string &name, bool as_callee) {
    (void)as_callee;
    if (const auto it = var_to_def_map_.find(var); it != var_to_def_map_.end()) {
        VarDef *def = it->second;
        if (def->defining_func == cur_func_ || def->defining_func != nullptr) {
            return LoadLocal(def);
        }
        // 文件级 local function：没有函数内局部槽，按注册原型构造闭包（对齐 CGen FlMakeClosure）。
    }
    if (const auto nit = named_protos_.find(name); nit != named_protos_.end()) {
        const int r = AllocReg();
        const int idx = static_cast<int>(cur_proto_->child_protos.size());
        cur_proto_->child_protos.push_back(nit->second);
        Emit(Op::CLOSURE, r, 0, 0, idx);
        return r;
    }
    if (name == "_VERSION") {
        const int r = AllocReg();
        CVar v{};
        v.type_ = static_cast<int>(VarType::StringId);
        v.data_.i = s_->GetConstString().Alloc(std::string("Fakelua ") + FAKELUA_VERSION_STRING);
        LoadConstTo(r, v);
        return r;
    }
    const int r = AllocReg();
    Emit(Op::GETGLOBAL, r, 0, 0, AddStringConst(name));
    return r;
}

void InterpCodegen::AssignToVar(const SyntaxTreeVar *var, int src) {
    const auto it = var_to_def_map_.find(var);
    if (it != var_to_def_map_.end()) {
        VarDef *def = it->second;
        if (def->defining_func == cur_func_) {
            StoreLocal(def->reg, src);
            return;
        }
        if (def->defining_func != nullptr) {
            int idx = 0;
            if (cur_func_) {
                const auto vit = std::find(cur_func_->captured_vars.begin(), cur_func_->captured_vars.end(), def);
                if (vit != cur_func_->captured_vars.end()) {
                    idx = static_cast<int>(vit - cur_func_->captured_vars.begin());
                }
            }
            Emit(Op::SETUPVAL, src, idx);
            return;
        }
        Emit(Op::SETGLOBAL, src, 0, 0, AddStringConst(def->name));
        return;
    }
    Emit(Op::SETGLOBAL, src, 0, 0, AddStringConst(var->GetName()));
}

std::string InterpCodegen::CompileFuncName(const SyntaxTreeInterfacePtr &ptr) {
    const auto name = std::dynamic_pointer_cast<SyntaxTreeFuncname>(ptr);
    const auto funcnamelistptr = name->FuncNameList();
    const auto funcnamelist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcnamelistptr);
    const auto &namelist = funcnamelist->Funcnames();
    return namelist[0];
}

void InterpCodegen::ResolveScopes(const SyntaxTreeInterfacePtr &node, std::vector<Scope> &scopes, std::vector<FuncInfo *> &func_stack, FuncInfo *cur_func) {
    if (!node) return;

    auto EnterScope = [&]() {
        Scope s;
        s.func = cur_func;
        scopes.push_back(s);
    };
    auto ExitScope = [&]() { scopes.pop_back(); };
    auto DefineVar = [&](const std::string &name, const SyntaxTreeInterface *def_node) {
        auto def = std::make_unique<VarDef>();
        def->name = name;
        def->def_node = def_node;
        def->defining_func = cur_func;
        def->is_captured = false;
        VarDef *ptr = def.get();
        scopes.back().vars[name] = ptr;
        all_defs_.push_back(std::move(def));
        stmt_var_to_def_[{def_node, name}] = ptr;
    };

    switch (node->Type()) {
        case SyntaxTreeType::Block: {
            EnterScope();
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (const auto &stmt: block->Stmts()) {
                ResolveScopes(stmt, scopes, func_stack, cur_func);
            }
            ExitScope();
            break;
        }
        case SyntaxTreeType::LocalVar: {
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            ResolveScopes(lv->Explist(), scopes, func_stack, cur_func);
            if (const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist())) {
                for (const auto &name: nl->Names()) {
                    DefineVar(name, lv.get());
                }
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            const auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            ResolveScopes(fl->ExpBegin(), scopes, func_stack, cur_func);
            ResolveScopes(fl->ExpEnd(), scopes, func_stack, cur_func);
            ResolveScopes(fl->ExpStep(), scopes, func_stack, cur_func);
            EnterScope();
            DefineVar(fl->Name(), fl.get());
            ResolveScopes(fl->Block(), scopes, func_stack, cur_func);
            ExitScope();
            break;
        }
        case SyntaxTreeType::ForIn: {
            const auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            ResolveScopes(fi->Explist(), scopes, func_stack, cur_func);
            EnterScope();
            if (const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(fi->Namelist())) {
                for (const auto &name: nl->Names()) {
                    DefineVar(name, fi.get());
                }
            }
            ResolveScopes(fi->Block(), scopes, func_stack, cur_func);
            ExitScope();
            break;
        }
        case SyntaxTreeType::Function:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::FunctionDef: {
            auto new_func = std::make_unique<FuncInfo>();
            new_func->node = node.get();
            new_func->parent = cur_func;

            std::string orig_name;
            if (node->Type() == SyntaxTreeType::Function) {
                orig_name = CompileFuncName(std::dynamic_pointer_cast<SyntaxTreeFunction>(node)->Funcname());
            } else if (node->Type() == SyntaxTreeType::LocalFunction) {
                orig_name = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node)->Name();
            } else {
                orig_name = std::format("__fakelua_lambda_{}", all_funcs_.size());
            }
            new_func->name = orig_name;
            if (cur_func != nullptr) {
                new_func->unique_c_name = std::format("__fl_func_{}", all_funcs_.size());
            } else {
                new_func->unique_c_name = orig_name;
            }

            SyntaxTreeInterfacePtr funcbody;
            if (node->Type() == SyntaxTreeType::Function) {
                funcbody = std::dynamic_pointer_cast<SyntaxTreeFunction>(node)->Funcbody();
            } else if (node->Type() == SyntaxTreeType::LocalFunction) {
                funcbody = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node)->Funcbody();
            } else {
                funcbody = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(node)->Funcbody();
            }
            new_func->funcbody = funcbody;
            if (funcbody) {
                const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
                if (const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(fb->Parlist())) {
                    if (const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist())) {
                        new_func->params = namelist->Names();
                    }
                    new_func->is_vararg = !new_func->params.empty() && (new_func->params.back().rfind("__fakelua_vararg_", 0) == 0);
                }
            }

            FuncInfo *pf = new_func.get();
            func_map_[node.get()] = pf;
            all_funcs_.push_back(std::move(new_func));

            if (node->Type() == SyntaxTreeType::LocalFunction) {
                DefineVar(orig_name, node.get());
            }

            func_stack.push_back(pf);
            EnterScope();
            if (funcbody) {
                const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
                if (const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(fb->Parlist())) {
                    if (const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist())) {
                        for (const auto &pname: namelist->Names()) {
                            auto def = std::make_unique<VarDef>();
                            def->name = pname;
                            def->def_node = parlist.get();
                            def->defining_func = pf;
                            VarDef *ptr = def.get();
                            all_defs_.push_back(std::move(def));
                            scopes.back().vars[pname] = ptr;
                            stmt_var_to_def_[{parlist.get(), pname}] = ptr;
                        }
                    }
                }
                ResolveScopes(fb->Block(), scopes, func_stack, pf);
            }
            ExitScope();
            func_stack.pop_back();
            break;
        }
        case SyntaxTreeType::Var: {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
            if (var->GetVarKind() == VarKind::kSimple) {
                const std::string &name = var->GetName();
                VarDef *found_def = nullptr;
                for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
                    if (const auto it = scopes[i].vars.find(name); it != scopes[i].vars.end()) {
                        found_def = it->second;
                        break;
                    }
                }
                if (found_def) {
                    var_to_def_map_[var.get()] = found_def;
                    FuncInfo *f_def = found_def->defining_func;
                    if (f_def != nullptr && f_def != cur_func) {
                        found_def->is_captured = true;
                        FuncInfo *p = cur_func;
                        while (p && p != f_def) {
                            if (p->captured_set.insert(found_def).second) {
                                p->captured_vars.push_back(found_def);
                            }
                            p = p->parent;
                        }
                    }
                }
            }
            ResolveScopes(var->GetPrefixexp(), scopes, func_stack, cur_func);
            ResolveScopes(var->GetExp(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::Return: {
            ResolveScopes(std::dynamic_pointer_cast<SyntaxTreeReturn>(node)->Explist(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::VarList: {
            const auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(node);
            for (const auto &v: vl->Vars()) {
                ResolveScopes(v, scopes, func_stack, cur_func);
            }
            break;
        }
        case SyntaxTreeType::ExpList: {
            const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            for (const auto &exp: el->Exps()) {
                ResolveScopes(exp, scopes, func_stack, cur_func);
            }
            break;
        }
        case SyntaxTreeType::Assign: {
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            ResolveScopes(assign->Varlist(), scopes, func_stack, cur_func);
            ResolveScopes(assign->Explist(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            ResolveScopes(fc->prefixexp(), scopes, func_stack, cur_func);
            ResolveScopes(fc->Args(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::Args: {
            const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            ResolveScopes(args->Explist(), scopes, func_stack, cur_func);
            ResolveScopes(args->Tableconstructor(), scopes, func_stack, cur_func);
            ResolveScopes(args->String(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::TableConstructor: {
            ResolveScopes(std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node)->Fieldlist(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::FieldList: {
            const auto fl = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            for (const auto &field: fl->Fields()) {
                ResolveScopes(field, scopes, func_stack, cur_func);
            }
            break;
        }
        case SyntaxTreeType::Field: {
            const auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            ResolveScopes(field->Key(), scopes, func_stack, cur_func);
            ResolveScopes(field->Value(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::While: {
            const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            ResolveScopes(while_node->Exp(), scopes, func_stack, cur_func);
            ResolveScopes(while_node->Block(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::Repeat: {
            // Lua：until 条件可见 repeat 块内声明的 local。不能走 Block 的
            // Enter/Exit，否则 until 里的名字会掉出作用域被当成 global。
            const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            EnterScope();
            if (rep->Block() && rep->Block()->Type() == SyntaxTreeType::Block) {
                const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(rep->Block());
                for (const auto &stmt: block->Stmts()) {
                    ResolveScopes(stmt, scopes, func_stack, cur_func);
                }
            } else {
                ResolveScopes(rep->Block(), scopes, func_stack, cur_func);
            }
            ResolveScopes(rep->Exp(), scopes, func_stack, cur_func);
            ExitScope();
            break;
        }
        case SyntaxTreeType::If: {
            const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            ResolveScopes(if_node->Exp(), scopes, func_stack, cur_func);
            ResolveScopes(if_node->Block(), scopes, func_stack, cur_func);
            ResolveScopes(if_node->ElseIfs(), scopes, func_stack, cur_func);
            ResolveScopes(if_node->ElseBlock(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::ElseIfList: {
            const auto eil = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
            for (size_t i = 0; i < eil->ElseifSize(); ++i) {
                ResolveScopes(eil->ElseifExp(i), scopes, func_stack, cur_func);
                ResolveScopes(eil->ElseifBlock(i), scopes, func_stack, cur_func);
            }
            break;
        }
        case SyntaxTreeType::Exp: {
            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            ResolveScopes(exp->Left(), scopes, func_stack, cur_func);
            ResolveScopes(exp->Right(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::PrefixExp: {
            ResolveScopes(std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node)->GetValue(), scopes, func_stack, cur_func);
            break;
        }
        default:
            break;
    }
}

int InterpCodegen::EmitClosure(FuncInfo *child) {
    CompileFunction(child);
    child->proto->upvalues.clear();
    for (VarDef *def: child->captured_vars) {
        UpvalDesc uv;
        if (def->defining_func == cur_func_) {
            uv.in_stack = true;
            uv.idx = static_cast<uint16_t>(def->reg);
        } else {
            uv.in_stack = false;
            int idx = 0;
            if (cur_func_) {
                const auto it = std::find(cur_func_->captured_vars.begin(), cur_func_->captured_vars.end(), def);
                if (it != cur_func_->captured_vars.end()) {
                    idx = static_cast<int>(it - cur_func_->captured_vars.begin());
                }
            }
            uv.idx = static_cast<uint16_t>(idx);
        }
        child->proto->upvalues.push_back(uv);
    }
    const int idx = static_cast<int>(cur_proto_->child_protos.size());
    cur_proto_->child_protos.push_back(child->proto);
    const int r = AllocReg();
    Emit(Op::CLOSURE, r, 0, 0, idx);
    return r;
}

void InterpCodegen::CompileFunction(FuncInfo *func) {
    if (!func || func->proto) {
        return;
    }
    FuncInfo *prev_func = cur_func_;
    FuncProto *prev_proto = cur_proto_;
    const int prev_stack = stack_top_;
    const int prev_local = local_top_;
    auto prev_loops = std::move(loops_);
    auto prev_labels = std::move(labels_);
    auto prev_gotos = std::move(pending_gotos_);

    auto proto = std::make_unique<FuncProto>();
    proto->unit = unit_;
    proto->name = func->unique_c_name;
    proto->param_count = static_cast<int>(func->params.size());
    proto->is_vararg = func->is_vararg;
    proto->max_stack = proto->param_count;
    func->proto = proto.get();
    unit_->protos.push_back(std::move(proto));

    if (func->parent == nullptr && !func->unique_c_name.empty()) {
        named_protos_[func->unique_c_name] = func->proto;
        if (!func->name.empty() && func->name != func->unique_c_name) {
            named_protos_[func->name] = func->proto;
        }
    }

    cur_func_ = func;
    cur_proto_ = func->proto;
    stack_top_ = func->proto->param_count;
    local_top_ = func->proto->param_count;
    loops_.clear();
    labels_.clear();
    pending_gotos_.clear();

    if (func->funcbody) {
        const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(func->funcbody);
        if (const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(fb->Parlist())) {
            if (const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist())) {
                int i = 0;
                for (const auto &pname: namelist->Names()) {
                    if (const auto it = stmt_var_to_def_.find({parlist.get(), pname}); it != stmt_var_to_def_.end()) {
                        it->second->reg = i;
                        if (it->second->is_captured) {
                            Emit(Op::NEWBOX, i);
                        }
                    }
                    ++i;
                }
            }
        }
        CompileStmtBlock(fb->Block());
        const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(fb->Block());
        const bool ends_ret = block_ptr && !block_ptr->Stmts().empty() && block_ptr->Stmts().back()->Type() == SyntaxTreeType::Return;
        if (!ends_ret) {
            const int r = AllocReg();
            Emit(Op::LOADNIL, r);
            Emit(Op::RETURN, r, 1);
        }
    } else {
        const int r = AllocReg();
        Emit(Op::LOADNIL, r);
        Emit(Op::RETURN, r, 1);
    }

    for (auto &[label, ips]: pending_gotos_) {
        const auto lit = labels_.find(label);
        if (lit == labels_.end()) {
            ThrowError("no visible label '" + label + "' for goto", func->funcbody);
        }
        for (int ip: ips) {
            PatchSbx(ip, lit->second);
        }
    }

    cur_func_ = prev_func;
    cur_proto_ = prev_proto;
    stack_top_ = prev_stack;
    local_top_ = prev_local;
    loops_ = std::move(prev_loops);
    labels_ = std::move(prev_labels);
    pending_gotos_ = std::move(prev_gotos);
}

void InterpCodegen::CompileStmtBlock(const SyntaxTreeInterfacePtr &block) {
    if (!block) return;
    const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);
    for (const auto &stmt: block_ptr->Stmts()) {
        CompileStmt(stmt);
    }
}

void InterpCodegen::CompileStmt(const SyntaxTreeInterfacePtr &stmt) {
    if (!stmt) return;
    if (IsPackageHeaderStmt(stmt)) return;
    const int saved = stack_top_;
    switch (stmt->Type()) {
        case SyntaxTreeType::Return:
            CompileStmtReturn(stmt);
            break;
        case SyntaxTreeType::LocalVar:
            CompileStmtLocalVar(stmt);
            break;
        case SyntaxTreeType::Assign:
            CompileStmtAssign(stmt);
            break;
        case SyntaxTreeType::FunctionCall:
            CompileFunctioncall(stmt, false);
            FreeTo(saved);
            break;
        case SyntaxTreeType::Block:
            CompileStmtBlock(stmt);
            break;
        case SyntaxTreeType::While:
            CompileStmtWhile(stmt);
            break;
        case SyntaxTreeType::Repeat:
            CompileStmtRepeat(stmt);
            break;
        case SyntaxTreeType::If:
            CompileStmtIf(stmt);
            break;
        case SyntaxTreeType::Break:
            CompileStmtBreak();
            break;
        case SyntaxTreeType::Continue:
            CompileStmtContinue();
            break;
        case SyntaxTreeType::ForLoop:
            CompileStmtForLoop(stmt);
            break;
        case SyntaxTreeType::ForIn:
            CompileStmtForIn(stmt);
            break;
        case SyntaxTreeType::LocalFunction:
            CompileStmtLocalFunction(stmt);
            break;
        case SyntaxTreeType::Empty:
            break;
        case SyntaxTreeType::Goto:
            CompileStmtGoto(stmt);
            break;
        case SyntaxTreeType::Label:
            CompileStmtLabel(stmt);
            break;
        default:
            ThrowError(std::format("not support stmt type: {}", SyntaxTreeTypeToString(stmt->Type())), stmt);
    }
}

void InterpCodegen::CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt) {
    const auto return_stmt = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);
    auto explist = return_stmt->Explist();
    if (!explist) {
        const int r = AllocReg();
        Emit(Op::LOADNIL, r);
        Emit(Op::RETURN, r, 1);
        return;
    }
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
    const auto &exps = explist_ptr->Exps();
    if (exps.empty()) {
        const int r = AllocReg();
        Emit(Op::LOADNIL, r);
        Emit(Op::RETURN, r, 1);
        return;
    }
    if (exps.size() == 1) {
        const int r = CompileExp(exps[0], true);
        Emit(Op::RETURN, r, 0);
        return;
    }
    const bool last_multi = LastPreservesMulti(exps.back());
    const int base = stack_top_;
    std::vector<int> regs;
    for (size_t i = 0; i < exps.size(); ++i) {
        const bool pres = (i + 1 == exps.size()) && last_multi;
        regs.push_back(CompileExp(exps[i], pres));
    }
    EnsureStack(base + static_cast<int>(regs.size()));
    for (size_t i = 0; i < regs.size(); ++i) {
        MoveTo(base + static_cast<int>(i), regs[i]);
    }
    if (last_multi) {
        Emit(Op::COMBINE, base, static_cast<int>(regs.size()) - 1);
        Emit(Op::RETURN, base, 0);
    } else {
        Emit(Op::MAKEMULTI, base, static_cast<int>(regs.size()));
        Emit(Op::RETURN, base, 0);
    }
}

void InterpCodegen::CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt) {
    const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
    const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
    const auto &names = namelist_ptr->Names();
    static const std::vector<SyntaxTreeInterfacePtr> empty_exps;
    const auto explist = local_var->Explist();
    const auto &exps = explist ? std::dynamic_pointer_cast<SyntaxTreeExplist>(explist)->Exps() : empty_exps;

    std::vector<VarDef *> defs;
    defs.reserve(names.size());
    for (const auto &name: names) {
        VarDef *def = stmt_var_to_def_.at({stmt.get(), name});
        if (def->defining_func == nullptr) {
            file_level_names_.insert(name);
            def->reg = -1;
        } else if (def->reg < 0) {
            def->reg = AllocLocal();
            if (def->is_captured) {
                Emit(Op::NEWBOX, def->reg);
            }
        }
        defs.push_back(def);
    }

    const bool last_multi = !exps.empty() && LastPreservesMulti(exps.back()) && names.size() > exps.size();
    if (last_multi) {
        for (size_t i = 0; i + 1 < exps.size(); ++i) {
            const int r = CompileExp(exps[i], false);
            if (defs[i]->defining_func == nullptr) {
                Emit(Op::SETGLOBAL, r, 0, 0, AddStringConst(names[i]));
            } else {
                StoreLocal(defs[i]->reg, r);
            }
        }
        const int callr = CompileExp(exps.back(), true);
        for (size_t i = exps.size() - 1; i < names.size(); ++i) {
            const int r = AllocReg();
            Emit(Op::UNBOX, r, callr, static_cast<int>(i - (exps.size() - 1)));
            if (defs[i]->defining_func == nullptr) {
                Emit(Op::SETGLOBAL, r, 0, 0, AddStringConst(names[i]));
            } else {
                StoreLocal(defs[i]->reg, r);
            }
        }
        return;
    }

    for (size_t i = 0; i < names.size(); ++i) {
        int r;
        if (i < exps.size()) {
            r = CompileExp(exps[i], false);
        } else {
            r = AllocReg();
            Emit(Op::LOADNIL, r);
        }
        if (defs[i]->defining_func == nullptr) {
            Emit(Op::SETGLOBAL, r, 0, 0, AddStringConst(names[i]));
        } else {
            StoreLocal(defs[i]->reg, r);
        }
    }
    for (size_t i = names.size(); i < exps.size(); ++i) {
        CompileExp(exps[i], false);
    }
}

void InterpCodegen::CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt) {
    const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
    const auto varlist_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
    const auto &vars = varlist_ptr->Vars();
    const auto &exps = explist_ptr->Exps();
    const auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(vars[0]);
    const int r = CompileExp(exps[0], false);
    AssignToVar(v_ptr.get(), r);
}

void InterpCodegen::CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt) {
    const auto while_stmt = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);
    LoopInfo loop;
    loop.kind = LoopInfo::kWhile;
    loop.continue_ip = static_cast<int>(cur_proto_->code.size());
    const int saved = stack_top_;
    const int cond = CompileExp(while_stmt->Exp(), false);
    const int jmpf = Emit(Op::TESTJMP, cond, 0);
    FreeTo(saved);
    loops_.push_back(std::move(loop));
    CompileStmtBlock(while_stmt->Block());
    Emit(Op::JMP, 0, 0, 0, loops_.back().continue_ip - static_cast<int>(cur_proto_->code.size()) - 1);
    const int end = static_cast<int>(cur_proto_->code.size());
    PatchSbx(jmpf, end);
    for (int ip: loops_.back().break_jmps) {
        PatchSbx(ip, end);
    }
    loops_.pop_back();
}

void InterpCodegen::CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt) {
    const auto repeat_stmt = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);
    LoopInfo loop;
    loop.kind = LoopInfo::kRepeat;
    const int start = static_cast<int>(cur_proto_->code.size());
    loops_.push_back(std::move(loop));
    CompileStmtBlock(repeat_stmt->Block());
    loops_.back().continue_ip = static_cast<int>(cur_proto_->code.size());
    const int saved = stack_top_;
    const int cond = CompileExp(repeat_stmt->Exp(), false);
    const int jmp_true = Emit(Op::TESTJMP, cond, 1);
    Emit(Op::JMP, 0, 0, 0, start - static_cast<int>(cur_proto_->code.size()) - 1);
    const int end = static_cast<int>(cur_proto_->code.size());
    PatchSbx(jmp_true, end);
    for (int ip: loops_.back().break_jmps) {
        PatchSbx(ip, end);
    }
    for (int ip: loops_.back().continue_jmps) {
        PatchSbx(ip, loops_.back().continue_ip);
    }
    FreeTo(saved);
    loops_.pop_back();
}

void InterpCodegen::CompileStmtIf(const SyntaxTreeInterfacePtr &stmt) {
    const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
    std::vector<int> end_jmps;
    auto emit_branch = [&](const SyntaxTreeInterfacePtr &cond_exp, const SyntaxTreeInterfacePtr &block) {
        const int saved = stack_top_;
        const int cond = CompileExp(cond_exp, false);
        const int jmpf = Emit(Op::TESTJMP, cond, 0);
        FreeTo(saved);
        CompileStmtBlock(block);
        end_jmps.push_back(Emit(Op::JMP));
        PatchSbx(jmpf, static_cast<int>(cur_proto_->code.size()));
    };
    emit_branch(if_stmt->Exp(), if_stmt->Block());
    if (const auto elseifs_node = if_stmt->ElseIfs()) {
        const auto elseif_list = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs_node);
        for (size_t i = 0; i < elseif_list->ElseifSize(); ++i) {
            emit_branch(elseif_list->ElseifExp(i), elseif_list->ElseifBlock(i));
        }
    }
    if (const auto else_block = if_stmt->ElseBlock()) {
        CompileStmtBlock(else_block);
    }
    const int end = static_cast<int>(cur_proto_->code.size());
    for (int ip: end_jmps) {
        PatchSbx(ip, end);
    }
}

void InterpCodegen::CompileStmtBreak() {
    if (loops_.empty()) {
        ThrowFakeluaException("break not inside loop");
    }
    loops_.back().break_jmps.push_back(Emit(Op::JMP));
}

void InterpCodegen::CompileStmtContinue() {
    if (loops_.empty()) {
        ThrowFakeluaException("continue not inside loop");
    }
    if (loops_.back().continue_ip >= 0) {
        const int ip = Emit(Op::JMP);
        PatchSbx(ip, loops_.back().continue_ip);
    } else {
        loops_.back().continue_jmps.push_back(Emit(Op::JMP));
    }
}

void InterpCodegen::CompileStmtGoto(const SyntaxTreeInterfacePtr &stmt) {
    const auto goto_stmt = std::dynamic_pointer_cast<SyntaxTreeGoto>(stmt);
    const auto &label = goto_stmt->GetLabel();
    const int ip = Emit(Op::JMP);
    if (const auto it = labels_.find(label); it != labels_.end()) {
        PatchSbx(ip, it->second);
    } else {
        pending_gotos_[label].push_back(ip);
    }
}

void InterpCodegen::CompileStmtLabel(const SyntaxTreeInterfacePtr &stmt) {
    const auto label_stmt = std::dynamic_pointer_cast<SyntaxTreeLabel>(stmt);
    labels_[label_stmt->GetName()] = static_cast<int>(cur_proto_->code.size());
}

void InterpCodegen::CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt) {
    const auto for_stmt = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
    const int saved = stack_top_;
    const int ctrl = AllocReg();
    const int endv = AllocReg();
    const int step = AllocReg();
    const int pos = AllocReg();
    MoveTo(ctrl, CompileExp(for_stmt->ExpBegin(), false));
    MoveTo(endv, CompileExp(for_stmt->ExpEnd(), false));
    if (for_stmt->ExpStep()) {
        MoveTo(step, CompileExp(for_stmt->ExpStep(), false));
    } else {
        LoadConstTo(step, interp_rt::Int(1));
    }
    Emit(Op::FORCHECK, pos, step);

    VarDef *ldef = stmt_var_to_def_.at({stmt.get(), for_stmt->Name()});
    if (ldef->reg < 0) {
        ldef->reg = AllocLocal();
    }

    LoopInfo loop;
    loop.kind = LoopInfo::kFor;
    const int loop_head = static_cast<int>(cur_proto_->code.size());
    const int cmp = AllocReg();
    const int skip_ge = Emit(Op::TESTJMP, pos, 0);
    Emit(Op::LE, cmp, ctrl, endv);
    const int after_le = Emit(Op::JMP);
    PatchSbx(skip_ge, static_cast<int>(cur_proto_->code.size()));
    Emit(Op::LE, cmp, endv, ctrl);
    PatchSbx(after_le, static_cast<int>(cur_proto_->code.size()));
    const int jmp_exit = Emit(Op::TESTJMP, cmp, 0);
    // 先换新 box 再写入，避免 MOVE 写穿上一轮闭包捕获的旧 box。
    if (ldef->is_captured) {
        Emit(Op::NEWBOX, ldef->reg);
    }
    StoreLocal(ldef->reg, ctrl);

    loop.continue_ip = -1;
    loops_.push_back(std::move(loop));
    CompileStmtBlock(for_stmt->Block());
    loops_.back().continue_ip = static_cast<int>(cur_proto_->code.size());
    for (int ip: loops_.back().continue_jmps) {
        PatchSbx(ip, loops_.back().continue_ip);
    }
    const int adv = Emit(Op::FORADVANCE, ctrl, step);
    Emit(Op::JMP, 0, 0, 0, loop_head - static_cast<int>(cur_proto_->code.size()) - 1);
    const int end = static_cast<int>(cur_proto_->code.size());
    PatchSbx(jmp_exit, end);
    PatchSbx(adv, end);
    for (int ip: loops_.back().break_jmps) {
        PatchSbx(ip, end);
    }
    // continue jumped to continue_ip which was -1 and stored in break_jmps if patched wrong.
    // Re-patch continues that were emitted before continue_ip was set: they used JMP with dest=-1
    // stored via break_jmps only when dest<0. Those went into break_jmps — WRONG, they'd become breaks.
    loops_.pop_back();
    FreeTo(saved);
}

InterpCodegen::PairsIpairsKind InterpCodegen::TryMatchPairsIpairs(const std::shared_ptr<SyntaxTreeExplist> &explist_ptr, SyntaxTreeInterfacePtr &out_tbl_arg) {
    if (explist_ptr->Exps().size() != 1) return PairsIpairsKind::kNone;
    const auto exp = explist_ptr->Exps()[0];
    if (!exp || exp->Type() != SyntaxTreeType::Exp) return PairsIpairsKind::kNone;
    const auto exp_ptr = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    if (!exp_ptr || exp_ptr->GetExpKind() != ExpKind::kPrefixExp || !exp_ptr->Right()) return PairsIpairsKind::kNone;
    const auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp_ptr->Right());
    if (!pe_ptr || pe_ptr->GetPrefixKind() != PrefixExpKind::kFunctionCall) return PairsIpairsKind::kNone;
    const auto fc_ptr = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe_ptr->GetValue());
    if (!fc_ptr || !fc_ptr->Name().empty()) return PairsIpairsKind::kNone;
    const auto func_pe = fc_ptr->prefixexp();
    if (!func_pe || func_pe->Type() != SyntaxTreeType::PrefixExp) return PairsIpairsKind::kNone;
    const auto func_pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(func_pe);
    if (!func_pe_ptr || func_pe_ptr->GetPrefixKind() != PrefixExpKind::kVar) return PairsIpairsKind::kNone;
    const auto func_var = std::dynamic_pointer_cast<SyntaxTreeVar>(func_pe_ptr->GetValue());
    if (!func_var) return PairsIpairsKind::kNone;
    const auto func_name = func_var->GetName();
    if (func_name != "pairs" && func_name != "ipairs") return PairsIpairsKind::kNone;
    if (var_to_def_map_.contains(func_var.get())) return PairsIpairsKind::kNone;
    const auto args_node = fc_ptr->Args();
    if (!args_node || args_node->Type() != SyntaxTreeType::Args) return PairsIpairsKind::kNone;
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args_node);
    if (!args_ptr || args_ptr->GetArgsKind() != ArgsKind::kExpList) return PairsIpairsKind::kNone;
    const auto args_explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
    if (!args_explist || args_explist->Exps().size() != 1) return PairsIpairsKind::kNone;
    out_tbl_arg = args_explist->Exps()[0];
    return func_name == "pairs" ? PairsIpairsKind::kPairs : PairsIpairsKind::kIpairs;
}

void InterpCodegen::CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt) {
    const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
    const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(for_in->Namelist());
    const auto &names = namelist_ptr->Names();
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(for_in->Explist());

    std::vector<VarDef *> defs;
    for (const auto &n: names) {
        VarDef *def = stmt_var_to_def_.at({stmt.get(), n});
        if (def->reg < 0) {
            def->reg = AllocLocal();
        }
        defs.push_back(def);
    }

    SyntaxTreeInterfacePtr tbl_exp_node;
    const auto kind = TryMatchPairsIpairs(explist_ptr, tbl_exp_node);
    if (kind != PairsIpairsKind::kNone) {
        const int tbl = CompileExp(tbl_exp_node, false);
        // if not table: THROW. Use GETTABLE on dummy? Use TABCOUNT which should throw from GetTable...
        // TableEntryCount on non-table: implement check via GETTABLE? runtime TableEntryCount:
        // check runtime
        const int sz = AllocReg();
        Emit(Op::TABCOUNT, sz, tbl);
        const int idx = AllocReg();
        LoadConstTo(idx, interp_rt::Int(0));
        LoopInfo loop;
        loop.kind = LoopInfo::kForIn;
        const int head = static_cast<int>(cur_proto_->code.size());
        loop.continue_ip = -1;
        const int cmp = AllocReg();
        Emit(Op::LT, cmp, idx, sz);
        const int jmp_exit = Emit(Op::TESTJMP, cmp, 0);
        const int kreg = AllocReg();
        const int vreg = AllocReg();
        Emit(Op::TABENT, tbl, kreg, vreg, 0);
        cur_proto_->code.back().sbx = 0;
        // idx is int CVar; TABENT uses sbx as uint index — need the runtime idx from register.
        // Fix: store idx in sbx is wrong for dynamic idx. Change: use B/C already; put idx in extra.
        // Interpreter TABENT uses inst.sbx as index. We need register index.
        // Patch: reuse inst.b/c for k/v, inst.a for table, and read index from a side register via sbx being the reg?
        // I'll encode idx register in inst.sbx as a register number (non-negative small).
        cur_proto_->code.back().sbx = idx;
        const int vnil = AllocReg();
        const int nlit = AllocReg();
        Emit(Op::LOADNIL, nlit);
        Emit(Op::EQ, vnil, vreg, nlit);
        const int skip_body = Emit(Op::TESTJMP, vnil, 1);
        if (defs[0]->is_captured) {
            Emit(Op::NEWBOX, defs[0]->reg);
        }
        StoreLocal(defs[0]->reg, kreg);
        if (defs.size() >= 2) {
            if (defs[1]->is_captured) {
                Emit(Op::NEWBOX, defs[1]->reg);
            }
            StoreLocal(defs[1]->reg, vreg);
        }
        loops_.push_back(std::move(loop));
        CompileStmtBlock(for_in->Block());
        const int cont = static_cast<int>(cur_proto_->code.size());
        loops_.back().continue_ip = cont;
        for (int ip: loops_.back().continue_jmps) {
            PatchSbx(ip, cont);
        }
        PatchSbx(skip_body, cont);
        const int one = AllocReg();
        LoadConstTo(one, interp_rt::Int(1));
        Emit(Op::ADD, idx, idx, one);
        Emit(Op::JMP, 0, 0, 0, head - static_cast<int>(cur_proto_->code.size()) - 1);
        const int end = static_cast<int>(cur_proto_->code.size());
        PatchSbx(jmp_exit, end);
        for (int ip: loops_.back().break_jmps) {
            PatchSbx(ip, end);
        }
        loops_.pop_back();
        return;
    }

    const auto &exps = explist_ptr->Exps();
    const int iter_f = AllocReg();
    const int iter_s = AllocReg();
    const int iter_var = AllocReg();
    if (exps.size() == 1) {
        const int e0 = CompileExp(exps[0], true);
        const int is_m = AllocReg();
        // Always unbox 0,1,2 — UnboxMulti on non-multi returns the value for idx 0 and nil else.
        Emit(Op::UNBOX, iter_f, e0, 0);
        Emit(Op::UNBOX, iter_s, e0, 1);
        Emit(Op::UNBOX, iter_var, e0, 2);
        (void)is_m;
    } else {
        MoveTo(iter_f, CompileExp(exps[0], false));
        if (exps.size() >= 2) {
            MoveTo(iter_s, CompileExp(exps[1], false));
        } else {
            Emit(Op::LOADNIL, iter_s);
        }
        if (exps.size() >= 3) {
            MoveTo(iter_var, CompileExp(exps[2], false));
        } else {
            Emit(Op::LOADNIL, iter_var);
        }
        Emit(Op::UNBOX, iter_f, iter_f, 0);
        Emit(Op::UNBOX, iter_s, iter_s, 0);
        Emit(Op::UNBOX, iter_var, iter_var, 0);
    }

    LoopInfo loop;
    loop.kind = LoopInfo::kForIn;
    const int head = static_cast<int>(cur_proto_->code.size());
    loop.continue_ip = head;
    const int base = stack_top_;
    EnsureStack(base + 3);
    MoveTo(base, iter_f);
    MoveTo(base + 1, iter_s);
    MoveTo(base + 2, iter_var);
    Emit(Op::CALL, base, 2, 0);
    stack_top_ = base + 1;
    const int res = base;
    for (size_t i = 0; i < defs.size(); ++i) {
        const int r = AllocReg();
        Emit(Op::UNBOX, r, res, static_cast<int>(i));
        if (defs[i]->is_captured) {
            Emit(Op::NEWBOX, defs[i]->reg);
        }
        StoreLocal(defs[i]->reg, r);
    }
    const int first = defs[0]->reg;
    const int nlit = AllocReg();
    Emit(Op::LOADNIL, nlit);
    const int isnil = AllocReg();
    Emit(Op::EQ, isnil, first, nlit);
    const int jmp_exit = Emit(Op::TESTJMP, isnil, 1);
    MoveTo(iter_var, first);
    loops_.push_back(std::move(loop));
    CompileStmtBlock(for_in->Block());
    const int cont = static_cast<int>(cur_proto_->code.size());
    loops_.back().continue_ip = cont;
    for (int ip: loops_.back().continue_jmps) {
        PatchSbx(ip, cont);
    }
    Emit(Op::JMP, 0, 0, 0, head - static_cast<int>(cur_proto_->code.size()) - 1);
    const int end = static_cast<int>(cur_proto_->code.size());
    PatchSbx(jmp_exit, end);
    for (int ip: loops_.back().break_jmps) {
        PatchSbx(ip, end);
    }
    loops_.pop_back();
}

void InterpCodegen::CompileStmtLocalFunction(const SyntaxTreeInterfacePtr &stmt) {
    const auto lf = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
    const auto &name = lf->Name();
    FuncInfo *func = func_map_[lf.get()];
    VarDef *def = stmt_var_to_def_.at({stmt.get(), name});
    if (def->defining_func == nullptr) {
        file_level_names_.insert(name);
        const int cl = EmitClosure(func);
        Emit(Op::SETGLOBAL, cl, 0, 0, AddStringConst(name));
        return;
    }
    if (def->reg < 0) {
        def->reg = AllocLocal();
    }
    if (def->is_captured) {
        Emit(Op::NEWBOX, def->reg);
    }
    const int cl = EmitClosure(func);
    StoreLocal(def->reg, cl);
}

int InterpCodegen::CompileExp(const SyntaxTreeInterfacePtr &exp, bool preserve_multi) {
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    switch (e->GetExpKind()) {
        case ExpKind::kNil: {
            const int r = AllocReg();
            Emit(Op::LOADNIL, r);
            return r;
        }
        case ExpKind::kFalse: {
            const int r = AllocReg();
            Emit(Op::LOADBOOL, r, 0);
            return r;
        }
        case ExpKind::kTrue: {
            const int r = AllocReg();
            Emit(Op::LOADBOOL, r, 1);
            return r;
        }
        case ExpKind::kNumber: {
            const int r = AllocReg();
            if (IsInteger(e->ExpValue())) {
                LoadConstTo(r, interp_rt::Int(ToInteger(e->ExpValue())));
            } else {
                LoadConstTo(r, interp_rt::Float(ToFloat(e->ExpValue())));
            }
            return r;
        }
        case ExpKind::kString: {
            const int r = AllocReg();
            CVar v{};
            v.type_ = static_cast<int>(VarType::StringId);
            v.data_.i = s_->GetConstString().Alloc(e->ExpValue());
            LoadConstTo(r, v);
            return r;
        }
        case ExpKind::kPrefixExp:
            return CompilePrefixexp(e->Right(), preserve_multi);
        case ExpKind::kTableConstructor:
            return CompileTableconstructor(e->Right());
        case ExpKind::kFunctionDef: {
            const auto func_def = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(e->Right());
            return EmitClosure(func_map_[func_def.get()]);
        }
        case ExpKind::kBinop:
            return CompileBinop(exp);
        case ExpKind::kUnop:
            return CompileUnop(exp);
        default:
            ThrowError("unsupported expression kind", e);
    }
}

int InterpCodegen::CompilePrefixexp(const SyntaxTreeInterfacePtr &pe, bool preserve_multi) {
    const auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe);
    if (pe_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
        const int r = CompileVar(pe_ptr->GetValue());
        const auto var_node = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_ptr->GetValue());
        if (!preserve_multi && var_node && var_node->GetVarKind() == VarKind::kSimple && var_node->GetName().rfind("__fakelua_vararg_", 0) == 0) {
            const int o = AllocReg();
            Emit(Op::UNBOX, o, r, 0);
            return o;
        }
        return r;
    }
    if (pe_ptr->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
        return CompileFunctioncall(pe_ptr->GetValue(), preserve_multi);
    }
    return CompileExp(pe_ptr->GetValue(), preserve_multi);
}

int InterpCodegen::CompileVar(const SyntaxTreeInterfacePtr &v) {
    const auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(v);
    if (v_ptr->GetVarKind() == VarKind::kSimple) {
        return ResolveSimpleName(v_ptr.get(), v_ptr->GetName(), false);
    }
    if (v_ptr->GetVarKind() == VarKind::kSquare) {
        const int t = CompilePrefixexp(v_ptr->GetPrefixexp(), false);
        const int k = CompileExp(v_ptr->GetExp(), false);
        const int r = AllocReg();
        Emit(Op::GETTABLE, r, t, k);
        return r;
    }
    // kDot
    const auto pe = v_ptr->GetPrefixexp();
    const auto name = v_ptr->GetName();
    if (pe && pe->Type() == SyntaxTreeType::PrefixExp) {
        if (const auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe); pe_ptr && pe_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
            const auto base_var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_ptr->GetValue());
            if (base_var && base_var->GetVarKind() == VarKind::kSimple && !var_to_def_map_.contains(base_var.get())) {
                if (base_var->GetName() == "math") {
                    const int r = AllocReg();
                    if (name == "pi") {
                        LoadConstTo(r, interp_rt::Float(3.14159265358979323846));
                        return r;
                    }
                    if (name == "huge") {
                        LoadConstTo(r, interp_rt::Float(HUGE_VAL));
                        return r;
                    }
                    if (name == "maxinteger") {
                        LoadConstTo(r, interp_rt::Int(std::numeric_limits<int64_t>::max()));
                        return r;
                    }
                    if (name == "mininteger") {
                        LoadConstTo(r, interp_rt::Int(std::numeric_limits<int64_t>::min()));
                        return r;
                    }
                }
                if (base_var->GetName() == "string" && name == "charpattern") {
                    const int r = AllocReg();
                    CVar s{};
                    s.type_ = static_cast<int>(VarType::StringId);
                    s.data_.i = s_->GetConstString().Alloc("[^%z]");
                    LoadConstTo(r, s);
                    return r;
                }
                if (base_var->GetName() == "utf8" && name == "charpattern") {
                    const int r = AllocReg();
                    CVar s{};
                    s.type_ = static_cast<int>(VarType::StringId);
                    s.data_.i = s_->GetConstString().Alloc("[\\x00-\\x7F\\xC2-\\xF4][\\x80-\\xBF]*");
                    LoadConstTo(r, s);
                    return r;
                }
            }
        }
    }
    const int t = CompilePrefixexp(pe, false);
    const int k = AllocReg();
    CVar key{};
    key.type_ = static_cast<int>(VarType::StringId);
    key.data_.i = s_->GetConstString().Alloc(name);
    LoadConstTo(k, key);
    const int r = AllocReg();
    Emit(Op::GETTABLE, r, t, k);
    return r;
}

void InterpCodegen::EmitCallArgs(const std::shared_ptr<SyntaxTreeArgs> &args_ptr, std::vector<int> &arg_regs, bool &last_expand) {
    last_expand = false;
    if (!args_ptr) return;
    const auto args_kind = args_ptr->GetArgsKind();
    if (args_kind == ArgsKind::kExpList) {
        const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
        const auto &raw_args = explist_ptr->Exps();
        last_expand = !raw_args.empty() && LastPreservesMulti(raw_args.back());
        for (size_t i = 0; i < raw_args.size(); ++i) {
            const bool pres = last_expand && (i + 1 == raw_args.size());
            arg_regs.push_back(CompileExp(raw_args[i], pres));
        }
    } else if (args_kind == ArgsKind::kTableConstructor) {
        arg_regs.push_back(CompileTableconstructor(args_ptr->Tableconstructor()));
    } else if (args_kind == ArgsKind::kString) {
        arg_regs.push_back(CompileExp(args_ptr->String(), false));
    }
}

int InterpCodegen::CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall, bool preserve_multi) {
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(functioncall);
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
    const auto pe_pre_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
    const int unbox = preserve_multi ? 0 : 1;

    std::vector<int> arg_regs;
    bool last_expand = false;
    auto place_callname = [&](const std::string &name, const std::vector<int> &args) {
        const int n = static_cast<int>(args.size());
        int base;
        if (n == 0) {
            base = AllocReg();
        } else {
            base = stack_top_;
            EnsureStack(base + n);
            for (int i = 0; i < n; ++i) {
                MoveTo(base + i, args[i]);
            }
            stack_top_ = base + n;
        }
        Emit(Op::CALLNAME, base, n, unbox, AddStringConst(name));
        stack_top_ = base + 1;
        return base;
    };
    auto place_call = [&](int fn, const std::vector<int> &args) {
        const int n = static_cast<int>(args.size());
        const int base = stack_top_;
        EnsureStack(base + 1 + n);
        MoveTo(base, fn);
        for (int i = 0; i < n; ++i) {
            MoveTo(base + 1 + i, args[i]);
        }
        stack_top_ = base + 1 + n;
        Emit(Op::CALL, base, n, unbox);
        stack_top_ = base + 1;
        return base;
    };

    if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
        const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_pre_ptr->GetValue());
        if (callee_var && callee_var->GetVarKind() == VarKind::kSimple && callee_var->GetName() == "FAKELUA_SET_TABLE") {
            const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
            const auto &raw = explist_ptr->Exps();
            const int t = CompileExp(raw[0], false);
            const int k = CompileExp(raw[1], false);
            const int val = CompileExp(raw[2], false);
            Emit(Op::SETTABLE, t, k, val);
            const int r = AllocReg();
            Emit(Op::LOADNIL, r);
            return r;
        }
    }

    EmitCallArgs(args_ptr, arg_regs, last_expand);

    if (!fc->Name().empty()) {
        int obj;
        if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
            obj = CompileVar(pe_pre_ptr->GetValue());
        } else {
            obj = CompilePrefixexp(fc->prefixexp(), false);
        }
        std::vector<int> final_args;
        final_args.push_back(obj);
        for (int a: arg_regs) final_args.push_back(a);
        if (kStringLibraryMethods.contains(fc->Name())) {
            return place_callname("string." + fc->Name(), final_args);
        }
        const int k = AllocReg();
        CVar key{};
        key.type_ = static_cast<int>(VarType::StringId);
        key.data_.i = s_->GetConstString().Alloc(fc->Name());
        LoadConstTo(k, key);
        const int method = AllocReg();
        Emit(Op::GETTABLE, method, obj, k);
        return place_call(method, final_args);
    }

    std::string func_name;
    const SyntaxTreeVar *var_ptr = nullptr;
    bool is_local_callee = false;
    bool file_level_func = false;
    if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
        auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_pre_ptr->GetValue());
        if (var && var->GetVarKind() == VarKind::kSimple) {
            func_name = var->GetName();
            var_ptr = var.get();
            if (const auto it = var_to_def_map_.find(var_ptr); it != var_to_def_map_.end()) {
                is_local_callee = true;
                file_level_func = it->second->defining_func == nullptr;
            }
        } else if (var && var->GetVarKind() == VarKind::kDot) {
            const auto base_pe = var->GetPrefixexp();
            if (base_pe && base_pe->Type() == SyntaxTreeType::PrefixExp) {
                const auto base_pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(base_pe);
                if (base_pe_ptr && base_pe_ptr->GetPrefixKind() == PrefixExpKind::kVar && base_pe_ptr->GetValue()) {
                    const auto base_var = std::dynamic_pointer_cast<SyntaxTreeVar>(base_pe_ptr->GetValue());
                    if (base_var && base_var->GetVarKind() == VarKind::kSimple && !var_to_def_map_.contains(base_var.get())) {
                        func_name = base_var->GetName() + "." + var->GetName();
                    }
                }
            }
        }
    }

    if (!func_name.empty() && (!is_local_callee || file_level_func || named_protos_.contains(func_name))) {
        return place_callname(func_name, arg_regs);
    }

    int fn;
    if (var_ptr) {
        fn = CompileVar(pe_pre_ptr->GetValue());
    } else {
        fn = CompilePrefixexp(fc->prefixexp(), false);
    }
    return place_call(fn, arg_regs);
}

int InterpCodegen::CompileTableconstructor(const SyntaxTreeInterfacePtr &tc) {
    const auto tc_ptr = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(tc);
    const int tbl = AllocReg();
    Emit(Op::NEWTABLE, tbl);
    if (const auto fieldlist = tc_ptr->Fieldlist()) {
        const auto fieldlist_ptr = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(fieldlist);
        std::shared_ptr<SyntaxTreeField> last_array_field;
        bool has_multi_list_field = false;
        for (const auto &field: fieldlist_ptr->Fields()) {
            const auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(field);
            if (field_ptr->GetFieldKind() == FieldKind::kArray && !field_ptr->Key()) {
                last_array_field = field_ptr;
                if (LastPreservesMulti(field_ptr->Value())) {
                    has_multi_list_field = true;
                }
            }
        }
        int array_idx = 1;
        for (const auto &field: fieldlist_ptr->Fields()) {
            const auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(field);
            const bool is_expand = (field_ptr == last_array_field) && LastPreservesMulti(field_ptr->Value());
            const int val = CompileExp(field_ptr->Value(), is_expand);
            if (field_ptr->GetFieldKind() == FieldKind::kObject) {
                const int k = AllocReg();
                CVar key{};
                key.type_ = static_cast<int>(VarType::StringId);
                key.data_.i = s_->GetConstString().Alloc(field_ptr->Name());
                LoadConstTo(k, key);
                Emit(Op::SETTABLE, tbl, k, val);
            } else if (const auto key = field_ptr->Key()) {
                const int k = CompileExp(key, false);
                Emit(Op::SETTABLE, tbl, k, val);
                // 与 CGen 特化路径一致：字面量整数键推进隐式下标。
                // 含 `...` / 多返回展开时走 Lua 列表语义（后续隐式字段从 1 起，可覆盖）。
                if (!has_multi_list_field) {
                    if (const auto key_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(key);
                        key_exp && key_exp->GetExpKind() == ExpKind::kNumber && IsInteger(key_exp->ExpValue())) {
                        const auto iv = ToInteger(key_exp->ExpValue());
                        if (iv >= 0 && iv < std::numeric_limits<int>::max()) {
                            array_idx = std::max(array_idx, static_cast<int>(iv) + 1);
                        }
                    }
                }
            } else if (is_expand) {
                Emit(Op::SETLIST, tbl, array_idx, val);
            } else {
                const int k = AllocReg();
                LoadConstTo(k, interp_rt::Int(array_idx));
                Emit(Op::SETTABLE, tbl, k, val);
                ++array_idx;
            }
        }
    }
    return tbl;
}

int InterpCodegen::CompileBinop(const SyntaxTreeInterfacePtr &exp) {
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
    const auto op_kind = op_ptr->GetOpKind();
    if (op_kind == BinOpKind::kAnd || op_kind == BinOpKind::kOr) {
        const int result = AllocReg();
        const int left = CompileExp(e->Left(), false);
        const int jmp = Emit(Op::TESTJMP, left, op_kind == BinOpKind::kAnd ? 0 : 1);
        const int right = CompileExp(e->Right(), false);
        MoveTo(result, right);
        const int jmp_end = Emit(Op::JMP);
        PatchSbx(jmp, static_cast<int>(cur_proto_->code.size()));
        MoveTo(result, left);
        PatchSbx(jmp_end, static_cast<int>(cur_proto_->code.size()));
        return result;
    }
    const int left = CompileExp(e->Left(), false);
    const int right = CompileExp(e->Right(), false);
    const int r = AllocReg();
    switch (op_kind) {
        case BinOpKind::kPlus:
            Emit(Op::ADD, r, left, right);
            break;
        case BinOpKind::kMinus:
            Emit(Op::SUB, r, left, right);
            break;
        case BinOpKind::kStar:
            Emit(Op::MUL, r, left, right);
            break;
        case BinOpKind::kSlash:
            Emit(Op::DIV, r, left, right);
            break;
        case BinOpKind::kDoubleSlash:
            Emit(Op::IDIV, r, left, right);
            break;
        case BinOpKind::kPow:
            Emit(Op::POW, r, left, right);
            break;
        case BinOpKind::kMod:
            Emit(Op::MOD, r, left, right);
            break;
        case BinOpKind::kBitAnd:
            Emit(Op::BAND, r, left, right);
            break;
        case BinOpKind::kXor:
            Emit(Op::BXOR, r, left, right);
            break;
        case BinOpKind::kBitOr:
            Emit(Op::BOR, r, left, right);
            break;
        case BinOpKind::kLeftShift:
            Emit(Op::SHL, r, left, right);
            break;
        case BinOpKind::kRightShift:
            Emit(Op::SHR, r, left, right);
            break;
        case BinOpKind::kConcat:
            Emit(Op::CONCAT, r, left, right);
            break;
        case BinOpKind::kLess:
            Emit(Op::LT, r, left, right);
            break;
        case BinOpKind::kLessEqual:
            Emit(Op::LE, r, left, right);
            break;
        case BinOpKind::kMore:
            Emit(Op::LT, r, right, left);
            break;
        case BinOpKind::kMoreEqual:
            Emit(Op::LE, r, right, left);
            break;
        case BinOpKind::kEqual:
            Emit(Op::EQ, r, left, right);
            break;
        case BinOpKind::kNotEqual:
            Emit(Op::EQ, r, left, right);
            Emit(Op::NOT, r, r);
            break;
        default:
            ThrowError("binary operator not supported", e->Op());
    }
    return r;
}

int InterpCodegen::CompileUnop(const SyntaxTreeInterfacePtr &exp) {
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op());
    const int operand = CompileExp(e->Right(), false);
    const int r = AllocReg();
    switch (op_ptr->GetOpKind()) {
        case UnOpKind::kNot:
            Emit(Op::NOT, r, operand);
            break;
        case UnOpKind::kMinus:
            Emit(Op::UNM, r, operand);
            break;
        case UnOpKind::kBitNot:
            Emit(Op::BNOT, r, operand);
            break;
        case UnOpKind::kNumberSign:
            Emit(Op::LEN, r, operand);
            break;
        default:
            ThrowError("unary operator not supported", e->Op());
    }
    return r;
}

void InterpCodegen::Generate(const ParseResult &pr, const AnalysisResult &ar, const CompileConfig &cfg) {
    (void)cfg;
    file_name_ = pr.file_name;
    ar_ = &ar;

    cur_package_name_.clear();
    if (pr.chunk && pr.chunk->Type() == SyntaxTreeType::Block) {
        const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);
        const std::vector<SyntaxTreeInterfacePtr> *stmts_to_check = &blk->Stmts();
        for (const auto &stmt: blk->Stmts()) {
            if (stmt && stmt->Type() == SyntaxTreeType::Function) {
                const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
                if (func && func->Funcname()) {
                    const auto fn = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
                    if (fn && fn->FuncNameList()) {
                        const auto fnl = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(fn->FuncNameList());
                        if (fnl && fnl->Funcnames().size() == 1 && fnl->Funcnames()[0] == kInitFunctionName) {
                            if (func->Funcbody()) {
                                const auto fbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(func->Funcbody());
                                if (fbody && fbody->Block()) {
                                    const auto init_blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(fbody->Block());
                                    if (init_blk) {
                                        stmts_to_check = &init_blk->Stmts();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!stmts_to_check->empty()) {
            ExtractPackageName((*stmts_to_check)[0], cur_package_name_);
        }
    }

    std::vector<Scope> scopes;
    std::vector<FuncInfo *> func_stack;
    ResolveScopes(pr.chunk, scopes, func_stack, nullptr);

    auto unit = std::make_shared<InterpUnit>();
    unit->state = s_;
    unit_ = unit.get();

    if (pr.chunk && pr.chunk->Type() == SyntaxTreeType::Block) {
        const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);
        for (const auto &stmt: blk->Stmts()) {
            if (stmt->Type() != SyntaxTreeType::LocalVar) continue;
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
            if (!nl) continue;
            static const std::vector<SyntaxTreeInterfacePtr> empty_exps;
            const auto explist = lv->Explist();
            const auto &exps = explist ? std::dynamic_pointer_cast<SyntaxTreeExplist>(explist)->Exps() : empty_exps;
            for (size_t i = 0; i < nl->Names().size(); ++i) {
                const auto &name = nl->Names()[i];
                file_level_names_.insert(name);
                unit_->globals[name] = (i < exps.size()) ? LiteralFromExp(exps[i]) : interp_rt::Nil();
            }
        }
    }

    for (const auto &func: all_funcs_) {
        if (func->parent == nullptr) {
            CompileFunction(func.get());
        }
    }

    const auto handle = std::static_pointer_cast<JITHandle>(unit);
    for (const auto &func: all_funcs_) {
        if (func->parent != nullptr || !func->proto) continue;
        const std::string &name = func->unique_c_name;
        void *addr = TagInterpProto(func->proto);
        s_->GetVM().RegisterFunction(VmFunction(name, func->proto->param_count, JIT_INTERP, addr, handle, func->is_vararg));
        if (!cur_package_name_.empty() && name != kInitFunctionName && !func->name.empty()) {
            const std::string pkg = cur_package_name_ + "." + func->name;
            s_->GetVM().RegisterFunction(VmFunction(pkg, func->proto->param_count, JIT_INTERP, addr, handle, func->is_vararg));
        }
        if (name == kInitFunctionName) {
            unit_->init_proto = func->proto;
        }
    }

    if (unit_->init_proto) {
        s_->SetInterpConstAlloc(true);
        InterpreterExecute(s_, unit_->init_proto, nullptr, 0, nullptr);
        s_->SetInterpConstAlloc(false);
        for (const auto &name: ar.global_const_names) {
            if (auto it = unit_->globals.find(name); it != unit_->globals.end()) {
                it->second.flag_ |= 0x1;
            }
        }
        for (const auto &name: file_level_names_) {
            if (auto it = unit_->globals.find(name); it != unit_->globals.end()) {
                if (it->second.type_ == static_cast<int>(VarType::Table) || it->second.type_ == static_cast<int>(VarType::Closure)) {
                    it->second.flag_ |= 0x1;
                }
            }
        }
    }
}

}// namespace fakelua
