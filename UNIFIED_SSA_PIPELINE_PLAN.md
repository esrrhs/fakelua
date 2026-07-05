# FakeLua 统一 SSA/CFG/Shape 编译管线 — 实施计划

> **编制日期**: 2026-07-03（持续更新）
> **设计规范**: `/root/lua-dialect-type-inference-spec.md`
> **前一状态文档**: `SSA_PIPELINE_STATUS.md`（部分已过时，以本计划为准）
> **目标**: 将当前"legacy 单遍推导 + 试推断"双轨系统，统一迁移到 SSA + CFG + Shape 抽象解释管线，最终删除 legacy 代码。

---

## 实施进度日志

### 2026-07-03 — Phase 0 + Phase 1 部分完成

**当前测试状态**: 756/756 全部通过 ✅

**已完成的修改**:

1. **扩展 `InferredType` 枚举** (`inferred_type.h`): 新增 `T_NIL`, `T_BOOL`, `T_STRING`, `T_RECORD`, `T_RECORD_OPEN`，新增 `IsNumericInferredType` 和 `IsRecordInferredType` 辅助函数。

2. **新增 `SSATypeInfo` 和 `SpecParam` 结构** (`compile_common.h`): 定义 SSA 路径的核心数据类型。

3. **扩展 `InferResult` 结构** (`compile_common.h`): 新增 SSA 路径字段（`ssa_version_types`, `node_ssa_version`, `shape_registry`, `specializable_params`, `main_ssa_types`, `spec_ssa_snapshots`, `spec_ssa_return_types`, `global_table_shapes`）。

4. **修复 UTA 类型引用** (`unified_type_analyzer.h`): 将 `InferResult::SSATypeInfo` 改为 `fakelua::SSATypeInfo`。

5. **删除重复的 `IsNumericInferredType`** (`compile_common.h`): 该函数已移到 `inferred_type.h`。

6. **接通 SSA 管线到 `InferTypes`** (`type_inferencer.cpp`): 新增 `RunSSAAnalysis` 和 `RunSSASpecialization` 函数，在 legacy 分析完成后对每个函数运行 CFG→SSA→UTA。

**发现的问题与修复**:
- ~~SSA 管线在 `SSABuilder::RenameBlock` 中崩溃~~ → **已修复**
  - 根因: `SSABuilder` 实例在多个函数间复用，但 `dom_children_` 等成员向量在 `Build()` 中用 `resize(n)` 重置，当新 size 小于旧 size 时，内层 vector 保留旧数据，导致 `dom_children_[0]` 包含已不存在的 block ID
  - 修复: 将 `resize(n)` 改为 `assign(n, {})`，确保内层 vector 被清空；同时清理其他成员（`var_def_blocks_`, `decl_stacks_`, `version_parent_`, `rpo_order_`, `next_phi_id_`）

**当前状态**: SSA 管线已完全接通，756/756 测试全通过 ✅

### 2026-07-03 (续) — Phase 2 接通 SSA 管线

**修复**: `SSABuilder::Build()` 中成员向量跨函数复用导致的数据污染
- 将 `dom_children_.resize(n)` 改为 `dom_children_.assign(n, {})`
- 同样修复 `df_.resize(n)` → `df_.assign(n, {})`
- 新增清理: `var_def_blocks_.clear()`, `decl_stacks_.clear()`, `version_parent_.clear()`, `rpo_order_.clear()`, `next_phi_id_ = 0`

**新增代码**:
- `TypeInferencer::RunSSAAnalysis()` — 遍历所有函数，依次执行 CFG→SSA→UTA
- `TypeInferencer::RunSSASpecialization()` — 预留接口，Phase 3 实现
- 新增 include: `cfg.h`, `shape_type.h`, `ssa.h`, `unified_type_analyzer.h`

**当前 SSA 管线能力**:
- ✅ CFG 构造（if/while/for/repeat/for-in/break/return/goto）
- ✅ SSA 构造（Cytron 算法：支配树 + 支配边界 + φ 插入 + 变量重命名）
- ✅ UTA worklist 数据流分析（流敏感类型推导）
- ✅ Shape 抽象解释（table 构造、字段读写、meet/widen）
- ⚠️ 数学参数发现尚未完成（`FindSpecializableParams` 返回空，需 Phase 3 修复）
- ⚠️ CGen 尚未切换到 SSA 路径（仍使用 legacy 字段）

