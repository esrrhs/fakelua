#include "compile/semantic_analysis.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include <algorithm>
#include <functional>

namespace fakelua {

SemanticAnalysis::SemanticAnalysis(State *s) : s_(s) {
}

// 文件级只承载声明：local 变量定义、函数定义，以及可选的首行 package 声明。
// if / while / for / 赋值 等可执行语句必须写在函数体里：它们在文件级没有确定的执行时机，
// 而且文件级 local 会被当成该文件的常量降级成 C 的 static const，再赋值就自相矛盾了。
void SemanticAnalysis::CheckFileLevelStmts(const ParseResult &pr) {
    file_name_ = pr.file_name;

    DEBUG_ASSERT(pr.chunk->Type() == SyntaxTreeType::Block);
    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);

    const auto &stmts = top_block->Stmts();
    for (size_t i = 0; i < stmts.size(); ++i) {
        const auto &stmt = stmts[i];
        switch (stmt->Type()) {
            case SyntaxTreeType::LocalVar:
            case SyntaxTreeType::Function:
            case SyntaxTreeType::LocalFunction:
            case SyntaxTreeType::Empty:
                continue;
            default:
                break;
        }
        // package 声明只承认写在文件第一条语句的形态，后续 CGen 也只在这个位置识别它
        if (std::string pkg_name; i == 0 && ExtractPackageName(stmt, pkg_name)) {
            continue;
        }
        ThrowError(std::format("unsupported file-level statement {}, only local definitions and function definitions are allowed at file level", SyntaxTreeTypeToString(stmt->Type())), stmt);
    }
}

AnalysisResult SemanticAnalysis::Analyze(const ParseResult &pr, const CompileConfig &cfg) {
    file_name_ = pr.file_name;

    AnalysisResult ar;
    AnalyzeGlobalConstNames(pr.chunk, ar);
    CheckUnsupportedSyntax(pr.chunk, ar);
    CheckUndeclaredVars(pr.chunk, ar);
    AnalyzeFunctionReturnCounts(pr.chunk, ar);

    WalkSyntaxTree(pr.chunk, [&](const SyntaxTreeInterfacePtr &node) {
        if (IsFunctionCallExp(node)) {
            ar.function_call_exps.insert(node.get());
            std::string name = GetCalleeName(node);
            ar.callee_names[node.get()] = name;

            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe->GetValue());
            if (fc) {
                ar.callee_names[fc.get()] = name;
            }
        }
    });

    return ar;
}

void SemanticAnalysis::AnalyzeGlobalConstNames(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar) {
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    for (const auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto namelist = local_var->Namelist();
            if (!namelist) {
                continue;
            }
            const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
            const auto &names = namelist_ptr->Names();
            for (const auto &name: names) {
                if (ar.global_const_names.contains(name)) {
                    ThrowError("duplicate global const variable: " + name, stmt);
                }
                ar.global_const_names.insert(name);
            }
        }
    }
}

namespace {

// 按词法作用域收集每个函数的返回路径，再定点求出「所有路径都相同的精确返回数」。
// return a, f() 的返回数是 1 + f 的返回数；任一路径是 ...、未知被调，或路径之间数量不一致，则为不可知。
struct ReturnCountGraph {
    struct Binding {
        bool is_func = false;
        bool file_level = false;
        const SyntaxTreeInterface *func = nullptr;
    };
    struct Tail {
        int prefix = 0;
        const SyntaxTreeInterface *callee = nullptr;
        bool unknown = false;
    };
    struct Info {
        bool has_exact = false;
        int exact = 0;
        bool conflict = false;
        bool force_taint = false;
        bool dynamic_max = false;
        int max_concrete = 0;
        std::vector<Tail> tails;
        std::vector<std::pair<const SyntaxTreeInterface *, Tail>> sole_calls;
    };
    struct FileFunc {
        std::string name;
        const SyntaxTreeInterface *node = nullptr;
        int max_returns = 0;
    };

    std::function<std::string(const SyntaxTreeInterfacePtr &)> callee_name;
    std::unordered_map<const SyntaxTreeInterface *, Info> funcs;
    std::unordered_map<std::string, const SyntaxTreeInterface *> globals;
    std::unordered_map<std::string, int> file_level_count;
    std::unordered_set<std::string> file_level_names;
    std::vector<std::unordered_map<std::string, Binding>> scopes;
    std::vector<FileFunc> file_funcs;
    const SyntaxTreeInterface *current = nullptr;
    size_t func_base = 0;

