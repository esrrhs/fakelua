#pragma once

#include "compile/compile_common.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua {

// ============================================================================
// 特化分析器（Specialization Analyzer）
//
// 核心思想：在 SSA (静态单赋值) 类型推导的基础之上，通过模拟/试探推导
// (Trial Inference)，自动识别出对数学计算 (算术/比较/循环边界等) 有关键
// 类型优化作用的函数形参 (数学参数，Math Parameters)；进而对这些参数的
// 所有数值类型组合 (2^k 穷举特化组合) 进行推导，确定特化组合下的返回值
// 类型和局部变量快照。
//
// 工作流：
//   1) 识别数学参数 (IdentifyMathParams)：
//      - 对每个函数，运行基准推导 (假设所有形参为 T_DYNAMIC) vs. 积极推导 (假设
//        所有形参为 T_INT)。
//      - 如果某个形参从 T_DYNAMIC 提升为 T_INT 后，使得函数体内存在任何算术节点、
//        比较节点、循环边界或嵌套调用的推导类型发生优化提升，则认定该形参为
//        数学特化参数。
//   2) 2^k 穷举特化组合返回值与快照推导 (InferSpecializationReturnTypes)：
//      - 假设有 k 个数学参数，穷举 bitmask 从 0 到 2^k - 1 (第 i 位为 0 表示整型
//        T_INT，为 1 表示浮点型 T_FLOAT)。
//      - 对每种类型假设运行试探推导，将得到的 AST 节点类型映射（Snapshot）以及
//        返回值推导类型（Return Type）保存至 InferResult 中，供 JIT 代码生成端
//        (CGen) 快速生成特化的原生函数及入口分发器。
// ============================================================================
class SpecializationAnalyzer {
public:
    // 启动特化推导主入口。根据解析结果 (语法树) 进行分析，并将生成的特化参数、
    // 类型快照和返回值映射表填充至推导结果 (ir) 中。
    void Analyze(const ParseResult &pr, InferResult &ir);

private:
    // 数学参数分析所需的临时函数结构信息
    struct MathFuncInfo {
        SyntaxTreeInterfacePtr block;       // 函数体 Block 语法树节点
        std::vector<std::string> params;    // 原始形参名称列表
        std::vector<int> param_versions;    // 形参对应的 SSA 初始版本号
    };
    using MathFuncInfoMap = std::unordered_map<std::string, MathFuncInfo>;

    // 收集到的函数返回语句信息
    struct FuncRetInfo {
        bool ends_with_return = false;                // 函数是否能保证在所有分支路径下均有显式 return
        std::vector<SyntaxTreeInterfacePtr> ret_exps; // 所有 return 语句返回 of 表达式树节点集合
    };

    // ────────────────────────────────────────────────────────────────────────
    // 数学参数识别与定位
    // ────────────────────────────────────────────────────────────────────────

    // 识别并收集所有函数的数学特化参数，返回过滤后的映射表
    MathFuncInfoMap IdentifyMathParams(const ParseResult &pr, InferResult &ir);

    // 判定某个函数中哪些形参是数学特化参数，返回发生类型提升的形参索引列表
    std::vector<int> FindMathParamIndices(const std::string &name, const MathFuncInfo &info,
                                          const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &baseline,
                                          const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &all_int,
                                          const std::unordered_map<std::string, std::vector<int>> &known_math_positions,
                                          InferResult &ir);

    // 执行 2^k 特化分支迭代推导，更新返回值类型与 AST 节点特化快照
    void InferSpecializationReturnTypes(InferResult &ir, const MathFuncInfoMap &math_func_info,
                                        const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache);

    // ────────────────────────────────────────────────────────────────────────
    // 试探推导与节点变化检测
    // ────────────────────────────────────────────────────────────────────────

    // 检测 trial_map 相较于 base_map 在函数体内部是否存在算术/比较/循环/调用的推导优化提升。
    // improvement_mode 表示是否仅寻找由 T_DYNAMIC 变为数值类型的提升。
    bool CheckArithmeticTypeChanges(const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                    const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                    const SyntaxTreeInterfacePtr &func_block, bool improvement_mode,
                                    const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    // 辅助检测：单个算术节点在两张表中的类型差异是否构成提升/改变
    bool CheckArithmeticNodeChange(const SyntaxTreeInterfacePtr &node,
                                   const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                   const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                   bool improvement_mode) const;

    // 辅助检测：单个比较节点在两张表中的操作数类型差异是否构成提升/改变
    bool CheckComparisonNodeChange(const SyntaxTreeInterfacePtr &node,
                                   const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                   const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                   bool improvement_mode) const;

    // 辅助检测：单个 for 循环控制变量或边界类型在两张表中的差异是否构成提升/改变
    bool CheckForLoopNodeChange(const SyntaxTreeInterfacePtr &node,
                                const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                bool improvement_mode) const;

    // 辅助检测：函数调用参数特化匹配状态的变化
    bool CheckCallNodeChange(const SyntaxTreeInterfacePtr &node,
                             const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                             const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                             const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    // 快速判断节点是否为算术二元或一元表达式
    bool IsArithmeticExpr(const SyntaxTreeInterfacePtr &node) const;
    // 快速判断节点是否为比较二元表达式
    bool IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const;

    // ────────────────────────────────────────────────────────────────────────
    // 参数类型环境构建与辅助方法
    // ────────────────────────────────────────────────────────────────────────

    // 构建试探推导所需的初始参数版本环境
    std::unordered_map<int, SSATypeInfo> MakeAssumedParamTypes(const std::vector<int> &param_versions, int special_idx,
                                                               InferredType special_type, InferredType default_type) const;

    // 根据 bitmask 组合构建 2^k 穷举推导时的初始参数环境
    std::unordered_map<int, SSATypeInfo> MakeSpecializedParamTypes(const std::vector<int> &param_versions,
                                                                   const std::vector<int> &math_indices, int bitmask) const;

    // 隔离运行一次试探推导，并返回该轮推导结束后，整张 AST 节点与推导类型的映射快照 (Snapshot)
    std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> RunTrialInference(
        const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, const CFGFunction &cfg, SSAFunction &ssa,
        const std::unordered_map<int, SSATypeInfo> &assumed_types, InferResult &ir);

    // 检测控制流语法树中是否所有的可能执行分支都能最终 return 到
    bool AllPathsReturn(const SyntaxTreeInterfacePtr &block_node) const;
    // 遍历收集语法树 Block 中包含的所有 return 语句的表达式节点
    bool CollectReturnExps(const SyntaxTreeInterfacePtr &block_node, std::vector<SyntaxTreeInterfacePtr> &ret_exps) const;

    // 根据当前的特化推导类型快照及返回表达式节点，计算出该特化版本最终的合并返回值类型
    InferredType ComputeReturnTypeFromSnapshot(const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &snapshot,
                                               const FuncRetInfo &ret_info) const;

    static constexpr int kMaxSpecIterations = 16;       // 解决互相依赖调用的特化收敛最大传播迭代轮数
    static constexpr int kMaxMathSpecializedParams = 5; // 限制单个函数进行 2^k 次方穷举的最大数学参数数量 (上限 32 个分支)
};

} // namespace fakelua