**下一步**: Phase 3 — 修复 `FindSpecializableParams` 使其正确发现数学参数，然后让 CGen 优先从 SSA 路径查询类型。

### 2026-07-03 (续) — Phase 3 SSA 数学参数发现验证

**验证结果**: SSA 管线的 `FindSpecializableParams` 已能正确发现数学参数，与 legacy 路径结果一致。

**对比测试**（fib, clamp_le, nested_call, min_param）:
- `fib`: legacy={0}, ssa={0} ✅
- `clamp_le`: legacy={0,1,2}, ssa={0,1,2} ✅
- `func2` (nested_call): legacy={0}, ssa={0} ✅
- `min`: legacy={0,1}, ssa={0,1} ✅

**当前状态**: 756/756 测试全通过 ✅

**下一步**: Phase 3 继续 — 实现特化版本的不动点迭代（`RunSSASpecialization`），让 SSA 路径生成 `spec_ssa_return_types`。然后让 CGen 优先从 SSA 路径查询类型。

### 2026-07-03 (续) — Phase 3 特化版本不动点迭代完成

**实现**: `TypeInferencer::RunSSASpecialization()`
- 对每个有 `specializable_params` 的函数，运行 2^k 次 UTA（k = 数学参数数）
- 每次用不同的 `param_assumption`（int/float 组合）
- 不动点迭代（最多 10 轮）收敛返回类型
- 结果写入 `ir.spec_ssa_return_types[func_name][bitmask]`

**当前 SSA 管线完整能力**:
- ✅ CFG 构造
- ✅ SSA 构造（Cytron 算法）
- ✅ UTA worklist 数据流分析
- ✅ Shape 抽象解释
- ✅ 数学参数发现（与 legacy 一致）
- ✅ 特化版本不动点迭代
- ⚠️ CGen 尚未切换到 SSA 路径（仍使用 legacy 字段）

**当前状态**: 756/756 测试全通过 ✅

**下一步**: Phase 3 继续 — 让 CGen 优先从 SSA 路径查询类型（`QueryTypeInfo` 改用 SSA 字段），legacy 字段作为 fallback。

---

## 0. 现状诊断（代码实测，非文档推测）

### 0.1 实际编译状态

| 模块 | 文件 | 实际状态 |
|------|------|---------|
| CFG 构造 | `cfg.cpp/h` | ✅ 完整，但**未被任何代码调用** |
| SSA 构造 | `ssa.cpp/h` | ✅ 完整，但**未被任何代码调用** |
| ShapeRegistry | `shape_type.h` | ✅ 完整，但**未被任何代码调用** |
| UTA | `unified_type_analyzer.cpp/h` | ❌ **编译失败** — 引用了未定义的类型和签名不匹配 |
| TypeInferencer 入口 | `type_inferencer.cpp` | ✅ 纯 legacy 路径，**从未调用 SSA/UTA** |
| CGen | `c_gen.cpp/h` | ✅ 纯 legacy 路径，有自己的 `table_spec_types_` 双轨 |

**关键发现**: 整个 SSA/CFG/Shape 管线是**死代码**。`compiler.cpp:70` 只调用 `inferencer.InferTypes(pr, cfg)`，该函数内部只走 legacy 路径（`InferNode` → `IdentifyMathParams` → `RunTrialInference` → `AnalyzeTableShapes`）。

### 0.2 UTA 编译失败的具体原因

1. **`InferResult::SSATypeInfo` / `InferResult::SpecParam` 未定义** — `compile_common.h` 的 `InferResult` 只有 legacy 字段，没有 SSA 字段。
2. **`T_RECORD` / `T_RECORD_OPEN` / `T_NIL` / `T_BOOL` / `T_STRING` 未定义** — `inferred_type.h` 只有 `{T_UNKNOWN, T_INT, T_FLOAT, T_DYNAMIC}`。
3. **`FindSpecializableParams` 签名不匹配** — `.h` 声明 4 个参数，`.cpp` 定义 5 个参数。
4. **`IsRecordInferredType` 未定义** — UTA 使用但未声明。