    static bool ReadDecl(const SyntaxTreeInterfacePtr &stmt, std::string &name, SyntaxTreeInterfacePtr &body, bool &is_local) {
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto funcname_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            const auto funcnamelist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcname_ptr->FuncNameList());
            if (!funcnamelist || funcnamelist->Funcnames().empty()) {
                return false;
            }
            name = funcnamelist->Funcnames()[0];
            body = func->Funcbody();
            is_local = false;
            return true;
        }
        if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
            name = func->Name();
            body = func->Funcbody();
            is_local = true;
            return true;
        }
        return false;
    }

    void Prescan(const std::shared_ptr<SyntaxTreeBlock> &chunk) {
        for (const auto &stmt: chunk->Stmts()) {
            std::string name;
            SyntaxTreeInterfacePtr body;
            bool is_local = false;
            if (!ReadDecl(stmt, name, body, is_local) || name.empty()) {
                continue;
            }
            file_level_names.insert(name);
            file_level_count[name] += 1;
            if (!is_local) {
                globals[name] = stmt.get();
            }
        }
    }

    void AddExact(Info &info, int count) {
        if (!info.has_exact) {
            info.has_exact = true;
            info.exact = count;
        } else if (info.exact != count) {
            info.conflict = true;
        }
        if (!info.dynamic_max) {
            info.max_concrete = std::max(info.max_concrete, count);
        }
    }

    [[nodiscard]] int FileCount(const std::string &name) const {
        const auto it = file_level_count.find(name);
        return it == file_level_count.end() ? 0 : it->second;
    }

    Tail ResolveCall(const SyntaxTreeInterfacePtr &call_exp, int prefix) const {
        Tail tail;
        tail.prefix = prefix;
        const std::string name = callee_name(call_exp);
        if (name.empty() || FileCount(name) > 1) {
            tail.unknown = true;
            return tail;
        }
        for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
            const auto it = scopes[static_cast<size_t>(i)].find(name);
            if (it == scopes[static_cast<size_t>(i)].end()) {
                continue;
            }
            const bool inside = current != nullptr && static_cast<size_t>(i) >= func_base;
            if (!it->second.is_func || (inside && !it->second.file_level && file_level_names.contains(name))) {
                // 形参/局部变量，或与文件级函数同名的嵌套 local function：不能用文件级简单名。
                tail.unknown = true;
                return tail;
            }
            tail.callee = it->second.func;
            return tail;
        }
        const auto git = globals.find(name);
        if (git == globals.end()) {
            tail.unknown = true;
            return tail;
        }
        tail.callee = git->second;
        return tail;
    }

    void NoteReturn(const SyntaxTreeInterfacePtr &stmt) {
        if (!current || !funcs.contains(current)) {
            return;
        }
        auto &info = funcs.at(current);
        const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);
        const auto el = ret->Explist() ? std::dynamic_pointer_cast<SyntaxTreeExplist>(ret->Explist()) : nullptr;
        if (!el || el->Exps().empty()) {
            AddExact(info, 0);
            return;
        }
        const auto &ret_exps = el->Exps();
        const int count = static_cast<int>(ret_exps.size());
        if (IsVarargExp(ret_exps.back())) {
            // return ... 与 return x, ... 的返回数都随调用者变化。
            info.dynamic_max = true;
            info.force_taint = true;
            return;
        }
        if (IsFunctionCallExp(ret_exps.back())) {
            info.dynamic_max = true;
            Tail tail = ResolveCall(ret_exps.back(), count - 1);
            if (tail.unknown) {
                info.force_taint = true;
            } else {
                info.tails.push_back(tail);
            }
            if (count == 1) {
                info.sole_calls.emplace_back(ret_exps[0].get(), tail);
            }
            return;
        }
        AddExact(info, count);
    }

    void AddParams(const SyntaxTreeInterfacePtr &funcbody) {
        const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
        if (!fb || !fb->Parlist()) {
            return;
        }
        const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(fb->Parlist());
        if (!parlist || !parlist->Namelist()) {
            return;
        }
        const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist());
        if (!namelist) {
            return;
        }
        for (const auto &pname: namelist->Names()) {
            scopes.back()[pname] = Binding{};
        }
    }

    void AddFunc(const SyntaxTreeInterface *node, const std::string &name, const SyntaxTreeInterfacePtr &funcbody, bool file_level) {
        funcs.emplace(node, Info{});
        const SyntaxTreeInterface *saved = current;
        const size_t saved_base = func_base;
        current = node;
        scopes.emplace_back();
        func_base = scopes.size() - 1;
        AddParams(funcbody);
        if (funcbody) {
            const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
            if (fb && fb->Block()) {
                WalkBlock(fb->Block());
            }
        }
        scopes.pop_back();
        current = saved;
        func_base = saved_base;

        if (!file_level || name.empty()) {
            return;
        }
        const auto &info = funcs.at(node);
        file_funcs.push_back(FileFunc{name, node, info.dynamic_max ? -1 : info.max_concrete});
    }

    void WalkBlock(const SyntaxTreeInterfacePtr &node) {
        if (!node) {
            return;
        }
        const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
        DEBUG_ASSERT(block);
        scopes.emplace_back();
        for (const auto &stmt: block->Stmts()) {
            WalkStmt(stmt);
        }
        scopes.pop_back();
    }

    void BindLocalFunc(const SyntaxTreeInterfacePtr &stmt, const std::string &name) {
        Binding binding;
        binding.is_func = true;
        binding.file_level = current == nullptr;
        binding.func = stmt.get();
        scopes.back()[name] = binding;
    }

    void WalkStmt(const SyntaxTreeInterfacePtr &stmt) {
        switch (stmt->Type()) {
            case SyntaxTreeType::Return:
                NoteReturn(stmt);
                break;
            case SyntaxTreeType::Block:
                WalkBlock(stmt);
                break;
            case SyntaxTreeType::If: {
                const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
                WalkBlock(if_node->Block());
                if (const auto elseifs = if_node->ElseIfs()) {
                    const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs);
                    for (const auto &blk: el->ElseifBlocks()) {
                        WalkBlock(blk);
                    }
                }
                WalkBlock(if_node->ElseBlock());
                break;
            }
            case SyntaxTreeType::While: {
                const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);
                WalkBlock(while_node->Block());
                break;
            }
            case SyntaxTreeType::Repeat: {
                const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);
                WalkBlock(rep->Block());
                break;
            }
            case SyntaxTreeType::ForLoop: {
                const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
                scopes.emplace_back();
                scopes.back()[for_loop->Name()] = Binding{};
                WalkBlock(for_loop->Block());
                scopes.pop_back();
                break;
            }
            case SyntaxTreeType::ForIn: {
                const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
                scopes.emplace_back();
                if (const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(for_in->Namelist())) {
                    for (const auto &name: nl->Names()) {
                        scopes.back()[name] = Binding{};
                    }
                }
                WalkBlock(for_in->Block());
                scopes.pop_back();
                break;
            }
            case SyntaxTreeType::LocalVar: {
                const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
                if (const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist())) {
                    for (const auto &name: nl->Names()) {
                        scopes.back()[name] = Binding{};
                    }
                }
                break;
            }
            case SyntaxTreeType::LocalFunction: {
                std::string name;
                SyntaxTreeInterfacePtr body;
                bool is_local = false;
                if (!ReadDecl(stmt, name, body, is_local)) {
                    break;
                }
                // 先绑定再进函数体，递归 local function 能解析到自己。
                BindLocalFunc(stmt, name);
                AddFunc(stmt.get(), name, body, current == nullptr);
                break;
            }
            case SyntaxTreeType::Function: {
                std::string name;
                SyntaxTreeInterfacePtr body;
                bool is_local = false;
                if (!ReadDecl(stmt, name, body, is_local)) {
                    break;
                }
                if (current != nullptr && !name.empty()) {
                    file_level_names.insert(name);
                    file_level_count[name] += 1;
                    globals[name] = stmt.get();
                }
                AddFunc(stmt.get(), name, body, current == nullptr);
                break;
            }
            case SyntaxTreeType::Assign:
            case SyntaxTreeType::FunctionCall:
            case SyntaxTreeType::Break:
            case SyntaxTreeType::Continue:
            case SyntaxTreeType::Goto:
            case SyntaxTreeType::Label:
            case SyntaxTreeType::Empty:
                break;
            default:
                ThrowFakeluaException(std::format("ReturnCountGraph: unexpected statement type {}", SyntaxTreeTypeToString(stmt->Type())));
        }
    }

    void Solve(AnalysisResult &ar) const {
        constexpr int kUnknown = -1;
        constexpr int kTainted = -2;
        std::unordered_map<const SyntaxTreeInterface *, int> eff;
        for (const auto &[node, info]: funcs) {
            if (info.force_taint || info.conflict) {
                eff[node] = kTainted;
            } else if (info.has_exact) {
                eff[node] = info.exact;
            } else if (info.tails.empty()) {
                eff[node] = 0;
            } else {
                eff[node] = kUnknown;
            }
        }
        for (size_t round = 0; round < funcs.size() + 1; ++round) {
            bool changed = false;
            for (const auto &[node, info]: funcs) {
                int &cur = eff.at(node);
                if (cur == kTainted) {
                    continue;
                }
                for (const auto &tail: info.tails) {
                    if (!tail.callee || !eff.contains(tail.callee)) {
                        cur = kTainted;
                        changed = true;
                        break;
                    }
                    const int cv = eff.at(tail.callee);
                    if (cv == kTainted) {
                        cur = kTainted;
                        changed = true;
                        break;
                    }
                    if (cv == kUnknown) {
                        continue;
                    }
                    const int total = tail.prefix + cv;
                    if (cur == kUnknown || (cur >= 0 && cur != total)) {
                        cur = (cur == kUnknown) ? total : kTainted;
                        changed = true;
                        if (cur == kTainted) {
                            break;
                        }
                    }
                }
            }
            if (!changed) {
                break;
            }
        }

        for (const auto &ff: file_funcs) {
            int value = eff.contains(ff.node) ? eff.at(ff.node) : kTainted;
            if (value < 0) {
                value = -1;
            }
            ar.function_effective_returns[ff.name] = value;
            ar.function_max_returns[ff.name] = ff.max_returns;
        }
        for (const auto &[node, info]: funcs) {
            (void) node;
            for (const auto &[exp, tail]: info.sole_calls) {
                int value = -1;
                if (!tail.unknown && tail.callee && eff.contains(tail.callee)) {
                    value = eff.at(tail.callee);
                    if (value < 0) {
                        value = -1;
                    }
                }
                ar.return_call_effective_returns[exp] = value;
            }
        }
    }

    void Run(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar) {
        DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
        const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
        Prescan(block);
        WalkBlock(chunk);
        Solve(ar);
    }
};

}// namespace

