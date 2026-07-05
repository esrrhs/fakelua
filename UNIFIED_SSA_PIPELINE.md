# FakeLua 统一 SSA/CFG/Shape 编译管线 — 完整状态与实施计划

> 最后更新：2026-07-02（本次会话）
> 核心目标：用专业的 SSA + CFG + Shape 抽象解释管线，完全替换旧的数学参数推导和 table 特化系统。

---

## 1. 项目概述

FakeLua 是一个 Lua 子集 JIT 编译器，把 Lua 编译成 C 代码再经 TCC/GCC JIT 执行。当前有两套类型推导系统并行运行，数据源不一致导致 60 个推断测试和 4 个异常测试失败。

### 1.1 当前编译流水线

```
Lua 源码
  ↓ flex/bison → AST (ParseResult{file_name, chunk})
  ↓ PreProcessor::Process（AST 原地改写，包括 shadow 重命名）
  ↓ SemanticAnalysis::Analyze → AnalysisResult
  ↓ TypeInferencer::InferTypes → InferResult  ← 双路径并存的问题所在
  ↓ CGen::Generate → GenResult{c_code, ...}
  ↓ TccJitter / GccJitter
```

### 1.2 两套类型推导系统

#### Legacy 路径（旧）
- **文件**：`type_inferencer.cpp/h` 中的 `InferNode/InferExp/InferBlock/...` 系列函数
- **算法**：流不敏感单遍 AST 遍历 + 试推断 (`RunTrialInference`)
- **输入**：AST
- **输出字段**（`compile_common.h`）：
  - `math_param_positions`：函数名 → math 参数下标列表
  - `specialization_snapshots`：函数名 → `vector<EvalTypeSnapshot>`（2^math 个）
  - `specialization_return_types`：函数名 → `vector<InferredType>`（2^math 个）
  - `main_eval_types`：节点指针 → InferredType
  - `global_const_vars`：文件级常量名 → InferredType
  - `table_spec_infos`：ctor 节点 → TableSpecInfo

#### SSA 路径（新，前一 AI 已完成基设）
- **文件**：`cfg.cpp/h`、`ssa.cpp/h`、`shape_type.h`、`unified_type_analyzer.cpp/h`
- **算法**：Cytron SSA 构造 + Worklist 数据流分析 + Shape 抽象解释
- **输入**：CFGFunction + SSAFunction
- **输出字段**（`compile_common.h` 新增字段）：
  - `ssa_version_types`：SSA 版本号 → SSATypeInfo{type, shape_id}
  - `node_ssa_version`：AST 节点 → SSA 版本号
  - `shape_registry`：shared_ptr\<ShapeRegistry\>
  - `specializable_params`：函数名 → vector\<SpecParam\>{param_index, is_math, is_shape}
  - `main_ssa_types`：node → SSATypeInfo（非特化主快照）
  - `spec_ssa_snapshots`：函数名 → vector\<map\<node, SSATypeInfo\>\>（2^(math+shape) 个）
  - `spec_ssa_return_types`：函数名 → vector\<SSATypeInfo\>

### 1.3 当前桥接机制

`TypeInferencer::InferTypes` 已经切换为纯 SSA 管线入口，调用 `PopulateLegacyFields` 将 SSA 结果转写为 legacy 字段供 CGen 消费。CGen 的 `QueryTypeInfo` 先查 SSA 快照，找不到则 fallback 到 legacy 字段。

### 1.4 根因：为什么 60 个测试仍然失败

1. **SSA 推导精度不足**：UTA 的 `InferExprType` 和 `TransferStmt` 对许多场景返回 T_DYNAMIC（如嵌套函数调用返回值、字段赋值 shape 变异、repeat-until 等），而 legacy 管线能正确推导
2. **特化返回类型未收敛**：不动点迭代 3 轮不够，`spec_ssa_return_types` 初始填充 T_DYNAMIC，3 轮后仍是 T_DYNAMIC，导致 CGen 写 CVar 签名
3. **bitmask 数量不匹配**（已在上一轮修复）：`specializable_params` 曾包含 shape 参数，使 snapshot 数量为 2^(math+shape) 而 CGen 按 2^math 索引 → 越界。现已修复为只包含 math 参数
4. **PreProcessor shadow 重命名掩盖重复参数**：`AddLocal` 自动添加 `_shadow_N` 后缀，导致 SemanticAnalysis 的重复参数检查失效
5. **table_spec_types_ 与 shape_registry 双轨**：CGen 有独立的 table spec 快照/恢复/join 机制，与 SSA 的 phi meet 完全脱节

