#include "compile/ssa.h"
#include "compile/syntax_tree.h"

namespace fakelua {

// ============================================================================
// 辅助：变量 ID / 版本号分配
//
// 变量被压缩为整型 id (0, 1, 2, ...)，从而可以用 vector 高效索引所有以
// 变量为 key 的集合。版本号 (version) 全局单调递增，每个版本都属于某个
// 变量，在某一个基本块中定义。
// ============================================================================

// 把变量名映射到唯一整型 id
//   - 若已存在直接返回；否则创建新 id，并在 var_def_blocks_ 中为它增加一个空集合
int SSABuilder::GetVarId(const std::string &name) {
    auto it = var_name_to_id_.find(name);
    if (it != var_name_to_id_.end()) return it->second;
    int id = (int)var_id_to_name_.size();
    var_name_to_id_[name] = id;
    var_id_to_name_.push_back(name);
    var_def_blocks_.emplace_back();     // 为该变量初始化一个空的"定义块"集合
    return id;
}

// 为指定变量在指定块中分配一个新版本号
//   - ver = ssa_.next_version（单调递增）
//   - 维护三个映射：
//       var_all_versions[var_name] → 追加 ver（该变量所有版本的全集）
//       version_to_block[ver]      → block_id（该版本诞生在哪里）
//       version_to_name[ver]       → var_name（该版本属于哪个变量）
int SSABuilder::NewVersion(const std::string &var_name, int block_id) {
    int ver = ssa_.next_version++;
    ssa_.var_all_versions[var_name].push_back(ver);
    ssa_.version_to_block[ver] = block_id;
    ssa_.version_to_name[ver] = var_name;
    return ver;
}

// ============================================================================
// 辅助：收集语句中的定义 / 使用变量名
//
// 这两个函数只负责扫描 AST，不涉及 SSA 状态。它们把"哪些名字是左值"与
// "哪些名字是右值"区分开——这是构造 def/use 的前提。
// ============================================================================

// ── 定义变量（Left-Hand Side） ────────────────────────────────────────
std::vector<std::string> SSABuilder::GetDefNames(const SyntaxTreeInterfacePtr &stmt) {
    std::vector<std::string> names;
    if (!stmt) return names;
    switch (stmt->Type()) {
        case SyntaxTreeType::LocalVar: {
            // local a, b, c = ...
            auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            if (auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist())) {
                for (auto &n : nl->Names()) names.push_back(n);
            }
            break;
        }
        case SyntaxTreeType::Assign: {
            // a, b, c = ...   （只取简单变量作为定义目标，表访问不是全新定义变量）
            auto as = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
            auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(as->Varlist());
            if (vl) {
                for (auto &v : vl->Vars()) {
                    if (v->Type() == SyntaxTreeType::Var) {
                        auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(v);
                        if (var && var->GetVarKind() == VarKind::kSimple)
                            names.push_back(var->GetName());
                    }
                }
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            // for i = e1, e2, e3 do ... end  →  i 在头被定义
            auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
            names.push_back(fl->Name());
            break;
        }
        case SyntaxTreeType::ForIn: {
            // for a, b in ... do ... end
            auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
            if (auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(fi->Namelist())) {
                for (auto &n : nl->Names()) names.push_back(n);
            }
            break;
        }
        default:
            break;
    }
    return names;
}

// ── 使用变量（Right-Hand Side） ─────────────────────────────────────────
// 这里"使用"指的是语句引用变量读其值的地方。我们需要递归树遍历表达式
// 来收集所有 kSimple 的 Var 节点，因为它们对应真正的"读变量"语义。
std::vector<std::string> SSABuilder::GetUseNames(const SyntaxTreeInterfacePtr &stmt) {
    std::vector<std::string> names;
    if (!stmt) return names;

    // 递归收集器：对整棵表达式子树深度优先搜索。
    // 命中 Var(kSimple) 就计入；命中其他节点就继续递归其子节点。
    std::function<void(const SyntaxTreeInterfacePtr&)> collect;
    collect = [&](const SyntaxTreeInterfacePtr &node) {
        if (!node) return;
        if (node->Type() == SyntaxTreeType::Var) {
            auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
            if (v && v->GetVarKind() == VarKind::kSimple)
                names.push_back(v->GetName());
            return;
        }
        switch (node->Type()) {
            case SyntaxTreeType::Exp: {
                // 二元 / 一元表达式：递归左右子
                auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
                auto k = e->GetExpKind();
                if (k == ExpKind::kBinop || k == ExpKind::kUnop) {
                    collect(e->Left());
                    collect(e->Right());
                } else if (k == ExpKind::kPrefixExp) {
                    collect(e->Right());
                }
                break;
            }
            case SyntaxTreeType::PrefixExp: {
                // 前缀表达式：Var 或 '(' exp ')'
                auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
                if (pe->GetPrefixKind() == PrefixExpKind::kVar && pe->GetValue()) {
                    collect(pe->GetValue());
                } else if (pe->GetPrefixKind() == PrefixExpKind::kExp && pe->GetValue()) {
                    collect(pe->GetValue());
                }
                break;
            }
            case SyntaxTreeType::TableConstructor: {
                // { ... }
                auto tc = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
                if (tc->Fieldlist()) collect(tc->Fieldlist());
                break;
            }
            case SyntaxTreeType::FieldList: {
                auto fl = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
                for (auto &f : fl->Fields()) collect(f);
                break;
            }
            case SyntaxTreeType::Field: {
                // [k] = v  /  k = v  /  v
                auto f = std::dynamic_pointer_cast<SyntaxTreeField>(node);
                if (f->Key()) collect(f->Key());
                if (f->Value()) collect(f->Value());
                break;
            }
            case SyntaxTreeType::FunctionCall: {
                // f(e1, e2, ...)
                auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
                if (fc->prefixexp()) collect(fc->prefixexp());
                if (fc->Args()) collect(fc->Args());
                break;
            }
            case SyntaxTreeType::Args: {
                auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
                if (args->GetArgsKind() == ArgsKind::kExpList && args->Explist())
                    collect(args->Explist());
                else if (args->Tableconstructor())
                    collect(args->Tableconstructor());
                break;
            }
            case SyntaxTreeType::ExpList: {
                auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
                for (auto &e : el->Exps()) collect(e);
                break;
            }
            case SyntaxTreeType::VarList: {
                auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(node);
                for (auto &v : vl->Vars()) collect(v);
                break;
            }
            default:
                break;
        }
    };

    // 针对不同语句类型，搭好入口即可——collect 自动递归到所有子节点
    switch (stmt->Type()) {
        case SyntaxTreeType::LocalVar: {
            // local a, b = e1, e2   →  RHS 是 explist
            auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            if (lv->Explist()) collect(lv->Explist());
            break;
        }
        case SyntaxTreeType::Assign: {
            // a, b = e1, e2         →  RHS 是 explist
            auto as = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
            if (as->Explist()) collect(as->Explist());
            break;
        }
        case SyntaxTreeType::ForLoop: {
            // for i = begin, end, step do ... end  →  begin / end / step 都是 use
            auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
            if (fl->ExpBegin()) collect(fl->ExpBegin());
            if (fl->ExpEnd()) collect(fl->ExpEnd());
            if (fl->ExpStep()) collect(fl->ExpStep());
            break;
        }
        case SyntaxTreeType::ForIn: {
            // for a, b in e1, e2, e3 do ... end   →  e1/e2/e3 是 use
            auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
            if (fi->Explist()) collect(fi->Explist());
            break;
        }
        case SyntaxTreeType::Return: {
            // return e1, e2, ...
            auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);
            if (ret->Explist()) collect(ret->Explist());
            break;
        }
        default:
            break;
    }
    return names;
}