### 0.3 当前测试状态（2026-07-03 实测）

| 类别 | 总数 | 通过 | 失败 |
|------|------|------|------|
| 全部 | ~756 | ~724 | ~32 |
| infer | ~120 | ~118 | 2 |
| exception | ~30 | ~28 | 2 |
| jitter | ~100 | ~74 | 26 |

**当前失败测试清单**:
- `infer.test_spec_assign_nonnumeric_int_throws` — 特化体内对 math param 赋非数值时应抛运行时错误
- `infer.test_spec_forloop_int_float_degrade` — for 循环变量从 int 退化为 float 时需生成两种特化
- `exception.test_const_unop_len_error` — `#a` 对 float 运行时应抛 "attempt to get length"
- `exception.test_const_unop_bitnot_error` — `~a` 对 string 运行时应抛 "attempt to perform bitwise"
- `jitter.test_assign` / `test_const_define_simple_var` / `test_const_binop_*` / `test_const_unop_*` / `test_do_block` / `test_complete` / `test_edge_cases` / `bitnot_on_dynamic_expr` — 多数是 C 代码生成错误

### 0.4 失败根因分类

| 根因 | 影响的测试 | 说明 |
|------|-----------|------|
| **CGen 对全局常量赋值的类型推断错误** | test_assign, test_const_define_simple_var, test_const_binop_* | 全局 `local a = 1` 被赋给局部变量时，LookupNodeType 返回 T_INT 但变量声明为 CVar，导致类型不匹配 |
| **CGen 对一元运算的动态操作数处理错误** | test_const_unop_len_error, test_const_unop_bitnot_error, bitnot_on_dynamic_expr | `#a` / `~a` 对非数值类型应生成带运行时检查的 CVar 代码，但当前生成了原生 int 赋值 |
| **CGen 对 do...end 块内赋值的类型推断错误** | test_do_block | do 块内 `a = 1; b = "2"` 后 return a，应推断 a 为 CVar |
| **特化体内类型守卫缺失** | test_spec_assign_nonnumeric_int_throws | 特化体内对 math param 赋非数值 CVar 时应生成运行时类型检查 |
| **for 循环退化检测不完整** | test_spec_forloop_int_float_degrade | `n = 1.5` 后 for 循环变量应退化为 float，需生成两种特化版本 |

---

## 1. 总体策略

### 1.1 核心原则

1. **先修复再迁移**: 先让现有 32 个失败测试全部通过（修复 legacy 路径的 bug），建立稳定基线。
2. **SSA 管线先行独立验证**: 让 SSA/CFG/Shape 管线先独立工作（不替换 legacy），通过对比测试验证正确性。
3. **渐进式替换**: 先让 CGen 同时支持 legacy 和 SSA 路径，逐步切换，最终删除 legacy。
4. **每步可编译可测试**: 每次改动后必须能编译并通过全部测试。

### 1.2 风险分析

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| SSA 管线庞大，修复周期长 | 延迟交付 | 分 Phase 独立交付，每 Phase 都有可测试产出 |
| CGen 改动影响面广 | 引入新 bug | 保留 legacy fallback，逐步切换 |
| 性能退化 | JIT 变慢 | benchmark 对比，必要时优化 hot path |
| 内存不足 OOM | 构建失败 | 始终 `--parallel 1` |

---

## 2. 分阶段实施计划

### Phase 0: 修复当前失败测试（优先级最高，1-2 天）

**目标**: 756 测试全绿，建立稳定基线。

#### 0.1 修复 CGen 全局常量赋值类型推断

**问题**: 全局 `local a = 1` 被赋给局部变量 `local b = a` 时，`LookupNodeType` 对 `a` 返回 T_INT，但 `b` 应声明为 CVar（因为 `a` 是 CVar 全局变量）。

**文件**: `c_gen.cpp` `CompileStmtLocalVar`

**修复**: 当 RHS 是全局变量引用时，不直接使用其数值类型，而应检查全局变量的声明类型。如果全局变量是 CVar（非特化），则局部变量也应为 CVar。