---

## 2. 当前代码状态详细分析

### 2.1 已正常工作（不要破坏）

| 模块 | 文件 | 状态 |
|------|------|------|
| CFG 构造 | `cfg.cpp/h` | 完整，处理 if/elseif/else/while/repeat/for/for-in/break/return/goto，pred/succ 双向正确 |
| SSA 构造 | `ssa.cpp/h` | Cytron 算法完整，phi placement + 变量重命名均工作 |
| ShapeRegistry | `shape_type.h` | Intern/Meet/Widen 均实现，已集成到 UTA |
| UTA worklist | `unified_type_analyzer.cpp` | 收敛、phi meet、loop header widening 均工作 |
| UTA transfer | `unified_type_analyzer.cpp` | 字面量、table 构造、字段读取、二元运算、math.* 调用 |
| TypeInferencer 入口 | `type_inferencer.cpp` | 已切换为纯 SSA 路径，PopulateLegacyFields 桥接正常 |
| 构建 | CMake | `cmake --build build --target unit_tests --parallel 1` 成功 |
| 测试 | 756 tests | 696 PASSED, 60 FAILED (infer), 4 FAILED (exception), 1 FAILED (algo) |

### 2.2 SSA InferExprType 覆盖缺口

当前 `UnifiedTypeAnalyzer::InferExprType`（`unified_type_analyzer.cpp:97-300`）对以下场景返回 T_DYNAMIC：

| 场景 | 代码位置 | 影响 |
|------|---------|------|
| 变量引用 kSimple：通过 use_versions→version_types 未命中时 | 172-180 行 | 所有未被 worklist 收敛到的变量引用 |
| 字段赋值后重新读取 | TransferStmt Assign 分支 | shape 变异后未更新后续版本 |
| 函数调用返回值（非 math.* 和非特化函数） | 218-288 行 | 嵌套调用、跨函数传递 |
| Repeat-Until 循环体和退出条件 | 缺失专门处理 | 循环变量类型 |
| ForIn 循环迭代器推断 | 576-651 行 | 仅支持 ipairs/pairs 简单模式 |
| and/or 的 meet 语义 | 139-149 行 | 仅数值场景，非数值返回 T_DYNAMIC |
| 三元模式 (cond and a or b) | 未专门处理 | test_spec_ternary 失败 |

### 2.3 TransferStmt 覆盖缺口

| 语句类型 | 当前状态 | 影响 |
|---------|---------|------|
| Block | 递归处理 ✅ | — |
| LocalVar | 完整处理 ✅ | — |
| Assign (kSimple) | 完整处理 ✅ | — |
| Assign (kDot/kSquare) | 已实现 shape 变异 ✅ | 但 SSA 版本更新不完整 |
| ForLoop | 完整处理 ✅ | — |
| ForIn | 已实现 ipairs/pairs ✅ | 复杂迭代器仍 T_DYNAMIC |
| LocalFunction | 标记为 T_DYNAMIC ✅ | — |
| Repeat-Until | **缺失** | 需要处理循环体和退出条件 |
| Return | 仅在 Analyze 中收集 | TransferStmt 中缺失 |
| If/While | 由 CFG builder 处理 ✅ | 但 SSA phi 可能未关联 |

### 2.4 CGen 中的双轨问题

CGen 同时消费以下数据源：

| 数据源 | 来源 | 说明 |
|--------|------|------|
| `ir().math_param_positions` | Legacy | 特化函数签名分配 |
| `ir().specialization_snapshots` | Legacy → 由 PopulateLegacyFields 填充 | 特化体内类型查询 |
| `ir().main_eval_types` | Legacy → 由 PopulateLegacyFields 填充 | 非特化类型查询 |
| `ir().table_spec_infos` | Legacy → 由 BuildTableSpecInfosFromShapes 填充 | table 构造特化 |
| `ir().spec_ssa_snapshots` | SSA | QueryTypeInfo 主路径 |
| `ir().main_ssa_types` | SSA | QueryTypeInfo 非 spec 主路径 |
| `ir().shape_registry` | SSA | spec struct 定义 |
| `table_spec_types_` | CGen 运行时 | CGen 自己维护的变量→spec 类型映射 |
| `global_table_spec_types_` | CGen 运行时 | 全局变量→spec 类型映射 |
| `spec_param_types_` | CGen 运行时 | 参数→特化类型映射 |