void SemanticAnalysis::AnalyzeFunctionReturnCounts(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar) {
    ReturnCountGraph graph;
    graph.callee_name = [this](const SyntaxTreeInterfacePtr &exp) { return GetCalleeName(exp); };
    graph.Run(chunk, ar);
}

void SemanticAnalysis::CollectReturnsForBlock(const SyntaxTreeInterfacePtr &node, std::vector<SyntaxTreeInterfacePtr> &returns) {
    if (!node) {
        return;
    }
    switch (node->Type()) {
        case SyntaxTreeType::Return: {
            returns.push_back(node);
            break;
        }
        case SyntaxTreeType::Block: {
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (const auto &stmt: block->Stmts()) {
                CollectReturnsForBlock(stmt, returns);
            }
            break;
        }
        case SyntaxTreeType::If: {
            const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            CollectReturnsForBlock(if_node->Block(), returns);
            CollectReturnsForBlock(if_node->ElseIfs(), returns);
            CollectReturnsForBlock(if_node->ElseBlock(), returns);
            break;
        }
        case SyntaxTreeType::ElseIfList: {
            const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
            for (const auto &blk: el->ElseifBlocks()) {
                CollectReturnsForBlock(blk, returns);
            }
            break;
        }
        case SyntaxTreeType::While: {
            const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            CollectReturnsForBlock(while_node->Block(), returns);
            break;
        }
        case SyntaxTreeType::Repeat: {
            const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            CollectReturnsForBlock(rep->Block(), returns);
            break;
        }
        case SyntaxTreeType::ForLoop: {
            const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            CollectReturnsForBlock(for_loop->Block(), returns);
            break;
        }
        case SyntaxTreeType::ForIn: {
            const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            CollectReturnsForBlock(for_in->Block(), returns);
            break;
        }
        case SyntaxTreeType::Function:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::FunctionDef:
        case SyntaxTreeType::Empty:
        case SyntaxTreeType::Label:
        case SyntaxTreeType::Assign:
        case SyntaxTreeType::FunctionCall:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::Continue:
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::LocalVar: {
            // These are valid statement types but do not contain return statements that belong to the current function scope.
            break;
        }
        case SyntaxTreeType::None:
        case SyntaxTreeType::VarList:
        case SyntaxTreeType::ExpList:
        case SyntaxTreeType::Var:
        case SyntaxTreeType::TableConstructor:
        case SyntaxTreeType::FieldList:
        case SyntaxTreeType::Field:
        case SyntaxTreeType::NameList:
        case SyntaxTreeType::FuncNameList:
        case SyntaxTreeType::FuncName:
        case SyntaxTreeType::FuncBody:
        case SyntaxTreeType::ParList:
        case SyntaxTreeType::Exp:
        case SyntaxTreeType::Binop:
        case SyntaxTreeType::Unop:
        case SyntaxTreeType::Args:
        case SyntaxTreeType::PrefixExp: {
            ThrowFakeluaException(std::format("unexpected non-statement syntax tree type in CollectReturnsForBlock: {}", SyntaxTreeTypeToString(node->Type())));
        }
        default: {
            ThrowFakeluaException(std::format("unknown syntax tree type in CollectReturnsForBlock: {}", static_cast<int>(node->Type())));
        }
    }
}

std::string SemanticAnalysis::GetCalleeName(const SyntaxTreeInterfacePtr &exp_node) {
    if (!IsFunctionCallExp(exp_node)) {
        return "";
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(exp_node);
    const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe->GetValue());
    const auto pe_pre = fc->prefixexp();
    if (pe_pre->Type() != SyntaxTreeType::PrefixExp) {
        return "";
    }
    const auto pe_pre_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe_pre);
    if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
        const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_pre_ptr->GetValue());
        if (callee_var->GetVarKind() == VarKind::kSimple) {
            return callee_var->GetName();
        }
    }
    return "";
}

void SemanticAnalysis::CheckUnsupportedSyntax(const SyntaxTreeInterfacePtr &chunk, const AnalysisResult &ar) {
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);

    top_level_stmts_.clear();
    for (const auto &stmt: top_block->Stmts()) {
        top_level_stmts_.insert(stmt.get());

        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            if (funcname) {
                if (!funcname->ColonName().empty()) {
                    ThrowError("Unsupported function name with method definition", stmt);
                }
                const auto fnlist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcname->FuncNameList());
                if (fnlist && fnlist->Funcnames().size() != 1) {
                    ThrowError(std::format("Unsupported function name with {} parts", fnlist->Funcnames().size()), stmt);
                }
            }
        }

        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
            if (!namelist) {
                ThrowError("local variable namelist is missing", stmt);
            }
            const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist());
            if (!el) {
                ThrowError("global constant must be initialized", stmt);
            }
            bool last_is_func = !el->Exps().empty() && IsFunctionCallExp(el->Exps().back());
            if (namelist->Names().size() != el->Exps().size() && !(last_is_func && namelist->Names().size() > el->Exps().size())) {
                ThrowError(std::format("local variable count {} not match expression count {}", namelist->Names().size(), el->Exps().size()), stmt);
            }
            for (const auto &exp: el->Exps()) {
                CheckGlobalConstExp(exp);
            }
        }
    }

    WalkSyntaxTree(chunk, [this, &ar](const SyntaxTreeInterfacePtr &node) { CheckNode(node, ar); });
    std::unordered_map<std::string, SyntaxTreeInterfacePtr> visible_labels;
    ValidateGotoInBlock(chunk, visible_labels, 0);
    ValidateConstAssignInBlock(chunk, {});
}