#### 0.2 修复 CGen 一元运算动态操作数

**问题**: `#a` 对 float 类型 `a` 应生成带运行时检查的 CVar 代码，但当前生成 `int64_t c = ...` 导致 CVar 赋值给 int64_t 失败。

**文件**: `c_gen.cpp` `CompileUnop`

**修复**: 当一元运算的操作数不是确定数值类型时（如全局 CVar 变量），生成 CVar 路径的 `OpLen`/`OpBitNot` 调用，结果存 CVar。

#### 0.3 修复 do...end 块内赋值推断

**问题**: do 块内 `a = 1; b = "2"` 后 return a，应推断 a 为 CVar（因为 b 的赋值表明参数可能是 dynamic）。

**文件**: `c_gen.cpp` `CompileStmtAssign`

**修复**: 当赋值 RHS 是非数值类型（如字符串）时，将变量标记为 CVar 并清除其原生类型声明。

#### 0.4 修复特化体内类型守卫

**问题**: 特化体内对 math param 赋非数值 CVar 时应生成运行时类型检查。

**文件**: `c_gen.cpp` `CompileStmtAssign`

**修复**: 在特化模式（`cur_spec_bitmask_ >= 0`）下，对已特化变量赋 CVar 类型 RHS 时，生成 `if (tmp.type_ == VAR_INT) ... else if (tmp.type_ == VAR_FLOAT) ... else FakeluaThrowError(...)` 守卫。

#### 0.5 修复 for 循环退化检测

**问题**: `n = 1.5` 后 for 循环变量应退化为 float，需生成两种特化版本。

**文件**: `type_inferencer.cpp` `InferForLoop`

**修复**: 当循环边界中有 T_FLOAT 时，循环变量类型应为 T_FLOAT；当有 T_DYNAMIC 时，应为 T_DYNAMIC。确保 `all_numeric` 判断正确处理 float 边界。

---

### Phase 1: 让 SSA/CFG/Shape 管线编译通过（2-3 天）

**目标**: UTA 代码能编译，类型系统扩展到支持 record/nil/bool/string。

#### 1.1 扩展 InferredType 枚举

**文件**: `inferred_type.h`

```cpp
enum InferredType {
    T_UNKNOWN = 0,
    T_NIL,
    T_BOOL,
    T_INT,
    T_FLOAT,
    T_STRING,
    T_RECORD,        // 封闭 record
    T_RECORD_OPEN,   // 开放 record
    T_DYNAMIC,       // 完全未知
    T_VAR,           // HM 类型变量（Phase 3）
    T_FUN,           // 函数类型（Phase 3）
    T_ARRAY,         // 数组部分（Phase 4）
    T_UNION,         // 类型 union（Phase 3）
};
```

#### 1.2 定义 SSATypeInfo 和 SpecParam

**文件**: `compile_common.h`

```cpp
struct SSATypeInfo {
    InferredType type = T_UNKNOWN;
    int shape_id = -1;  // -1 表示非 record，>= 0 表示 shape_registry 中的 ID
    bool operator==(const SSATypeInfo &o) const { return type == o.type && shape_id == o.shape_id; }
    bool operator!=(const SSATypeInfo &o) const { return !(*this == o); }
};

struct SpecParam {
    int param_index;
    bool is_math;
    bool is_shape;
};
```

#### 1.3 扩展 InferResult 结构

**文件**: `compile_common.h` `InferResult`

新增字段：
```cpp
// SSA 路径输出
std::unordered_map<int, SSATypeInfo> ssa_version_types;
std::unordered_map<const SyntaxTreeInterface *, int> node_ssa_version;
std::shared_ptr<ShapeRegistry> shape_registry;
std::unordered_map<std::string, std::vector<SpecParam>> specializable_params;
std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> main_ssa_types;
std::unordered_map<std::string, std::vector<std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo>>> spec_ssa_snapshots;
std::unordered_map<std::string, std::vector<SSATypeInfo>> spec_ssa_return_types;
std::unordered_map<std::string, SSATypeInfo> global_table_shapes;
```