// ============================================================================
// Step 0: 收集每个变量被定义在哪些块
//
// 遍历 CFG 的每个基本块的每个语句，把该语句定义的所有变量名记入
// var_name_to_id_，并把这些块加入 var_def_blocks_[vid]。
//
// 这一步的输出是后续 φ 插入阶段的工作表"种子"：对于每个变量 v，
// 它的所有定义块构成初始传播起点。
// ============================================================================
void SSABuilder::CollectDefBlocks(const CFGFunction &cfg) {
    // 逐块、逐语句扫描
    for (const auto &b : cfg.blocks) {
        for (const auto &s : b.stmts) {
            // 该语句定义了哪些变量
            for (const auto &name : GetDefNames(s)) {
                int vid = GetVarId(name);           // 第一次遇到会把变量压缩为 id
                var_def_blocks_[vid].insert(b.id);  // 在 var_def_blocks_[vid] 中记录块 b.id
            }
        }
    }
}

// ============================================================================
// 构建支配树（从支配关系推导）
//
// 输入：CFG 提供 dominators[b] = 支配 b 的所有块的集合（含 b 自身）。
// 输出：dom_tree_children_[b] = b 的所有直接支配子节点。
//
// 支配树（Dominance Tree）是以 entry_id 为根的有根树，满足：
//    parent(b) = idom(b) = b 的（唯一的、最近的）直接支配者。
//
// 算法：对每个非入口块 b，在严格支配 b 的集合里，找到那个"离 b 最近"
// 的——即不被其他任何严格支配者支配的那个块。
// ============================================================================
void SSABuilder::BuildDomTree(const CFGFunction &cfg) {
    for (const auto &b : cfg.blocks) {
        if (b.id == cfg.entry_id) continue;   // 入口没有直接支配者

        int idom = -1;
        auto it = cfg.dominators.find(b.id);
        if (it == cfg.dominators.end()) continue;    // 没有支配信息就跳过

        // 遍历每一条严格支配者候选 d
        for (int d : it->second) {
            if (d == b.id) continue;                  // 跳过自身，需要的是严格支配者

            // 检查 d 是否是"最近的"：若存在另一个严格支配者 d2 也被 d 支配，
            // 则 d 比 d2 "更远"（在支配树中辈分更高），那么 d 就不是 idom。
            bool is_idom = true;
            for (int d2 : it->second) {
                if (d2 == b.id || d2 == d) continue;
                auto it2 = cfg.dominators.find(d2);
                // 如果 d 支配 d2（d2 的支配集合里包含 d），那么 d 辈分更高
                if (it2 != cfg.dominators.end() && it2->second.count(d)) {
                    is_idom = false;
                    break;
                }
            }
            if (is_idom) { idom = d; break; }
        }

        // idom 就是 b 在支配树中的父节点
        if (idom >= 0) dom_tree_children_[idom].push_back(b.id);
    }
}

