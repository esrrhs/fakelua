# Phase 1 实施进度 — 最小可运行（规范 §13 Phase 1）

> **日期**: 2026-07-06
> **分支**: `ssa-pipeline-v2`
> **状态**: ✅ 完成

---

## 概述

Phase 1 目标：按规范 §13 Phase 1 实现最小可运行的 SSA/CFG/Shape 编译管线。

```
源码 → Lexer/Parser → AST → CFG构造 → SSA构造 → 类型推导引擎(Shape) → 验证
```

**验收示例**（规范 §14.1）：
```lua
local a = {b=1, c=2}
local s = a.b + a.c
```

---

## 实施步骤

### Step 1.1: CFG 构造 + DumpToString ✅

**文件**: `src/compile/cfg.h`, `src/compile/cfg.cpp`

**改动**:
- 修复 `BuildIf` 汇合块 bug：原来所有分支直接连到 exit，现在创建正确的 merge 块
- 重构 `BuildBlock` 返回尾块 id，不再自动连接 exit（由调用者控制）
- 修复 `ComputeDominanceFrontier`：使用正确的 Cytron 算法
  - 旧实现：只在多前驱块的 pred 上迭代，逻辑有误
  - 新实现：DF(n) = { y | ∃p ∈ pred(y), n 支配 p 但 n 不严格支配 y }
  - 关键：当 n ∈ pred(y) 时，n 支配自身（reflexive），应包含该情况
- 新增 `CFGFunction::DumpToString()` 用于测试验证

**测试** (`test/test_pipeline.cpp`):
- `cfg_sequential` — 顺序语句产生 entry + exit = 2 blocks
- `cfg_if_else_merge` — if-else 产生合并块，merge 有 2 个前驱
- `cfg_while_loop` — while 循环有回边 body→header
- `cfg_for_loop` — for 循环有 exit 后继
- `cfg_dominators_basic` — entry 支配所有块，每块支配自身

**验证输出示例**:
```
CFG[test_func] entry=0 exits=[1]
  block 0: preds=[] succs=[3,4] stmts=2
  block 1: preds=[2] succs=[] stmts=0
  block 2: preds=[3,4] succs=[1] stmts=1   ← merge 块
  block 3: preds=[0] succs=[2] stmts=1     ← then 分支
  block 4: preds=[0] succs=[2] stmts=1     ← else 分支
```

---

### Step 1.2: 完整 SSA 构造（Cytron 算法）✅

**文件**: `src/compile/ssa.h`, `src/compile/ssa.cpp`

**数据结构重构**:
```cpp
struct PhiNode {
    int var_id;              // 变量索引
    std::string var_name;    // 变量名（调试用）
    int result_version;      // φ 产生的新版本号
    std::vector<int> arg_versions; // 各前驱块传入的版本号
};

struct SSAFunction {
    std::unordered_map<int, std::vector<PhiNode>> block_phis;  // block_id → φ 列表
    std::unordered_map<int, std::string> version_to_name;     // 反查版本→变量名
    // ... 其他字段
};
```

**算法实现**:

1. **CollectDefBlocks**: 遍历每个块的语句，收集变量的定义块集合
   - 支持 LocalVar、Assign、ForLoop、ForIn

2. **BuildDomTree**: 从支配关系推导支配树
   - 对每个非入口块，找到直接支配者（idom）

3. **InsertPhis (Cytron Step 1)**:
   ```
   for each variable v:
       worklist = 所有定义 v 的块
       while worklist 非空:
           n = worklist.pop()
           for d in DF(n):
               if d 中还没有 v 的 φ:
                   在 d 入口插入 v 的 φ 节点
                   if d 不定义 v: worklist.add(d)
   ```

4. **RenameVariables (Cytron Step 2)**:
   - DFS 遍历支配树
   - 对每个块：先处理 φ 节点（分配结果版本），再处理语句
   - 替换使用（使用栈顶版本），分配定义（压入新版本）
   - 处理后继块中的 φ 参数

**辅助功能**:
- `GetDefNames()`: 从语句 AST 提取定义的变量名
- `GetUseNames()`: 从语句 AST 提取使用的变量名（递归进入表达式）
- `SSAFunction::DumpToString()`: 序列化为可读字符串

**测试** (`test/test_pipeline.cpp`):
- `ssa_sequential_assign` — 顺序赋值产生多版本，无 φ
- `ssa_if_else_phi` — if-else 在 merge 块插入 φ
- `ssa_while_phi` — while 循环在 header 插入 φ
- `ssa_param_versions` — 函数参数有初始版本

**验证输出示例**:
```
SSA[test_func] params=[]
  phi in block 2: av1 = φ(3, 4)         ← φ 节点，merge 块
  var b: v2@2
  var a: v0@0, v1@2, v3@3, v4@4        ← a 有 4 个版本
```