#### 1.4 修复 UTA 签名不一致

**文件**: `unified_type_analyzer.h` / `.cpp`

统一 `FindSpecializableParams` 签名（5 参数版本），添加 `IsRecordInferredType` 辅助函数。

#### 1.5 添加辅助函数

**文件**: `inferred_type.h` 或 `shape_type.h`

```cpp
inline bool IsRecordInferredType(InferredType t) {
    return t == T_RECORD || t == T_RECORD_OPEN;
}
```

---

### Phase 2: 接通 SSA 管线到编译主流程（3-5 天）

**目标**: `TypeInferencer::InferTypes` 在 legacy 分析之后，额外运行 SSA 管线，填充 SSA 字段。

#### 2.1 在 InferTypes 中调用 SSA 管线

**文件**: `type_inferencer.cpp` `InferTypes`

```cpp
// 在 legacy 分析之后，对每个函数运行 SSA 管线
auto cfg_builder = CFGBuilder();
auto ssa_builder = SSABuilder();
UnifiedTypeAnalyzer uta(/*registry=*/ir.shape_registry.get(), /*summaries=*/nullptr);

for (auto &func_info : CollectFunctionSpecInfos(pr)) {
    auto cfg = cfg_builder.Build(func_info.block, func_info.params, func_info.name);
    auto ssa = ssa_builder.Build(cfg);
    uta.Analyze(func_info.name, func_info.block, cfg, ssa, ir);
    auto params = uta.FindSpecializableParams(func_info.block, cfg, ssa, ir);
    if (!params.empty()) {
        ir.specializable_params[func_info.name] = std::move(params);
    }
}
```

#### 2.2 实现特化版本的不动点迭代

**文件**: `type_inferencer.cpp`

对每个有 `specializable_params` 的函数，运行 2^k 次 UTA（k = math 参数数），每次用不同的 `param_assumptions`，通过不动点迭代收敛返回类型。

```cpp
for (auto &[func_name, spec_params] : ir.specializable_params) {
    int num_math = count_math_params(spec_params);
    int num_specs = 1 << num_math;
    auto &rets = ir.spec_ssa_return_types[func_name];
    rets.assign(num_specs, SSATypeInfo{T_INT, -1});  // 乐观初值

    for (int pass = 0; pass < 10; ++pass) {
        bool changed = false;
        for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
            ParamAssumption assumptions = make_assumptions(spec_params, bitmask);
            uta.Analyze(func_name, func_block, cfg, ssa, ir, bitmask, assumptions);
            // 比较返回类型是否变化
        }
        if (!changed) break;
    }
}
```

#### 2.3 验证 SSA 输出与 legacy 一致

**方法**: 在 debug 模式下，对比 `ir.math_param_positions`（legacy）和 `ir.specializable_params`（SSA），确保两者发现的数学参数一致。

---

### Phase 3: CGen 切换到 SSA 路径（3-5 天）

**目标**: CGen 优先从 SSA 字段查询类型，legacy 字段作为 fallback。

#### 3.1 简化 QueryTypeInfo

**文件**: `c_gen.cpp` `QueryTypeInfo`

```cpp
SSATypeInfo CGen::QueryTypeInfo(SyntaxTreeInterface *node) const {
    // 1. SSA spec snapshot
    if (cur_spec_ssa_snapshot_) {
        auto it = cur_spec_ssa_snapshot_->find(node);
        if (it != cur_spec_ssa_snapshot_->end() && it->second.type != T_UNKNOWN)
            return it->second;
    }
    // 2. SSA 主类型
    auto it = ir().main_ssa_types.find(node);
    if (it != ir().main_ssa_types.end() && it->second.type != T_UNKNOWN)
        return it->second;
    // 3. SSA 版本号桥接
    auto vit = ir().node_ssa_version.find(node);
    if (vit != ir().node_ssa_version.end()) {
        auto tit = ir().ssa_version_types.find(vit->second);
        if (tit != ir().ssa_version_types.end() && tit->second.type != T_UNKNOWN)
            return tit->second;
    }
    // 4. Legacy fallback（Phase 3 保留，Phase 4 删除）
    // ...
    return {T_DYNAMIC, -1};
}
```