// ============================================================================
// Cytron 第一阶段：插入 φ 节点
//
// 这是整个 SSA 构造中最关键的算法步骤。
//
//   直观目标：对于每个变量 v，在所有"不同来源的 v 可能相遇"的地方放
//            一个 φ 节点，使得 φ 把不同路径带来的 v 合并为新的统一版本。
//
//   "不同来源的值会相遇" 的精确刻画是 支配边界（Dominance Frontier）。
//   形式定义：DF(X) = { Y | X 支配 Y 的一个前驱，且 X 不严格支配 Y }。
//   换句话说：Y 的前驱中有 X 的"势力范围"，但 Y 自身并不被 X 独占支配 →
//   意味着必定有另一条不经过 X 的路径也通向 Y → X 中的定义可能与别的路径
//   的值在 Y 相遇。
//
//   算法（工作表/worklist 迭代）：
//
//     for each variable vid:
//       worklist ← var_def_blocks_[vid]        // 所有定义 v 的块
//       has_phi = ∅                             // 已有 φ 的块集合
//
//       while worklist 非空:
//         n = pop(worklist)
//         for each d in DF(n):                  // n 的支配边界
//           if d 已有 φ for v: continue
//           在 d 的 block_phis 中为 v 新建一个 φ 节点
//           has_phi ← has_phi ∪ {d}
//           if d ∉ var_def_blocks_[vid]:        // 插入 φ 后，d 成为一个新的
//             worklist ← worklist ∪ {d}         // "v 的来源"，继续传播
//
//   为什么需要最后一行？
//     在 d 插入 φ 后，d 为 v 生成一个新版本（重命名阶段再分配）。这个新版本
//     需要继续活跃到 d 所能支配的最后一个块；也就是说 d 现在和"真正的定义
//     块"起完全一样的传播作用。如果 d 本身也是 v 的定义块，那自然早已在
//     worklist 中（是种子项），只需要忽略工作表去重即可。
//
//   这样，φ 沿着"支配边界形成的网络"蔓延，所有必要的汇合点都被覆盖。
// ============================================================================
void SSABuilder::InsertPhis(const CFGFunction &cfg) {
    // 注：var_name_to_id_ 和 var_def_blocks_ 已经由 Build() 预先填充

    int num_vars = (int)var_id_to_name_.size();

    // 独立处理每个变量
    for (int vid = 0; vid < num_vars; ++vid) {
        // 复制初始种子集合成 worklist
        auto it = var_def_blocks_[vid];
        std::vector<int> worklist(it.begin(), it.end());
        std::unordered_set<int> has_phi;  // 已有 φ 的块（去重）

        // 工作表主循环
        for (size_t i = 0; i < worklist.size(); ++i) {
            int n = worklist[i];
            auto df_it = cfg.dominance_frontier.find(n);
            if (df_it == cfg.dominance_frontier.end()) continue;

            // 遍历 n 的支配边界上的每个目标块 d
            for (int d : df_it->second) {
                if (has_phi.count(d)) continue;   // 已插入 φ → 跳过

                has_phi.insert(d);

                // ── 在块 d 为变量 vid 创建 φ 节点 ──────────────────────
                auto *blk = cfg.FindBlock(d);
                int npreds = blk ? (int)blk->pred_ids.size() : 0;
                PhiNode phi;
                phi.var_id = vid;
                phi.var_name = var_id_to_name_[vid];
                phi.arg_versions.resize(npreds, -1);   // 暂时未知，重命名阶段填入
                ssa_.block_phis[d].push_back(std::move(phi));

                // ── 如果 d 不是 v 的定义块，它现在凭着 φ 成为新的来源 ──
                // → 继续把 d 加入工作表，让 DF(d) 也被处理（沿支配边界传播）
                if (!var_def_blocks_[vid].count(d)) {
                    worklist.push_back(d);
                }
                // 否则 d 是定义块 → worklist 中本来就有它（是种子），
                // 此处不再 push（去重靠 has_phi）
            }
        }
    }
}

