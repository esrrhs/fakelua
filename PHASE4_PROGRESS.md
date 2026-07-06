# Phase 4 实施进度 — 函数摘要（规范 §13 Phase 4）

> **日期**: 2026-07-06
> **分支**: `ssa-pipeline-v2`
> **状态**: ✅ 完成

---

## 概述

Phase 4 目标：实现过程间分析框架 — 函数摘要构造、调用点实例化、递归函数处理。

**验收示例**（规范 §14.1 #4）：
```lua
local function make() return {x=1, y=2} end
local p = make()
local v = p.x   -- 跨函数推导
```

---

## 实施步骤

### Step 4.1: FuncSummary 增强 ✅

**文件**: `src/compile/unified_type_analyzer.cpp`

**改动**: `BuildSummary` 增加参数类型推导：

```cpp
// 从 version_types 推导参数类型（规范 §7.2）
for (size_t i = 0; i < ssa.param_versions.size(); ++i) {
    int ver = ssa.param_versions[i];
    auto vt_it = version_types.find(ver);
    if (vt_it != version_types.end()) {
        s.param_types[i] = vt_it->second;
    } else {
        s.param_types[i] = {T_DYNAMIC, -1};
    }
}
```

**FuncSummary 结构**（compile_common.h）:
```cpp
struct FuncSummary {
    std::string func_name;
    std::vector<SSATypeInfo> param_types;    // 每个参数的 SSA 类型
    SSATypeInfo ret_type{T_UNKNOWN, -1};      // 返回类型（所有 return 的 meet）
    std::vector<bool> param_escape;           // 参数是否逃逸
    bool is_vararg = false;
    bool being_built = false;                 // 递归检测标记
};
```

---

### Step 4.2: 调用点实例化 ✅

**文件**: `src/compile/unified_type_analyzer.cpp`

**改动**: 增强 `InferExprType` 的 `FunctionCall` handler：

```cpp
case SyntaxTreeType::FunctionCall: {
    // 支持 Function 和 PrefixExp(Var) 形式的调用
    std::string callee = extract_callee_name(fc);
    if (!callee.empty()) {
        auto it = ir.func_summaries.find(callee);
        if (it != ir.func_summaries.end()) {
            if (it->second.being_built) return {T_DYNAMIC, -1};  // 递归
            if (it->second.ret_type.type != T_UNKNOWN && it->second.ret_type.type != T_DYNAMIC)
                return it->second.ret_type;
        }
    }
    return {T_DYNAMIC, -1};
}
```

**特性**:
- 支持 `f()` 和 `obj:method()` 形式的调用名提取
- 递归检测（`being_built` 标记）
- 返回类型直接来自摘要

---

### Step 4.3: 递归函数处理 ✅

**机制**:
1. 第一次见到函数：`being_built = true`，分析函数体
2. 递归调用：检测到 `being_built = true`，返回 `T_DYNAMIC`
3. 分析完成：`being_built = false`，摘要可用

**实现**: 在 `RunSSAAnalysis`（type_inferencer.cpp）中设置标记：
```cpp
ir.func_summaries[name].being_built = true;
uta.Analyze(name, ...);
uta.BuildSummary(name, ...);
ir.func_summaries[name].being_built = false;
```

---

### Step 4.4: 跨函数测试 ✅

**测试** (`test/test_pipeline.cpp`):
- `interprocedural_basic` — `make()` 返回 `{x=1, y=2}`，摘要记录返回类型为 Record
- `interprocedural_param_types` — `add(a, b)` 记录 2 个参数类型
- `interprocedural_polymorphic` — `id(x)` 多态函数

**验证输出**:
```
make() ret_type: T_RECORD shape=0
add() params: 2
  param 0: T_DYNAMIC
  param 1: T_DYNAMIC
id() ret_type: T_INT
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
| **Phase 4 新增** | **3** | ✅ |
| Phase 1+2+3 新增 | 26 | ✅ |
| 原有基础设施测试 | 287 | ✅ |
| **总计** | **319** | ✅ |

---

## 文件变更清单

| 文件 | 行数变化 | 说明 |
|------|---------|------|
| `src/compile/unified_type_analyzer.cpp` | +25/-5 | BuildSummary 参数类型，FunctionCall 增强 |
| `test/test_pipeline.cpp` | +120 | AnalyzeSource 增强（函数分析），3 个新测试 |
| `PHASE4_PROGRESS.md` | +130 (新文件) | 本文档 |

---

## 未实现（留待后续 Phase）

| 规范章节 | 内容 | 原因 |
|---------|------|------|
| §10 代码生成 | 基于 shape 的 LOAD_FIELD | Phase 6 |
| §8 逃逸分析 | EscapeTransfer | Phase 5 |
| §12.3 | 递归类型 occurs_check | 需要 HM 类型变量 |
| §12.2 | 完整多态（每次调用独立实例化） | 需要类型变量替换 |

---

## 下一步：Phase 5（逃逸分析）

1. 逃逸判定规则（规范 §8.1）
2. 栈分配优化
3. dynamic 退化路径

---

**文档结束。**