#### 3.2 统一 table 特化到 shape_registry

**文件**: `c_gen.cpp`

删除 `table_spec_types_` / `global_table_spec_types_` / `spec_field_names_` / `spec_field_indices_` / `spec_field_c_names_` / `spec_field_types_` / `generated_spec_typedefs_` / `SpecSnapshot` / `ComputeSpecTypeName`。

替换为：通过 `QueryTypeInfo(base_node)` 获取 `shape_id`，查 `shape_registry->Get(shape_id)` 获取字段布局。

#### 3.3 删除 SpecSnapshot 机制

**文件**: `c_gen.cpp` `CompileStmtIf`

删除 `SaveSpecSnapshot` / `RestoreSpecSnapshot` / `JoinSpecSnapshots` 调用。SSA 的 phi meet 已经在 UTA 中完成流敏感类型推导。

#### 3.4 统一特化返回类型查询

**文件**: `c_gen.cpp` `GetSpecReturnType`

```cpp
SSATypeInfo CGen::GetSpecReturnType(const std::string &func_name, int bitmask) const {
    auto it = ir().spec_ssa_return_types.find(func_name);
    if (it != ir().spec_ssa_return_types.end() && bitmask < (int)it->second.size()) {
        if (it->second[bitmask].type != T_UNKNOWN) return it->second[bitmask];
    }
    // Legacy fallback
    auto lit = ir().specialization_return_types.find(func_name);
    if (lit != ir().specialization_return_types.end() && bitmask < (int)lit->second.size()) {
        return {lit->second[bitmask], -1};
    }
    return {T_DYNAMIC, -1};
}
```

---

### Phase 4: 删除 Legacy 代码（2-3 天）

**目标**: 删除所有 legacy 推导函数和 InferResult legacy 字段。

#### 4.1 删除 legacy 推导函数

从 `type_inferencer.cpp` 和 `.h` 中删除：
- `InferNode` / `InferExp` / `InferPrefixExp` / `InferVar` / `InferBlock`
- `InferLocalVar` / `InferAssign` / `InferForLoop` / `InferForIn`
- `InferWhile` / `InferRepeat` / `InferIf`
- `IdentifyMathParams` / `FindMathParamIndices` / `GenerateInitialSnapshots`
- `RunTrialInference` / `BuildFunctionReturnCache` / `InferSpecializationReturnTypes`
- `MakeAssumedParamTypes` / `MakeSpecializedParamTypes`
- `IsArithmeticExpr` / `IsNativeComparisonExpr` / `CheckArithmeticTypeChanges`
- `CheckArithmeticNodeChange` / `CheckComparisonNodeChange` / `CheckForLoopNodeChange` / `CheckCallNodeChange`
- `CollectFunctionSpecInfos` / `AllPathsReturn` / `CollectReturnExps`
- `ResolveCallReturnType` / `ComputeReturnTypeFromSnapshot`
- `CollectGlobalConstVars` / `AnalyzeTableShapes`
- `BuildCtorFields` / `MergeFieldsInto` / `FieldKeyDescriptor`
- `TypeEnvironment` 类
- `EvalTypeMap` / `EvalTypeSnapshot` 类型别名（如果不再需要）

#### 4.2 删除 InferResult legacy 字段

从 `compile_common.h` 中删除：
- `math_param_positions`
- `specialization_snapshots`
- `specialization_return_types`
- `main_eval_types`
- `table_spec_infos`
- `TableFieldInfo` / `TableSpecInfo` / `TableKeyKind`（如果 shape_registry 完全替代）

#### 4.3 简化 TypeInferencer 入口

```cpp
InferResult TypeInferencer::InferTypes(const ParseResult &pr, const CompileConfig &cfg) {
    InferResult ir;
    ir.shape_registry = std::make_shared<ShapeRegistry>();

    // 1. 全局常量分析（简化版，直接走 SSA）
    // 2. 对每个函数运行 CFG → SSA → UTA
    // 3. 不动点迭代收敛特化返回类型
    // 4. 填充 global_const_vars 和 global_table_shapes

    return ir;
}
```

---