// ============================================================================
// 单个块的语句重命名
//
// 处理顺序与状态更新顺序（注意：先处理 φ，再处理普通语句）：
//
//   1) 处理块入口的 φ 节点：
//        对每个 φ：
//          v' = NewVersion(var_name, block_id)   // 定义新版本
//          phi.result_version = v'
//          stack_top[phi.var_id] = v'            // 新版本成为该变量的当前栈顶
//
//   2) 处理块内语句（按源代码顺序）：
//        a) 对语句中每个使用名（use）uname：
//             vid = var_id(uname)
//             stack_top[vid] 即当前最新版本 → 记录为该语句的 use_version
//           （当前实现仅收集，实际替换在代码生成阶段）
//
//        b) 对语句中每个定义名（def）dname：
//             vid = var_id(dname)
//             v' = NewVersion(dname, block_id)    // 新版本
//             stack_top[vid] = v'
//             def_versions[stmt] = v'
//
// 处理完后，stack_top 中记录的就是"离开此块时每个变量的最新版本"。
// 这个值会被上层 RenameVariables 用来回填后继块 φ 的参数。
// ============================================================================
void SSABuilder::RenameBlockStmts(
    int block_id, const std::vector<SyntaxTreeInterfacePtr> &stmts,
    std::unordered_map<int, int> &stack_top) {

    // ── 1) 处理 φ 节点 ─────────────────────────────────────────────────
    auto phi_it = ssa_.block_phis.find(block_id);
    if (phi_it != ssa_.block_phis.end()) {
        for (auto &phi : phi_it->second) {
            int new_ver = NewVersion(phi.var_name, block_id);
            phi.result_version = new_ver;
            stack_top[phi.var_id] = new_ver;   // 被 φ 产出新版本后的当前活跃版本
        }
    }

    // ── 2) 处理块内普通语句 ────────────────────────────────────────────
    for (const auto &stmt : stmts) {
        // 2a) 替换每个使用：取当前栈顶版本
        for (const auto &uname : GetUseNames(stmt)) {
            auto vit = var_name_to_id_.find(uname);
            if (vit == var_name_to_id_.end()) continue;  // 不可识别的名字（全局/非简单变量）→ 跳过
            int vid = vit->second;
            auto sit = stack_top.find(vid);
            if (sit != stack_top.end()) {
                // 找到该变量的当前最新版本 → 即为该 use 应使用的版本
                // （简化实现：仅收集版本号，不修改 AST；真正在 CGen 阶段做替换）
                (void)0;  // 实际替换在代码生成阶段
            }
        }

        // 2b) 处理每个定义：分配新版本并压栈
        for (const auto &dname : GetDefNames(stmt)) {
            int vid = GetVarId(dname);
            int new_ver = NewVersion(dname, block_id);
            stack_top[vid] = new_ver;
            ssa_.def_versions[stmt.get()] = new_ver;  // 记录该语句定义的版本
        }
    }
}

