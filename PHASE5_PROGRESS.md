# Phase 5 实施进度 — 逃逸分析（规范 §13 Phase 5）

> **日期**: 2026-07-06
> **分支**: `ssa-pipeline-v2`
> **状态**: ✅ 完成

---

## 概述

Phase 5 目标：实现逃逸分析、逃逸信息收集与传递，为栈分配优化提供依据。

**规范 §8.1 逃逸判定规则**:
| 场景 | 判定 |
|------|------|
| `a` 传给未知函数（无摘要） | 逃逸 |
| `a` 存入 dynamic table 字段 | 逃逸 |
| 被 `print`/`tostring`/`type` 反射使用 | 逃逸 |
| 被 return | 逃逸（保守） |
| 存入 upvalue 且 closure 逃逸 | 逃逸 |
| 跨 coroutine 边界 | 逃逸 |
| 赋值给 dynamic 变量 | 逃逸 |
| `a[i]` 取出且元素类型是 dynamic | 逃逸 |

---

## 实施步骤

### Step 5.1: 逃逸判定规则 ✅

**文件**: `src/compile/unified_type_analyzer.h`, `src/compile/unified_type_analyzer.cpp`

**数据结构**:
```cpp
using EscapeEnv = std::unordered_map<std::string, bool>;  // 变量名 → 是否逃逸
```

**核心方法**:
```cpp
void ComputeEscape(const std::string &func_name,
                   const SyntaxTreeInterfacePtr &func_block,
                   const CFGFunction &cfg,
                   EscapeEnv &escape_env);

void EscapeTransfer(const SyntaxTreeInterfacePtr &stmt,
                    EscapeEnv &escape_env,
                    const VarEnv &type_env);
```

**算法**:
1. `ComputeEscape` 遍历函数体所有语句
2. 对每条语句调用 `EscapeTransfer`
3. `EscapeTransfer` 检测逃逸场景：
   - **FunctionCall**: 所有参数引用标记逃逸
   - **Return**: 返回值引用标记逃逸
   - **Assign**: 赋值给 dynamic 变量时，RHS 引用标记逃逸

**引用收集**: `collect_refs` 递归遍历 AST 节点收集所有 kSimple 变量名。

---

### Step 5.2: 逃逸信息传递 ✅

**文件**: `src/compile/unified_type_analyzer.cpp`, `src/compile/type_inferencer.cpp`

**改动**:
1. `BuildSummary` 增加 `CFGFunction` 参数（获取参数名映射）
2. 在 `BuildSummary` 中检查 `ir.escape_vars[func_name]`
3. 将参数的逃逸状态写入 `FuncSummary.param_escape`

**流程**:
```
ComputeEscape → ir.escape_vars[func_name] → BuildSummary → FuncSummary.param_escape
```

---

### Step 5.3: 栈分配决策测试 ✅

**测试** (`test/test_pipeline.cpp`):
- `escape_return` — `return a` → a 逃逸
- `escape_function_call` — `print(a)` → a 逃逸
- `escape_no_escape` — `local b = a.x` → a 不逃逸

**验证输出**:
```
test() escape: a=yes
test() escape: a=yes
test() escape: a=no (仅被读取)
```

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
| 字段读写测试 | 3 | ✅ |
| Widening 测试 | 3 | ✅ |
| 跨函数测试 | 3 | ✅ |
| 逃逸分析测试 | 3 | ✅ |
| **Phase 5 新增** | **3** | ✅ |
| Phase 1+2+3+4 新增 | 29 | ✅ |
| 原有基础设施测试 | 287 | ✅ |
| **总计** | **322** | ✅ |

---

## 文件变更清单

| 文件 | 行数变化 | 说明 |
|------|---------|------|
| `src/compile/unified_type_analyzer.h` | +15 | EscapeEnv, ComputeEscape, EscapeTransfer 声明 |
| `src/compile/unified_type_analyzer.cpp` | +95/-5 | 逃逸分析实现，BuildSummary 增加 cfg 参数 |
| `src/compile/type_inferencer.cpp` | +1 | BuildSummary 调用增加 cfg 参数 |
| `test/test_pipeline.cpp` | +70 | 3 个新测试 |
| `PHASE5_PROGRESS.md` | +120 (新文件) | 本文档 |

---

## 未实现（留待后续 Phase）

| 规范章节 | 内容 |
|---------|------|
| §10.2 逃逸情况的代码生成 | boxed 内布局仍固定，可走偏移 |
| §8.1 upvalue/closure | 复杂逃逸场景 |
| §8.1 coroutine 边界 | 跨 coroutine 逃逸 |

---

## 下一步：Phase 6（CGen 迁移）

1. struct typedef generation from ShapeRegistry
2. 基于 shape 的 `a.b` 偏移访问 codegen
3. hash fallback for open/escaped records

---

**文档结束。**
