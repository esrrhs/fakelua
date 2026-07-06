# Phase 2 实施进度 — 控制流（规范 §13 Phase 2）

> **日期**: 2026-07-06
> **分支**: `ssa-pipeline-v2`
> **状态**: ✅ 完成

---

## 概述

Phase 2 目标：实现控制流合并时的类型推导，包括 Meet 操作、φ 节点类型推导、工作表不动点迭代。

**验收示例**（规范 §14.1 #3）：
```lua
local c
if cond then c = {x=1} else c = {x=2} end
local v = c.x   -- v : int，可走偏移
```

---

## 实施步骤

### Step 2.1: Meet 操作测试 ✅

**文件**: `test/test_pipeline.cpp`

**验证内容**（规范 §5.3 Meet 真值表）:

| Meet 操作 | 结果 | 验证 |
|-----------|------|------|
| int ∧ int | int | ✅ |
| int ∧ float | float（提升）| ✅ |
| int ∧ string | dynamic | ✅ |
| dynamic ∧ 任意 | dynamic | ✅ |
| nil ∧ int | int | ✅ |
| Rec{b:int} ∧ Rec{b:float} | Rec{b:float} | ✅ |
| Rec{b:int} ∧ Rec{c:int} | Rec{b:int, c:int} | ✅ |
| 封闭 ∧ 开放 | 开放 | ✅ |

**实现位置**:
- `ShapeRegistry::MeetType()` — 标量 meet（shape_type.h:136-143）
- `ShapeRegistry::Meet()` — Record meet（shape_type.h:101-133）
- `UnifiedTypeAnalyzer::Meet()` — SSATypeInfo meet（需要 registry 合并 shape）

**关键修复**: `UnifiedTypeAnalyzer::Meet()` 当两个 record shape_id 不同时，调用 `registry_->Meet(shape_id_a, shape_id_b)` 创建合并后的 shape，而非简单设为 -1。

---

### Step 2.2: φ 节点类型推导 ✅

**文件**: `src/compile/unified_type_analyzer.cpp`, `src/compile/ssa.h`

**实现**:

1. **SSAFunction 新增字段**:
   ```cpp
   std::unordered_map<int, SSATypeInfo> version_types;  // 版本号 → 类型
   ```

2. **φ 类型计算**（在 `Analyze` 中，RunWorklist 完成后）:
   ```cpp
   for (auto &kv : ssa.block_phis) {
       for (auto &phi : kv.second) {
           SSATypeInfo merged = ...;
           for (int pred_id : blk->pred_ids) {
               auto pred_type = block_outs[pred_id][phi.var_name];
               merged = Meet(merged, pred_type, registry_);
           }
           version_types[phi.result_version] = merged;
       }
   }
   ```

**算法**: φ 结果类型 = 所有前驱块 out_env 中该变量的类型的 Meet。

**测试** (`test/test_pipeline.cpp`):
- `phi_type_if_else` — if-else 中 `a = 2.0`(float) / `a = 3`(int) → φ 结果类型 = float

**验证输出**:
```
version 1: type=T_FLOAT shape=-1
```

---

### Step 2.3: 工作表不动点验证 ✅

**文件**: `src/compile/unified_type_analyzer.cpp`（已有实现）

**验证内容**:
- 分支合并时不同类型赋值 → Meet 正确工作
- while 循环中的 φ 节点正确插入
- 工作表迭代到不动点

**测试** (`test/test_pipeline.cpp`):
- `flow_sensitive_if_merge_same_field` — 相同字段不同数值类型
- `flow_sensitive_branch_isolation` — 不同变量名分支隔离
- `flow_sensitive_numeric_promotion` — 数值类型提升
- `flow_sensitive_while_convergence` — while 循环收敛

**验证输出示例**:
```
while header 2 phis: 1
SSA[test_func] params=[]
  phi in block 2: iv1 = φ(0, 2)
  var i: v0@0, v1@2, v2@3
```

---

### Step 2.4: 流敏感类型验证 ✅

**验证内容**:
- 分支内对同一变量赋不同类型 → 合并后正确 Meet
- 分支内对不同变量赋值 → 互不干扰
- 循环中的类型收敛

**测试**: 同 Step 2.3（flow_sensitive_* 测试）

---

## 关键修复记录

### 1. UTA::Meet 需要 registry 参数

**问题**: 静态 `Meet` 方法无法访问 `registry_` 成员。

**修复**: 添加 `ShapeRegistry *reg = nullptr` 默认参数：
```cpp
static SSATypeInfo Meet(const SSATypeInfo &a, const SSATypeInfo &b, ShapeRegistry *reg = nullptr);
```

非静态调用点传递 `registry_`，静态调用点（MeetEnv）使用默认 nullptr。

### 2. LinkExprToTargetShape 递归修复

**问题**: 只处理 Block 和 TableConstructor，不处理 Exp/ExpList 包装。

**修复**: 增加 Exp 和 ExpList 的递归分支：
```cpp
case SyntaxTreeType::Exp: {
    auto *e = static_cast<SyntaxTreeExp *>(node.get());
    LinkExprToTargetShape(e->Right(), target_name, ...);
    break;
}
case SyntaxTreeType::ExpList: {
    auto *el = static_cast<SyntaxTreeExplist *>(node.get());
    for (auto &exp : el->Exps()) LinkExprToTargetShape(exp, ...);
    break;
}
```

### 3. SSAFunction 添加 version_types

**问题**: φ 结果版本没有类型信息。

**修复**: 在 `ssa.h` 添加 `unordered_map<int, SSATypeInfo> version_types`，在 φ 类型计算时填充。

---

## 测试结果

| 测试套件 | 数量 | 状态 |
|---------|------|------|
| CFG 测试 | 5 | ✅ |
| SSA 测试 | 4 | ✅ |
| 类型推导测试 | 3 | ✅ |
| Shape 测试 | 3 | ✅ |
| Meet 测试 | 3 | ✅ |
| φ 类型测试 | 1 | ✅ |
| 流敏感测试 | 4 | ✅ |
| **Phase 2 新增** | **11** | ✅ |
| Phase 1 新增 | 12 | ✅ |
| 原有基础设施测试 | 287 | ✅ |
| **总计** | **310** | ✅ |

---

## 文件变更清单

| 文件 | 行数变化 | 说明 |
|------|---------|------|
| `src/compile/ssa.h` | +5 | 添加 version_types 字段 |
| `src/compile/unified_type_analyzer.cpp` | +40/-5 | φ 类型计算，Meet 修复，LinkExprToTargetShape 递归 |
| `test/test_pipeline.cpp` | +110 | 11 个新测试 |
| `PHASE2_PROGRESS.md` | +120 (新文件) | 本文档 |

---

## 未实现（留待后续 Phase）

| 规范章节 | 内容 | 原因 |
|---------|------|------|
| §5.4 Widening | 循环中 shape 增长截断 | Phase 3 |
| §10 代码生成 | 基于 shape 的 LOAD_FIELD | Phase 6 |
| §7 函数摘要 | 跨函数类型推导 | Phase 4 |
| §8 逃逸分析 | EscapeTransfer | Phase 5 |
| §12.1 | 循环里 shape 增长收敛 | Phase 3 (Widening) |
| §12.3 | 递归类型 occurs_check | Phase 4 |

---

## 下一步：Phase 3（Shape 演化 + Widening）

1. 支持 `a.b = v` 给开放 record 加字段
2. Widening 机制（字段数超 16 → 截断，迭代超 3 → widen）
3. 收敛性测试（规范 §14.3）

---

**文档结束。**