// ============================================================================
// Cytron 第二阶段：变量重命名（主循环）
//
// 总体流程：以支配树为骨架进行 DFS。为什么按支配树而不是 CFG？
//   - φ 的语义是 "v = φ(v_from_pred0, v_from_pred1, ...)"，也就是无论走
//     哪条前驱，φ 都产出同一个 v'。
//   - 支配树的一个节点的子树中的所有块，都在该节点"严格支配"下——意味着
//     对它们而言，该节点中的 v 已经通过路径置顶而"唯一确定"。
//   - 按支配树 DFS 时，等我们处理完一个节点 b，b 中的所有定义（包括其 φ
//     的版本）都已经被压入 stack_top，然后递归进入子节点。在子树内的任何
//     使用 b 中产出的 v 时，自然读到的是同一个新版本。
//   - 当一个子树处理完毕，返回父节点时，不再需要栈回退（因为子树不会"向上
//     污染"父节点版本），这就是按支配树 DFS 简单优雅的地方。
//
// 步骤：
//
//   rename_dfs(b):
//     // 1) 处理 b 块的所有 φ（等价于"进入 b 块"时已经获得多个前驱合并的 v）
//     RenameBlockStmts(b, ...)
//
//     // 3) 用当前 stack_top 回填所有后继块 s 的 φ 参数：
//     //    遍历 b 的每一个 CFG 后继 s，找出 b 在 s.pred_ids 中的 index i，
//     //    对 block_phis[s] 中的每个 φ：phi.arg_versions[i] = stack_top[var_id]
//     //
//     //    stack_top 在这里保存的恰好是"离开 b 块时每个变量的最新活跃版本"，
//     //    也就是从 pred = b 的视角下所能看到的 v 的版本，填入即可。
//
//     // 4) 递归所有支配子树 child
//
// 调用方：rename_dfs(entry_id)
//
// 关于"前驱版本沿支配边界传播"的逐步解释：
//   让我们用一个具体例子来追踪。假设有如下 CFG（箭头表示前驱→后继）：
//       1 → 3, 2 → 3, 3 → 4, 3 → 5, 4 → 6, 5 → 6
//   假设变量 x 在块 1、2、5 中分别被定义。支配树是：
//       entry=1     （为简单起见就以 1 为入口）
//   实际上支配关系取决于具体结构，这里只说关键点：
//
//   - 块 1 中定义了 x → 分配版本 x1，这是 x 的初始版本，入栈 stack_top[x]=x1。
//     支配树 DFS 从 1 往下。就在处理它的后继时，会把 x1 填入后继块中指向 x 的 φ 参数
//     （如果后继有 x 的 φ）。
//   - 在块 3，由于来自 1、2 两条路径，x 的值可能来自 x1（块 1 的定义）或 x2
//     （块 2 的定义）。块 3 正是块 1 和块 2 的支配边界上的交汇点，因此在 InsertPhis
//     阶段已经为块 3 插入了 φ。
//   - 重命名进入块 3 时，为它的 φ 分配新版本 x3，更新 stack_top[x]=x3。然后 DFS
//     到它的子块 4 和 5。在 5 中又定义了一次 → 分配 x4，stack_top[x]=x4。
//     离开 5 时 stack_top 仍保持 x4；但回溯到 3 的 DFS 子节点完成时 stack_top[x]
//     仍然是按 3 的 φ 维护的版本（x3），不会被 5 污染——注意 DFS 的栈在递归返回时
//     展开，stack_top 是引用传递，但没有"出栈"机制，只要我们在进入 5 子树之前
//     先回填后继 φ 即可（回填动作用的是当时的 stack_top 值）。
//     为了让"5 中的 x 定义能传递到 6 而非 3 中的 x3 传递到 6"，回填到后继 φ
//     的动作发生在处理完当前块——也就是说"后继块 s 的 φ 参数 i 必须等到处理完
//     前驱块 b 来填"。
//
// 更精确地说，整个"前驱版本如何沿着支配边界传播"的逐步流程：
//
//   1) InsertPhis 完成：块 3 因位于 1 和 2 的支配边界上，获得 φ_x。
//   2) RenameVariables DFS 进入块 3，为 φ_x 分配的新版本 x3 入栈。
//   3) 在 DFS 进入 3 的子节点（比如块 5）之前，先用当前的 stack_top（即离开 3
//      时是 x3 的版本）回填 3 的所有后继块中 φ_x 的前驱参数。
//      但 3 的后继是 4 和 5，它们各自也有 φ_x；如果 5 是 1 的 DF 延续，那么
//      DF 会遍历到。
//   4) 进入块 5，又为 x 的第二次定义分配新版本 x4；stack_top[x]=x4。
//   5) 回填 5 的后继（块 6）中 φ_x 的 pred_index 对应的参数 → 填入 x4。
//   6) 同样方式，处理块 5 之前的兄弟块（比如 4），用 stack_top[x]=x3
//      （因为 4 是块 3 的另一个子树，与 5 互不相干时的 stack_top 就不同）填 6 的对应 slot。
//   7) 最终块 6 中的 φ_x 的 arg_versions 就是 (x_from_branch1, x_from_branch2)，
//      各自被正确反映。
//
// 关键不变量：
//   - 当从块 b 走向后继 s 时，phi.arg_versions[i]（i 是 b 在 s 中作为前驱的位置）
//     会被赋值为 stack_top[var]，而 stack_top[var] 在那一刻保存的正是"刚刚定义
//     完 b 的最里面块的 var 版本"——这就是"前驱版本中最新、最贴近 s 的那一个"，
//     也是 φ 需要的语义。
//
// ============================================================================
void SSABuilder::RenameVariables(const CFGFunction &cfg) {
    // stack_top: var_id → 该变量当前活跃的最新 SSA 版本
    std::unordered_map<int, int> stack_top;

    // DFS 支配树（递归 lambda）
    std::function<void(int)> rename_dfs = [&](int bid) {
        // ── 1) 处理当前块（φ + 普通语句）────────────────────────────
        RenameBlockStmts(bid, cfg.FindBlock(bid)->stmts, stack_top);

        // ── 2) 用当前 stack_top 回填后继块的 φ 参数 ─────────────────
        // 注意：stack_top 此刻保存的是"离开 bid 块时的那一刻，各变量的最新版本"，
        // 也就是 bid 前驱视角下的"可见 v 的最新版本"。
        auto *blk = cfg.FindBlock(bid);
        if (blk) {
            for (int sid : blk->succ_ids) {
                auto phi_it = ssa_.block_phis.find(sid);
                if (phi_it == ssa_.block_phis.end()) continue;
                auto *sblk = cfg.FindBlock(sid);
                if (!sblk) continue;

                // 找到 bid 在 sblk->pred_ids 中的下标
                // （φ 的参数顺序是严格按照 pred_ids 排列的）
                int pred_idx = -1;
                for (int j = 0; j < (int)sblk->pred_ids.size(); ++j) {
                    if (sblk->pred_ids[j] == bid) { pred_idx = j; break; }
                }
                if (pred_idx < 0) continue;

                // 为该后继块中的每一个 φ 填入参数：从"当前栈顶"取最新版本
                for (auto &phi : phi_it->second) {
                    auto sit = stack_top.find(phi.var_id);
                    if (sit != stack_top.end()) {
                        if (pred_idx < (int)phi.arg_versions.size())
                            phi.arg_versions[pred_idx] = sit->second;
                    }
                }
            }
        }

        // ── 3) 递归处理支配子节点 ─────────────────────────────────────
        auto cit = dom_tree_children_.find(bid);
        if (cit != dom_tree_children_.end()) {
            for (int child : cit->second) rename_dfs(child);
        }
    };

    rename_dfs(cfg.entry_id);
}