// Lua 在编译期就拒绝对 <const> 变量赋值，这里按块作用域收集 const 名字后做同样的检查。
void SemanticAnalysis::ValidateConstAssignInBlock(const SyntaxTreeInterfacePtr &chunk, std::unordered_set<std::string> const_names) {
    const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    if (!blk) return;

    for (const auto &stmt: blk->Stmts()) {
        switch (stmt->Type()) {
            case SyntaxTreeType::LocalVar: {
                // 先查赋值再登记：local x <const> = x 里右边的 x 是外层的同名变量。
                const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
                const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
                if (!namelist) break;
                const auto &names = namelist->Names();
                const auto &attribs = namelist->Attribs();
                for (size_t i = 0; i < names.size(); ++i) {
                    const bool is_const = i < attribs.size() && attribs[i] == "const";
                    if (is_const) {
                        const_names.insert(names[i]);
                    } else {
                        // 同名的非 const 局部变量会遮蔽外层的 const
                        const_names.erase(names[i]);
                    }
                }
                break;
            }
            case SyntaxTreeType::Assign: {
                const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
                const auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
                if (!varlist) break;
                for (const auto &var_node: varlist->Vars()) {
                    const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(var_node);
                    if (var && var->GetVarKind() == VarKind::kSimple && const_names.contains(var->GetName())) {
                        ThrowError("attempt to assign to const variable '" + var->GetName() + "'", stmt);
                    }
                }
                break;
            }
            case SyntaxTreeType::Block:
                ValidateConstAssignInBlock(stmt, const_names);
                break;
            case SyntaxTreeType::While:
                ValidateConstAssignInBlock(std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt)->Block(), const_names);
                break;
            case SyntaxTreeType::Repeat:
                ValidateConstAssignInBlock(std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt)->Block(), const_names);
                break;
            case SyntaxTreeType::ForLoop:
                ValidateConstAssignInBlock(std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt)->Block(), const_names);
                break;
            case SyntaxTreeType::ForIn:
                ValidateConstAssignInBlock(std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt)->Block(), const_names);
                break;
            case SyntaxTreeType::If: {
                const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
                ValidateConstAssignInBlock(if_stmt->Block(), const_names);
                if (const auto elseif_list = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs())) {
                    for (const auto &elseif_blk: elseif_list->ElseifBlocks()) {
                        ValidateConstAssignInBlock(elseif_blk, const_names);
                    }
                }
                if (if_stmt->ElseBlock()) ValidateConstAssignInBlock(if_stmt->ElseBlock(), const_names);
                break;
            }
            case SyntaxTreeType::Function: {
                // 函数体是新的作用域，外层局部变量在里面是 upvalue，Lua 同样禁止赋值，
                // 但 fakelua 不支持闭包捕获，所以这里只从空集合重新开始。
                const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt)->Funcbody());
                if (fb) ValidateConstAssignInBlock(fb->Block(), {});
                break;
            }
            case SyntaxTreeType::LocalFunction: {
                const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt)->Funcbody());
                if (fb) ValidateConstAssignInBlock(fb->Block(), {});
                break;
            }
            default:
                break;
        }
    }
}

void SemanticAnalysis::CheckNode(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar) {
    switch (node->Type()) {
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::Label: {
            CheckGotoOrLabel(node);
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            CheckFunctionCall(node);
            break;
        }
        case SyntaxTreeType::ParList: {
            CheckParList(node, ar);
            break;
        }
        case SyntaxTreeType::LocalVar: {
            CheckLocalVar(node, ar);
            break;
        }
        case SyntaxTreeType::Return: {
            break;
        }
        case SyntaxTreeType::ForLoop: {
            CheckForLoop(node);
            break;
        }
        case SyntaxTreeType::ForIn: {
            CheckForIn(node);
            break;
        }
        case SyntaxTreeType::Exp: {
            CheckExp(node);
            break;
        }
        case SyntaxTreeType::Block: {
            CheckBlockReturnPosition(node);
            break;
        }
        case SyntaxTreeType::None:
        case SyntaxTreeType::Empty:
        case SyntaxTreeType::Assign:
        case SyntaxTreeType::VarList:
        case SyntaxTreeType::ExpList:
        case SyntaxTreeType::Var:
        case SyntaxTreeType::TableConstructor:
        case SyntaxTreeType::FieldList:
        case SyntaxTreeType::Field:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::Continue:
        case SyntaxTreeType::While:
        case SyntaxTreeType::Repeat:
        case SyntaxTreeType::If:
        case SyntaxTreeType::ElseIfList:
        case SyntaxTreeType::NameList:
        case SyntaxTreeType::Function:
        case SyntaxTreeType::FuncNameList:
        case SyntaxTreeType::FuncName:
        case SyntaxTreeType::FuncBody:
        case SyntaxTreeType::FunctionDef:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::Binop:
        case SyntaxTreeType::Unop:
        case SyntaxTreeType::Args:
        case SyntaxTreeType::PrefixExp: {
            break;
        }
        default: {
            ThrowFakeluaException(std::format("unexpected SyntaxTreeType in CheckNode: {}", SyntaxTreeTypeToString(node->Type())));
        }
    }
}

void SemanticAnalysis::CheckGotoOrLabel(const SyntaxTreeInterfacePtr &node) {
    // 不再直接拒绝，具体验证在 ValidateGotoInBlock 中完成
}

void SemanticAnalysis::CollectBlockLabels(const SyntaxTreeInterfacePtr &block, std::unordered_map<std::string, SyntaxTreeInterfacePtr> &labels) {
    const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);
    if (!blk) return;
    std::unordered_set<std::string> in_this_block;
    for (const auto &stmt: blk->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Label) {
            const auto label = std::dynamic_pointer_cast<SyntaxTreeLabel>(stmt);
            const auto &name = label->GetName();
            if (!in_this_block.insert(name).second) {
                ThrowError(std::format("label '{}' already defined", name), stmt);
            }
            labels[name] = stmt;
        }
    }
}

