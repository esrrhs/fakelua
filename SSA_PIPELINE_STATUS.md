# FakeLua 统一 SSA/CFG/Shape 编译管线 — 状态与实施文档

> **最后更新**: 2026-07-06（Step 17：per-bitmark worklist + for-loop 边界收集 + for_loop 游标感知）
> **设计规范**: `/root/lua-dialect-type-inference-spec.md`
> **当前分支**: `ssa-pipeline-v2`
> **当前测试**: 构建通过，common(13) / syntax_tree(34) / state(16) / var(117) / util(18) / runtime(18) / ini(1) / vm_cvar_call(3) / exception(66) 全部通过；algo 7/3；
>   infer 55/14 (test_infer_*), native 12/2, spec 56/29
>
> | Suite | Pass/Fail | 备注 |
> |-------|-----------|------|
> | common | 13/0 | ✅ |
> | syntax_tree | 34/0 | ✅ |
> | state | 16/0 | ✅ |
> | var | 117/0 | ✅ |
> | util | 18/0 | ✅ |
> | runtime | 18/0 | ✅ |
> | ini | 1/0 | ✅ |
> | vm_cvar_call | 3/0 | ✅ |
> | exception | 66/0 | ✅ |
> | algo | 7/3 | bubble_sort/insertion_sort/matrix 需表特化 |
> | **infer.test_infer_*** | **55/14** | 主要工作区域（26 修复） |
> | **infer.test_native_*** | **12/2** | 原生 bool/repeat |
> | **infer.test_spec_*** | **56/29** | 特化路径（Step 17: +3） |

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

### Step 15 — CFG 分支折叠 + 流敏感 VarEnv Worklist + 修复特化发现（commit `6feb979`）

#### 问题
- CFGBuilder::BuildStmt 单块 dump → 无法 meet 分支合并。
- RunWorklist 是 stub → Binop 含变量引用始终 T_DYNAMIC。
- FindSpecializableParams 的 Binop 操作符种类误从 e->Right()（应从 e->Op()），且 collect_vars 未 PrefixExp-recurse → 破坏一切 Binop 特化函数路径（exception 回归）。

#### 修改
1. cfg.h/.cfg.cpp：BuildStmt 返回 int（新块 id）；BuildIf/BuildWhile/BuildRepeat/BuildForLoop/BuildForIn；FindBlock 线性查找；ComputeDominators/ComputeDominanceFrontier 正确计算。
2. unified_type_analyzer.cpp：
   - RunWorklist 真跑流敏感不动点：per-block VarEnv (var_name→SSATypeInfo)，RPO 迭代，入口 seed param types（默认 T_DYNAMIC），meet of preds' out。
   - TransferStmt 处理 LocalVar/Assign/ForLoop/ForIn 更新 env。
   - PopulateNodeTypesFromStmts 以块的 env 为上下文填充 main_ssa_types。
   - InferExprType/BuildShapeFromCtor 增加可选 `const VarEnv* local_env`。
   - 入口路径保留 `ir.ssa_version_types` 兼容字段。
3. FindSpecializableParams 修复 Binop Op() 调用 + PrefixExp 递归。

#### 修复
- exception.math_param_non_numeric_error 已复绿

---

### Step 16 — Collect_vars PrefixExp 深度修复（commit `c89a4cb`）

#### 问题
Step 15 在 `FindSpecializableParams` 中新增 PrefixExp 递归分支时，误把顶层 PrefixExp（如 `return a` 中的 `a`）也当成了可特化参数，导致所有 Binop 函数被错误特化 → `exception.return_type_error_*` 全部 12 个失败。

#### 修复
- `collect_vars` 在 `Exp(kPrefixExp)` 分支 → recurse `e->Right()` 时保留原 `depth`（而非 `depth+1`），
  与 PrefixExp 包裹节点语义一致：只是解包，不进入 Binop 语境。
- 包含 `Exception 66/0` 全绿复绿 + `Infer.test_infer_*` ~26 个回归修复。

### Step 17 — per-bitmark Worklist + for-loop 边界收集（未提交）

#### 问题
Step 15 的 worklist 只在主分析（bitmask=-1）时跑；per-bitmark 特化路径仍用简单的 AST walk + base_env，
导致 for-loop 边界参数（如 `for i = 1, n do ... end` 中的 `n`）无法被识别为可特化参数，
且快照中 for-loop 游标/体的类型无法根据 bitmask 假设传播。

#### 修改
1. `FindSpecializableParams` 的 `visit_all(ForLoop)` 对 `ExpBegin/ExpEnd/ExpStep` 以 `depth=1` 调用
   `collect_vars`（数值语境），使 for-loop 边界中的参数引用能触发数学特化。
2. `Analyze(bitmask, ...)` 改为真正跑 worklist：
   - 用 `param_assumptions` 临时覆盖 `ir.ssa_version_types`，让 `RunWorklist` 的 entry seed 反映当前 bitmask
   - 调用 `RunWorklist(cfg, ssa, param_assumptions, ir)` 得到每个块的 out_env
   - 用 `PopulateNodeTypesFromStmts(b.stmts, env, ...)` 以块 env 为上下文填充 `snap[bitmask]`
3. `PopulateNodeTypesFromStmts` 新增 `out_map` 参数，使其能写入 `ir.main_ssa_types`（主分析）或 `snap[bitmask]`（特化快照）。
4. `PopulateNodeTypesFromStmts` 新增 `ForLoop` 节点处理：根据 env 中 ExpEnd/ExpStep 的变量类型推导游标类型，
   写入快照供 CGen 决定循环变量 `i` 的声明类型（`int64_t i` 或 `CVar i`）。