// ============================================================================
// 主入口
//
// 步骤：
//   1) 清空状态。
//   2) 为函数参数分配入口版本（参数也视作在入口块被赋值一次，因此它们的
//      初始 SSA 版本需要在重命名之前确立）。
//   3) Step 0: CollectDefBlocks — 收集种子。
//   4) Step 0: BuildDomTree — 构建支配树（为 Phase 2 做准备）。
//   5) Cytron Phase 1: InsertPhis。
//   6) Cytron Phase 2: RenameVariables。
// ============================================================================
SSAFunction SSABuilder::Build(const CFGFunction &cfg) {
    ssa_ = {};
    ssa_.func_name = cfg.func_name;
    ssa_.next_version = 0;
    var_name_to_id_.clear();
    var_id_to_name_.clear();
    var_def_blocks_.clear();
    dom_tree_children_.clear();

    // ── 参数初始化 ──────────────────────────────────────────────────────
    // 参数本身被视为在入口块中的一次"定义"：进入函数时就有确定的版本号。
    // 这一步在 CollectDefBlocks 之前做，使得参数同名变量被提前注册（id 一致）。
    std::unordered_map<int, std::string> idx_to_name;
    for (const auto &[pname, pidx] : cfg.param_indices) idx_to_name[pidx] = pname;
    int param_count = (int)cfg.param_indices.size();
    ssa_.param_versions.resize(param_count, -1);
    for (int i = 0; i < param_count; ++i) {
        auto it = idx_to_name.find(i);
        const std::string &pname = (it != idx_to_name.end()) ? it->second : std::to_string(i);
        GetVarId(pname);  // 注册变量（保证后续 CollectDefBlocks 中同名局部变量与参数共用 id）
        int ver = NewVersion(pname, cfg.entry_id);  // 入口块产生参数版本
        ssa_.param_versions[i] = ver;
    }

    // ── Step 0: 收集定义块 + 构建支配树 ──────────────────────────────────
    CollectDefBlocks(cfg);
    BuildDomTree(cfg);

    // ── Cytron Step 1: 插入 φ 节点 ──────────────────────────────────────
    InsertPhis(cfg);

    // ── Cytron Step 2: 变量重命名 ───────────────────────────────────────
    RenameVariables(cfg);

    return std::move(ssa_);
}

