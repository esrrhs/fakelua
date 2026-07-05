# FakeLua 统一类型推导管线 — 实施进度文档

> 最后更新：2026-07-01
> 本文档记录了项目当前状态、失败测试分析、已完成和待完成的修改，供下次会话继续开发。

---

## 1. 项目概述

FakeLua 是一个 Lua 子集 JIT 编译器，编译流水线：
```
Lua源码 → Lexer/Parser → AST → PreProcessor → SemanticAnalysis → TypeInferencer → CGen → TCC/GCC JIT
```

当前有两套类型推导系统并存：
1. **Legacy 路径**：`TypeInferencer` 的流不敏感单遍遍历 + 试推断（`RunTrialInference`）
2. **SSA 路径**：`CFGBuilder` → `SSABuilder` → `UnifiedTypeAnalyzer`（基于 worklist 数据流分析）

两套系统数据源不一致，CGen 混合读取两者，导致 **34 个测试失败**（已修复 1 个编译错误后变为 44 个，下文说明）。

## 2. 当前测试状态

### 构建
```bash
cmake --build build --target unit_tests --parallel 1  # 必须 -j1，否则 OOM
cd build/bin && ./unit_tests
```

### 最新测试结果（44 FAILED）
- 总测试数：756
- 通过：712
- 失败：44

### 失败分类

| 类别 | 数量 | 测试名 | 根因分析 |
|---|---|---|---|
| **变量退化失效** | 5 | test_infer_mutation, test_infer_assign_degraded_var, test_infer_return_stale_type, test_infer_do_shadow_typed_over_dynamic, test_infer_while_scope_degrade | SSA 流敏感推导正确检测到退化(T_DYNAMIC)，但 CGen 声明变量时使用 QueryTypeInfo 读 SSA → 如果 SSA 说 T_DYNAMIC 则声明 CVar（正确），但当前 QueryTypeInfo 的 fallback 允许 legacy T_INT 覆盖 SSA T_DYNAMIC |
| **数学特化返回类型** | 8 | test_spec_fib, test_spec_reassign_gcd, test_spec_reassign_powmod, test_spec_for_bound_param, test_spec_do_return, test_spec_repeat_arith, test_spec_for_in_body, test_spec_bare_return | SSA `spec_ssa_return_types` 返回 T_DYNAMIC（乐观初始值未收敛到正确值），CGen GetSpecReturnType fallback 到 legacy 后返回正确类型但签名仍写 CVar |
| **test_spec_repeat_local_until 等** | 4 | test_spec_repeat_local_until, test_spec_assign_nonnumeric_int_throws, test_spec_for_dynamic_bound, test_spec_forloop_int_float_degrade | 特化体内 SSA 未正确推导循环变量和相关表达式类型 |
| **test_spec_ternary** | 1 | test_spec_ternary | 三元模式(n > 0 and a or b) 的类型推导 |
| **test_math_spec_mixed** | 1 | test_math_spec_mixed | 数学库特化混合参数 |
| **Table spec** | 5 | test_global_table_spec, test_table_spec_empty, test_table_spec_control_flow, test_table_spec_if_else_soundness | CGen 的 table_spec_types_ 系统与 SSA shape_registry 未集成 |
| **异常检测** | 4 | function_param_duplicate, const_define_duplicate, const_define_func_param_duplicate, global_duplicate_lvalue_error | 预处理器的 AddLocal 会将重复参数名重命名为 shadow_N，掩盖了 SemanticAnalysis 的重复检测 |
| **Jitter 端到端** | 12 | test_binop_and, test_binop_or, test_complete, dup/shadow errors, spec_call_*, bitwise_expr_as_cvar, dynamic_for_loop_bounds | 上游类型信息错误导致 CGen 生成错误代码 |

## 3. 已完成的修改

### 3.1 修复 type_inferencer.cpp 编译错误
**文件**：`src/compile/type_inferencer.cpp:385-386`

**问题**：多余的 `}` 提前关闭了 SSA 分析块的 scope，导致后续代码（不动点迭代循环）在 scope 外引用 `func_infos` 和 `uta` 变量失败。

**修复**：
```cpp
// 改前（第385行）：
            } }
        }
// 改后：
            }
        }
```
删除了多余的一个 `}`，使 SSA 分析块（含两步不动点迭代）完整包含在同一个 scope 内。

### 3.2 修改 QueryTypeInfo（当前状态需要调整）
**文件**：`src/compile/c_gen.cpp:708-767`

**当前修改**：将 QueryTypeInfo 改为"SSA 有明确结论就信任 SSA，不回退 legacy 数值类型"。

**问题**：过于激进 — 在特化体内，SSA 的 `InferExprType` 尚未完善，对 `n - 1` 这样的算术表达式返回 T_DYNAMIC，而 legacy 能正确推导为 T_INT/T_FLOAT。导致 44 个失败（比原来 34 个多了 10 个回归）。

