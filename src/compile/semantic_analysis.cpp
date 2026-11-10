#include "compile/semantic_analysis.h"
#include "interp/opcode.h"
#include "interp/runtime.h"
#include "util/string_util.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include <algorithm>
#include <functional>
#include <memory>

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

    // 每函数寄存器高峰：与解释器 AllocLocal/AllocReg/FreeTo 寿命规则相同，
    // 作为语言约束在语义分析阶段统一门禁，TCC/GCC 不再依赖解释器 codegen。
    CheckRegisterLimits(pr, ar);

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

void SemanticAnalysis::ValidateConstInFuncbody(const SyntaxTreeInterfacePtr &funcbody, std::unordered_set<std::string> const_names,
                                               const std::unordered_set<std::string> &extra_shadow) {
    if (!funcbody) {
        return;
    }
    for (const auto &name: extra_shadow) {
        const_names.erase(name);
    }
    const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
    if (!fb) {
        return;
    }
    if (const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(fb->Parlist())) {
        if (const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist())) {
            for (const auto &pname: namelist->Names()) {
                const_names.erase(pname);
            }
        }
    }
    ValidateConstAssignInBlock(fb->Block(), std::move(const_names));
}

void SemanticAnalysis::ValidateConstInExprs(const SyntaxTreeInterfacePtr &node, const std::unordered_set<std::string> &const_names) {
    if (!node) {
        return;
    }
    WalkSyntaxTreePruned(node, [this, &const_names](const SyntaxTreeInterfacePtr &n) -> bool {
        if (!n) {
            return false;
        }
        if (n->Type() == SyntaxTreeType::FunctionDef) {
            ValidateConstInFuncbody(std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(n)->Funcbody(), const_names);
            return false;
        }
        // Function / LocalFunction 作为语句由 ValidateConstAssignInBlock 处理；
        // 出现在表达式子树里时也按同样规则继承外层 const。
        if (n->Type() == SyntaxTreeType::Function) {
            ValidateConstInFuncbody(std::dynamic_pointer_cast<SyntaxTreeFunction>(n)->Funcbody(), const_names);
            return false;
        }
        if (n->Type() == SyntaxTreeType::LocalFunction) {
            const auto lf = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(n);
            ValidateConstInFuncbody(lf->Funcbody(), const_names, {lf->Name()});
            return false;
        }
        return true;
    });
}