void SemanticAnalysis::CollectGotosInBlock(const SyntaxTreeInterfacePtr &block, std::vector<std::pair<std::string, SyntaxTreeInterfacePtr>> &out) {
    const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);
    if (!blk) return;
    for (const auto &stmt: blk->Stmts()) {
        CollectGotosInStmt(stmt, out);
    }
}

void SemanticAnalysis::CollectGotosInStmt(const SyntaxTreeInterfacePtr &stmt, std::vector<std::pair<std::string, SyntaxTreeInterfacePtr>> &out) {
    if (!stmt) return;
    switch (stmt->Type()) {
        case SyntaxTreeType::Goto: {
            const auto g = std::dynamic_pointer_cast<SyntaxTreeGoto>(stmt);
            out.emplace_back(g->GetLabel(), stmt);
            break;
        }
        case SyntaxTreeType::Block:
            CollectGotosInBlock(stmt, out);
            break;
        case SyntaxTreeType::While:
            CollectGotosInBlock(std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt)->Block(), out);
            break;
        case SyntaxTreeType::Repeat:
            CollectGotosInBlock(std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt)->Block(), out);
            break;
        case SyntaxTreeType::If: {
            const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
            CollectGotosInBlock(if_stmt->Block(), out);
            if (if_stmt->ElseIfs()) {
                const auto elseif_list = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs());
                if (elseif_list) {
                    for (const auto &elseif_blk: elseif_list->ElseifBlocks()) {
                        CollectGotosInBlock(elseif_blk, out);
                    }
                }
            }
            if (if_stmt->ElseBlock()) CollectGotosInBlock(if_stmt->ElseBlock(), out);
            break;
        }
        case SyntaxTreeType::ForLoop:
            CollectGotosInBlock(std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt)->Block(), out);
            break;
        case SyntaxTreeType::ForIn:
            CollectGotosInBlock(std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt)->Block(), out);
            break;
        default:
            break;
    }
}

void SemanticAnalysis::ValidateGotoInBlock(const SyntaxTreeInterfacePtr &chunk, std::unordered_map<std::string, SyntaxTreeInterfacePtr> visible_labels, int loop_depth) {
    const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    if (!blk) return;

    CollectBlockLabels(chunk, visible_labels);

    std::unordered_map<const SyntaxTreeInterface *, size_t> label_index;
    for (size_t i = 0; i < blk->Stmts().size(); ++i) {
        if (blk->Stmts()[i]->Type() == SyntaxTreeType::Label) {
            label_index[blk->Stmts()[i].get()] = i;
        }
    }

    std::vector<size_t> local_positions;
    for (size_t i = 0; i < blk->Stmts().size(); ++i) {
        const auto st = blk->Stmts()[i]->Type();
        if (st == SyntaxTreeType::LocalVar || st == SyntaxTreeType::LocalFunction) {
            local_positions.push_back(i);
        }
    }

    auto check_skip_locals = [&](size_t from, size_t label_pos, const std::string &target_name, const SyntaxTreeInterfacePtr &err_node) {
        if (from < label_pos && label_pos < blk->Stmts().size()) {
            for (auto lp: local_positions) {
                if (lp > from && lp <= label_pos) {
                    ThrowError(std::format("goto '{}' jumps over local variable declaration", target_name), err_node);
                }
            }
        }
    };

    for (size_t i = 0; i < blk->Stmts().size(); ++i) {
        const auto &stmt = blk->Stmts()[i];
        if (stmt->Type() == SyntaxTreeType::Goto) {
            const auto goto_stmt = std::dynamic_pointer_cast<SyntaxTreeGoto>(stmt);
            const auto &target_name = goto_stmt->GetLabel();
            auto it = visible_labels.find(target_name);
            if (it == visible_labels.end()) {
                ThrowError(std::format("goto target '{}' not found", target_name), stmt);
            }
            auto lit = label_index.find(it->second.get());
            if (lit != label_index.end()) {
                check_skip_locals(i, lit->second, target_name, stmt);
            }
        } else if (stmt->Type() == SyntaxTreeType::Continue) {
            if (loop_depth <= 0) {
                ThrowError("'continue' statement not inside a loop", stmt);
            }
            for (auto lp: local_positions) {
                if (lp > i) {
                    ThrowError("'continue' jumps over local variable declaration", stmt);
                }
            }
        } else if (stmt->Type() == SyntaxTreeType::Block || stmt->Type() == SyntaxTreeType::While || stmt->Type() == SyntaxTreeType::Repeat ||
                   stmt->Type() == SyntaxTreeType::If || stmt->Type() == SyntaxTreeType::ForLoop || stmt->Type() == SyntaxTreeType::ForIn) {
            std::vector<std::pair<std::string, SyntaxTreeInterfacePtr>> nested;
            CollectGotosInStmt(stmt, nested);
            for (const auto &[target_name, goto_node]: nested) {
                auto it = visible_labels.find(target_name);
                if (it == visible_labels.end()) continue;
                auto lit = label_index.find(it->second.get());
                if (lit != label_index.end()) {
                    check_skip_locals(i, lit->second, target_name, goto_node);
                }
            }
        }
    }

    for (const auto &stmt: blk->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Block) {
            ValidateGotoInBlock(stmt, visible_labels, loop_depth);
        } else if (stmt->Type() == SyntaxTreeType::While) {
            ValidateGotoInBlock(std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt)->Block(), visible_labels, loop_depth + 1);
        } else if (stmt->Type() == SyntaxTreeType::Repeat) {
            ValidateGotoInBlock(std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt)->Block(), visible_labels, loop_depth + 1);
        } else if (stmt->Type() == SyntaxTreeType::If) {
            const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
            ValidateGotoInBlock(if_stmt->Block(), visible_labels, loop_depth);
            if (if_stmt->ElseIfs()) {
                const auto elseif_list = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs());
                if (elseif_list) {
                    for (const auto &elseif_blk: elseif_list->ElseifBlocks()) {
                        ValidateGotoInBlock(elseif_blk, visible_labels, loop_depth);
                    }
                }
            }
            if (if_stmt->ElseBlock()) ValidateGotoInBlock(if_stmt->ElseBlock(), visible_labels, loop_depth);
        } else if (stmt->Type() == SyntaxTreeType::ForLoop) {
            ValidateGotoInBlock(std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt)->Block(), visible_labels, loop_depth + 1);
        } else if (stmt->Type() == SyntaxTreeType::ForIn) {
            ValidateGotoInBlock(std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt)->Block(), visible_labels, loop_depth + 1);
        } else if (stmt->Type() == SyntaxTreeType::Function) {
            std::unordered_map<std::string, SyntaxTreeInterfacePtr> func_labels;
            const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt)->Funcbody());
            if (fb) ValidateGotoInBlock(fb->Block(), func_labels, 0);
        } else if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            std::unordered_map<std::string, SyntaxTreeInterfacePtr> func_labels;
            const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt)->Funcbody());
            if (fb) ValidateGotoInBlock(fb->Block(), func_labels, 0);
        }
    }
}