**决策**：用户要求**去掉 legacy 系统**，所以正确策略是先增强 SSA InferExprType 使其完全覆盖 legacy 功能，然后再删除 legacy，而非做 fallback。

## 4. 关键代码路径分析

### 4.1 SSA InferExprType 的不足（需要增强）

当前 `UnifiedTypeAnalyzer::InferExprType`（`unified_type_analyzer.cpp:97-300`）对以下场景返回 T_DYNAMIC：

1. **变量引用 kSimple**：通过 `ssa.use_versions` 查 `version_types` — 如果运行 worklist 时该版本类型是 T_DYNAMIC（未收敛或参数假设未传播），则返回 T_DYNAMIC
2. **二元运算 kBinop**：调用 `InferNumericBinopResultType` — 依赖操作数类型
3. **函数调用**：查 `ir.specializable_params` 和</think><tool_call>Write<arg_key>content</arg_key><arg_value># FakeLua 统一类型推导管线 — 实施进度文档

> 最后更新：2026-07-01
> 本文档记录项目当前状态、失败测试分析、已完成和待完成的修改，供下次会话继续开发。

---

## 1. 项目概述

FakeLua 是一个 Lua 子集 JIT 编译器，编译流水线：

```
Lua源码 → Lexer/Parser(AST) → PreProcessor(AST改写) → SemanticAnalysis → TypeInferencer → CGen → TCC/GCC JIT
```

当前有两套类型推导系统并存：

1. **Legacy 路径**：`TypeInferencer` 的流不敏感单遍遍历 `InferNode/InferExp` + 试推断 `RunTrialInference`，生成 `main_eval_types`/`specialization_snapshots`/`math_param_positions`/`table_spec_infos`/`specialization_return_types`
2. **SSA 路径**：`CFGBuilder` → `SSABuilder` → `UnifiedTypeAnalyzer`，生成 `main_ssa_types`/`spec_ssa_snapshots`/`specializable_params`/`spec_ssa_return_types`/`shape_registry`/`ssa_version_types`/`node_ssa_version`

两套系统数据源不一致，CGen 混合读取两者，导致 **34 个测试失败**。

**用户决策**：直接去掉 Legacy 数值推导系统，增强 SSA 路径使其完全覆盖 Legacy 功能，最终统一为 SSA 驱动 CGen。

---

## 2. 当前测试状态

### 构建命令
```bash
cmake --build build --target unit_tests --parallel 1  # 必须 -j1，否则 OOM
cd build/bin && ./unit_tests
```

### 原始失败（修复编译错误后）：34 个
修复 `type_inferencer.cpp` 编译错误后，34 个测试失败。

### 当前失败：44 个（因 QueryTypeInfo 改动导致 10 个回归，需要恢复后再走正确路径）

---

## 3. 失败测试根因分类

### 3.1 变量退化/流敏感失效（5 个）
- `test_infer_mutation` — `state = "error"` 退化到 T_DYNAMIC，应声明 CVar
- `test_infer_assign_degraded_var` — `sum = "done"` 退化
- `test_infer_return_stale_type` — `x = "modified"` 退化影响 return
- `test_infer_do_shadow_typed_over_dynamic` — do block 内的遮蔽语义
- `test_infer_while_scope_degrade` — while 循环内退化

**根因**：SSA 流敏感推导已正确检测退化（phi meet 后 T_DYNAMIC），但 CGen 声明变量时读 legacy `main_eval_types`（流不敏感，仍为 T_INT）。需要让 CGen 完全用 SSA 类型。

### 3.2 数学特化返回类型（8 个）
- `test_spec_fib` — 返回类型应为 int64_t 但 CGen 写 CVar
- `test_spec_reassign_gcd`、`test_spec_reassign_powmod`
- `test_spec_for_bound_param`、`test_spec_do_return`
- `test_spec_repeat_arith`、`test_spec_for_in_body`
- `test_spec_bare_return`

**根因**：SSA `spec_ssa_return_types` 初始填充 T_DYNAMIC，3 轮不动点迭代的语义是：每轮用上一轮的返回类型假设重新分析，逐步收敛。但当前实现可能未正确传播返回类型更新。

### 3.3 特化体内类型推导缺失（4 个）
- `test_spec_repeat_local_until`、`test_spec_assign_nonnumeric_int_throws`
- `test_spec_for_dynamic_bound`、`test_spec_forloop_int_float_degrade`

### 3.4 三元模式（1 个）
- `test_spec_ternary` — `(cond and val1) or val2` 模式推导

### 3.5 数学库特化（1 个）
- `test_math_spec_mixed` — math.* 函数调用特化