`QueryTypeInfo`（`c_gen.cpp:721-800`）的优先级链：
1. SSA spec snapshot → shape → legacy spec snapshot (数值) → SSA 数值 → legacy main_eval (数值) → SSA 其他 → legacy spec 其他 → T_UNKNOWN

这个 fallback 链过于复杂，且 SSA 推导不完整时依赖 legacy 数值推导，但 legacy 数值推导又被废弃了，导致大量失败。

---

## 3. 测试失败详细分类（60 infer + 4 exception + 1 algo = 65 total）

### 3.1 数学特化返回类型失败（36 个）

所有 `test_spec_*` 测试的根本原因：SSA `spec_ssa_return_types` 未收敛到正确类型（停留在 T_DYNAMIC），CGen fallback 到 legacy `specialization_return_types` 但因为 legacy 推导器不再运行，legacy 字段也被 PopulateLegacyFields 从 SSA 结果填充了 T_DYNAMIC。

**失败测试**：
- test_spec_fib, test_spec_reassign_gcd, test_spec_reassign_powmod
- test_spec_for_bound_param, test_spec_do_return, test_spec_repeat_arith
- test_spec_for_in_body, test_spec_bare_return
- test_spec_repeat_local_until, test_spec_assign_nonnumeric_int_throws
- test_spec_for_dynamic_bound, test_spec_forloop_int_float_degrade
- test_spec_ternary, test_spec_clamp_le, test_spec_clamp_param
- test_spec_compare_equal, test_spec_leftshift_param, test_spec_rightshift_param
- test_spec_max_param, test_spec_min_param, test_spec_or_param
- test_spec_wrapper_var, test_spec_nested_call, test_spec_no_arith_compare_only
- test_spec_args_syntax_mix, test_spec_local_func, test_spec_funcdef_assignment
- test_spec_local_from_func_call, test_spec_local_chain_from_func_call
- test_spec_callee_return_propagates_in_local
- test_spec_return_arith_of_calls, test_spec_return_call_arg_is_call
- test_spec_return_call_arg_is_other_call, test_spec_fib (recursive)

### 3.2 流敏感退化/赋值失败（7 个）

SSA phi meet 正确检测到退化→T_DYNAMIC，但 CGen 声明变量时通过 legacy 路径读到旧类型。

- test_infer_mutation, test_infer_assign_degraded_var
- test_infer_return_stale_type, test_infer_do_shadow_typed_over_dynamic
- test_infer_while_scope_degrade, test_infer_degrade_param
- test_infer_for_shadow_case2, test_infer_for_shadow_case4
- test_infer_for_step_dynamic

### 3.3 Table 特化失败（4 个）

CGen 的 table_spec_types_ 与 SSA shape_registry 脱节。

- test_global_table_spec, test_table_spec_empty
- test_table_spec_control_flow, test_table_spec_if_else_soundness

### 3.4 其他语义失败（5 个）

- test_bitwise_float_operand, test_infer_bitand_integer_repr_float
- test_or_left_numeric, test_native_bool_while, test_count_loop
- test_discovery_order, test_table_field_expr

### 3.5 数学库特化失败（2 个）

- test_math_spec_9params, test_math_spec_mixed

### 3.6 异常检测失败（4 个）

PreProcessor 的 AddLocal 自动添加 shadow 后缀，掩盖了重复参数错误。

- function_param_duplicate, const_define_duplicate
- const_define_func_param_duplicate, global_duplicate_lvalue_error

### 3.7 算法测试失败（1 个）

- algo.insertion_sort（可能由上游类型错误连锁导致）

---

## 4. 重构计划

### 核心原则

1. **SSA 是唯一真相源**：CGen 完全从 SSA 路径查询类型，删除所有 legacy fallback
2. **渐进式迁移**：每步确保可编译、可测试
3. **旧测试无法通过可先注释**：重点是重构编译管线，旧的 test case 如果与新管线语义不一致可以先禁用
4. **构建约束**：必须 `--parallel 1`，否则 OOM

### Phase 0：止血与稳定基线（1-2小时）

**目标**：确认当前 696 通过的状态稳定，记录基线