void SemanticAnalysis::CheckFunctionCall(const SyntaxTreeInterfacePtr &node) {
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
    const auto callee_prefixexp = fc->prefixexp();
    if (!callee_prefixexp || callee_prefixexp->Type() != SyntaxTreeType::PrefixExp) {
        ThrowError("function call callee must be a prefix expression", node);
    }
    const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(callee_prefixexp);
    if (callee_pe->GetPrefixKind() == PrefixExpKind::kVar) {
        const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
        if (callee_var && callee_var->GetVarKind() == VarKind::kSimple && callee_var->GetName() == "FAKELUA_SET_TABLE") {
            const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
            if (!args_ptr || args_ptr->GetArgsKind() != ArgsKind::kExpList) {
                ThrowError("FAKELUA_SET_TABLE expects exactly 3 arguments", node);
            }
            const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
            if (!explist_ptr || explist_ptr->Exps().size() != 3) {
                ThrowError("FAKELUA_SET_TABLE expects exactly 3 arguments", node);
            }
        }
    }
}

void SemanticAnalysis::CheckParList(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar) {
    const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(node);
    if (parlist->VarParams()) {
        ThrowError("varargs (...) is not supported", node);
    }
    const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist());
    if (namelist) {
        std::set<std::string> param_names_set;
        for (const auto &name: ar.global_const_names) {
            param_names_set.insert(name);
        }
        for (const auto &name: namelist->Names()) {
            if (param_names_set.contains(name)) {
                ThrowError("the param name is duplicated: " + name, namelist);
            }
            param_names_set.insert(name);
        }
    }
    if (const size_t param_size = namelist ? namelist->Names().size() : 0; param_size > kMaxFunctionInputParams) {
        ThrowError(std::format("function input parameters exceed limit {}, got {}", kMaxFunctionInputParams, param_size), node);
    }
}

void SemanticAnalysis::CheckLocalVar(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar) {
    const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
    const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
    if (!namelist) {
        ThrowError("local variable namelist is missing", node);
    }
    if (!top_level_stmts_.contains(node.get())) {
        if (namelist) {
            for (const auto &name: namelist->Names()) {
                if (ar.global_const_names.contains(name)) {
                    ThrowError("local variable conflicts with global constant: " + name, node);
                }
            }
        }
    }

    // Lua 只认 <const> 和 <close> 两种属性，其它一律报 unknown attribute。
    for (const auto &attrib: namelist->Attribs()) {
        if (!attrib.empty() && attrib != "const" && attrib != "close") {
            ThrowError("unknown attribute '" + attrib + "'", node);
        }
    }
}

// Lua 的文法是 block ::= {stat} [retstat]，return 只能是所在块的最后一条语句。
// FakeLua 的文法把 retstat 当成了普通 stmt，所以这里补上位置校验，否则会比 Lua 宽松，
// 接受 Lua 明确拒绝的代码（差分 fuzz 已经抓到过这种分歧）。
// retstat 不吞掉结尾的分号，因此 return 后面允许跟若干空语句。
void SemanticAnalysis::CheckBlockReturnPosition(const SyntaxTreeInterfacePtr &node) {
    const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
    if (!blk) {
        return;
    }
    const auto &stmts = blk->Stmts();

    size_t last_effective = stmts.size();
    for (size_t i = stmts.size(); i > 0; --i) {
        if (stmts[i - 1]->Type() != SyntaxTreeType::Empty) {
            last_effective = i - 1;
            break;
        }
    }

    for (size_t i = 0; i < stmts.size(); ++i) {
        if (stmts[i]->Type() == SyntaxTreeType::Return && i != last_effective) {
            ThrowError("'return' must be the last statement in a block", stmts[i]);
        }
    }
}

void SemanticAnalysis::CheckForLoop(const SyntaxTreeInterfacePtr &node) {
    const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
    if (const auto step_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(for_loop->ExpStep())) {
        TableKeyKind kind = TableKeyKind::kInt;
        std::string canonical;
        int64_t int_value = 0;
        double float_value = 0;
        if (ClassifyConstNumberExp(step_exp, kind, canonical, int_value, float_value)) {
            if ((kind == TableKeyKind::kInt && int_value == 0) || (kind == TableKeyKind::kFloat && float_value == 0.0)) {
                ThrowError("'for' step is zero", step_exp);
            }
        }
    }
}

void SemanticAnalysis::CheckForIn(const SyntaxTreeInterfacePtr &node) {
    const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);

    const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(for_in->Namelist());
    if (!namelist || namelist->Names().empty()) {
        ThrowError("for in loop requires at least one variable name", node);
    }

    const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(for_in->Explist());
    if (!explist || explist->Exps().empty()) {
        ThrowError("for in loop requires an expression list", node);
    }
}

void SemanticAnalysis::CheckExp(const SyntaxTreeInterfacePtr &node) {
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    const auto kind = exp->GetExpKind();
    if (kind == ExpKind::kVarParams) {
        ThrowError("... is not supported", node);
    }
}

void SemanticAnalysis::CheckGlobalConstExp(const SyntaxTreeInterfacePtr &exp) {
    DEBUG_ASSERT(exp && exp->Type() == SyntaxTreeType::Exp);
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto exp_kind = e->GetExpKind();
    if (exp_kind == ExpKind::kTableConstructor) {
        ThrowError("table constructor is not supported in global variable initialization", exp);
    } else if (exp_kind == ExpKind::kBinop) {
        ThrowError("binary operator is not supported in global variable initialization", exp);
    } else if (exp_kind == ExpKind::kUnop) {
        ThrowError("unary operator is not supported in global variable initialization", exp);
    } else if (exp_kind == ExpKind::kPrefixExp) {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        if (pe) {
            if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
                ThrowError("variable reference is not allowed in global variable initialization", exp);
            } else if (pe->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
                ThrowError("function call is not allowed in global variable initialization", exp);
            }
        }
    }
}

[[noreturn]] void SemanticAnalysis::ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) {
    ThrowFakeluaException(std::format("SemanticAnalysis check failed, {} at {}", msg, SyntaxTreeLocationStr(file_name_, ptr)));
}

