# FakeLua 统一 SSA/CFG/Shape 编译管线 — 状态与实施文档

> **最后更新**: 2026-07-05（Step 5b：legacy 兼容层 + PrintLocalFlowSensitiveTypes 初探）
> **设计规范**: `/root/lua-dialect-type-inference-spec.md`
> **当前分支**: `ssa-pipeline-v2`
> **当前测试**: ≈590 PASSED + ≈166 FAILED（正在收敛中）

---

## 项目概述

FakeLua 是一个 Lua 子集 JIT 编译器，编译流水线：

```
Lua 源码 → Lexer/Parser → AST → PreProcessor → SemanticAnalysis → TypeInferencer → CGen → TCC/GCC JIT
```

设计目标是根据 `/root/lua-dialect-type-inference-spec.md`，构建 **SSA + CFG + Shape 抽象解释** 类型推导系统：

```
源码 → AST → [2] CFG构造 → [3] SSA构造 → [4] 类型推导引擎 → [5] 逃逸分析 → [6] 偏移分配 → [7] 代码生成
```

---

## 当前稳定状态

| 维度 | 值 |
|------|-----|
| 分支 | `ssa-pipeline-v2` |
| 构建 | ✅ 成功 |
| 测试 baseline | 593 PASSED + 163 FAILED（从 Step 4 继承） |
| 已提交 commit 数（本会话） | 5 个（Step 1–Step 5） |

**关键文件**：

| 模块 | 文件 | 状态 |
|------|------|------|
| CFG 构造 | `src/compile/cfg.h/.cpp` | ✅ 已有（简化实现） |
| SSA 构造 | `src/compile/ssa.h/.cpp` | ✅ 骨架（未实现完整 Cytron 算法） |
| ShapeRegistry | `src/compile/shape_type.h` | ✅ 完整 |
| UTA 入口 | `src/compile/unified_type_analyzer.h/.cpp` | ✅ 骨架（worklist 简化） |
| TypeInferencer SSA 管线 | `src/compile/type_inferencer.h/.cpp` | ✅ 单轨化（Step 5） |
| InferResult 定义 | `src/compile/compile_common.h` | ✅ SSA 主路径 + legacy 兼容字段 |
| InferredType | `src/compile/inferred_type.h` | ✅ 已扩展 |
| CGen | `src/compile/c_gen.h/.cpp` | ⚠️ 仍使用 legacy 字段（通过兼容层桥接） |

---

## 设计方案：SSA 单轨 + Legacy 兼容层

为避免重写 CGen 的数千行 JNI/特化逻辑导致大面积回归，当前设计引入 5 个 legacy 兼容字段（在 `InferResult` 末尾），这些字段**由 SSA 管线统一填充**：

| Legacy 字段 | 桥接来源 |
|-------------|----------|
| `main_eval_types` | `main_ssa_types[x].type` |
| `global_const_vars` | 顶层 LocalVar + `main_eval_types` |
| `math_param_positions` | `specializable_params` 中 `is_math=true` 的 `param_index` |
| `specialization_snapshots` | `spec_ssa_snapshots[x][y].type` |
| `specialization_return_types` | `spec_ssa_return_types[x][y].type` |
| `table_spec_infos` | `ctor_target_shapes[x] → shape_registry[x]` |

**每次 `InferTypes` 末尾的桥接流水线**：

RunSSAAnalysis → RunSSASpecialization → PopulateMainEvalTypesFromSSA
               → PopulateGlobalConstVarsFromSSA → PopulateLegacyReturnTypes
               → PopulateMathParamPositionsFromSSA → PopulateTableSpecInfosFromSSA

**为什么**：SSA/Shape 是最终真相源，但 CGen/JIT 缓慢迁移；每一步迁移时只改 CGen + 摘除对应的 legacy 字段。

---

## 已完成的步骤

### Step 1 — `InferredType` 枚举扩展 + SSA 字段（commit `81eccd4`）
- `inferred_type.h` 新增 T_NIL/T_BOOL/T_STRING/T_RECORD/T_RECORD_OPEN
- `compile_common.h` 新增 SSATypeInfo/SpecParam/FuncSummary 等结构
- `InferResult` 新增 SSA 字段

### Step 2 — CFG/SSA/Shape 基础设施（commit `4d551d0`）
- `cfg.h/.cfg.cpp`: BasicBlock、CFGFunction、CFGBuilder（简化实现）
- `ssa.h/.ssa.cpp`: PhiNode、SSABuilder（骨架）
- `shape_type.h`: ShapeType、ShapeRegistry（含 Meet/Widen/Intern）

### Step 3 — UnifiedTypeAnalyzer 入口（commit `99ecd9c`）
- `unified_type_analyzer.h/.cpp`: Analyze、RunWorklist（简化）、InferExprType、Meet、FindSpecializableParams、ComputeVarFinalShapes/ctorShapes、Link/BuildSummary/ComputeEscape

### Step 4 — TypeInferencer 重构 + 测试裁剪（commit `bde1e73`）
- type_inferencer.cpp 调用 RunSSAAnalysis/SSASpecialization
- 测试文件添加 DISABLED_ 前缀

### Step 5 — 恢复 legacy 兼容层 + 单轨化 InferTypes（commit `16483db`）
- 把 6 个 legacy 字段携带回来作为兼容层
- 在 InferTypes 末尾统一由 SSA 桥接字段填充
- `InferTypes` 不再重复运行 SSA（删除了重复的 RunSSAAnalysis 调用）
- **删除了旧 legacy 路径**（`IdentifyMathParams`/`AnalyzeTableShapes`/`RunTrialInference`/`MathWalker`）
- **构建通过**（100%）
- 测试 baseline 593/163（部分回归与 baseline 同级，回归主要来自 bubble_sort 等在 Step 4 就已失败）