### 3.6 Table 特化（5 个）
- `test_global_table_spec`、`test_table_spec_empty`
- `test_table_spec_control_flow`、`test_table_spec_if_else_soundness`

**根因**：CGen 有独立的 `table_spec_types_` 系统（Phase 1/2 join 逻辑），与 SSA `shape_registry` 完全脱节。需要统一。

### 3.7 异常检测（4 个）
- `function_param_duplicate`、`const_define_duplicate`
- `const_define_func_param_duplicate`、`global_duplicate_lvalue_error`

**根因**：`PreProcessor::AddLocal` 在检测到重复名时自动创建 `_shadow_N` 后缀，而不是报错。重复参数检查仅在 `SemanticAnalysis::CheckParList` 和 `CGen::CompileParList` 中运行，但预处理器在它们之前就把名字改了。

### 3.8 Jitter 端到端（12 个）
上游类型错误导致的连锁失败，修复上游后应自动通过。

剩余算法测试 `algo.insertion_sort` 也需要验证。

---

## 4. 已完成的修改

### 4.1 修复 type_inferencer.cpp 编译错误 ✅
**文件**：`src/compile/type_inferencer.cpp:385`

**问题**：多余的 `}` 提前关闭了 SSA 分析块 scope。

**修复**：
```cpp
// 改前（第385行多余一个}）：
            } }
        }
// 改后：
            }
        }
```

### 4.2 QueryTypeInfo 修改（需要回退重做）⚠️
**文件**：`src/compile/c_gen.cpp:708-767`

**当前状态**：改为"SSA 有结论就信任 SSA，不回退 legacy"。但这导致回归，因为 SSA InferExprType 在特化体内对算术表达式返回 T_DYNAMIC。

**正确策略**：先增强 SSA 使其完全覆盖 legacy，再删除 legacy fallback。需要先回退此修改。

---

## 5. 待实施方案（按优先级排序）

### Step 1：回退 QueryTypeInfo 的激进修改，恢复原来的 fallback 逻辑
恢复到原始的 fallback 结构（SSA shape 优先 → legacy numeric 优先 → SSA fallback → legacy non-numeric），确保回到 34 个失败的基线。

### Step 2：增强 SSA InferExprType — 关键步骤
增强 `UnifiedTypeAnalyzer::InferExprType`（`unified_type_analyzer.cpp:97-300`），使其在特化体内正确推导数值类型：

**2a. 变量引用增强**
- `kSimple` 变量：当前查 `use_versions → version_types`。问题在于 worklist 运行完毕后，`version_types` 中的类型可能是 T_DYNAMIC（默认值），因为 UTA 的 TransferStmt 不覆盖所有赋值场景。
- 需要确保 TransferStmt 中所有赋值路径都能正确更新 `version_types`：
  - `ForLoop` 循环变量的处理（已实现但可能不完整）
  - 函数返回值赋值（`local x = f()` 需要能推导 f 的返回类型）
  - 全局变量引用（应返回 T_DYNAMIC）

**2b. 二元运算增强**
- 确保当操作数类型为 T_INT/T_FLOAT 时，算术运算返回正确的结果类型
- 需要区分"不知道类型"（T_UNKNOWN）和"动态类型"（T_DYNAMIC）
  - T_UNKNOWN：尚未推导，不应该出现在最终结果
  - T_DYNAMIC：明确推导为动态（退化或无法确定）

**2c. 函数调用增强**
- 特化函数调用：查 `spec_ssa_return_types[callee][bitmask]`
- 非特化函数调用：默认 T_DYNAMIC
- 递归函数：使用不动点迭代已收敛的类型

**2d. TransferStmt 增强**
- 覆盖 `Repeat-Until`（当前缺失）
- 覆盖 `Return`（收集返回类型）
- 覆盖嵌套函数定义的处理

**2e. 不动点迭代增强**
- 当前 3 轮 pass 可能不够，需要检查收敛条件
- 确保 `spec_ssa_return_types` 在每轮迭代后更新并传播到调用点

### Step 3：修复预处理器的重复参数检测
**文件**：`src/compile/preprocessor.cpp:120-126`

在 `AddLocal` 中，当检测到当前 scope 已有同名参数时，需要在进入函数参数列表之前检查重复：
- 在处理函数参数列表时，逐个调用 `AddLocal` 之前先检查参数列表内是否有重复名
- 或者：在 `Process` 处理 ParList 节点时，先检查 `namelist` 内部有无重复名，有则直接 `ThrowFakeluaException`

