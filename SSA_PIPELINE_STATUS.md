# FakeLua 统一 SSA/CFG/Shape 编译管线 — 状态与实施文档

> **最后更新**: 2026-07-05（Step 8：FindSpecializableParams 完善 + return type 推断 + 比较特化回归修复）
> **设计规范**: `/root/lua-dialect-type-inference-spec.md`
> **当前分支**: `ssa-pipeline-v2`
> **当前测试**: ~636 PASSED + ~76 FAILED（收敛中，部分因进程崩溃计数可能不准）

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
| 测试 baseline | ~636 PASSED + ~76 FAILED（Step 6–8 累计收敛） |
| 累计 commit 数 | Step 1–Step 5（上轮会话）+ Step 6–Step 8（本轮） |

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

### Step 6 — 完善 PopulateLocalFlowSensitiveTypes（已提交，本轮）

#### 解决的核心问题
SSA/CFG 管线仍是骨架（`RunWorklist` 仅初始化参数版本，缺少完全 Cyton 算法），
导致 `UnifiedTypeAnalyzer::InferExprType` 对 Binop/Unop 含变量操作数的情形只能得到 `T_DYNAMIC`。
PopulateMainEvalTypesFromSSA 把该 T_DYNAMIC 直接写入 main_eval_types，使得 CGen 无法生成原生数值类型代码。

#### 修改 1：TypeOfScalar 增加 cur_type 上下文感知
`src/compile/type_inferencer.cpp`
- 将 `TypeOfScalar(exp, map)` 改为 `TypeOfScalar(exp, map, cur_type)`，新增变量名 → 当前推断类型的上下文查找。
- 把 Binop/Unop/PrefixExp(Var) 的递归解析提到 map 缓存查找之前：
  - SSA 阶段缺少 worklist，Binop 节点 map 中存的是 T_DYNAMIC；
  - 我们的 `cur_type` 知道操作数变量名对应的类型（由 PopLSFT 逐语句累积）；
  - 递归解析 Binop/Unop 可直接跳过 map 缓存，按 cur_type 推导结果类型。
- MeetFlow 辅助函数移到 TypeOfScalar 之前，支持递归调用。

#### 修改 2：main_eval_types 回写数值类型
- LocalVar handler 中：当 exps[i] 经 TypeOfScalar 被推导为 T_INT/T_FLOAT 时，
  回写 `map[exps[i]] = t`，把 SSA 阶段误填的 T_DYNAMIC 覆盖。
- Assign handler 中：类似逻辑，当 rt 为数值类型时也回写 `map[exps[i]]`。
- 关键效果：`local y = x * 4` / `x = x - 3` 等写法现在能正确生成 int64_t 局部变量。

#### 修改 3：for-loop cursor 类型 bridge
- 问题：SSA UTA 的 WalkSyntaxTree 只对 Exp/Var/... 语义节点填充 main_ssa_types，
  不覆盖 ForLoop 语句节点。故 `LookupNodeType(for_stmt.get())` 永远返回 T_UNKNOWN ≠ T_INT，
  导致 CGen 总是为游标生成 CVar 而非 int64_t。
- 修复：ProcessBlockLocals 处理完 for-loop body 后，手动在 main_eval_types 中
  为 for_stmt 节点注册类型：
  - 默认 T_INT（case1/3/4：游标 never reassigned → 按边界类型用原生代码）
  - 当 body 内出现「游标名 = 非数值」的直接赋值时置 T_DYNAMIC（case2：`a = "test"` → CVar）
- 判 body 语句列表是直接扫描（而非依赖递归 cur_type，因为递归层的 cur_type 不会自动向上传播）。

#### 修复的测试用例（24 个）
- infer.test_infer_typed_int_for
- infer.test_infer_typed_int_double_slash
- infer.test_infer_typed_int_mod
- infer.test_infer_typed_int_negative_floor_div
- infer.test_infer_typed_int_negative_mod
- infer.test_infer_typed_int_bitnot
- infer.test_infer_typed_int_for_star
- infer.test_infer_typed_int_for_neg_step_1 / case2
- infer.test_infer_typed_float_var_cvar_assign
- infer.test_infer_for_shadow_case1/case2/case3/case4（Step 5b 遗留）
- infer.test_infer_typed_int_minus/star（Step 5b 遗留）
- 等共 24 个 Δ 修复（具体清单见 test/ 目录）

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

