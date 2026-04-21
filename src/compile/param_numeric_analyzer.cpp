#include "compile/param_numeric_analyzer.h"

#include <queue>
#include <unordered_set>

#include "compile/syntax_tree.h"
#include "util/common.h"

namespace fakelua {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static bool IsMathBinop(const std::string &op_name) {
    return op_name == "PLUS" || op_name == "MINUS" || op_name == "STAR" || op_name == "SLASH" ||
           op_name == "DOUBLE_SLASH" || op_name == "POW" || op_name == "MOD" || op_name == "BITAND" ||
           op_name == "XOR" || op_name == "BITOR" || op_name == "LEFT_SHIFT" || op_name == "RIGHT_SHIFT" ||
           op_name == "LESS" || op_name == "LESS_EQUAL" || op_name == "MORE" || op_name == "MORE_EQUAL";
}

// Collect all simple variable names that appear directly in the numeric
// sub-expression rooted at `node`.  We stop at function-call boundaries (i.e.
// variables that are only passed as arguments are NOT collected here — they are
// not direct arithmetic operands).
static void CollectSimpleVarsInNumericExpr(const SyntaxTreeInterfacePtr &node,
                                            std::unordered_set<std::string> &vars) {
    if (!node) return;
    if (node->Type() != SyntaxTreeType::Exp) return;
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    const auto &exp_type = exp->ExpType();

    if (exp_type == "number") return;// literal — no variable

    if (exp_type == "prefixexp") {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
        if (!pe) return;
        if (pe->GetType() == "var") {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
            if (var && var->GetType() == "simple") {
                vars.insert(var->GetName());
            }
        } else if (pe->GetType() == "exp") {
            CollectSimpleVarsInNumericExpr(pe->GetValue(), vars);
        }
        // functioncall: stop — variables inside call arguments are not
        // direct arithmetic operands.
        return;
    }

    if (exp_type == "binop") {
        CollectSimpleVarsInNumericExpr(exp->Left(), vars);
        CollectSimpleVarsInNumericExpr(exp->Right(), vars);
        return;
    }

    if (exp_type == "unop") {
        // Only MINUS propagates a numeric context.
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(exp->Op());
        if (op && op->GetOp() == "MINUS") {
            CollectSimpleVarsInNumericExpr(exp->Right(), vars);
        }
    }
}

// Recursive AST walk that:
//   * Collects simple variable names that are direct operands of math binops
//     into `math_vars`.
//   * Collects simple variable names that appear on the LHS of assignments
//     into `assigned_vars`.
//   * Builds a var→var forward-assignment map (only simple-var = simple-var
//     assignments) into `assign_map`.
//
// The walk does NOT descend into nested function definitions (Function,
// LocalFunction, FuncBody, FunctionDef nodes).
static void AnalyzeNode(const SyntaxTreeInterfacePtr &node, std::unordered_set<std::string> &math_vars,
                         std::unordered_set<std::string> &assigned_vars,
                         std::unordered_map<std::string, std::string> &assign_map) {
    if (!node) return;

    switch (node->Type()) {
        case SyntaxTreeType::Block: {
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (const auto &stmt: block->Stmts()) {
                AnalyzeNode(stmt, math_vars, assigned_vars, assign_map);
            }
            break;
        }
        // Stop at nested function definitions.
        case SyntaxTreeType::Function:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::FuncBody:
        case SyntaxTreeType::FunctionDef:
            break;

        case SyntaxTreeType::Return: {
            const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            if (ret->Explist()) AnalyzeNode(ret->Explist(), math_vars, assigned_vars, assign_map);
            break;
        }
        case SyntaxTreeType::Assign: {
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            // Collect assigned vars from LHS.
            if (assign->Varlist()) {
                const auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
                for (const auto &var_node: varlist->Vars()) {
                    if (var_node->Type() == SyntaxTreeType::Var) {
                        const auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(var_node);
                        if (v->GetType() == "simple") {
                            assigned_vars.insert(v->GetName());
                        }
                    }
                }
            }
            // Analyze RHS for math vars.
            if (assign->Explist()) AnalyzeNode(assign->Explist(), math_vars, assigned_vars, assign_map);
            // Build assign_map: lhs = rhs (simple var to simple var).
            if (assign->Varlist() && assign->Explist()) {
                const auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
                const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
                if (varlist->Vars().size() == 1 && explist->Exps().size() == 1) {
                    if (varlist->Vars()[0]->Type() == SyntaxTreeType::Var &&
                        explist->Exps()[0]->Type() == SyntaxTreeType::Exp) {
                        const auto lhs = std::dynamic_pointer_cast<SyntaxTreeVar>(varlist->Vars()[0]);
                        const auto rhs_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(explist->Exps()[0]);
                        if (lhs && lhs->GetType() == "simple" && rhs_exp &&
                            rhs_exp->ExpType() == "prefixexp") {
                            const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(rhs_exp->Right());
                            if (pe && pe->GetType() == "var") {
                                const auto rhs_var =
                                        std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
                                if (rhs_var && rhs_var->GetType() == "simple") {
                                    assign_map[lhs->GetName()] = rhs_var->GetName();
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        case SyntaxTreeType::LocalVar: {
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            // Analyze RHS for math vars.
            if (lv->Explist()) AnalyzeNode(lv->Explist(), math_vars, assigned_vars, assign_map);
            // Build assign_map: local x = y (simple var to simple var).
            if (lv->Namelist() && lv->Explist()) {
                const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
                const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist());
                if (namelist->Names().size() == 1 && explist->Exps().size() == 1 &&
                    explist->Exps()[0]->Type() == SyntaxTreeType::Exp) {
                    const auto &lhs_name = namelist->Names()[0];
                    const auto rhs_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(explist->Exps()[0]);
                    if (rhs_exp && rhs_exp->ExpType() == "prefixexp") {
                        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(rhs_exp->Right());
                        if (pe && pe->GetType() == "var") {
                            const auto rhs_var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
                            if (rhs_var && rhs_var->GetType() == "simple") {
                                assign_map[lhs_name] = rhs_var->GetName();
                            }
                        }
                    }
                }
            }
            break;
        }
        case SyntaxTreeType::ExpList: {
            const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            for (const auto &exp: explist->Exps()) {
                AnalyzeNode(exp, math_vars, assigned_vars, assign_map);
            }
            break;
        }
        case SyntaxTreeType::Exp: {
            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            if (exp->ExpType() == "binop") {
                const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
                if (op && IsMathBinop(op->GetOp())) {
                    // Collect direct numeric operands.
                    CollectSimpleVarsInNumericExpr(exp->Left(), math_vars);
                    CollectSimpleVarsInNumericExpr(exp->Right(), math_vars);
                }
                // Recurse for nested binops / sub-expressions.
                AnalyzeNode(exp->Left(), math_vars, assigned_vars, assign_map);
                AnalyzeNode(exp->Right(), math_vars, assigned_vars, assign_map);
            } else if (exp->ExpType() == "prefixexp") {
                AnalyzeNode(exp->Right(), math_vars, assigned_vars, assign_map);
            } else if (exp->ExpType() == "unop") {
                AnalyzeNode(exp->Right(), math_vars, assigned_vars, assign_map);
            }
            break;
        }
        case SyntaxTreeType::PrefixExp: {
            const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
            if (pe->GetType() == "functioncall" || pe->GetType() == "exp") {
                AnalyzeNode(pe->GetValue(), math_vars, assigned_vars, assign_map);
            }
            // "var": no sub-expressions worth analyzing here.
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            if (fc->Args()) AnalyzeNode(fc->Args(), math_vars, assigned_vars, assign_map);
            break;
        }
        case SyntaxTreeType::Args: {
            const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            if (args->GetType() == "explist" && args->Explist()) {
                AnalyzeNode(args->Explist(), math_vars, assigned_vars, assign_map);
            }
            break;
        }
        case SyntaxTreeType::If: {
            const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            AnalyzeNode(if_stmt->Exp(), math_vars, assigned_vars, assign_map);
            AnalyzeNode(if_stmt->Block(), math_vars, assigned_vars, assign_map);
            if (const auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs())) {
                for (size_t i = 0; i < elseifs->ElseifSize(); ++i) {
                    AnalyzeNode(elseifs->ElseifExp(i), math_vars, assigned_vars, assign_map);
                    AnalyzeNode(elseifs->ElseifBlock(i), math_vars, assigned_vars, assign_map);
                }
            }
            AnalyzeNode(if_stmt->ElseBlock(), math_vars, assigned_vars, assign_map);
            break;
        }
        case SyntaxTreeType::While: {
            const auto while_stmt = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            AnalyzeNode(while_stmt->Exp(), math_vars, assigned_vars, assign_map);
            AnalyzeNode(while_stmt->Block(), math_vars, assigned_vars, assign_map);
            break;
        }
        case SyntaxTreeType::Repeat: {
            const auto repeat = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            AnalyzeNode(repeat->Block(), math_vars, assigned_vars, assign_map);
            AnalyzeNode(repeat->Exp(), math_vars, assigned_vars, assign_map);
            break;
        }
        case SyntaxTreeType::ForLoop: {
            const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            AnalyzeNode(for_loop->ExpBegin(), math_vars, assigned_vars, assign_map);
            AnalyzeNode(for_loop->ExpEnd(), math_vars, assigned_vars, assign_map);
            if (for_loop->ExpStep()) AnalyzeNode(for_loop->ExpStep(), math_vars, assigned_vars, assign_map);
            AnalyzeNode(for_loop->Block(), math_vars, assigned_vars, assign_map);
            break;
        }
        case SyntaxTreeType::ForIn: {
            const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            AnalyzeNode(for_in->Explist(), math_vars, assigned_vars, assign_map);
            AnalyzeNode(for_in->Block(), math_vars, assigned_vars, assign_map);
            break;
        }
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// ParamNumericAnalyzer
// ---------------------------------------------------------------------------

void ParamNumericAnalyzer::Process(CompileResult &cr, const CompileConfig &cfg) {
    if (cfg.skip_jit) return;

    const auto chunk = cr.chunk;
    if (!chunk || chunk->Type() != SyntaxTreeType::Block) return;

    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    for (const auto &stmt: block->Stmts()) {
        std::string name;
        SyntaxTreeInterfacePtr funcbody;
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            // Resolve function name (must be simple, not a.b.c or a:b).
            const auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            if (!funcname) continue;
            const auto fnlist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcname->FuncNameList());
            if (!fnlist || fnlist->Funcnames().size() != 1 || !funcname->ColonName().empty()) continue;
            name = fnlist->Funcnames()[0];
            funcbody = func->Funcbody();
        } else if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
            name = func->Name();
            funcbody = func->Funcbody();
        }

        if (name.empty() || !funcbody) continue;

        DEBUG_ASSERT(funcbody->Type() == SyntaxTreeType::FuncBody);
        const auto funcbody_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);

        std::vector<std::string> params;
        if (const auto parlist = funcbody_ptr->Parlist()) {
            DEBUG_ASSERT(parlist->Type() == SyntaxTreeType::ParList);
            const auto parlist_ptr = std::dynamic_pointer_cast<SyntaxTreeParlist>(parlist);
            if (parlist_ptr->VarParams()) continue;// skip vararg functions
            if (const auto namelist = parlist_ptr->Namelist()) {
                const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
                params = nl->Names();
            }
        }

        if (params.empty()) continue;

        AnalyzeFunction(name, params, funcbody_ptr->Block(), cr);
    }
}

void ParamNumericAnalyzer::AnalyzeFunction(const std::string &name, const std::vector<std::string> &params,
                                            const SyntaxTreeInterfacePtr &block, CompileResult &cr) {
    std::unordered_set<std::string> math_vars;
    std::unordered_set<std::string> assigned_vars;
    std::unordered_map<std::string, std::string> assign_map;// lhs -> rhs (simple var assignments)

    AnalyzeNode(block, math_vars, assigned_vars, assign_map);

    if (math_vars.empty()) return;

    // Build forward map: source_var -> set of vars assigned from it.
    std::unordered_map<std::string, std::unordered_set<std::string>> forward_map;
    for (const auto &[lhs, rhs]: assign_map) {
        forward_map[rhs].insert(lhs);
    }

    std::vector<int> math_param_indices;
    for (int i = 0; i < static_cast<int>(params.size()) &&
                    static_cast<int>(math_param_indices.size()) < kMaxMathParams;
         ++i) {
        const auto &param = params[i];

        // Skip params that are directly reassigned in the function body.
        if (assigned_vars.count(param)) continue;

        // BFS: compute closure of all vars derivable from `param` through
        // simple variable assignments.
        std::unordered_set<std::string> closure;
        std::queue<std::string> q;
        q.push(param);
        closure.insert(param);
        while (!q.empty()) {
            const auto cur = q.front();
            q.pop();
            auto it = forward_map.find(cur);
            if (it == forward_map.end()) continue;
            for (const auto &next: it->second) {
                if (!closure.count(next)) {
                    closure.insert(next);
                    q.push(next);
                }
            }
        }

        // Param is a math param if any variable in its closure participates in
        // a math operation.
        bool is_math = false;
        for (const auto &v: closure) {
            if (math_vars.count(v)) {
                is_math = true;
                break;
            }
        }

        if (is_math) {
            math_param_indices.push_back(i);
        }
    }

    if (!math_param_indices.empty()) {
        cr.math_param_positions[name] = math_param_indices;
        LOG_INFO("ParamNumericAnalyzer: {} math params for {}", math_param_indices.size(), name);
    }
}

}// namespace fakelua