### Step 4：CGen 迁移到 SSA-only 查询
当 SSA InferExprType 完善后：
- `QueryTypeInfo` 删除所有 legacy fallback
- `CompileStmtLocalVar` 直接用 `QueryTypeInfo` 获取初始化表达式类型
- `CompileStmtForLoop` 循环变量类型从 `QueryTypeInfo` 获取
- 删除 `SaveKeySpecSnapshot/RestoreKeySpecSnapshot/JoinKeySpecSnapshots`
- Table 特化统一使用 SSA `shape_registry` 而非 `table_spec_types_`
- 字段访问统一通过 `QueryTypeInfo → shape_id → registry → field layout → FL_SPEC`

### Step 5：删除 Legacy TypeInferencer 代码
**文件**：
- `src/compile/type_inferencer.cpp` — 删除 `InferNode/InferExp/InferVar/InferBlock/InferLocalVar/InferAssign/InferForLoop/InferForIn/InferWhile/InferRepeat/InferIf`，删除 `IdentifyMathParams/GenerateInitialSnapshots/RunTrialInference/InferSpecializationReturnTypes`，删除 `TypeEnvironment` 类等
- `src/compile/type_inferencer.h` — 删除对应声明
- `src/compile/compile_common.h` — 删除 legacy InferResult 字段：`math_param_positions`、`specialization_snapshots`、`specialization_return_types`、`main_eval_types`、`global_const_vars`、`table_spec_infos`；删除 `SpecParam.is_shape`

`InferTypes` 函数简化为纯 SSA 路径（构建 CFG → SSA → UTA 分析 → 不动点迭代）。

### Step 6：清理 CGen
- 删除 `table_spec_types_`/`global_table_spec_types_`/`spec_field_names_`/`spec_field_indices_`/`spec_field_c_names_`/`spec_field_types_`/`generated_spec_typedefs_`
- 删除 `SpecSnapshot`/`SaveKeySpecSnapshot`/`RestoreKeySpecSnapshot`/`JoinKeySpecSnapshots`/`ComputeSpecTypeName`
- `CompileStmtIf` 中删除快照 save/restore/join
- `CompileFuncBody` 中 `math_param_positions` 替换为从 `specializable_params` 派生的辅助函数 `MathParamIndicesOf`

---

## 6. 关键代码路径参考

### SSA 分析核心路径
- CFG 构建：`src/compile/cfg.h/.cpp`（已完成，无需修改）
- SSA 构建：`src/compile/ssa.h/.cpp`（已完成，无需修改）
- Shape 注册：`src/compile/shape_type.h`（已完成，无需修改）
- **UTA 分析**：`src/compile/unified_type_analyzer.cpp`（需要加强，核心工作区）
  - `InferExprType`（97-300 行）：表达式类型推导 — 需增强
  - `TransferStmt`（412-665 行）：语句转移函数 — 需增强
  - `RunWorklist`（682-758 行）：数据流主循环 — 可能需要调整
  - `Analyze`（871-989 行）：主入口 — 可能需要调整
  - `FindSpecializableParams`（994-1047 行）：已正确（只返回 math）

### CGen 核心路径
- `QueryTypeInfo`（`c_gen.cpp:708-767`）：类型查询入口 — 需简化
- `GetKeySpecReturnType`（`c_gen.cpp:791-785`）：特化返回类型 — 需简化
- `CompileFuncBody`（`c_gen.cpp:1085-1145`）：特化函数编译 — 需适配
- `CompileStmtLocalVar`（`c_gen.cpp:1482-1620`）：变量声明 — 需切换到 SSA
- `CompileStmtForLoop`（`c_gen.cpp:1720+`）：循环变量 — 需切换到 SSA
- `CompileTableconstructor`：表构造 — 需统一到 shape_registry
- `CompileVar kDot/kSquare`：字段访问 — 需统一到 shape_registry
- `CompileStmtIf 快照逻辑`（多处）：需删除

### Legacy 代码（待删除）
- `src/compile/type_inferencer.cpp` 中约 1500 行代码
- `src/compile/type_inferencer.h` 声明
- `src/compile/compile_common.h` 中 legacy 字段

### 预处理器
- `src/compile/preprocessor.cpp:120-126`：`AddLocal` 的 shadow 重命名逻辑需修改

---

## 7. 验证步骤

每完成一个 Step 后：
```bash
cmake --build build --target unit_tests --parallel 1
cd build/bin && ./unit_tests 2>&1 | tail -5
```

最终目标：756+ 全绿，0 FAILED。

特别注意：
- 构建**必须** `--parallel 1`，否则 cc1plus OOM 被 kill
- 测试**必须**从 `build/bin/` 运行，因为测试脚本用相对路径 `./infer/xxx.lua`

---

## 8. 设计文档参考

- 主规范：`/root/lua-dialect-type-inference-spec.md`（完整的 SSA+CFG+Shape 类型推导系统设计）
- 前一 AI 的计划：`/root/.codebuddy/plans/electric-thunder-curie.md`（部分过时，但根因分析仍有价值）