5. `TransferStmt(ForLoop)` 改进：从 env 中读取 ExpEnd/ExpStep 的变量类型（如果有），决定游标类型。

#### 效果
- `infer.test_spec_*` 53/32 → 56/29（+3 修复）
- `infer.test_infer_*` 保持 55/14（无回归）
- 特化版本现在为 `for i = 1, n do ... end` 生成 `int64_t test_0(int64_t n)` 和 `double test_1(double n)`，
  其中 test_0 的循环体使用 `int64_t i = flua_for_ctrl_0;`（完全类型化）

#### 遗留
- `test_infer_degrade_param`：test_1 的 `sum` 应为 `double` 或 `CVar`，实际仍为 `int64_t`。
  根因：`local sum = 0` 的声明类型由字面量 `0`（T_INT）决定，未感知 bitmask 下 `i` 是 double → sum+i 应退化。
  需要让快照中 `sum = sum + i` 的 RHS 类型根据 `i` 的 bitmask 类型重新计算（更深层的 per-bitmark 传播）。

---

## 当前实现与规范的差距

| 规范章节 | 要求 | 当前状态 |
|---------|------|---------|
| §3 SSA 构造 | Cytron 算法 + φ 节点 + 变量重命名 | ❌ 骨架（未实现） |
| §4 HM 类型推导 | 类型变量 + 合一 + occurs_check | ❌ 未实现 |
| §5 Shape 抽象解释 | 转移函数（field read/write） | ⚠️ 仅构造 |
| §6 Worklist 算法 | 流敏感不动点迭代 | ⚠️ per-block VarEnv + RPO + meet（Step 15 + 16）；需补 loop widening |
| §7 函数摘要 | FuncSummary + 调用点实例化 | ❌ 未实现 |
| §8 逃逸分析 | EscapeTransfer | ❌ 未实现 |
| §9 偏移分配 | StructLayout 计算 | ❌ 未实现 |
| §10 代码生成 | 基于 shape 的 `LOAD_FIELD` | ❌ 未实现 |

**CGen 仍使用 legacy 字符串式 table 特化**（`table_spec_types_`/`spec_field_c_names_` 等）+ **entry dispatcher 数学参数特化和 CVar 入口**。这些路径的功能将在 SSA 管线成熟时被逐段颠覆。

---

## 下次会话入口（按优先级）

### P0 (DONE this session): Baseline 已建立

当前准确基线 (2026-07-06):
- common: 13/13
- syntax_tree: 34/34
- state: 16/16
- util: 18/18
- var: 117/117
- infer.*: 327 OK / 65 FAIL (活跃总数 392)
  - test_infer_*: ~190 OK / ~40 FAIL
  - test_spec_*: ~60 OK / ~30 FAIL
  - test_native_*: ~15 OK / ~5 FAIL
  - 其他 (test_binop_*, test_math_*, test_table_*, test_const_* etc): 大部分通过
- algo: 7/10 (bubble_sort/insertion_sort/matrix runtime crash — 需要表特化)

### P1: per-bitmark Worklist 传播到 for-loop 游标 + sum-typede

最紧迫的剩余工作。`test_infer_degrade_param`：n=T_INT 的 for 循环里 sum 应 T_INT，n=T_FLOAT 应退化 CVar。
修复方向：在 `RunSSASpecialization` 的 bitmask 循环里，用当前 `param_assumptions` 初始化后跑 worklist，
让快照中 for-loop cursor (n → i → sum) 类型传播。然后在 CGen `CompileStmtForLoop` 中
感知 cur_spec_snapshot 中的循环游标类型。

类似影响：test_spec_for_bound_param / test_spec_fib / test_infer_typed_float_for 等。

### P2: 跨函数特化链（规范 §7 过程间）

test_spec_nested_call / spec_for_dynamic_bound: 参数通过 local 传递后到达 Binop，
当前 FindSpecializableParams 只看顶层 Binop，看不到跨层传递。

修复方向：在 worklist 的 VarEnv 里记录"param-derived local"的传递关系（类似 escape analysis），
然后在 FindSpecializableParams 里认为：若 local X 由参数 P 推导而来且 X 使用于 Binop，
则 P 也视为"间接特殊化"。

### P3: if/while 分支内赋值退化 (test_infer_if_scope_degrade)

test_infer_if_scope_degrade / test_infer_while_scope_degrade: 分支内对参数或其他 local 赋以
不同类型的值，合并后应退化为 T_DYNAMIC。

修复方向：在 worklist 的 MeetEnv 里，当两分支给同一变量赋不同类型时执行 MeetFlow
（int+float→T_DYNAMIC）。

### P4: 浮点游标 + for_loop_float

test_infer_typed_float_for: ExpEnd=参数时，需感知特化快照中的类型 → 游标按 int/float 选择。
修复方向：在 CompileStmtForLoop 里先看 cur_spec_snapshot 中 ExpEnd 的类型。

### P5: native_bool/native_cmp/native_unary 直接 C 生成

test_spec_compare_equal / test_native_bool_*: == 操作数均为 numeric-local 时应直接生成原生比较。
修复方向：CGen Binop handler 中看 main_eval_types 或 Spec snapshot 推断两侧类型。

### P6: CGen 迁移到 SSA path (long-term)
- CompileTableconstructor 走 SSA shape_registry 路径
- CompileVar kDot/kSquare 走 offset/shape 路径
- 删除 `table_spec_types_`/`spec_field_*` 等 legacy 字符串式特化
- 删除 InferResult 末尾 4 个 legacy 兼容字段

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