void SemanticAnalysis::CheckUndeclaredVars(const SyntaxTreeInterfacePtr &chunk, const AnalysisResult &ar) {
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);

    // 文件级声明名：预处理后文件级 local 仍留在顶层（复杂初始化被搬进 __fakelua_init
    // 的赋值语句，右侧名字也都来自这些声明），文件级 function/local function 同样在顶层
    // （含编译器合成的 __fakelua_init）。函数体内对这些名字的引用是合法的文件级符号。
    std::unordered_set<std::string> file_level_names = ar.global_const_names;
    for (const auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            file_level_names.insert(std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt)->Name());
        } else if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto fname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            if (fname && fname->ColonName().empty()) {
                if (const auto fnl = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(fname->FuncNameList());
                    fnl && fnl->Funcnames().size() == 1) {
                    file_level_names.insert(fnl->Funcnames()[0]);
                }
            }
        }
    }

    // 沿 Var 的 kVar 前缀取上一级变量（a.b / a.b.c 的基变量），前缀不是 kVar 时返回空。
    auto base_var_of = [](const std::shared_ptr<SyntaxTreeVar> &v) -> std::shared_ptr<SyntaxTreeVar> {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(v->GetPrefixexp());
        if (!pe || pe->GetPrefixKind() != PrefixExpKind::kVar) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
    };

    // 豁免变量集合（不按"必须声明"检查），来源有两类：
    //   1. 直接调用位 f(...) / a.b.f(...) 的整条点号调用链——不含冒号方法调用 a:m()，
    //      其接收者 a 仍是普通变量引用。同文件函数已在 file_level_names；宿主原生函数
    //      允许编译后再注册；其余未知名字维持运行时 FakeluaCallByName 的 "not found" 报错。
    //      链上遇到 kSquare 即停止：a[1].f() 里的 a 是必须声明的表变量。
    //   2. 点号原生库模块根名（math/string/utf8 ...），覆盖非调用位的模块常量访问
    //      （math.pi、string.charpattern）；模块名由已注册的 "math.xxx" 原生函数前缀识别，
    //      它本身不是脚本变量。
    std::unordered_set<const SyntaxTreeInterface *> exempt_vars;
    // 赋值左值集合，用于给出「先 local 再赋值」的定向提示。
    std::unordered_set<const SyntaxTreeInterface *> lvalue_vars;
    WalkSyntaxTree(chunk, [&](const SyntaxTreeInterfacePtr &n) {
        if (n->Type() == SyntaxTreeType::FunctionCall) {
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(n);
            if (!fc->Name().empty()) {
                return;
            }
            const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
            if (!pe || pe->GetPrefixKind() != PrefixExpKind::kVar) {
                return;
            }
            for (auto cur = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue()); cur;) {
                exempt_vars.insert(cur.get());
                if (cur->GetVarKind() != VarKind::kDot) {
                    break;
                }
                cur = base_var_of(cur);
            }
        } else if (n->Type() == SyntaxTreeType::Assign) {
            // package = "X" 形式的 package 声明（CheckFileLevelStmts 已限定其只能出现在
            // 文件首行；预处理会把它搬进 __fakelua_init）：左值 package 是语法关键字位，
            // 不是脚本变量。package("X") 调用形式已由上面的调用链豁免覆盖。
            if (std::string pkg_name; ExtractPackageName(n, pkg_name)) {
                if (const auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(
                        std::dynamic_pointer_cast<SyntaxTreeAssign>(n)->Varlist())) {
                    for (const auto &var_node: vl->Vars()) {
                        exempt_vars.insert(var_node.get());
                    }
                }
                return;
            }
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(n);
            const auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
            if (!vl) {
                return;
            }
            for (const auto &var_node: vl->Vars()) {
                const auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(var_node);
                if (v && v->GetVarKind() == VarKind::kSimple) {
                    lvalue_vars.insert(v.get());
                }
            }
        }
    });
    WalkSyntaxTree(chunk, [&](const SyntaxTreeInterfacePtr &n) {
        if (n->Type() != SyntaxTreeType::Var) {
            return;
        }
        auto cur = std::dynamic_pointer_cast<SyntaxTreeVar>(n);
        if (!cur || cur->GetVarKind() != VarKind::kDot) {
            return;
        }
        // 只沿点号链下行到根；链中夹 kSquare（a[1].b）时根 a 仍是普通表变量引用。
        while (cur && cur->GetVarKind() == VarKind::kDot) {
            cur = base_var_of(cur);
        }
        if (cur && cur->GetVarKind() == VarKind::kSimple &&
            s_->GetVM().HasNativeFunctionWithPrefix(std::string(cur->GetName()) + ".")) {
            exempt_vars.insert(cur.get());
        }
    });

    std::vector<std::unordered_set<std::string>> scopes;
    CheckVarScopes(chunk, scopes, file_level_names, exempt_vars, lvalue_vars);
}

bool SemanticAnalysis::IsDeclaredSimpleName(const std::string &name, const std::vector<std::unordered_set<std::string>> &scopes,
                                           const std::unordered_set<std::string> &file_level_names) const {
    if (name == "_VERSION") {
        return true;
    }
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->contains(name)) {
            return true;
        }
    }
    if (file_level_names.contains(name)) {
        return true;
    }
    // 宿主用 RegisterNativeFunction 声明的全局原生函数（print/type/pairs 及用户回调名）。
    if (s_->GetVM().FindNativeFunction(name) != nullptr) {
        return true;
    }
    return false;
}