1. 记录当前测试通过/失败精确列表
2. 确认构建流程正确
3. 记录本次修复的 `LookupExpType` / `LookupForLoopVarType` 实现

### Phase 1：增强 UTA InferExprType — 让 SSA 推导完全覆盖数值类型（核心工作量，3-5小时）

**目标**：SSA 推导结果不再依赖 legacy fallback 就能正确推导所有数值场景

#### 1.1 改进函数调用返回类型推导

`InferExprType` 的 `FunctionCall` 分支（218-288行）需要：

- **跨函数返回类型传播**：当前不动点迭代 3 轮可能不够。改进方案：
  - 第一轮：所有函数都是默认分析(T_DYNAMIC参数假设)
  - 收集初始返回类型
  - 后续每轮：用上一轮的返回类型作为调用点的假设
  - 直到收敛或达到最大轮数（10轮）
  
- **递归函数处理**：fib 等递归函数，第一轮假设返回 T_DYNAMIC，第二轮用实际推导结果更新

- **嵌套调用链**：`local a = f() + g()` — 需要 f 和 g 的返回类型都已收敛

具体修改：
```cpp
// type_inferencer.cpp 中 InferTypes 的不动点迭代部分
// 改为: 持续迭代直到 spec_ssa_return_types 不再变化或达到 max_iters
for (int pass = 0; pass < 10; ++pass) {
    bool changed = false;
    for (const auto &fi : func_infos) {
        // ... 重新分析每个特化版本
        // 用上一轮的返回类型假设更新本轮
    }
    if (!changed) break;
}
```

#### 1.2 改进 and/or meet 语义

当前 `InferExprType` 对 and/or 的处理（139-149行）过于保守：

```cpp
// 改进：支持布尔 + 数值混合
case BinOpKind::kAnd:
    // Lua 语义：and 返回第一个为假的值，或最后一个值
    // 类型推断：meet(lty, rty)
    return Meet(lty, rty);  // 而不是仅检查数值
    
case BinOpKind::kOr:
    // Lua 语义：or 返回第一个为真的值，或最后一个值
    return Meet(lty, rty);  // 更保守但正确
```

实际上对于 `test_or_left_numeric` 这种 `local x = y or 0` 场景，结果类型应为 y 和 0 的 meet。如果 y 是 T_INT，0 是 T_INT，结果是 T_INT。

#### 1.3 改进二元运算推导

当前 `InferNumericBinopResultType` 仅处理数值。需要确保对于 `test_bitwise_float_operand` 等测试：

- 位运算（&, |, ~, <<, >>）对浮点操作数应推导为 T_INT（Lua 语义：浮点转整数）
- 比较运算始终返回 T_BOOL

#### 1.4 增强变量版本类型传播

当前 worklist 对参数版本初始化为 T_DYNAMIC，但特化版本分析时参数类型应为假设值。确认 `RunWorklist` 中参数初始化正确使用 `param_assumptions`。

#### 1.5 增加 Repeat-Until 的 TransferStmt 处理

```cpp
case SyntaxTreeType::Repeat: {
    auto *rp = static_cast<SyntaxTreeRepeat *>(stmt.get());
    // 递归处理循环体
    if (rp->Block()) {
        TransferStmt(rp->Block(), ssa, version_types, ir);
    }
    // 处理退出条件
    // 退出条件在 CFG 中是单独的基本块
    break;
}
```

### Phase 2：删除 CGen 的 legacy fallback，统一到 SSA 查询（2-3小时）

**目标**：CGen 的 `QueryTypeInfo` 简化为纯 SSA 路径

#### 2.1 简化 QueryTypeInfo

```cpp
InferResult::SSATypeInfo CGen::QueryTypeInfo(SyntaxTreeInterface *node) const {
    // 1. 特化快照
    if (cur_spec_ssa_snapshot_) {
        if (const auto it = cur_spec_ssa_snapshot_->find(node); it != cur_spec_ssa_snapshot_->end()) {
            if (it->second.type != T_UNKNOWN) return it->second;
        }
    }
    // 2. 主 SSA 类型
    if (const auto it = ir().main_ssa_types.find(node); it != ir().main_ssa_types.end()) {
        if (it->second.type != T_UNKNOWN) return it->second;
    }
    // 3. SSA 版本号桥接
    const auto ssa_it = ir().node_ssa_version.find(node);
    if (ssa_it != ir().node_ssa_version.end()) {
        const auto ty_it = ir().ssa_version_types.find(ssa_it->second);
        if (ty_it != ir().ssa_version_types.end() && ty_it->second.type != T_UNKNOWN) {
            return ty_it->second;
        }
    }
    return {.type = T_DYNAMIC, .shape_id = -1};
}
```

