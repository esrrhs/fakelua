#pragma once

#include "compile/type_inference/hm_type.h"
#include "compile/type_inference/inferred_type.h"
#include "syntax_tree.h"
#include "util/debug.h"
#include <format>
#include <string>
#include <unordered_set>
#include <vector>

namespace fakelua {

struct CFGFunction;
struct SSAFunction;

inline constexpr const char *kInitFunctionName = "__fakelua_init";

inline std::string InferredTypeToString(InferredType type) {
    switch (type) {
        case T_UNKNOWN:
            return "T_UNKNOWN";
        case T_NIL:
            return "T_NIL";
        case T_BOOL:
            return "T_BOOL";
        case T_INT:
            return "T_INT";
        case T_FLOAT:
            return "T_FLOAT";
        case T_STRING:
            return "T_STRING";
        case T_RECORD:
            return "T_RECORD";
        case T_RECORD_OPEN:
            return "T_RECORD_OPEN";
        case T_DYNAMIC:
            return "T_DYNAMIC";
        default:
            return "T_UNKNOWN";
    }
}

// 将函数调用的 args 节点展开为原始参数节点数组，
// 覆盖三种语法形式：args ::= (explist) | tableconstructor | LiteralString。
inline std::vector<SyntaxTreeInterfacePtr> ExtractCallRawArgs(const std::shared_ptr<SyntaxTreeArgs> &args_ptr) {
    std::vector<SyntaxTreeInterfacePtr> raw_args;
    DEBUG_ASSERT(args_ptr);
    const auto args_kind = args_ptr->GetArgsKind();
    if (args_kind == ArgsKind::kExpList) {
        const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
        DEBUG_ASSERT(explist_ptr);
        const auto &exps = explist_ptr->Exps();
        raw_args.insert(raw_args.end(), exps.begin(), exps.end());
        return raw_args;
    }
    if (args_kind == ArgsKind::kString) {
        const auto str_exp = args_ptr->String();
        DEBUG_ASSERT(str_exp);
        raw_args.push_back(str_exp);
        return raw_args;
    }
    if (args_kind == ArgsKind::kTableConstructor) {
        const auto table_arg = args_ptr->Tableconstructor();
        DEBUG_ASSERT(table_arg);
        raw_args.push_back(table_arg);
    }
    return raw_args;
}

// ---- vararg 辅助函数 ---------------------------------------------------------

// 判断一个表达式节点是否为 __fakelua_vararg_* 变量引用。
inline bool IsVarargExp(const SyntaxTreeInterfacePtr &exp_node) {
    if (!exp_node || exp_node->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(exp_node);
    if (exp->GetExpKind() != ExpKind::kPrefixExp) {
        return false;
    }
    const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
    if (!pe || pe->GetPrefixKind() != PrefixExpKind::kVar) {
        return false;
    }
    const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
    if (!var || var->GetVarKind() != VarKind::kSimple) {
        return false;
    }
    return var->GetName().rfind("__fakelua_vararg_", 0) == 0;
}

// ---- 阶段一：解析结果 -------------------------------------------------------
// Parser 的输出：源文件名和语法树根节点。
// 由 Compiler::Compile 在词法/语法解析阶段填充，
// 后续各阶段均以此作为只读输入。
struct ParseResult {
    // 源代码文件名
    std::string file_name;
    // 语法树根节点（代码块）
    SyntaxTreeInterfacePtr chunk;
};

// ---- 阶段三：语义与控制流分析结果 -----------------------------------------
// SemanticAnalysis 的输出。
// 由 SemanticAnalysis::Analyze 填充，供 CGen 使用。
struct AnalysisResult {
    // 函数名 -> 最大返回值数量（-1 代表动态，例如以函数调用结尾）
    std::unordered_map<std::string, int> function_max_returns;
    // 语法分析出的所有函数调用表达式节点集合，供 CGen 直接查询
    std::unordered_set<const SyntaxTreeInterface *> function_call_exps;
    // 语法分析出的所有函数调用到其被调用者名字的映射，供 CGen 直接查询
    std::unordered_map<const SyntaxTreeInterface *, std::string> callee_names;
    // 文件级/全局常量名称集合
    std::unordered_set<std::string> global_const_names;
};

// ── SSA 类型信息 ──────────────────────────────────────────────────────────
// SSATypeInfo 是 SSA 类型推导系统中的一个"类型快照"。
// 每个 SSA 版本（变量的一次赋值产生一个版本）都关联一个 SSATypeInfo，
// 用于在 CFG 上做类型流分析。
struct SSATypeInfo {
    // type: SSA 层面推导出的抽象类型。它与 shape_id 组合来表达"这个 SSA 版本
    // 此刻看起来是什么样"：
    //   - 对简单类型（int/float/bool/string/nil/dynamic），shape_id 固定为 -1，
    //     此时 type 已经足够表达类型信息，无需额外的结构/字段拓扑信息；
    //   - 对 T_RECORD / T_RECORD_OPEN（即结构化对象 / 开放记录），仅有 type 无法
    //     描述"这个版本此刻具有哪些字段、这些字段又是什么类型"，此时用一个全局
    //     shape_id 指向 ShapeRegistry 中的一个 Shape 条目，ConcreteCGen 通过
    //     比较 shape_id 来判断两个变量是否具有相同的字段布局，从而选择"偏移偏移
    //     访问"或"字典访问"。
    // 这样设计是因为：type 使用简单的 enum 类型可以支持O(1) 比较 / meet / join，
    // 而 Shape 则是一个独立的、可演化的复杂对象，两者解耦便于各自独立收敛。
    InferredType type = T_UNKNOWN;

    // shape_id: 全局 ShapeRegistry 中某个 Shape 条目的索引，或 -1（无形状）。
    // 当且仅当 type 为 T_RECORD / T_RECORD_OPEN 时才可能非 -1。
    // 注意 shape_id 仅作为"句柄"存在，实际的字段布局、widening 状态都保存在
    // ShapeRegistry[shape_id] 里；CompareCGen / ConcreteCGen 统一通过读取
    // 这个 shape_id 来决定走结构化还是字典式读写路径。
    int shape_id = -1;

    bool operator==(const SSATypeInfo &o) const {
        return type == o.type && shape_id == o.shape_id;
    }

    bool operator!=(const SSATypeInfo &o) const {
        return !(*this == o);
    }
};

// ── 历史遗留类型快照（已不直接使用） ──────────────────────────────────────
// EvalTypeSnapshot 原来是 eval 解释器（非 SSA 路径）的按节点类型快照，
// 随着本次"统一到 SSA/CFG/Shape 管线"的重构（详见 SSA_PIPELINE_STATUS.md），
// 它已被 InferResult::main_ssa_types / ssa_version_types 接管。
// 保留此 typedef 仅为兼容可能的存量引用/残留日志；新代码不应再使用。
// TODO: 在确认无任何外部引用后移除。
typedef std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> EvalTypeSnapshot;

// 函数摘要：用于过程间分析（规范 §7）。
//
// 在 SSA 类型推导管线中，过程间分析（interprocedural analysis）需要知道每个
// 函数的"行为摘要"，以便在调用点把形参类型传播给实参、把返回类型回填给调用方。
// FuncSummary 就是这种摘要的运行时承载类型。
//
// 推导流程（见 unified_type_analyzer.cpp::BuildFuncSummary）：
//   1) 进入函数体时，对每个形参分配一个 SSA 版本的占位；
//   2) 在函数体内做前向类型流传播；
//   3) 出去时汇总：param_types 取形参所有版本的 meet，ret_type 取所有 return
//      语句返回表达式的 meet；
//   4) 逃逸判定（见 Phase 5）、HM 签名推导（见下方）在此期间同步完成。
struct FuncSummary {
    // 函数名（全局唯一标识，也是 InferResult::func_summaries / GenResult::function_names 的 key）
    std::string func_name;

    // param_types: 每个形参的 SSA 类型（形参个数 = param_types.size()）。
    // 如何推导：
    //   - 初始时每个形参绑定一个 SSA 占位，类型为 T_UNKNOWN；
    //   - 函数体内每当形参被"读取"时，读取占位的当前类型；
    //   - 函数被调用时（在另一个函数的摘要构建过程中），实参类型 meet 进形参；
    //   - 多次调用 / 多次形参读写后取 meet 得到最终 param_types[i]。
    // 也就是说 param_types[i] 是"该形参在所有观察到的调用点中都兼容的最精确类型"。
    // 当形参实际是 T_RECORD / T_RECORD_OPEN 时 param_types[i].shape_id != -1。
    //
    // 消费者：
    //   - CGen 调 callee 时据此决定是否需要做动态类型检查；
    //   - CompareCGen 调用点实例化把 param_types 的 shape_id 与实参 shape 做比较；
    //   - 递归函数自调用时把 param_types 与自身摘要做 meet（不动点迭代）。
    std::vector<SSATypeInfo> param_types;

    // ret_type: 函数返回值的 SSA 类型。
    // 如何推导：
    //   - 函数体内收集所有 return 语句中的返回表达式节点；
    //   - 对每一个 return 表达式的推导出的 SSATypeInfo 做 Meet；
    //   - 即 ret_type = meet(ret_expr_1, ret_expr_2, ...)。
    // Meet 是向上收敛的：T_INT meet T_FLOAT -> T_FLOAT；T_RECORD meet T_RECORD ->
    // 开字段较多的那个；没有 return 时保持 T_UNKNOWN。
    //
    // 消费者：
    //   - 调用点用 ret_type 作为被调用方产生结果的"已知类型"，向内联/优化传播；
    //   - CGen 依 ret_type 决定返回值的形状（是直接返回还是复用入参内存等）。
    SSATypeInfo ret_type{T_UNKNOWN, -1};

    // param_escape: 与 param_types 等长，标记每个形参是否"逃逸"出当前函数。
    // 逃逸判定（Phase 5）：
    //   - 当形参被写入 table/闭包上 / 作为不透明句柄传递 / 被返回到调用方之外，
    //     视为逃逸；
    //   - 逃逸的形参不允许按值就地传递，CGen 必须通过额外的间接层（或盒子）
    //     写入，以保证外部仍然能看到修改。
    // 如何影响 CGen：
    //   - 非逃逸 + shape 已知 → CGen 倾向用 by-value/栈拷贝，或按偏移就地更新；
    //   - 逃逸 → CGen 改用 by-reference / 临时变量 + 回写模式，避免把形状信息
    //     丢失；在 Struct Typedef 路径下还会多生成一层 heap 装箱。
    //
    // 消费者：
    //   - CGen / CompareCGen 的参数传递策略选择；
    //   - Struct Typedef 路径下的偏移/间接访问选择。
    std::vector<bool> param_escape;

    // 是否是变长参数函数（function(...)）
    bool is_vararg = false;

    // 递归检测标记。
    // 过程间构建摘要时采用不动点迭代（worklist）：
    //   - 进入某函数摘要构建前把 being_built 设为 true；
    //   - 若同一次构建过程中重新进入同一函数（直接或间接递归），
    //     检测到 being_built == true 就用当前已积累的不动点结果作为 self-call
    //     的摘要，避免无限递归；
    //   - 构建完成后复位为 false，并把新结果 meet 进不动点结果。
    bool being_built = false;

    // ── Hindley-Milner 多态签名（可选扩展） ───────────────────────────────
    // 背景：当函数的形参在 SSA 层面推不出来更具体的类型（通常是 T_DYNAMIC，或
    // T_UNKNOWN 兜底时），纯 SSA 路径只能给出 "dynamic" 这种最宽泛的结果。
    // 此时我们退回到 Hindley-Milner（HM）类型系统重新推导一个更精确的多态
    // 签名，用来在调用点做"HM→SSA"回填。
    //
    // HM 多态签名工作的完整流程：
    //   1) 进入函数前，给每个形参生成一个 HM 类型变量 α_i（param_hm_types[i]），
    //      同时给返回类型也保留一个 HM 类型变量 β（ret_hm_type）。
    //   2) 在函数体内部推导出各个使用点的 HM 约束（例如 f(x) 是 "x.int" 则
    //      约束 α_i ≈ { int }，即一个 HM 的记录形状）。
    //   3) 用 HM unification 解出 α_i / β 的最一般类型。
    //   4) 把得到的 HM 多态表达式存回 param_hm_types / ret_hm_type，并置
    //      must_use_hm = true。
    //   5) 在调用点：从实参类型实例化 α_i 得到一个具体的 HM 类型 t'，再把 t'
    //      转换回 SSA 类型并回填到 arg 节点的 SSATypeInfo。
    //   6) 这样即使 SSA 层面只有 T_DYNAMIC / T_UNKNOWN，调用点也能得到一个
    //      具体的形状（例如"字段的 shape_id"），让后续 CGen 走偏移访问。
    //
    // 注意：
    //   - 仅当 must_use_hm == true 且 param_hm_types 非空时才启用 HM 路径；
    //   - 未启用时统一走纯 SSA 路径，HM 路径不影响结果正确性，只影响精度；
    //   - HM 路径目前作为一个"二次精化"步骤，不作为主推导真相源。

    // 启不启 HM 路径的开关。当形参全为 T_DYNAMIC 且推导端具备 HM 推导能力时
    // 置 true；否则忽略下方字段。
    bool must_use_hm = false;

    // 每个形参对应的 HM 类型变量/表达式，与 param_types 等长。
    // 例如形参 x 对应的 HM 表达式可以是：
    //   - 类型变量 α（未知）；
    //   - 原子类型 int / float / string；
    //   - 记录类型 { a: α1, b: α2 }（结构形式，HM 需要枚举字段才能做更细约束）；
    //   - 配合 HM-Instance 调用点时做 let-polymorphism 风格实例化。
    std::vector<Type *> param_hm_types;

    // 函数返回值的 HM 表达式。
    // 与形参类型变量一起解出，如果函数没有实际 return（或返回 dynamic）则为
    // 一个留空的 Type。在调用点上 ret_hm_type 经实例化后被塞回实参的 SSATypeInfo。
    Type *ret_hm_type = nullptr;
};

// TypeInferencer 的输出 — SSA/CFG/Shape 管线的真相源。
//
// 整个统一推导器（unified_type_analyzer.cpp）的产出，分为以下几类真相：
//   A. SSA 类型：每个 SSA 版本 / 每个 AST 节点 / 每个变量的最终类型；
//   B. Shape 拓扑：所有记录形式的全局形状；
//   C. 逃逸信息：每个函数的哪些变量逃逸；
//   D. 函数摘要：过程间分析用的跨函数缓存。
//
// 消费者：
//   - CGen / CompareCGen 是最大两读者，几乎读 A/B/C/D 的全部；
//   - HM 推导步骤（hm_type.cpp）读 param_hm_types 以决定是否回填主推导；
//   - JIT 侧只依赖 GenResult，不直接读 InferResult（除调试/日志外）。
struct InferResult {
    // ── 字段 1: SSA 版本 → 类型信息 ────────────────────────────────────────
    // 这是 SSA 分配器为每个变量版本分配的 version -> 该版本此刻推导出类型 的映射。
    // 数据结构：每次赋值产生一个 SSA 记录，记录一个 SSATypeInfo。
    // 消费者：
    //   - TypeInferencer 自身在工作表不动点迭代时读写；
    //   - CGen 查读以判断某个 SSA 版本是否已经到达静态类型（用于是否 emit 动态检查）。
    std::unordered_map<int, SSATypeInfo> ssa_version_types;

    // ── 字段 2: AST 节点 → SSA 版本号 ──────────────────────────────────────
    // 节点 → 它关联的最后一个 SSA 版本（phi 合并后的新版本）。
    // 消费者：
    //   - CGen 用来把代码位置定位到它对应的 SSA 版本；
    //   - 逃逸分析通过节点反查版本，再反查 SSA def 到"creator"；
    //   - CompareCGen 调用点实例化时由此转发实参 shape 到形参 shape。
    std::unordered_map<const SyntaxTreeInterface *, int> node_ssa_version;

    // ── 字段 3: AST 节点 → SSA 类型推导结果（主路径） ─────────────────────
    // 这是整个推导器最核心的产物：每个参与 SSA 推导的 AST 节点的最终类型。
    // 注意：param_types / ret_type 是从"变量"维度汇总的，而 main_ssa_types 是从
    // "节点"粒度给出的——每个表达式变量都能查到它此刻推导出的 SSATypeInfo。
    // 如何得到：对每个节点先在 SSA 工作表中解出版本号 + 版本对应的类型，再合并
    // （φ 合并点做 Meet，widing 时按规则收敛）。
    // 消费者：
    //   - CGen 是主读者——读它决定表达式走哪条代码生成路径；
    //   - CompareCGen 通过它做字段布局对齐：若两节点的 shape_id 不同则 fallback；
    //   - 逃逸分析读它判断它们所处的作用域/逃逸状态。
    std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> main_ssa_types;

    // ── 字段 4: 全局 shape 注册表 ──────────────────────────────────────────
    // 全局共享的 Shape 集合。Shape 是一个可演化的"字段布局拓扑"：
    //   - 每个 T_RECORD/T_RECORD_OPEN 的 SSA 形状对应一个条目；
    //   - Shape 支持 Join/Meet/Compare；在 Widing 阶段后稳定。
    // 生命周期：InferResult 持有 shared_ptr 以便 CGen 延长其生命周期。
    // 消费者：
    //   - CGen / CompareCGen 唯一负责查 Shape 的组件（查字段偏移、字段类型）；
    //   - Struct Typedef 路径下 Shape 还决定结构体 typedef 的字段类型串。
    std::shared_ptr<struct ShapeRegistry> shape_registry;

    // ── 字段 5: 变量名 → 最终 shape_id ─────────────────────────────────────
    // 每个变量在所有 SSA 版本上的形状 ID Meet（所有版本的"公共基础"）。
    // 填入时机：当 SSA 不动点收敛后，遍历 ssa_version_types，按变量名聚合各版本的
    // shape_id 并与当前条目 Meet。
    // 消费者：
    //   - CGen: 即使某个局部版本因为尚未处理只有 -1，仍然可以基于 var_final_shapes
    //     决定该变量整体适合哪种访问模式（整体偏 struct 还是 dict）；
    //   - CompareCGen 用它做"保守选择"：只要任一版本为未知则退化为字典访问，否则走偏移。
    //
    // 注意：这个并不是 precise-per-version 的形状，是"保守"索引，用于 CGen 的启发式选择。
    std::unordered_map<std::string, int> var_final_shapes;

    // ── 字段 6: table constructor AST 节点 → shape_id ─────────────────────┤
    // 专门收集 tableconstructor 节点的 shape 结果——因为表的构造阶段往往是最先
    // 确定字段布局的源头。
    // 填入时机：当一个 AST 节点是 tableconstructor 且成功推导出 shape_id 时记录。
    // 消费者：
    //   - CGen 据此在生成表构造时直接写入 Struct Typedef 字段偏移，而无需
    //     重新推导；
    //   - CompareCGen 在调用点把该 shape_id 作为"实参期待"与形参 shape_id 比较。
    std::unordered_map<const SyntaxTreeInterface *, int> ctor_target_shapes;

    // ── 字段 7: 逃逸分析结果：函数名 → (变量名 → 是否逃逸) ─────────────────
    // Phase 5 逃逸分析的输出。"逃逸"指：变量的引用或副作用作用出了当前函数作用域。
    // 填入时机：函数摘要构建时同步收集，依据写 table / 传给不透明调用 / 返回等方式。
    // 消费者：
    //   - CGen 借此决定：逃逸变量必须走 by-reference/临时变量+回写模式；非逃逸
    //     变量在 shape 已知时可走 by-value / 就地偏移更新；
    //   - Struct Typedef 路径下逃逸会多生成一层 heap 内/外配合的存储模式。
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> escape_vars;

    // ── 字段 8: 函数摘要：函数名 → 摘要 ────────────────────────────────────
    // 过程间分析的缓存。在一个"module"内多个函数互相调用时，函式之间通过 FuncSummary
    // 传递参数和返回类型，构成一个函式依赖图上的不动点迭代。
    // 写入时机：函式摘要构建（见上面 FuncSummary 自身注释的推导流程）。
    // 消费者：
    //   - CGen 调用 callee 时读 ret_type 决定外部处理方式；
    //   - CompareCGen 调用点实例化比较 param_types shape 与实参 shape；
    //   - 工作表迭代（worklist）函数处理函式间的 meet，用以收敛不动点。
    std::unordered_map<std::string, FuncSummary> func_summaries;

    // ── 字段 9: AST 根节点（保持生命周期） ─────────────────────────────────
    // 持有本次分析的 AST 根节点 shared_ptr。
    //
    // 为什么需要：main_ssa_types / node_ssa_version / ctor_target_shapes 中存储的是
    // AST 节点的原始指针（const SyntaxTreeInterface*）。这些节点由 chunk 的
    // shared_ptr 持有生命周期。如果 InferResult 不持有 chunk，当 analyze 函数返回时
    // chunk 可能首先被析构，导致上述 map 中的原始指针变成悬空。
    //
    // 典型案例（曾导致 CI 崩溃）：测试框架先构建 InferResult 再遍历 main_ssa_types
    // 调用 node->Type()，若 chunk 已析构则为 UB（表现为 "pure virtual method called"）。
    //
    // 注意：chunk 必须是 InferResult 的最后一个字段（或与上述 map 中的指针同源），
    // 确保 map 中指针的有效性覆盖 InferResult 整个使用周期。
    // 函数名 -> CFG 控制流图
    std::unordered_map<std::string, std::shared_ptr<CFGFunction>> cfg_functions;
    // 函数名 -> SSA 函数
    std::unordered_map<std::string, std::shared_ptr<SSAFunction>> ssa_functions;

    SyntaxTreeInterfacePtr chunk;
};

// JIT 侧用的函数元信息；仅记录形参数量和是否 vararg，与 SSA 推导结构无关。
struct JitFunctionInfo {
    int params_count = 0;
    bool is_vararg = false;
};

// ---- 阶段五：代码生成结果 ---------------------------------------------------
// CGen 的输出。
// CGen 的输入是 InferResult（A: SSA 真值 + B: Shape + C: 逃逸 + D: 摘要），
// 负责把这些高层类型信息翻译成具体的 C 源码字符串，再由 CMake 后续编译成
// 可加载的 JIT 模块。
struct GenResult {
    // ── 字段 1: 生成的 C 代码字符串 ───────────────────────────────────────
    // CGen 的"热输出"：会被 CMake/CXXCompiler 直接编译进 .so / .obj 的目标源码。
    // 包含：公共头部（由 CGen 注入）+ 全局变量 + 函数声明 + 函数实现。
    // 消费者：
    //   - Compiler::Compile 把它交给 CMake 编译；
    //   - 调试/日志路径把它写到磁盘以便人工检查。
    std::string c_code;

    // ── 字段 2: 记录的 C 代码（不含公共头部） ────────────────────────────
    // 与 c_code 内容相同，但去掉了 CGen 注入的公共头部（例如 #include、宏定义等），
    // 仅保留"用户关心的"部分：全局变量、函数声明、函数实现。
    // 仅当 CompileConfig::record_c_code == true 时由 CGen 填充；否则为空。
    // 消费者：
    //   - 测试/回归用例用它做快照对比（避免公共头部差异污染 diff）；
    //   - 调试器/日志用它展示"用户视角"的生成结果。
    std::string recorded_c_code;

    // ── 字段 3: 函数名 → 函数元信息 ──────────────────────────────────────
    // 告诉 JIT 侧每个函数的形参个数和是否 vararg，用于在调用时正确设置参数帧。
    // 消费者：
    //   - JIT 入口（vm_function 侧）用它做参数合法性检查；
    //   - 调试器用它做函数签名展示。
    std::unordered_map<std::string, JitFunctionInfo> function_names;
};

// ─────────────────────────────────────────────────────────────────────────
// 编译管线完整结果（CompileFile 系列接口的返回值）
//
// 该结构体是 Compiler::CompileFile / CompileStringTo 的返回值，持有从源码到
// 最终 C 代码的完整管线产物。
//
// 设计目标：
//   1. 把原本通过 GetLastRecordedCCode 全局状态获取的 C 代码纳入返回值，
//       消除隐式的全局状态依赖。
//   2. 把原本藏在各子模块（CFG / SSA / TypeInference）内部的中间结构暴露出来，
//       让测试 / 调试 / 工具可以直接访问管线的每一步产物。
//   3. 支持"慢路径"分析和"快路径"调用：
//      - 调试/测试: 拿到完整 CompileResult 后读 ir / gr。
//      - 纯执行: 仅使用 gr.c_code 调用 JIT，不要 ir。
//
// 线程安全：
//   - CompileResult 不涉及全局状态，但内部持有对 AST (chunk) 的 shared_ptr，
//     多个 CompileResult 可以并发读写各自的 chunk。
//   - 同一个 CompileResult 不保证线程安全（与原来的 Global 一样）。
//
// 生命周期注意：
//   - InferResult 内的 raw pointer (例如 main_ssa_types 的 key) 指向 AST 节点，
//     由 result.chunk 持有生命周期。访问 raw pointer 前必须保证 result 存活。
// ─────────────────────────────────────────────────────────────────────────
struct CompileResult {
    // ── 阶段 1: 解析结果 ────────────────────────────────────────────────
    // 词法和语法解析器的直接产物：按文件/字符串区分源，附带 AST 根节点。
    // 注意: source 字段仅在 CompileString 来源时由 Compiler::CompileStringTo
    // 填充（文件来源时 ParseResult.file_name 已经是有效路径，不需要额外存储）。
    ParseResult parse_result;

    // ── 阶段 2: 预处理 + 语义分析结果 ──────────────────────────────────
    // 预处理器会把 Macro / Vararg / LocalFunctionDef 等语法糖"降糖"为更简单
    // 的 AST 结构，随后语义分析器检查：
    //   - 常量不可重新赋值、不可在非 const 位置读写。
    //   - 函数参数和返回值数量正确性。
    //   - 全局变量命名不冲突。
    // 这一阶段同时产出 AnalysisResult（函数最大返回数、调用者列表等），
    // 给 TypeInference 和 CGen 提供元信息。
    AnalysisResult analysis_result;

    // ── 阶段 3: 类型推导（含 SSA）结果 ─────────────────────────────────
    // TypeInferencer::InferTypes 的完整输出:
    //   - main_ssa_types: 每个 AST 节点对应的 SSA 类型。
    //   - ssa_version_types: 每个 SSA 版本号线性对应的 SSATypeInfo。
    //   - var_final_shapes / ctor_target_shapes: 变量/构造点的 shape 信息。
    //   - escape_vars / func_summaries: 逃逸分析和过程间摘要。
    //   - shape_registry: 全局 shape 注册表。
    //   - chunk: AST 根节点（保持生命周期，理由见上）。
    InferResult infer_result;

    // ── 阶段 4: 代码生成结果 ────────────────────────────────────────────
    // CGen::Generate 的产物。最关键的输出 gr.c_code 是最终可用于编译的 C 源码，
    // gr.function_names 是 JIT 侧需要的函数签名。
    GenResult gen_result;

    // ─────────────────────────────────────────────────────────────────
    // 便捷访问器（ conveniences ）
    // ─────────────────────────────────────────────────────────────────
    // 便捷访问器（ conveniences ）
    // ─────────────────────────────────────────────────────────────────

    // 获取生成的 C 代码（包含公共头部）
    [[nodiscard]] const std::string &GetCCode() const {
        return gen_result.c_code;
    }

    // 获取记录的 C 代码（不含公共头部，原 recorded_c_code 字段）
    [[nodiscard]] const std::string &GetRecordedCCode() const {
        return gen_result.recorded_c_code;
    }

    // 获取类型推导中间结果（包含 main_ssa_types、shape_registry 等）
    [[nodiscard]] const InferResult &GetInferResult() const {
        return infer_result;
    }

    // 推断出的 AST 节点类型映射
    [[nodiscard]] const auto &GetNodeTypes() const {
        return infer_result.main_ssa_types;
    }

    // 推断出的 SSA 版本→类型映射
    [[nodiscard]] const auto &GetVersionTypes() const {
        return infer_result.ssa_version_types;
    }

    // Shape registry getter
    [[nodiscard]] const auto &GetShapeRegistry() const {
        return infer_result.shape_registry;
    }

    // 逃逸分析结果
    [[nodiscard]] const auto &GetEscapeVars() const {
        return infer_result.escape_vars;
    }

    // 函数摘要
    [[nodiscard]] const auto &GetFuncSummaries() const {
        return infer_result.func_summaries;
    }
};


}// namespace fakelua