void SemanticAnalysis::CheckVarScopes(const SyntaxTreeInterfacePtr &node, std::vector<std::unordered_set<std::string>> &scopes,
                                      const std::unordered_set<std::string> &file_level_names,
                                      const std::unordered_set<const SyntaxTreeInterface *> &exempt_vars,
                                      const std::unordered_set<const SyntaxTreeInterface *> &lvalue_vars) {
    if (!node) {
        return;
    }

    // 检查 kSimple 变量引用。kDot/kSquare 的基表达式随递归继续检查。
    auto check_simple_var = [&](const SyntaxTreeInterfacePtr &vnode) {
        const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(vnode);
        if (!var || var->GetVarKind() != VarKind::kSimple || exempt_vars.contains(var.get())) {
            return;
        }
        const std::string &name = var->GetName();
        if (IsDeclaredSimpleName(name, scopes, file_level_names)) {
            return;
        }
        if (lvalue_vars.contains(var.get())) {
            ThrowError(std::format("undeclared variable '{}' on the left side of assignment; fakelua has no implicit "
                                   "globals, declare it with 'local' before assigning",
                                   name),
                       vnode);
        }
        ThrowError(std::format("unknown variable '{}'; fakelua has no implicit globals, declare it with 'local' first", name), vnode);
    };

    switch (node->Type()) {
        case SyntaxTreeType::Block: {
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            scopes.emplace_back();
            for (const auto &stmt: block->Stmts()) {
                CheckVarScopes(stmt, scopes, file_level_names, exempt_vars, lvalue_vars);
            }
            scopes.pop_back();
            break;
        }
        case SyntaxTreeType::LocalVar: {
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            // 先用外层作用域解析初始化表达式（local x = x 右边的 x 是外层同名变量）
            CheckVarScopes(lv->Explist(), scopes, file_level_names, exempt_vars, lvalue_vars);
            if (const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist())) {
                for (const auto &name: nl->Names()) {
                    scopes.back().insert(name);
                }
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            const auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            CheckVarScopes(fl->ExpBegin(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(fl->ExpEnd(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(fl->ExpStep(), scopes, file_level_names, exempt_vars, lvalue_vars);
            scopes.emplace_back();
            scopes.back().insert(fl->Name());
            CheckVarScopes(fl->Block(), scopes, file_level_names, exempt_vars, lvalue_vars);
            scopes.pop_back();
            break;
        }
        case SyntaxTreeType::ForIn: {
            const auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            CheckVarScopes(fi->Explist(), scopes, file_level_names, exempt_vars, lvalue_vars);
            scopes.emplace_back();
            if (const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(fi->Namelist())) {
                for (const auto &name: nl->Names()) {
                    scopes.back().insert(name);
                }
            }
            CheckVarScopes(fi->Block(), scopes, file_level_names, exempt_vars, lvalue_vars);
            scopes.pop_back();
            break;
        }
        case SyntaxTreeType::Function:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::FunctionDef: {
            // local function f 对自身函数体可见（支持递归），在开新作用域前登记。
            if (node->Type() == SyntaxTreeType::LocalFunction) {
                scopes.back().insert(std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node)->Name());
            }
            SyntaxTreeInterfacePtr funcbody;
            if (node->Type() == SyntaxTreeType::Function) {
                funcbody = std::dynamic_pointer_cast<SyntaxTreeFunction>(node)->Funcbody();
            } else if (node->Type() == SyntaxTreeType::LocalFunction) {
                funcbody = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node)->Funcbody();
            } else {
                funcbody = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(node)->Funcbody();
            }
            scopes.emplace_back();
            if (funcbody) {
                const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
                if (const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(fb->Parlist())) {
                    if (const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist())) {
                        for (const auto &pname: namelist->Names()) {
                            scopes.back().insert(pname);
                        }
                    }
                }
                CheckVarScopes(fb->Block(), scopes, file_level_names, exempt_vars, lvalue_vars);
            }
            scopes.pop_back();
            break;
        }
        case SyntaxTreeType::Var: {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
            check_simple_var(node);
            CheckVarScopes(var->GetPrefixexp(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(var->GetExp(), scopes, file_level_names, exempt_vars, lvalue_vars);
            break;
        }
        case SyntaxTreeType::Return: {
            CheckVarScopes(std::dynamic_pointer_cast<SyntaxTreeReturn>(node)->Explist(), scopes, file_level_names, exempt_vars, lvalue_vars);
            break;
        }
        case SyntaxTreeType::VarList: {
            const auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(node);
            for (const auto &v: vl->Vars()) {
                CheckVarScopes(v, scopes, file_level_names, exempt_vars, lvalue_vars);
            }
            break;
        }
        case SyntaxTreeType::ExpList: {
            const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            for (const auto &exp: el->Exps()) {
                CheckVarScopes(exp, scopes, file_level_names, exempt_vars, lvalue_vars);
            }
            break;
        }
        case SyntaxTreeType::Assign: {
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            CheckVarScopes(assign->Varlist(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(assign->Explist(), scopes, file_level_names, exempt_vars, lvalue_vars);
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            CheckVarScopes(fc->prefixexp(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(fc->Args(), scopes, file_level_names, exempt_vars, lvalue_vars);
            break;
        }
        case SyntaxTreeType::Args: {
            const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            CheckVarScopes(args->Explist(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(args->Tableconstructor(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(args->String(), scopes, file_level_names, exempt_vars, lvalue_vars);
            break;
        }
        case SyntaxTreeType::TableConstructor: {
            CheckVarScopes(std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node)->Fieldlist(), scopes, file_level_names, exempt_vars,
                           lvalue_vars);
            break;
        }
        case SyntaxTreeType::FieldList: {
            const auto fl = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            for (const auto &field: fl->Fields()) {
                CheckVarScopes(field, scopes, file_level_names, exempt_vars, lvalue_vars);
            }
            break;
        }
        case SyntaxTreeType::Field: {
            const auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            CheckVarScopes(field->Key(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(field->Value(), scopes, file_level_names, exempt_vars, lvalue_vars);
            break;
        }
        case SyntaxTreeType::While: {
            const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            CheckVarScopes(while_node->Exp(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(while_node->Block(), scopes, file_level_names, exempt_vars, lvalue_vars);
            break;
        }
        case SyntaxTreeType::Repeat: {
            // Lua 语义（与 CGen::CompileStmtRepeat 一致）：until 条件在 repeat body 的
            // 作用域内解析，可以引用 body 中声明的 local（repeat local x ... until x > 0）。
            const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            const auto rep_body = std::dynamic_pointer_cast<SyntaxTreeBlock>(rep->Block());
            scopes.emplace_back();
            if (rep_body) {
                for (const auto &stmt: rep_body->Stmts()) {
                    CheckVarScopes(stmt, scopes, file_level_names, exempt_vars, lvalue_vars);
                }
            }
            CheckVarScopes(rep->Exp(), scopes, file_level_names, exempt_vars, lvalue_vars);
            scopes.pop_back();
            break;
        }
        case SyntaxTreeType::If: {
            const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            CheckVarScopes(if_node->Exp(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(if_node->Block(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(if_node->ElseIfs(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(if_node->ElseBlock(), scopes, file_level_names, exempt_vars, lvalue_vars);
            break;
        }
        case SyntaxTreeType::ElseIfList: {
            const auto eil = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
            for (size_t i = 0; i < eil->ElseifSize(); ++i) {
                CheckVarScopes(eil->ElseifExp(i), scopes, file_level_names, exempt_vars, lvalue_vars);
                CheckVarScopes(eil->ElseifBlock(i), scopes, file_level_names, exempt_vars, lvalue_vars);
            }
            break;
        }
        case SyntaxTreeType::Exp: {
            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            CheckVarScopes(exp->Left(), scopes, file_level_names, exempt_vars, lvalue_vars);
            CheckVarScopes(exp->Right(), scopes, file_level_names, exempt_vars, lvalue_vars);
            break;
        }
        case SyntaxTreeType::PrefixExp: {
            CheckVarScopes(std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node)->GetValue(), scopes, file_level_names, exempt_vars,
                           lvalue_vars);
            break;
        }
        default: {
            // 其余节点（break/continue/goto/label/空语句/名字列表等）不含变量引用。
            break;
        }
    }
}

}// namespace fakelua