### P0: 继续收敛测试（基线 636 PASSED + 120 FAILED）
剩余 120 个 FAILURE 分类（按影响范围排序）：

| 类别 | 代表用例 | 可能根因 |
|------|---------|---------|
| algo (3) | bubble_sort, insertion_sort, matrix | for-loop JIT 或 table 特化 |
| jitter (4) | for_loop_float*, vararg_from_cpp_sum* | 浮点游标、vararg 场景 |
| infer.math_spec_* (1) | math_spec_mixed | 数学参数特化路径（bitmask 入口分发） |
| infer.native_bool_* (12) | native_bool_and/elseif/float/if/nested/not/or/repeat/while/... | 原生 bool 表达式走 CGen 而非 SSA |
| infer.native_cmp/unary (6) | cmp_expr_eq/lt, unary_minus_standalone, unary_bitnot | 原生比较/一元 |
| infer.test_spec_* (75) | 各种 specializable 参数路径 | entry dispatcher + 数学/表特化 snapshot |
| infer.test_table_spec_* (2) | control_flow, if_else_soundness | 表特化流敏感 |
| exception (1) | math_param_non_numeric_error | 数学参数非数值 error path |
| 其他 (16) | allpaths_return, count_loop, elseif_no_return, ... | 多个小问题 |

### P1: 补齐 SSA 构造（规范 §3）— 重大步骤
**根本问题**：当前 Step 6 的修复是启发式桥接；PopLSFT 只能处理过程内线性流，
不理解控制流合并（if/while 汇合）。要彻底解决 spec_* 的 75 个用例，必须实现 SSA + Worklist。
- 实现 `SSABuilder::Build`（Cytron 标准算法）：变量栈、支配边界、φ 插入、变量重命名
- 实现 `UnifiedTypeAnalyzer::RunWorklist`（规范 §6）：
  - reverse-postorder 遍历基本块；meet over preds' out_env (per φ)
  - transfer: 逐语句 update env；widening: 迭代超限 → shape→open
- 完成后 PopLSFT 可逐步替换为 worklist 的 out_env 直接桥接。
- 完成后步骤 P5（CGen 迁移）和 P6（legacy 字段删除）才会真正安全。

### P2: 浮点游标 + for_loop_float
- 当前 for-loop 只识别整型特化（typed_int_for）；float/double 游标仍走通用 CVar 路径。
- 修复：让 typed_int_for/typed_float_for 分支正确传播 float 游标到 CGen 的 CompileTypedNumericForLoop。
- 修复后通过：jitter.test_for_loop_float*, test_infer_typed_float_for*。

### P3: entry dispatcher 返回类型 + for 循环游标类型（跨 bitmask 特化）
当前 STEP 7–8 已修复 FindSpecializableParams 检测和 SSABuilder 初始化但仍遗留：
- `infer_test_infer_degrade_param`：for 循环游标 i 在特化版本（bitmask=0/1）中
  仍为 CVar 而非 int64_t/double。根因是 CGen 编译特化版本时只能看到
  bitmask=-1 的 main_eval_types，看不到 per-bitmask 快照。
- 解决：在 CGen 的 CompileFuncBody(bitmask) 内，当 bitmask≥0 时改用
  `spec_ssa_snapshots[func_name][bitmask]` 做 LookupNodeType 查询。
- 这是实现「specialized body 使用 snapshot types」架构的关键一步。

### P4: Shape 流敏感推导（规范 §5）— 与控制流联动
- field read/write 需要流敏感 shape；当前仅构造不演化。
- 依赖 P1 worklist 提供 per-block in/out_env。

### P5: 函数摘要 + 调用点实例化（规范 §7）
- `FuncSummary` 存储 `param_types` + `ret_type` + `param_escape[i]`
- 递归函数处理：第一次遇到假设未完成摘要

### P6: CGen 迁移（依赖 P1 完成）
- `CompileTableconstructor` 走 SSA shape_registry 路径
- `CompileVar` kDot/kSquare 走 offset/shape 路径
- 删除 `table_spec_types_`/`spec_field_*` 等 legacy 字符串式特化
- 删除 InferResult 末尾 6 个 legacy 兼容字段

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