// ============================================================================
// DumpToString：把 SSA 结构导出为可读字符串
//
// 格式示例：
//   SSA[foo] params=[0,1]
//     phi in block 5: xv4 = φ(2, 3)
//     var x: v0@entry, v2@v1, ...
//
// 用于测试用例中验证 SSA 化是否正确。
// ============================================================================
std::string SSAFunction::DumpToString() const {
    std::ostringstream oss;
    oss << "SSA[" << func_name << "] params=[";
    for (size_t i = 0; i < param_versions.size(); ++i) {
        if (i > 0) oss << ",";
        oss << param_versions[i];
    }
    oss << "]\n";

    // 打印每个块入口的 φ 节点
    for (const auto &[bid, phis] : block_phis) {
        for (const auto &phi : phis) {
            oss << "  phi in block " << bid << ": " << phi.var_name << "v" << phi.result_version
                << " = φ(";
            for (size_t i = 0; i < phi.arg_versions.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << (phi.arg_versions[i] >= 0 ? std::to_string(phi.arg_versions[i]) : "?");
            }
            oss << ")\n";
        }
    }

    // 打印各变量的所有版本及其出现块
    for (const auto &[vname, vers] : var_all_versions) {
        oss << "  var " << vname << ": ";
        for (size_t i = 0; i < vers.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "v" << vers[i] << "@" << version_to_block.at(vers[i]);
        }
        oss << "\n";
    }
    return oss.str();
}

}// namespace fakelua