// Lua 在编译期就拒绝对 <const> 变量赋值，这里按块作用域收集 const 名字后做同样的检查。
// 嵌套函数（LocalFunction / FunctionDef / Function）继承外层词法 const（upvalue），
// 形参与同名 local 可遮蔽；块内 do/for/if 等嵌套作用域同样尊重遮蔽。
void SemanticAnalysis::ValidateConstAssignInBlock(const SyntaxTreeInterfacePtr &chunk, std::unordered_set<std::string> const_names) {
    const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    if (!blk) return;

    for (const auto &stmt: blk->Stmts()) {
        switch (stmt->Type()) {
            case SyntaxTreeType::LocalVar: {
                // 先处理初始化表达式里的 FunctionDef（右边的名字仍是外层作用域），再登记本句 local。
                const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
                ValidateConstInExprs(lv->Explist(), const_names);
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
                if (varlist) {
                    for (const auto &var_node: varlist->Vars()) {
                        const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(var_node);
                        if (var && var->GetVarKind() == VarKind::kSimple && const_names.contains(var->GetName())) {
                            ThrowError("attempt to assign to const variable '" + var->GetName() + "'", stmt);
                        }
                        // t[f()] 等左侧表达式里也可能嵌匿名函数
                        if (var) {
                            ValidateConstInExprs(var->GetPrefixexp(), const_names);
                            ValidateConstInExprs(var->GetExp(), const_names);
                        }
                    }
                }
                ValidateConstInExprs(assign->Explist(), const_names);
                break;
            }
            case SyntaxTreeType::Block:
                ValidateConstAssignInBlock(stmt, const_names);
                break;
            case SyntaxTreeType::While: {
                const auto w = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);
                ValidateConstInExprs(w->Exp(), const_names);
                ValidateConstAssignInBlock(w->Block(), const_names);
                break;
            }
            case SyntaxTreeType::Repeat: {
                const auto r = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);
                ValidateConstAssignInBlock(r->Block(), const_names);
                ValidateConstInExprs(r->Exp(), const_names);
                break;
            }
            case SyntaxTreeType::ForLoop: {
                const auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
                ValidateConstInExprs(fl->ExpBegin(), const_names);
                ValidateConstInExprs(fl->ExpEnd(), const_names);
                ValidateConstInExprs(fl->ExpStep(), const_names);
                auto nested = const_names;
                nested.erase(fl->Name());
                ValidateConstAssignInBlock(fl->Block(), std::move(nested));
                break;
            }
            case SyntaxTreeType::ForIn: {
                const auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
                ValidateConstInExprs(fi->Explist(), const_names);
                auto nested = const_names;
                if (const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(fi->Namelist())) {
                    for (const auto &name: nl->Names()) {
                        nested.erase(name);
                    }
                }
                ValidateConstAssignInBlock(fi->Block(), std::move(nested));
                break;
            }
            case SyntaxTreeType::If: {
                const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
                ValidateConstInExprs(if_stmt->Exp(), const_names);
                ValidateConstAssignInBlock(if_stmt->Block(), const_names);
                if (const auto elseif_list = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs())) {
                    for (size_t i = 0; i < elseif_list->ElseifSize(); ++i) {
                        ValidateConstInExprs(elseif_list->ElseifExp(i), const_names);
                        ValidateConstAssignInBlock(elseif_list->ElseifBlock(i), const_names);
                    }
                }
                if (if_stmt->ElseBlock()) ValidateConstAssignInBlock(if_stmt->ElseBlock(), const_names);
                break;
            }
            case SyntaxTreeType::Function: {
                // 继承外层词法 const（作为 upvalue）；嵌套命名 function 语句会在 CheckNode 中另行拒绝。
                ValidateConstInFuncbody(std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt)->Funcbody(), const_names);
                break;
            }
            case SyntaxTreeType::LocalFunction: {
                const auto lf = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
                ValidateConstInFuncbody(lf->Funcbody(), const_names, {lf->Name()});
                // local function 名遮蔽后续语句中的同名 const
                const_names.erase(lf->Name());
                break;
            }
            case SyntaxTreeType::Return:
                ValidateConstInExprs(std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt)->Explist(), const_names);
                break;
            case SyntaxTreeType::FunctionCall:
                ValidateConstInExprs(stmt, const_names);
                break;
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
        case SyntaxTreeType::Function: {
            // 命名 function 语句在非文件级会写成隐式全局；fakelua 不支持隐式全局，
            // 在语义分析阶段拒绝，避免拖到 codegen 才报内部 "not support stmt type"。
            if (!top_level_stmts_.contains(node.get())) {
                ThrowError("named function statement is not allowed here; fakelua has no implicit globals, "
                           "use 'local function name(...)' or assign a function expression to a local",
                           node);
            }
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
        } else if (stmt->Type() == SyntaxTreeType::Break) {
            if (loop_depth <= 0) {
                ThrowError("'break' statement not inside a loop", stmt);
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
    // 宿主用 RegisterNativeFunction 注册的全局原生函数不在此列：裸原生名只有出现在
    // 直接调用位（f(...) / a.b.f(...)，整条链已在 exempt_vars 豁免）才合法；出现在
    // 值位置由 check_simple_var 给出「native function cannot be used as a value」
    // 的明确编译期报错（fakelua 按名字派发原生调用，原生函数不是一等值）。
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
        // 原生函数出现在值位置（local t = type、pcall(print, ...)、return next 等）：
        // fakelua 在编译期按名字派发原生调用，原生函数不是一等值。此前这里放行，
        // 到运行时才以 "attempt to call/index a non-function/table value" 失败，
        // 报错位置与原因都有误导性；改为编译期明确报错。调用位已在 exempt_vars 豁免。
        if (s_->GetVM().FindNativeFunction(name) != nullptr) {
            ThrowError(std::format("native function '{}' cannot be used as a value; fakelua dispatches native calls "
                                   "by name at compile time, call it directly or wrap it in a Lua function",
                                   name),
                       vnode);
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
            // 点号原生库函数出现在值位置（local f = table.concat、pcall(table.concat, t)、
            // { cb = string.format } 等）：调用位的整条点号链已在 exempt_vars 豁免，能走到
            // 这里的 kDot 链都是值位置。全名命中原生函数注册表时给出与裸原生名一致的明确
            // 编译期报错。常量访问（math.pi、string.charpattern）不在注册表里；根名是局部
            // 变量时（local t = {} 之后的 t.concat）是普通字段访问，两者都不报错。
            if (var->GetVarKind() == VarKind::kDot && !exempt_vars.contains(var.get())) {
                std::string full;
                std::shared_ptr<SyntaxTreeVar> root;
                for (auto cur = var; cur && cur->GetVarKind() == VarKind::kDot;) {
                    full = cur->GetName() + (full.empty() ? "" : "." + full);
                    const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(cur->GetPrefixexp());
                    root = (pe && pe->GetPrefixKind() == PrefixExpKind::kVar)
                               ? std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue())
                               : nullptr;
                    cur = root;
                }
                if (root && root->GetVarKind() == VarKind::kSimple && !full.empty()) {
                    const std::string full_name = root->GetName() + "." + full;
                    bool root_declared = file_level_names.contains(root->GetName());
                    for (auto it = scopes.rbegin(); it != scopes.rend() && !root_declared; ++it) {
                        root_declared = it->contains(root->GetName());
                    }
                    if (!root_declared && s_->GetVM().FindNativeFunction(full_name) != nullptr) {
                        ThrowError(std::format("native function '{}' cannot be used as a value; fakelua dispatches native "
                                               "calls by name at compile time, call it directly or wrap it in a Lua function",
                                               full_name),
                                   node);
                    }
                }
            }
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


namespace {

// 语义分析内部的寄存器高峰检查：只按与 InterpCodegen 相同的
// AllocLocal / AllocReg / FreeTo / EnsureStack 寿命规则计量并发槽，
// 不生成字节码、不创建 FuncProto / InterpUnit、不 intern 常量串、不访问 State。
// 槽位决策（含预处理插入的 __fakelua_tmp_* / __fakelua_lhs_* 与表达式临时）
// 必须与解释器后端保持一致；本类不是字节码生成器，也不对外暴露。
class RegisterPeakChecker {
public:
    void Check(const ParseResult &pr, const AnalysisResult &ar) {
        Walk(pr, ar);
    }

private:
    struct FuncInfo;
    struct VarDef;

    struct Scope {
        FuncInfo *func = nullptr;
        std::unordered_map<std::string, VarDef *> vars;
    };

    struct VarDef {
        std::string name;
        const SyntaxTreeInterface *def_node = nullptr;
        FuncInfo *defining_func = nullptr;
        bool is_captured = false;
        int reg = -1;
    };

    struct FuncInfo {
        const SyntaxTreeInterface *node = nullptr;
        SyntaxTreeInterfacePtr funcbody;
        FuncInfo *parent = nullptr;
        std::string name;
        std::string unique_c_name;
        std::vector<std::string> params;
        bool is_vararg = false;
        std::vector<VarDef *> captured_vars;
        std::unordered_set<VarDef *> captured_set;
        bool compiled = false;
    };

    struct PairHash {
        template<class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &p) const {
            return std::hash<T1>{}(p.first) ^ (std::hash<T2>{}(p.second) << 1);
        }
    };

    struct LoopInfo {
        enum Kind { kWhile, kRepeat, kFor, kForIn } kind = kWhile;
        int continue_ip = -1;
        std::vector<int> break_jmps;
        std::vector<int> continue_jmps;
    };

    void ResolveScopes(const SyntaxTreeInterfacePtr &node, std::vector<Scope> &scopes, std::vector<FuncInfo *> &func_stack, FuncInfo *cur_func);
    std::string CompileFuncName(const SyntaxTreeInterfacePtr &ptr);

    void CompileFunction(FuncInfo *func);
    void CompileStmtBlock(const SyntaxTreeInterfacePtr &block);
    void CompileStmt(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtIf(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLocalFunction(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtBreak();
    void CompileStmtContinue();
    void CompileStmtGoto(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLabel(const SyntaxTreeInterfacePtr &stmt);

    int CompileExp(const SyntaxTreeInterfacePtr &exp, bool preserve_multi = false, int dest = -1);
    int CompilePrefixexp(const SyntaxTreeInterfacePtr &pe, bool preserve_multi, int dest = -1);
    int CompileVar(const SyntaxTreeInterfacePtr &v);
    int CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall, bool preserve_multi);
    int CompileTableconstructor(const SyntaxTreeInterfacePtr &tc);
    int CompileBinop(const SyntaxTreeInterfacePtr &exp, int dest = -1);
    int CompileUnop(const SyntaxTreeInterfacePtr &exp, int dest = -1);

    int EmitClosure(FuncInfo *child);
    void EmitCallArgs(const std::shared_ptr<class SyntaxTreeArgs> &args_ptr, std::vector<int> &arg_regs, bool &last_expand);

    void Walk(const ParseResult &pr, const AnalysisResult &ar);
    int CodeSize() const;
    int64_t StringId(const std::string &s);

    int AllocReg();
    int AllocLocal();
    void FreeTo(int top);
    void EnsureStack(int n);
    int AddConst(const CVar &v);
    int AddStringConst(const std::string &s);
    int Emit(Op op, int a = 0, int b = 0, int c = 0, int sbx = 0);
    void PatchSbx(int ip, int dest);

    int LoadConstTo(int dest, const CVar &v);
    int MoveTo(int dest, int src);
    int CommitDest(int dest, int src);
    int LocalAssignDest(const SyntaxTreeVar *var);
    void StoreLocal(int dest, int src);
    int LoadLocal(VarDef *def);
    int ResolveSimpleName(const SyntaxTreeVar *var, const std::string &name, bool as_callee);
    void AssignToVar(const SyntaxTreeVar *var, int src);
    int EmitThrow(const std::string &msg);
    bool LastPreservesMulti(const SyntaxTreeInterfacePtr &exp) const;
    bool IsPackageHeaderStmt(const SyntaxTreeInterfacePtr &stmt) const;

    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    enum class PairsIpairsKind { kNone, kPairs, kIpairs };
    [[nodiscard]] PairsIpairsKind TryMatchPairsIpairs(const std::shared_ptr<class SyntaxTreeExplist> &explist_ptr, SyntaxTreeInterfacePtr &out_tbl_arg);

    static const std::unordered_set<std::string> kStringLibraryMethods;

    int check_ip_ = 0;
    std::string file_name_;
    const AnalysisResult *ar_ = nullptr;

    std::vector<std::unique_ptr<VarDef>> all_defs_;
    std::vector<std::unique_ptr<FuncInfo>> all_funcs_;
    std::unordered_map<const SyntaxTreeInterface *, FuncInfo *> func_map_;
    std::unordered_map<std::pair<const SyntaxTreeInterface *, std::string>, VarDef *, PairHash> stmt_var_to_def_;
    std::unordered_map<const SyntaxTreeInterface *, VarDef *> var_to_def_map_;
    std::unordered_map<std::string, std::pair<int, bool>> file_func_sigs_;
    std::string cur_package_name_;
    std::unordered_set<std::string> file_level_names_;

    FuncInfo *cur_func_ = nullptr;
    int cur_line_ = 0;
    int stack_top_ = 0;
    int local_top_ = 0;
    std::vector<LoopInfo> loops_;

    struct LabelScope {
        std::unordered_map<std::string, int> defined;
        std::unordered_map<std::string, std::vector<int>> pending;
    };
    std::vector<LabelScope> label_scopes_;
};



namespace {

// 把「当前行」临时切到节点的源码行，离开作用域还原：嵌套表达式编译完后，
// 外层语句后续的 Emit 仍要盖外层自己的行号。
class LineScope {
public:
    LineScope(int &cur, const SyntaxTreeInterface *node) : cur_(cur), saved_(cur) {
        if (node) {
            cur_ = static_cast<int>(node->Loc().begin.line);
        }
    }
    ~LineScope() {
        cur_ = saved_;
    }
    LineScope(const LineScope &) = delete;
    LineScope &operator=(const LineScope &) = delete;

private:
    int &cur_;
    int saved_;
};

}// namespace

const std::unordered_set<std::string> RegisterPeakChecker::kStringLibraryMethods = {"len", "sub", "rep", "reverse", "lower", "upper", "byte", "char", "format", "dump", "find", "match", "gmatch", "gsub"};

[[noreturn]] void RegisterPeakChecker::ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) {
    ThrowFakeluaException(std::format("Code generate failed, {} at {}", msg, SyntaxTreeLocationStr(file_name_, ptr)));
}

int RegisterPeakChecker::AllocReg() {
    if (stack_top_ < local_top_) {
        stack_top_ = local_top_;
    }
    const int r = stack_top_++;
    if (r > kMaxFunctionRegisterIndex) {
        ThrowFakeluaException("function uses too many registers");
    }
    return r;
}

int RegisterPeakChecker::AllocLocal() {
    // 循环控制槽是 AllocReg 临时量，循环体里的 AllocLocal 不能踩上去。
    if (local_top_ < stack_top_) {
        local_top_ = stack_top_;
    }
    const int r = local_top_++;
    stack_top_ = local_top_;
    if (r > kMaxFunctionRegisterIndex) {
        ThrowFakeluaException("function uses too many registers");
    }
    return r;
}

void RegisterPeakChecker::FreeTo(int top) {
    if (top < local_top_) {
        top = local_top_;
    }
    stack_top_ = top;
}

void RegisterPeakChecker::EnsureStack(int n) {
    if (stack_top_ < n) {
        stack_top_ = n;
    }
}

int RegisterPeakChecker::CodeSize() const {
    return check_ip_;
}

int64_t RegisterPeakChecker::StringId(const std::string &s) {
    (void)s;
    return 0;
}

int RegisterPeakChecker::AddConst(const CVar &v) {
    (void)v;
    return 0;
}

int RegisterPeakChecker::AddStringConst(const std::string &s) {
    (void)s;
    return 0;
}

int RegisterPeakChecker::Emit(Op op, int a, int b, int c, int sbx) {
    (void)op;
    (void)sbx;
    if (a < 0 || a > kMaxFunctionRegisterIndex || b < 0 || b > kMaxFunctionRegisterIndex || c < 0 || c > kMaxFunctionRegisterIndex) {
        ThrowFakeluaException("interpreter: instruction operand out of range");
    }
    return check_ip_++;
}

void RegisterPeakChecker::PatchSbx(int ip, int dest) {
    (void)ip;
    (void)dest;
}


int RegisterPeakChecker::LoadConstTo(int dest, const CVar &v) {
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

int RegisterPeakChecker::MoveTo(int dest, int src) {
    if (dest != src) {
        Emit(Op::MOVE, dest, src);
    }
    return dest;
}

int RegisterPeakChecker::CommitDest(int dest, int src) {
    return dest >= 0 ? MoveTo(dest, src) : src;
}

int RegisterPeakChecker::LocalAssignDest(const SyntaxTreeVar *var) {
    if (!var || var->GetVarKind() != VarKind::kSimple) {
        return -1;
    }
    const auto it = var_to_def_map_.find(var);
    if (it == var_to_def_map_.end()) {
        return -1;
    }
    VarDef *def = it->second;
    if (def->defining_func == cur_func_ && def->reg >= 0) {
        return def->reg;
    }
    return -1;
}

void RegisterPeakChecker::StoreLocal(int dest, int src) {
    MoveTo(dest, src);
}

int RegisterPeakChecker::LoadLocal(VarDef *def) {
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

int RegisterPeakChecker::EmitThrow(const std::string &msg) {
    return Emit(Op::THROW, 0, 0, 0, AddStringConst(msg));
}

bool RegisterPeakChecker::LastPreservesMulti(const SyntaxTreeInterfacePtr &exp) const {
    if (!ar_) return false;
    if (IsVarargExp(exp)) return true;
    if (!ar_->function_call_exps.contains(exp.get())) return false;
    const std::string callee = ar_->callee_names.contains(exp.get()) ? ar_->callee_names.at(exp.get()) : "";
    if (!callee.empty() && ar_->function_max_returns.contains(callee) && ar_->function_max_returns.at(callee) == 1) {
        return false;
    }
    return true;
}

bool RegisterPeakChecker::IsPackageHeaderStmt(const SyntaxTreeInterfacePtr &stmt) const {
    if (cur_package_name_.empty()) return false;
    std::string ignored;
    return ExtractPackageName(stmt, ignored);
}

int RegisterPeakChecker::ResolveSimpleName(const SyntaxTreeVar *var, const std::string &name, bool as_callee) {
    (void)as_callee;
    if (const auto it = var_to_def_map_.find(var); it != var_to_def_map_.end()) {
        VarDef *def = it->second;
        if (def->defining_func == cur_func_ || def->defining_func != nullptr) {
            return LoadLocal(def);
        }
        // 文件级 local function：没有函数内局部槽，按注册原型构造闭包（对齐 CGen FlMakeClosure）。
    }
    if (name == "_VERSION") {
        const int r = AllocReg();
        CVar v{};
        v.type_ = static_cast<int>(VarType::StringId);
        v.data_.i = StringId(std::string("Fakelua ") + FAKELUA_VERSION_STRING);
        LoadConstTo(r, v);
        return r;
    }
    const int r = AllocReg();
    Emit(Op::GETGLOBAL, r, 0, 0, AddStringConst(name));
    return r;
}

void RegisterPeakChecker::AssignToVar(const SyntaxTreeVar *var, int src) {
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

std::string RegisterPeakChecker::CompileFuncName(const SyntaxTreeInterfacePtr &ptr) {
    const auto name = std::dynamic_pointer_cast<SyntaxTreeFuncname>(ptr);
    const auto funcnamelistptr = name->FuncNameList();
    const auto funcnamelist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcnamelistptr);
    const auto &namelist = funcnamelist->Funcnames();
    return namelist[0];
}

void RegisterPeakChecker::ResolveScopes(const SyntaxTreeInterfacePtr &node, std::vector<Scope> &scopes, std::vector<FuncInfo *> &func_stack, FuncInfo *cur_func) {
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

int RegisterPeakChecker::EmitClosure(FuncInfo *child) {
    CompileFunction(child);
    const int r = AllocReg();
    Emit(Op::CLOSURE, r);
    return r;
}

void RegisterPeakChecker::CompileFunction(FuncInfo *func) {
    if (!func || func->compiled) {
        return;
    }
    const LineScope func_line(cur_line_, func->node);
    FuncInfo *prev_func = cur_func_;
    const int prev_stack = stack_top_;
    const int prev_local = local_top_;
    auto prev_loops = std::move(loops_);
    auto prev_label_scopes = std::move(label_scopes_);

    func->compiled = true;
    const int prev_check_ip = check_ip_;
    check_ip_ = 0;

    cur_func_ = func;
    stack_top_ = static_cast<int>(func->params.size());
    local_top_ = stack_top_;
    loops_.clear();
    label_scopes_.clear();

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

    check_ip_ = prev_check_ip;
    cur_func_ = prev_func;
    stack_top_ = prev_stack;
    local_top_ = prev_local;
    loops_ = std::move(prev_loops);
    label_scopes_ = std::move(prev_label_scopes);
}

void RegisterPeakChecker::CompileStmtBlock(const SyntaxTreeInterfacePtr &block) {
    if (!block) return;
    const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);
    label_scopes_.emplace_back();
    for (const auto &stmt: block_ptr->Stmts()) {
        CompileStmt(stmt);
    }
    LabelScope &sc = label_scopes_.back();
    if (label_scopes_.size() == 1) {
        for (auto &[label, ips]: sc.pending) {
            const auto it = sc.defined.find(label);
            if (it == sc.defined.end()) {
                ThrowError("no visible label '" + label + "' for goto", block);
            }
            for (int ip: ips) {
                PatchSbx(ip, it->second);
            }
        }
    } else {
        LabelScope &parent = label_scopes_[label_scopes_.size() - 2];
        for (auto &[label, ips]: sc.pending) {
            bool resolved = false;
            for (int i = static_cast<int>(label_scopes_.size()) - 2; i >= 0; --i) {
                const auto it = label_scopes_[static_cast<size_t>(i)].defined.find(label);
                if (it != label_scopes_[static_cast<size_t>(i)].defined.end()) {
                    for (int ip: ips) {
                        PatchSbx(ip, it->second);
                    }
                    resolved = true;
                    break;
                }
            }
            if (!resolved) {
                auto &dst = parent.pending[label];
                dst.insert(dst.end(), ips.begin(), ips.end());
            }
        }
    }
    label_scopes_.pop_back();
}

void RegisterPeakChecker::CompileStmt(const SyntaxTreeInterfacePtr &stmt) {
    if (!stmt) return;
    if (IsPackageHeaderStmt(stmt)) return;
    const LineScope line(cur_line_, stmt.get());
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

void RegisterPeakChecker::CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt) {
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
    std::vector<int> regs;
    for (size_t i = 0; i < exps.size(); ++i) {
        const bool pres = (i + 1 == exps.size()) && last_multi;
        regs.push_back(CompileExp(exps[i], pres));
    }
    // 必须在 CompileExp 之后再取 base：表达式临时槽从当时的 stack_top_ 往上长，
    // 若先占 [base, base+n) 再 MoveTo，会把已经算好的字面量/临时量冲掉
    // （return f, t, 0 在 t 是参数时会变成 f, t, t）。
    const int base = stack_top_;
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

void RegisterPeakChecker::CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt) {
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
        const int dest = (defs[i]->defining_func == nullptr) ? -1 : defs[i]->reg;
        if (i < exps.size()) {
            r = CompileExp(exps[i], false, dest);
        } else {
            r = dest >= 0 ? dest : AllocReg();
            Emit(Op::LOADNIL, r);
        }
        if (defs[i]->defining_func == nullptr) {
            Emit(Op::SETGLOBAL, r, 0, 0, AddStringConst(names[i]));
        } else if (r != dest) {
            StoreLocal(defs[i]->reg, r);
        }
    }
    for (size_t i = names.size(); i < exps.size(); ++i) {
        CompileExp(exps[i], false);
    }
}

void RegisterPeakChecker::CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt) {
    const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
    const auto varlist_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
    const auto &vars = varlist_ptr->Vars();
    const auto &exps = explist_ptr->Exps();
    const auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(vars[0]);
    const int dest = LocalAssignDest(v_ptr.get());
    const int r = CompileExp(exps[0], false, dest);
    if (dest < 0) {
        AssignToVar(v_ptr.get(), r);
    } else if (r != dest) {
        StoreLocal(dest, r);
    }
}

void RegisterPeakChecker::CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt) {
    const auto while_stmt = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);
    LoopInfo loop;
    loop.kind = LoopInfo::kWhile;
    loop.continue_ip = CodeSize();
    const int saved = stack_top_;
    const int cond = CompileExp(while_stmt->Exp(), false);
    const int jmpf = Emit(Op::TESTJMP, cond, 0);
    FreeTo(saved);
    loops_.push_back(std::move(loop));
    CompileStmtBlock(while_stmt->Block());
    Emit(Op::JMP, 0, 0, 0, loops_.back().continue_ip - CodeSize() - 1);
    const int end = CodeSize();
    PatchSbx(jmpf, end);
    for (int ip: loops_.back().break_jmps) {
        PatchSbx(ip, end);
    }
    loops_.pop_back();
}

void RegisterPeakChecker::CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt) {
    const auto repeat_stmt = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);
    LoopInfo loop;
    loop.kind = LoopInfo::kRepeat;
    const int start = CodeSize();
    loops_.push_back(std::move(loop));
    CompileStmtBlock(repeat_stmt->Block());
    loops_.back().continue_ip = CodeSize();
    const int saved = stack_top_;
    const int cond = CompileExp(repeat_stmt->Exp(), false);
    const int jmp_true = Emit(Op::TESTJMP, cond, 1);
    Emit(Op::JMP, 0, 0, 0, start - CodeSize() - 1);
    const int end = CodeSize();
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

void RegisterPeakChecker::CompileStmtIf(const SyntaxTreeInterfacePtr &stmt) {
    const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
    std::vector<int> end_jmps;
    auto emit_branch = [&](const SyntaxTreeInterfacePtr &cond_exp, const SyntaxTreeInterfacePtr &block) {
        const int saved = stack_top_;
        const int cond = CompileExp(cond_exp, false);
        const int jmpf = Emit(Op::TESTJMP, cond, 0);
        FreeTo(saved);
        CompileStmtBlock(block);
        end_jmps.push_back(Emit(Op::JMP));
        PatchSbx(jmpf, CodeSize());
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
    const int end = CodeSize();
    for (int ip: end_jmps) {
        PatchSbx(ip, end);
    }
}

void RegisterPeakChecker::CompileStmtBreak() {
    if (loops_.empty()) {
        ThrowFakeluaException("break not inside loop");
    }
    loops_.back().break_jmps.push_back(Emit(Op::JMP));
}

void RegisterPeakChecker::CompileStmtContinue() {
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

void RegisterPeakChecker::CompileStmtGoto(const SyntaxTreeInterfacePtr &stmt) {
    const auto goto_stmt = std::dynamic_pointer_cast<SyntaxTreeGoto>(stmt);
    const auto &label = goto_stmt->GetLabel();
    const int ip = Emit(Op::JMP);
    for (int i = static_cast<int>(label_scopes_.size()) - 1; i >= 0; --i) {
        const auto it = label_scopes_[static_cast<size_t>(i)].defined.find(label);
        if (it != label_scopes_[static_cast<size_t>(i)].defined.end()) {
            PatchSbx(ip, it->second);
            return;
        }
    }
    if (label_scopes_.empty()) {
        ThrowError("no visible label '" + label + "' for goto", stmt);
    }
    label_scopes_.back().pending[label].push_back(ip);
}

void RegisterPeakChecker::CompileStmtLabel(const SyntaxTreeInterfacePtr &stmt) {
    const auto label_stmt = std::dynamic_pointer_cast<SyntaxTreeLabel>(stmt);
    const auto &name = label_stmt->GetName();
    if (label_scopes_.empty()) {
        ThrowError("label '" + name + "' outside a block", stmt);
    }
    LabelScope &sc = label_scopes_.back();
    if (sc.defined.contains(name)) {
        ThrowError("label '" + name + "' already defined", stmt);
    }
    const int ip = CodeSize();
    sc.defined[name] = ip;
    if (const auto it = sc.pending.find(name); it != sc.pending.end()) {
        for (int jmp: it->second) {
            PatchSbx(jmp, ip);
        }
        sc.pending.erase(it);
    }
}

void RegisterPeakChecker::CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt) {
    const auto for_stmt = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
    const int saved = stack_top_;
    const int ctrl = AllocReg();
    const int endv = AllocReg();
    const int step = AllocReg();
    VarDef *ldef = stmt_var_to_def_.at({stmt.get(), for_stmt->Name()});
    if (ldef->reg < 0) {
        ldef->reg = AllocLocal();
    }
    // FORPREP/FORLOOP 约定 R(A+3) 就是用户可见的循环变量；先占槽再编译初值，
    // 避免 begin/end/step 的临时寄存器插到 ctrl 和 i 中间。
    if (ldef->reg != ctrl + 3) {
        ThrowError("numeric for: loop variable is not R(A+3)", stmt);
    }
    CompileExp(for_stmt->ExpBegin(), false, ctrl);
    CompileExp(for_stmt->ExpEnd(), false, endv);
    if (for_stmt->ExpStep()) {
        CompileExp(for_stmt->ExpStep(), false, step);
    } else {
        LoadConstTo(step, interp_rt::Int(1));
    }

    LoopInfo loop;
    loop.kind = LoopInfo::kFor;
    const int prep = Emit(Op::FORPREP, ctrl);
    const int loop_head = CodeSize();
    // 先换新 box。FORLOOP 写 R(A+3) 时会清 box 指针，避免写穿上一轮闭包。
    if (ldef->is_captured) {
        Emit(Op::NEWBOX, ldef->reg);
    }

    loop.continue_ip = -1;
    loops_.push_back(std::move(loop));
    CompileStmtBlock(for_stmt->Block());
    loops_.back().continue_ip = CodeSize();
    for (int ip: loops_.back().continue_jmps) {
        PatchSbx(ip, loops_.back().continue_ip);
    }
    const int loop_op = Emit(Op::FORLOOP, ctrl);
    PatchSbx(loop_op, loop_head);
    const int end = CodeSize();
    PatchSbx(prep, end);
    for (int ip: loops_.back().break_jmps) {
        PatchSbx(ip, end);
    }
    loops_.pop_back();
    FreeTo(saved);
}

RegisterPeakChecker::PairsIpairsKind RegisterPeakChecker::TryMatchPairsIpairs(const std::shared_ptr<SyntaxTreeExplist> &explist_ptr, SyntaxTreeInterfacePtr &out_tbl_arg) {
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

void RegisterPeakChecker::CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt) {
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
    auto bind_loop_vars = [&](int kreg, int vreg) {
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
    };
    if (kind == PairsIpairsKind::kIpairs) {
        const int tbl = CompileExp(tbl_exp_node, false);
        const int idx = AllocReg();
        LoadConstTo(idx, interp_rt::Int(1));
        LoopInfo loop;
        loop.kind = LoopInfo::kForIn;
        const int head = CodeSize();
        loop.continue_ip = -1;
        const int vreg = AllocReg();
        Emit(Op::GETTABLE, vreg, tbl, idx);
        const int nlit = AllocReg();
        Emit(Op::LOADNIL, nlit);
        const int vnil = AllocReg();
        Emit(Op::EQ, vnil, vreg, nlit);
        const int jmp_exit = Emit(Op::TESTJMP, vnil, 1);
        bind_loop_vars(idx, vreg);
        loops_.push_back(std::move(loop));
        CompileStmtBlock(for_in->Block());
        const int cont = CodeSize();
        loops_.back().continue_ip = cont;
        for (int ip: loops_.back().continue_jmps) {
            PatchSbx(ip, cont);
        }
        const int one = AllocReg();
        LoadConstTo(one, interp_rt::Int(1));
        Emit(Op::ADD, idx, idx, one);
        Emit(Op::JMP, 0, 0, 0, head - CodeSize() - 1);
        const int end = CodeSize();
        PatchSbx(jmp_exit, end);
        for (int ip: loops_.back().break_jmps) {
            PatchSbx(ip, end);
        }
        loops_.pop_back();
        return;
    }
    if (kind == PairsIpairsKind::kPairs) {
        const int tbl = CompileExp(tbl_exp_node, false);
        const int sz = AllocReg();
        Emit(Op::TABCOUNT, sz, tbl);
        const int idx = AllocReg();
        LoadConstTo(idx, interp_rt::Int(0));
        LoopInfo loop;
        loop.kind = LoopInfo::kForIn;
        const int head = CodeSize();
        loop.continue_ip = -1;
        const int cmp = AllocReg();
        Emit(Op::LT, cmp, idx, sz);
        const int jmp_exit = Emit(Op::TESTJMP, cmp, 0);
        const int kreg = AllocReg();
        const int vreg = AllocReg();
        Emit(Op::TABENT, tbl, kreg, vreg, 0);
        // TABENT 的 sbx 在解释器后端回填为 idx；校验路径不写指令流。
        const int vnil = AllocReg();
        const int nlit = AllocReg();
        Emit(Op::LOADNIL, nlit);
        Emit(Op::EQ, vnil, vreg, nlit);
        const int skip_body = Emit(Op::TESTJMP, vnil, 1);
        bind_loop_vars(kreg, vreg);
        loops_.push_back(std::move(loop));
        CompileStmtBlock(for_in->Block());
        const int cont = CodeSize();
        loops_.back().continue_ip = cont;
        for (int ip: loops_.back().continue_jmps) {
            PatchSbx(ip, cont);
        }
        PatchSbx(skip_body, cont);
        const int one = AllocReg();
        LoadConstTo(one, interp_rt::Int(1));
        Emit(Op::ADD, idx, idx, one);
        Emit(Op::JMP, 0, 0, 0, head - CodeSize() - 1);
        const int end = CodeSize();
        PatchSbx(jmp_exit, end);
        for (int ip: loops_.back().break_jmps) {
            PatchSbx(ip, end);
        }
        loops_.pop_back();
        return;
    }

    const auto &exps = explist_ptr->Exps();
    // 迭代器三元组必须是 local：CALL 的结果槽是 AllocReg 临时量，
    // 若 f/s/var 也只是 temp，stack_top_ 回落后会和 CALL 帧重叠，
    // 第一次迭代就把函数冲掉，或 MoveTo 把 state/ctrl 搅乱。
    const int iter_f = AllocLocal();
    const int iter_s = AllocLocal();
    const int iter_var = AllocLocal();
    if (exps.size() == 1) {
        const int e0 = CompileExp(exps[0], true);
        // Always unbox 0,1,2 — UnboxMulti on non-multi returns the value for idx 0 and nil else.
        Emit(Op::UNBOX, iter_f, e0, 0);
        Emit(Op::UNBOX, iter_s, e0, 1);
        Emit(Op::UNBOX, iter_var, e0, 2);
    } else if (exps.size() == 2) {
        MoveTo(iter_f, CompileExp(exps[0], false));
        const int e1 = CompileExp(exps[1], true);
        Emit(Op::UNBOX, iter_f, iter_f, 0);
        Emit(Op::UNBOX, iter_s, e1, 0);
        Emit(Op::UNBOX, iter_var, e1, 1);
    } else {
        MoveTo(iter_f, CompileExp(exps[0], false));
        MoveTo(iter_s, CompileExp(exps[1], false));
        const int e2 = CompileExp(exps[2], exps.size() == 3);
        Emit(Op::UNBOX, iter_f, iter_f, 0);
        Emit(Op::UNBOX, iter_s, iter_s, 0);
        Emit(Op::UNBOX, iter_var, e2, 0);
        for (size_t i = 3; i < exps.size(); ++i) {
            (void)CompileExp(exps[i], i + 1 == exps.size());
        }
    }

    LoopInfo loop;
    loop.kind = LoopInfo::kForIn;
    const int head = CodeSize();
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
    const int cont = CodeSize();
    loops_.back().continue_ip = cont;
    for (int ip: loops_.back().continue_jmps) {
        PatchSbx(ip, cont);
    }
    Emit(Op::JMP, 0, 0, 0, head - CodeSize() - 1);
    const int end = CodeSize();
    PatchSbx(jmp_exit, end);
    for (int ip: loops_.back().break_jmps) {
        PatchSbx(ip, end);
    }
    loops_.pop_back();
}

void RegisterPeakChecker::CompileStmtLocalFunction(const SyntaxTreeInterfacePtr &stmt) {
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

int RegisterPeakChecker::CompileExp(const SyntaxTreeInterfacePtr &exp, bool preserve_multi, int dest) {
    const LineScope line(cur_line_, exp.get());
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    if (e->GetExpKind() == ExpKind::kNumber || e->GetExpKind() == ExpKind::kUnop) {
        TableKeyKind kind = TableKeyKind::kInt;
        std::string canonical;
        int64_t int_value = 0;
        double float_value = 0;
        if (ClassifyConstNumberExp(exp, kind, canonical, int_value, float_value)) {
            const int r = dest >= 0 ? dest : AllocReg();
            if (ConstNumberExpIsIntValue(kind, exp)) {
                LoadConstTo(r, interp_rt::Int(int_value));
            } else {
                LoadConstTo(r, interp_rt::Float(ConstNumberExpToDouble(kind, int_value, float_value, exp)));
            }
            return r;
        }
    }
    switch (e->GetExpKind()) {
        case ExpKind::kNil: {
            const int r = dest >= 0 ? dest : AllocReg();
            Emit(Op::LOADNIL, r);
            return r;
        }
        case ExpKind::kFalse: {
            const int r = dest >= 0 ? dest : AllocReg();
            Emit(Op::LOADBOOL, r, 0);
            return r;
        }
        case ExpKind::kTrue: {
            const int r = dest >= 0 ? dest : AllocReg();
            Emit(Op::LOADBOOL, r, 1);
            return r;
        }
        case ExpKind::kNumber: {
            const int r = dest >= 0 ? dest : AllocReg();
            if (IsInteger(e->ExpValue())) {
                LoadConstTo(r, interp_rt::Int(ToInteger(e->ExpValue())));
            } else {
                LoadConstTo(r, interp_rt::Float(ToFloat(e->ExpValue())));
            }
            return r;
        }
        case ExpKind::kString: {
            const int r = dest >= 0 ? dest : AllocReg();
            CVar v{};
            v.type_ = static_cast<int>(VarType::StringId);
            v.data_.i = StringId(e->ExpValue());
            LoadConstTo(r, v);
            return r;
        }
        case ExpKind::kPrefixExp:
            return CompilePrefixexp(e->Right(), preserve_multi, dest);
        case ExpKind::kTableConstructor:
            return CommitDest(dest, CompileTableconstructor(e->Right()));
        case ExpKind::kFunctionDef: {
            const auto func_def = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(e->Right());
            return CommitDest(dest, EmitClosure(func_map_[func_def.get()]));
        }
        case ExpKind::kBinop:
            return CompileBinop(exp, dest);
        case ExpKind::kUnop:
            return CompileUnop(exp, dest);
        default:
            ThrowError("unsupported expression kind", e);
    }
}

int RegisterPeakChecker::CompilePrefixexp(const SyntaxTreeInterfacePtr &pe, bool preserve_multi, int dest) {
    const auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe);
    if (pe_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
        const int r = CompileVar(pe_ptr->GetValue());
        const auto var_node = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_ptr->GetValue());
        if (!preserve_multi && var_node && var_node->GetVarKind() == VarKind::kSimple && var_node->GetName().rfind("__fakelua_vararg_", 0) == 0) {
            const int o = dest >= 0 ? dest : AllocReg();
            Emit(Op::UNBOX, o, r, 0);
            return o;
        }
        return CommitDest(dest, r);
    }
    if (pe_ptr->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
        return CommitDest(dest, CompileFunctioncall(pe_ptr->GetValue(), preserve_multi));
    }
    return CompileExp(pe_ptr->GetValue(), preserve_multi, dest);
}

int RegisterPeakChecker::CompileVar(const SyntaxTreeInterfacePtr &v) {
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
                    // Lua 5.4：[\0-\255]，匹配任意单字节。
                    static const std::string kCharPattern = [] {
                        std::string p = "[";
                        p.push_back('\0');
                        p.push_back('-');
                        p.push_back(static_cast<char>(0xFF));
                        p.push_back(']');
                        return p;
                    }();
                    const int r = AllocReg();
                    CVar s{};
                    s.type_ = static_cast<int>(VarType::StringId);
                    s.data_.i = StringId(kCharPattern);
                    LoadConstTo(r, s);
                    return r;
                }
                if (base_var->GetName() == "utf8" && name == "charpattern") {
                    // Lua 5.4 utf8.charpattern：由原始字节构成的 Lua 模式
                    // [\0-\x7F\xC2-\xF4][\x80-\xBF]*（非字面 \xHH）。
                    static const std::string kUtf8Pattern = [] {
                        std::string p = "[";
                        p.push_back('\0');
                        p.push_back('-');
                        p.push_back(static_cast<char>(0x7F));
                        p.push_back(static_cast<char>(0xC2));
                        p.push_back('-');
                        p.push_back(static_cast<char>(0xF4));
                        p.push_back(']');
                        p.push_back('[');
                        p.push_back(static_cast<char>(0x80));
                        p.push_back('-');
                        p.push_back(static_cast<char>(0xBF));
                        p.push_back(']');
                        p.push_back('*');
                        return p;
                    }();
                    const int r = AllocReg();
                    CVar s{};
                    s.type_ = static_cast<int>(VarType::StringId);
                    s.data_.i = StringId(kUtf8Pattern);
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
    key.data_.i = StringId(name);
    LoadConstTo(k, key);
    const int r = AllocReg();
    Emit(Op::GETTABLE, r, t, k);
    return r;
}

void RegisterPeakChecker::EmitCallArgs(const std::shared_ptr<SyntaxTreeArgs> &args_ptr, std::vector<int> &arg_regs, bool &last_expand) {
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

int RegisterPeakChecker::CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall, bool preserve_multi) {
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(functioncall);
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
    const auto pe_pre_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
    const int unbox = preserve_multi ? 0 : 1;

    std::vector<int> arg_regs;
    bool last_expand = false;
    auto place_callname = [&](const std::string &name, const std::vector<int> &args) {
        const int n = static_cast<int>(args.size());
        // 末尾是 f() / ... 这类展开时实参个数运行期才知道，检查不了。
        if (!last_expand) {
            if (const auto it = file_func_sigs_.find(name); it != file_func_sigs_.end()) {
                const auto &[expected, is_vararg] = it->second;
                if (!is_vararg && expected != n) {
                    ThrowError(std::format("wrong number of arguments to '{}': expected {}, got {}", name, expected, n), functioncall);
                }
            }
        }
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

    int method_obj = -1;
    if (!fc->Name().empty()) {
        if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
            method_obj = CompileVar(pe_pre_ptr->GetValue());
        } else {
            method_obj = CompilePrefixexp(fc->prefixexp(), false);
        }
    }

    EmitCallArgs(args_ptr, arg_regs, last_expand);

    if (!fc->Name().empty()) {
        std::vector<int> final_args;
        final_args.push_back(method_obj);
        for (int a: arg_regs) final_args.push_back(a);
        if (kStringLibraryMethods.contains(fc->Name())) {
            return place_callname("string." + fc->Name(), final_args);
        }
        const int k = AllocReg();
        CVar key{};
        key.type_ = static_cast<int>(VarType::StringId);
        key.data_.i = StringId(fc->Name());
        LoadConstTo(k, key);
        const int method = AllocReg();
        Emit(Op::GETTABLE, method, method_obj, k);
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
                // 只有文件级 local function 才能走 CALLNAME：named_protos_ 只登记
                // 函数定义。文件级 local 变量（如 local gv = f）即使 defining_func
                // 为 nullptr 也不在 named_protos_ 里，必须按闭包调用，否则运行时报
                // "function 'gv' not found"（C 后端此处按闭包处理，行为一致）。
                file_level_func = it->second->defining_func == nullptr && it->second->def_node != nullptr &&
                                  it->second->def_node->Type() == SyntaxTreeType::LocalFunction;
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

    // 形参、局部变量、嵌套 local function 即使与文件级函数同名，也必须调用这个绑定。
    // named_protos_ 只登记文件级函数，不能拿来覆盖局部值。
    if (!func_name.empty() && (!is_local_callee || file_level_func)) {
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

int RegisterPeakChecker::CompileTableconstructor(const SyntaxTreeInterfacePtr &tc) {
    const auto tc_ptr = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(tc);
    const int tbl = AllocReg();
    Emit(Op::NEWTABLE, tbl);
    if (const auto fieldlist = tc_ptr->Fieldlist()) {
        const auto fieldlist_ptr = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(fieldlist);
        std::shared_ptr<SyntaxTreeField> last_array_field;
        for (const auto &field: fieldlist_ptr->Fields()) {
            const auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(field);
            if (field_ptr->GetFieldKind() == FieldKind::kArray && !field_ptr->Key()) {
                last_array_field = field_ptr;
            }
        }
        int array_idx = 1;
        for (const auto &field: fieldlist_ptr->Fields()) {
            const auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(field);
            const bool is_expand = (field_ptr == last_array_field) && LastPreservesMulti(field_ptr->Value());
            const int saved = stack_top_;
            const int val = CompileExp(field_ptr->Value(), is_expand);
            if (field_ptr->GetFieldKind() == FieldKind::kObject) {
                const int k = AllocReg();
                CVar key{};
                key.type_ = static_cast<int>(VarType::StringId);
                key.data_.i = StringId(field_ptr->Name());
                LoadConstTo(k, key);
                Emit(Op::SETTABLE, tbl, k, val);
            } else if (const auto key = field_ptr->Key()) {
                const int k = CompileExp(key, false);
                Emit(Op::SETTABLE, tbl, k, val);
            } else if (is_expand) {
                Emit(Op::SETLIST, tbl, 0, val, array_idx);
            } else {
                const int k = AllocReg();
                LoadConstTo(k, interp_rt::Int(array_idx));
                Emit(Op::SETTABLE, tbl, k, val);
                ++array_idx;
            }
            FreeTo(saved);
        }
    }
    return tbl;
}

int RegisterPeakChecker::CompileBinop(const SyntaxTreeInterfacePtr &exp, int dest) {
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
    const auto op_kind = op_ptr->GetOpKind();
    if (op_kind == BinOpKind::kAnd || op_kind == BinOpKind::kOr) {
        const int result = dest >= 0 ? dest : AllocReg();
        const int left = CompileExp(e->Left(), false);
        const int jmp = Emit(Op::TESTJMP, left, op_kind == BinOpKind::kAnd ? 0 : 1);
        const int right = CompileExp(e->Right(), false);
        MoveTo(result, right);
        const int jmp_end = Emit(Op::JMP);
        PatchSbx(jmp, CodeSize());
        MoveTo(result, left);
        PatchSbx(jmp_end, CodeSize());
        return result;
    }
    const int left = CompileExp(e->Left(), false);
    const int right = CompileExp(e->Right(), false);
    const int r = dest >= 0 ? dest : AllocReg();
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

int RegisterPeakChecker::CompileUnop(const SyntaxTreeInterfacePtr &exp, int dest) {
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op());
    const int operand = CompileExp(e->Right(), false);
    const int r = dest >= 0 ? dest : AllocReg();
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



void RegisterPeakChecker::Walk(const ParseResult &pr, const AnalysisResult &ar) {
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

    // 先登记所有文件级函数的签名：调用点可能在函数定义之前，等 CompileFunction 再登记就漏了。
    for (const auto &func: all_funcs_) {
        if (func->parent != nullptr) continue;
        const std::pair<int, bool> sig(static_cast<int>(func->params.size()), func->is_vararg);
        if (!func->name.empty()) file_func_sigs_[func->name] = sig;
        if (!func->unique_c_name.empty()) file_func_sigs_[func->unique_c_name] = sig;
    }

    if (pr.chunk && pr.chunk->Type() == SyntaxTreeType::Block) {
        const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);
        for (const auto &stmt: blk->Stmts()) {
            if (stmt->Type() != SyntaxTreeType::LocalVar) continue;
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
            if (!nl) continue;
            for (const auto &name: nl->Names()) {
                file_level_names_.insert(name);
            }
        }
    }

    for (const auto &func: all_funcs_) {
        if (func->parent == nullptr) {
            CompileFunction(func.get());
        }
    }
}

}// namespace

void SemanticAnalysis::CheckRegisterLimits(const ParseResult &pr, const AnalysisResult &ar) {
    RegisterPeakChecker checker;
    checker.Check(pr, ar);
}
}// namespace fakelua