---

### Step 1.3: 简单类型推导 ✅

**文件**: `src/compile/unified_type_analyzer.cpp`（已有实现，验证完整性）

**已实现规则**（规范 §5.2）:

| 语法 | 推导规则 | 实现位置 |
|------|---------|---------|
| `1` | T_INT | `InferExprType(kNumber)` |
| `1.0` | T_FLOAT | `InferExprType(kNumber)` |
| `"x"` | T_STRING | `InferExprType(kString)` |
| `nil` | T_NIL | `InferExprType(kNil)` |
| `true/false` | T_BOOL | `InferExprType(kTrue/kFalse)` |
| `a + b` | MeetType(lt, rt) | `InferExprType(kBinop)` |
| `local x = e` | type(x) = type(e) | `TransferStmt(LocalVar)` |
| `x = e` | type(x) = type(e) | `TransferStmt(Assign)` |
| `{k=v}` | Record{fields} | `InferExprType(kTableConstructor)` |

**测试** (`test/test_pipeline.cpp`):
- `type_literals` — 字面量类型推导（验证 main_ssa_types 非空）
- `type_binop` — 二元运算类型推导（验证 binop 节点有类型）
- `type_var_propagation` — 变量赋值传播

---

### Step 1.4: Shape 字面量构造 ✅

**文件**: `src/compile/unified_type_analyzer.cpp`（已有实现）

**已实现功能**:
- `BuildShapeFromCtor()`: 解析 table constructor AST，构建 `ShapeType`
- 字段类型推导：递归调用 `InferExprType` 获取值类型
- `ShapeRegistry::Intern()`: 相同签名的 shape 共用同一个 shape_id
- 封闭 record：`is_open = false`（字面量默认封闭）

**修复**:
- `LinkExprToTargetShape()`: 增加对 `Exp` 和 `ExpList` 的递归，正确找到嵌套在表达式中的 TableConstructor

**测试** (`test/test_pipeline.cpp`):
- `shape_literal` — `{b=1, c=2.0}` 产生封闭 record，字段 b:T_INT, c:T_FLOAT
- `shape_empty` — `{}` 不产生有字段的 shape
- `shape_nested` — `{p={x=1, y=2}}` 产生 2 个 shape

**验证输出示例**:
```
shape count: 1
  shape 0: open=0 fields=[b:T_INT, c:T_FLOAT]
```

---

## 测试结果

| 测试套件 | 数量 | 状态 |
|---------|------|------|
| CFG 测试 | 5 | ✅ 全部通过 |
| SSA 测试 | 4 | ✅ 全部通过 |
| 类型推导测试 | 3 | ✅ 全部通过 |
| Shape 测试 | 3 | ✅ 全部通过 |
| **Phase 1 新增** | **15** | ✅ |
| 原有基础设施测试 | 287 | ✅ |
| **总计** | **302** | ✅ |

---

## 文件变更清单

| 文件 | 行数变化 | 说明 |
|------|---------|------|
| `src/compile/cfg.h` | +10 | 添加 DumpToString 声明，修正 BuildBlock 签名 |
| `src/compile/cfg.cpp` | +30/-20 | 修复 BuildIf merge 块，修正 DF 算法，新增 DumpToString |
| `src/compile/ssa.h` | +60/-10 | 完整重构数据结构 |
| `src/compile/ssa.cpp` | +370/-40 | 完整实现 Cytron 算法 |
| `src/compile/unified_type_analyzer.cpp` | +15/-5 | 修复 LinkExprToTargetShape 递归 |
| `test/test_pipeline.cpp` | +430 (新文件) | 15 个测试用例 |

---

## 未实现（留待后续 Phase）

| 规范章节 | 内容 | 原因 |
|---------|------|------|
| §4 HM 类型推导 | 类型变量 + 合一 + occurs_check | Phase 1 使用简单 Type 枚举 |
| §9 偏移分配 | StructLayout 计算 | 用户要求跳过，直接用 C 结构体指针 |
| §10 代码生成 | 基于 shape 的 LOAD_FIELD | CGen 已退化为纯 CVar |
| §5.2.3/5.2.4 | 字段读取/写入 `a.b` / `a.b = v` | Phase 1 仅构造 |
| §5.3/5.4 | Meet/Widening | Phase 2 控制流 |
| §7 函数摘要 | FuncSummary + 调用点实例化 | Phase 4 |
| §8 逃逸分析 | EscapeTransfer | Phase 5 |

---

## 下一步：Phase 2（控制流）

1. 完整 φ 节点类型推导（当前 φ 结果版本的类型未设置）
2. Meet 操作（控制流合并时类型 meet）
3. 工作表算法（流敏感不动点迭代）
4. flow-sensitive 类型

---

**文档结束。**