### Phase 5: 增强 UTA 能力（3-5 天）

**目标**: UTA 完全覆盖 legacy 的所有能力，并超越。

#### 5.1 完善 InferExprType

- 函数调用返回类型：通过 `spec_ssa_return_types` 跨函数传播
- 嵌套调用链：不动点迭代保证收敛
- 递归函数：类型变量 + 不动点
- 三元模式：(cond and a or b) 的 meet 语义

#### 5.2 完善 TransferStmt

- ForIn 循环体递归处理（已修复）
- Repeat-Until 循环体递归处理（已修复）
- 字段赋值后的版本更新

#### 5.3 收敛性保证

- Widening 机制（已实现）
- 不动点迭代轮数上限（10 轮）
- 乐观初值 + meet 收敛

---

### Phase 6: 实现 HM 合一求解器（5-7 天）

**目标**: 支持多态函数的类型推导，使 `id(x) return x end` 能正确推导。

#### 6.1 类型变量系统

```cpp
struct TypeVar {
    int var_id;
    TypeVar *bound = nullptr;  // nullptr = free
};

struct FuncType {
    std::vector<Type> params;
    Type ret;
};
```

#### 6.2 合一算法

- `prune(Type *t)` — 路径压缩
- `occurs_check(Type *var, Type *t)` — 递归类型检测
- `unify(Type *a, Type *b)` — 结构递归合一

#### 6.3 约束生成与求解

- 为每个表达式生成约束
- 对每个函数做一次约束求解
- 调用点独立实例化类型变量（call-site polymorphism）

---

### Phase 7: 实现函数摘要系统（5-7 天）

**目标**: 规范 §7 的过程间分析。

#### 7.1 摘要结构

```cpp
struct FuncSummary {
    std::vector<Type> param_types;
    Type ret_type;
    std::vector<bool> param_escape;
    std::vector<FieldMod> modifications;
};
```

#### 7.2 摘要构造与调用点应用

- 对每个函数运行 UTA 分析
- 收集参数逃逸情况和对字段的修改
- 在调用点应用摘要

---

### Phase 8: 实现逃逸分析（3-5 天）

**目标**: 规范 §8，支持栈分配优化。

#### 8.1 逃逸判定规则

- 传给未知函数 → 逃逸
- 存入 dynamic table → 逃逸
- 被 return → 逃逸（保守）
- 被反射使用 → 逃逸

#### 8.2 利用逃逸信息

- NoEscape table → 栈分配结构体
- NoEscape record → 字段拆分为独立局部变量
- 未读字段 → 死字段消除

---

### Phase 9: 实现偏移分配（3-5 天）

**目标**: 规范 §9，编译期计算字段偏移。

#### 9.1 StructLayout 计算

```cpp
struct FieldLayout {
    std::string field_name;
    Type field_type;
    size_t offset;
    size_t size;
};

struct StructLayout {
    std::vector<FieldLayout> fields;
    size_t total_size;
    size_t alignment;
};
```

#### 9.2 直接偏移访问代码生成

替换 `FL_SPEC_GET/FL_SPEC_SET` 为直接的指针偏移访问：
```c
// 旧：FL_SPEC_GET(base, "x", &result)
// 新：result = *(int64_t*)((char*)base + 0)  // x 在偏移 0
```

---

## 3. 关键数据结构变更汇总

### 3.1 InferredType 枚举扩展

```cpp
enum InferredType {
    T_UNKNOWN = 0,
    T_NIL, T_BOOL, T_INT, T_FLOAT, T_STRING,
    T_RECORD, T_RECORD_OPEN,
    T_DYNAMIC,
    T_VAR, T_FUN, T_ARRAY, T_UNION  // Phase 3+
};
```

### 3.2 InferResult 结构演进

| Phase | 保留字段 | 新增字段 | 删除字段 |
|-------|---------|---------|---------|
| 0 | 全部 legacy | — | — |
| 1 | 全部 legacy | SSA 字段 | — |
| 2 | 全部 legacy | SSA 字段（已填充） | — |
| 3 | legacy（fallback） | SSA 字段 | — |
| 4 | — | SSA 字段 | 全部 legacy |