#### 2.2 简化 GetSpecReturnType

```cpp
InferredType CGen::GetSpecReturnType(const std::string &func_name, int bitmask) const {
    const auto it = ir().spec_ssa_return_types.find(func_name);
    if (it != ir().spec_ssa_return_types.end() && bitmask >= 0 &&
        bitmask < static_cast<int>(it->second.size())) {
        auto ty = it->second[static_cast<size_t>(bitmask)].type;
        if (ty != T_UNKNOWN) return ty;
    }
    return T_DYNAMIC;
}
```

#### 2.3 替换 math_param_positions 为 specializable_params 派生

```cpp
std::vector<int> CGen::MathParamIndicesOf(const std::string &func_name) const {
    std::vector<int> result;
    const auto it = ir().specializable_params.find(func_name);
    if (it == ir().specializable_params.end()) return result;
    for (const auto &p : it->second) {
        if (p.is_math) result.push_back(p.param_index);
    }
    return result;
}
```

所有 `ir().math_param_positions` 使用替换为 `MathParamIndicesOf()`。

### Phase 3：统一 table 特化到 SSA Shape 系统（2-3小时）

**目标**：删除 CGen 中独立的 `table_spec_types_` 系统，统一使用 shape_registry

#### 3.1 字段访问统一查询

当前 `CompileVar kDot/kSquare` 先查 `table_spec_types_` 再决定走 FL_SPEC 还是 hash，改为：
- 通过 `QueryTypeInfo(base_node)` 获取 shape_id
- 查 `shape_registry->Get(shape_id)` 获取字段布局
- 已知字段走 FL_SPEC，未知字段走 hash

#### 3.2 删除 SpecSnapshot 机制

`CompileStmtIf` 中的 `SaveSpecSnapshot/RestoreSpecSnapshot/JoinSpecSnapshots` 全部删除。SSA 的 phi meet 已经在 UTA 中完成了流敏感类型推导，CGen 不需要再做运行时类型快照。

#### 3.3 删除 CGen 中的 table spec 辅助数据

- `table_spec_types_`
- `global_table_spec_types_`
- `spec_field_names_`
- `spec_field_indices_`
- `spec_field_c_names_`
- `spec_field_types_`
- `generated_spec_typedefs_`

所有字段信息统一从 `shape_registry` 获取。

### Phase 4：删除 Legacy TypeInferencer 代码（1-2小时）

**目标**：删除所有 legacy 推导函数和 InferResult legacy 字段

#### 4.1 删除 legacy 推导函数

从 `type_inferencer.cpp` 和 `.h` 中删除：
- `PopulateLegacyFields` 函数
- `BuildTableSpecInfosFromShapes` 函数
- `BuildCtorFields` 函数
- `MergeFieldsInto` 函数
- `FieldKeyDescriptor` 函数
- `QueryTypeInfoFromSSA` 静态函数

`InferTypes` 函数保持简洁的 SSA 入口（构建 CFG → SSA → UTA → 不动点迭代）。

#### 4.2 删除 InferResult legacy 字段

从 `compile_common.h` 中删除：
- `math_param_positions`
- `specialization_snapshots`
- `specialization_return_types`
- `main_eval_types`
- `global_const_vars`
- `table_spec_infos`
- `SpecParam.is_shape` 字段

#### 4.3 CGen 全局搜索并清理

删除所有对已删除字段的引用。

### Phase 5：修复 PreProcessor 异常检测 & 注释不一致的旧测试（1小时）

#### 5.1 修复 PreProcessor 重复参数检测

方案：在 `PreProcessor::Process` 处理 ParList 时，在逐个调用 `AddLocal` 之前，先检查参数列表内是否有重复名：
```cpp
// 在 Process 处理 FuncBody 时
std::unordered_set<std::string> param_names;
for (const auto &name : namelist) {
    if (param_names.count(name)) {
        ThrowFakeluaException("duplicate parameter: " + name);
    }
    param_names.insert(name);
}
```

#### 5.2 注释/禁用与新管线语义不一致的旧测试