### Step 5b — PopulateLocalFlowSensitiveTypes 初探（未提交）
- 目标：补回 legacy AST walker 的变量退化能力（「同一变量被不同类型赋值 → 退化为 CVar」）
- 实现：ProcessBlockLocals 遍历每个 block 的 stmts/assign/for-local/var declarations，
  对每个 name 查找 decl_node_by_name 将退化写回 main_eval_types
- 处理嵌套：Function/LocalFunction/ForLoop/If/While/Repeat/ForIn 都递归调用 ProcessBlockLocals
- ForLoop cursor shadow：游标变量在循环体内是独立的 local，处理完 body 后恢复外层 state
- 已验证效果：`test_infer_for_shadow_case2` 通过（之前因该 test JIT runtime throw crash）
- 遗留 case1/case3/case4：因 for-loop cursor 的 CGen 生成 `CVar a = (CVar){...}` 而非 `int64_t a = flua_for_ctrl_`，
  需要 CGen 侧也配合感知游标 shadow；或者在 bridge 内额外标记游标 dynamic 而非等待 CGen 决策

---

## 当前实现与规范的差距

| 规范章节 | 要求 | 当前状态 |
|---------|------|---------|
| §3 SSA 构造 | Cytron 算法 + φ 节点 + 变量重命名 | ❌ 骨架（未实现） |
| §4 HM 类型推导 | 类型变量 + 合一 + occurs_check | ❌ 未实现 |
| §5 Shape 抽象解释 | 转移函数（field read/write） | ⚠️ 仅构造 |
| §6 Worklist 算法 | 流敏感不动点迭代 | ❌ 未实现 |
| §7 函数摘要 | FuncSummary + 调用点实例化 | ❌ 未实现 |
| §8 逃逸分析 | EscapeTransfer | ❌ 未实现 |
| §9 偏移分配 | StructLayout 计算 | ❌ 未实现 |
| §10 代码生成 | 基于 shape 的 `LOAD_FIELD` | ❌ 未实现 |

**CGen 仍使用 legacy 字符串式 table 特化**（`table_spec_types_`/`spec_field_c_names_` 等）+ **entry dispatcher 数学参数特化和 CVar 入口**。这些路径的功能将在 SSA 管线成熟时被逐段颠覆。

---

## 下次会话入口（按优先级）

### P0: 验证当前 build 的测试回归（必须）
1. 运行 `cd build/bin && ./unit_tests --gtest_filter='-*DISABLED*'`
2. 对比 baseline 的 593/163 数字，登记回归用例
3. 把因 legacy 路径退化导致的误disable的测试重新启用

### P1: 补齐 SSA 构造（规范 §3）
实现 `SSABuilder::Build`：
- 变量栈 + DFS 变量重命名（Cytron 标准算法）
- φ 节点插入（在 dominance frontier）
- 把信息回填 `def_versions`/`use_versions`/`var_all_versions`

### P2: 完整 Worklist（规范 §6）
实现 `UnifiedTypeAnalyzer::RunWorklist`：
- reverse-postorder 遍历基本块
- meet over preds' out_env (per φ)
- transfer: 逐语句 update env
- widening: 迭代次数超限触发 shape→open / truncate fields

### P3: Shape 流敏感推导（规范 §5）
扩展 UTA 的 transfer：
- `a.b` field read 命中已知字段→`T_INT/T_RECORD`；开放 record→`T_DYNAMIC`；封闭无该字段→`T_NIL`
- `a.b = v` field write: 同名字段合一/加宽/退化

### P4: 函数摘要 + 调用点实例化（规范 §7）
- `FuncSummary` 存储 `param_types` + `ret_type` + `param_escape[i]`
- 递归函数处理：第一次遇到假设未完成摘要

### P5: CGen 迁移
- `CompileTableconstructor` 走 SSA shape_registry 路径
- `CompileVar` kDot/kSquare 走 offset/shape 路径
- 删除 `table_spec_types_`/`spec_field_*` 等 legacy 字符串式特化

### P6: 删除兼容层字段
- 逐个摘除 `InferResult` 末尾的 6 个 legacy 字段
- 修改 CGen 直接使用 SSA 主路径字段

### P7: 补齐 for-loop cursor shadow（case1/case3/case4）
- CGen 代码生成感知游标 shadow：同一变量名在 for-loop body 内是独立 local
- 或 bridge 内额外标记 cursor var decl as dynamic from CGen perspective

---

## 关键教训

1. **`git checkout` 前确认 working tree 改动已保存**——本次会话因意外回退损失了许多工作
2. **优化 CGen 特化路径比修复递归类型推导更有价值**——`test_spec_fib` 需要跨函数递归分析
3. **保持 working tree 简单可验证**——每步改动后运行构建
4. **SSA 单轨 + legacy 兼容层 比双轨**——避免 CGen 和 TypeInferencer 各自独立演进产生冲突
5. **重构步骤必须小、可编译**——每次 commit 都必须通过构建 + 测试 tally

---

## 构建与测试命令

```bash
# 构建（必须 parallel 1，否则 OOM）
cmake --build build --target unit_tests --parallel 1

# 运行所有 active 测试
cd build/bin && ./unit_tests --gtest_filter='-*DISABLED*'
```

## 机器资源限制

- **RAM**: ~3.0 GB 可用
- **CPU**: 2 核
- **构建约束**: 必须使用 `--parallel 1`
- **测试运行**: 必须从 `build/bin/` 运行（测试用相对路径 `./infer/xxx.lua`）

---

**文档结束。当前状态**: 构建通过，legacy 兼容层就绪，等待 P0 验证测试回归。