### 3.3 CGen 成员演进

| Phase | 保留成员 | 新增成员 | 删除成员 |
|-------|---------|---------|---------|
| 0 | 全部 | — | — |
| 3 | 全部 | `shape_registry_` 引用 | — |
| 4 | 原生变量管理 | — | `table_spec_types_` 等 table spec 辅助数据 |

---

## 4. 构建与测试约束

```bash
# 构建（必须 parallel 1，否则 OOM）
cmake --build build --target unit_tests --parallel 1

# 运行所有测试
cd build/bin && ./unit_tests

# 只跑 infer 测试
cd build/bin && ./unit_tests --gtest_filter='infer.*'

# 只跑特定测试
cd build/bin && ./unit_tests --gtest_filter='infer.test_spec_fib'

# 调试输出（SSA 推导结果和 C 代码转储到 /tmp/fakelua/）
# CompileConfig debug_mode = true
```

**机器资源限制**:
- RAM: ~3.0 GB 可用
- CPU: 2 核
- 构建约束: 必须 `--parallel 1`

---

## 5. 验收标准

### Phase 0 验收
- [ ] 756 测试全部通过
- [ ] 无回归

### Phase 1 验收
- [ ] UTA 代码编译通过
- [ ] 现有测试不受影响（因为 UTA 还未接入主流程）

### Phase 2 验收
- [ ] SSA 管线运行后填充的 `specializable_params` 与 legacy `math_param_positions` 一致
- [ ] 所有测试仍通过

### Phase 3 验收
- [ ] CGen 优先从 SSA 路径查询类型
- [ ] 所有测试仍通过
- [ ] table spec 辅助数据开始清理

### Phase 4 验收
- [ ] 所有 legacy 推导函数删除
- [ ] 所有 legacy InferResult 字段删除
- [ ] 所有测试仍通过

### Phase 5 验收
- [ ] UTA 完全覆盖 legacy 能力
- [ ] 所有测试仍通过

### Phase 6-9 验收
- [ ] 规范 §4 (HM)、§7 (函数摘要)、§8 (逃逸分析)、§9 (偏移分配) 全部实现
- [ ] 规范 §14 测试用例全部通过
- [ ] 收敛性测试通过（100 万次循环不卡死）

---

## 6. 风险与备选方案

| 风险 | 备选方案 |
|------|---------|
| SSA 管线性能不如 legacy | 保留 legacy 作为 fast path，SSA 作为 fallback |
| HM 合一实现复杂度过高 | 先用简单类型传播（当前 UTA 方式），推迟 HM |
| 函数摘要跨函数分析不准确 | 保守策略：未知调用一律标记逃逸 |
| 偏移分配引入 ABI 兼容问题 | 保留 FL_SPEC 宏作为 fallback |

---

## 7. 文件修改清单（预估）

| Phase | 文件 | 改动类型 |
|-------|------|---------|
| 0 | `c_gen.cpp` | 修复类型推断 bug |
| 0 | `type_inferencer.cpp` | 修复 for 循环退化 |
| 1 | `inferred_type.h` | 扩展枚举 |
| 1 | `compile_common.h` | 新增 SSATypeInfo/SpecParam/InferResult 字段 |
| 1 | `unified_type_analyzer.h/.cpp` | 修复编译错误 |
| 2 | `type_inferencer.cpp` | 接通 SSA 管线 |
| 3 | `c_gen.cpp/h` | 切换到 SSA 路径 |
| 4 | `type_inferencer.cpp/h` | 删除 legacy 函数 |
| 4 | `compile_common.h` | 删除 legacy 字段 |
| 4 | `c_gen.cpp/h` | 删除 table spec 辅助数据 |
| 5 | `unified_type_analyzer.cpp` | 增强推导能力 |
| 6 | 新增 `type_system.h/.cpp` | HM 合一求解器 |
| 7 | 新增 `func_summary.h/.cpp` | 函数摘要系统 |
| 8 | 新增 `escape_analysis.h/.cpp` | 逃逸分析 |
| 9 | 新增 `offset_layout.h/.cpp` | 偏移分配 |

---

**文档结束。请评估后确认执行顺序和优先级。**