以下测试场景在新 SSA 管线下可能有不同的预期行为，暂时注释：
- 如果旧的推导器对流不敏感场景有特殊预期（如变量退化后仍然返回旧类型），这些在新管线中是错误的预期

### Phase 6：验证与回归测试（1小时）

```bash
cmake --build build --target unit_tests --parallel 1
cd build/bin && ./unit_tests
```

目标：756 全绿，0 FAILED。

---

## 5. 关键数据结构参考

### 5.1 SSA 版本号 → 类型信息

```
SSAFunction:ssa_version_types[int] → SSATypeInfo{type, shape_id}
```

CGen 通过 `node_ssa_version[node] → version → ssa_version_types[version]` 桥接查询。

### 5.2 Shape 注册表

```
ShapeRegistry::shapes_[shape_id] → ShapeType{shape_id, is_open, fields: [FieldDef{name, c_field_name, type, optional, is_int_key}]}
```

### 5.3 特化参数

```
SpecParam{param_index, is_math, is_shape}
specializable_params[func_name] → vector<SpecParam>
```

当前只含 `is_math=true` 的参数。bitmask = 2^math_params 种组合。

### 5.4 φ 节点

```
PhiNode{phi_id, var_name, result_version, arg_versions[], arg_block_ids[]}
```

UTA 在 `ProcessPhiNodes` 中对 `arg_versions` 做 meet，写入 `version_types[result_version]`。

---

## 6. 构建与测试注意事项

1. **构建必须 `--parallel 1`**：该机器内存有限，并行构建导致 cc1plus OOM 被 kill
2. **测试必须从 `build/bin/` 运行**：测试用相对路径 `./infer/xxx.lua` 加载 Lua 文件
3. **完整测试命令**：
   ```bash
   cmake --build build --target unit_tests --parallel 1
   cd build/bin && ./unit_tests
   ```
4. **单测过滤**：
   ```bash
   cd build/bin && ./unit_tests --gtest_filter='infer.test_spec_fib'
   ```
5. **调试输出**：CompileConfig `debug_mode = true` 会将 SSA 推导结果和生成的 C 代码转储到 `/tmp/fakelua/`

---

## 7. 文件修改清单

| 文件 | Phase | 改动摘要 |
|------|-------|---------|
| `src/compile/unified_type_analyzer.cpp` | 1 | 增强 InferExprType（函数调用、and/or、位运算、repeat-until） |
| `src/compile/unified_type_analyzer.h` | 1 | 可能新增辅助方法 |
| `src/compile/type_inferencer.cpp` | 1,4 | 增强不动点迭代轮数；删除 PopulateLegacyFields 等 |
| `src/compile/type_inferencer.h` | 4 | 删除 legacy 接口声明 |
| `src/compile/c_gen.cpp` | 2,3 | 简化 QueryTypeInfo/GetSpecReturnType；删除 table_spec_types_ 机制 |
| `src/compile/c_gen.h` | 2,3 | 删除 SpecSnapshot、table spec 辅助数据和接口 |
| `src/compile/compile_common.h` | 4 | 删除 legacy InferResult 字段 |
| `src/compile/shape_type.h` | 1（可能） | 可能新增 set-field helper |
| `src/compile/preprocessor.cpp` | 5 | 修复重复参数检测 |
| `test/test_infer.cpp` | 5 | 注释不一致的旧测试 |

---

## 8. 风险与降级策略

1. **Phase 4 风险最高**：大量删除 legacy 代码，务必每步跑测试
2. **Phase 1 最关键**：如果不增强 SSA 推导就删除 legacy fallback，会导致更多测试失败
3. **降级**：任何时候如果 SSA 管线出问题，可以通过 `QueryTypeInfo` 返回 `T_DYNAMIC` 安全退化到 CVar 路径

---

## 9. 设计文档参考

- **主规范**：`/root/lua-dialect-type-inference-spec.md`（完整的 SSA+CFG+Shape 类型推导系统设计）
- **前一 AI 的计划**：`/root/.codebuddy/plans/electric-thunder-curie.md`（Phase A-D，核心洞察仍有价值）
- **前一 AI 的进度**：`SSA_MIGRATION_PROGRESS.md`（44 个失败时的状态记录，部分已过时）
- **本次会话修复**：`LookupExpType` / `LookupForLoopVarType` 实现补充（解决链接错误）
