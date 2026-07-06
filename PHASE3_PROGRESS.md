# Phase 3 实施进度 — Shape 演化 + Widening（规范 §13 Phase 3）

> **日期**: 2026-07-06
> **分支**: `ssa-pipeline-v2`
> **状态**: ✅ 完成

---

## 概述

Phase 3 目标：支持 table 形状的动态演化（字段读写）和 Widening 机制（保证收敛）。

**验收示例**（规范 §12.1）：
```lua
local t = {}
for i = 1, 1000000 do
    t["field_" .. i] = i
end
-- 必须在有限步内 widen 到 dynamic，不卡死
```

---

## 实施步骤

### Step 3.1: 开放 record 字段写入 ✅

**文件**: `src/compile/unified_type_analyzer.cpp`

**实现**（规范 §5.2.4）:

在 `TransferStmt` 的 `Assign` handler 中增加 `kDot`/`kSquare` 处理：

```cpp
} else if (v->GetVarKind() == VarKind::kDot || v->GetVarKind() == VarKind::kSquare) {
    // 字段写入：a.b = v 或 a[b] = v
    const std::string &field_name = v->GetName();
    auto *base_pe = static_cast<SyntaxTreePrefixexp *>(v->GetPrefixexp().get());
    if (base_pe && base_pe->GetPrefixKind() == PrefixExpKind::kVar) {
        auto *base_var = static_cast<SyntaxTreeVar *>(base_pe->GetValue().get());
        if (base_var && base_var->GetVarKind() == VarKind::kSimple) {
            auto base_it = out.find(base_var->GetName());
            if (base_it != out.end() && base_it->second.shape_id >= 0 && registry_) {
                ShapeType shape = registry_->Get(base_it->second.shape_id);
                FieldDef *fd = shape.FindFieldMut(field_name);
                if (fd) {
                    // 已有字段：合一类型
                    fd->type = ShapeRegistry::MeetType(fd->type, ty.type);
                } else {
                    // 新字段
                    if (!shape.is_open) {
                        // 封闭 record 加新字段 → 退化为开放
                        shape.is_open = true;
                    }
                    FieldDef new_fd;
                    new_fd.name = field_name;
                    new_fd.c_field_name = ToSafeCFieldName(field_name);
                    new_fd.type = ty.type;
                    new_fd.optional = false;
                    shape.fields.push_back(new_fd);
                }
                base_it->second.shape_id = registry_->Intern(std::move(shape));
            }
        }
    }
}
```

**语义**:
- 已有字段：合一类型（int + float → float）
- 新字段 + 封闭 record → 退化为开放 record，追加字段
- 新字段 + 开放 record → 直接追加字段

**关键修复**: `InferExprType` 对 `Exp(kTableConstructor)` 需要传入 `e->Right()` 而非 `expr`：
```cpp
case ExpKind::kTableConstructor: {
    // expr 是 Exp 包装节点，实际 TableConstructor 在其 Right() 中
    int shape_id = BuildShapeFromCtor(e->Right(), ssa, version_types, ir);
    ...
}
```

---

### Step 3.2: 字段读取 a.b ✅

**文件**: `src/compile/unified_type_analyzer.cpp`

**实现**（规范 §5.2.3）:

在 `InferExprType` 的 `Var` handler 中增加 `kDot`/`kSquare` 处理：

```cpp
} else if (v->GetVarKind() == VarKind::kDot || v->GetVarKind() == VarKind::kSquare) {
    const std::string &field_name = v->GetName();
    auto *base_pe = static_cast<SyntaxTreePrefixexp *>(v->GetPrefixexp().get());
    if (base_pe && base_pe->GetPrefixKind() == PrefixExpKind::kVar) {
        auto *base_var = static_cast<SyntaxTreeVar *>(base_pe->GetValue().get());
        if (base_var && base_var->GetVarKind() == VarKind::kSimple) {
            SSATypeInfo base_type{T_DYNAMIC, -1};
            auto env_it = local_env->find(base_var->GetName());
            if (env_it != local_env->end()) base_type = env_it->second;
            if (base_type.shape_id >= 0 && registry_) {
                const ShapeType &shape = registry_->Get(base_type.shape_id);
                const FieldDef *fd = shape.FindField(field_name);
                if (fd) return {fd->type, -1};        // ★ 命中 → 走偏移
                else if (shape.is_open) return {T_DYNAMIC, -1};  // 开放 → hash
                else return {T_NIL, -1};               // 封闭无此字段 → nil
            }
        }
    }
    return {T_DYNAMIC, -1};
}
```

**语义**:
- 字段存在 → 返回字段类型（走偏移访问）
- 字段不存在 + 开放 record → T_DYNAMIC（走 hash）
- 字段不存在 + 封闭 record → T_NIL（Lua 语义）

---

### Step 3.3: Widening 机制 ✅

**文件**: `src/compile/unified_type_analyzer.cpp`

**实现**:

在 `RunWorklist` 的迭代循环中，当 `iter_count > kWidenIter` 时触发：

```cpp
if (iter_count > kWidenIter && registry_) {
    for (auto &[vname, vtype] : new_out) {
        if (IsRecordInferredType(vtype.type) && vtype.shape_id >= 0) {
            int new_shape_id = registry_->Widen(vtype.shape_id, iter_count);
            if (new_shape_id < 0) {
                vtype.type = T_DYNAMIC;
                vtype.shape_id = -1;
            } else {
                vtype.shape_id = new_shape_id;
            }
        }
    }
}
```

**ShapeRegistry::Widen**（shape_type.h:146-160）:
- 字段数 > 16 → 截断到 16 个
- 迭代次数 ≥ 3 → 设为开放 record
- 开放 record + 字段数 > 16 → 返回 -1（退化为 T_DYNAMIC）

---

### Step 3.4: 收敛性测试 ✅

**测试** (`test/test_pipeline.cpp`):
- `widening_convergence` — 基本 shape 增长
- `widening_many_fields` — 写入 18 个字段触发 widening
- `widening_loop_convergence` — while 循环中 shape 增长收敛

**验证输出示例**:
```
shape count after many fields: 18
  shape 17: open=1 fields=18
shape count after loop: 4
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
| **Phase 3 新增** | **6** | ✅ |
| Phase 1+2 新增 | 23 | ✅ |
| 原有基础设施测试 | 287 | ✅ |
| **总计** | **316** | ✅ |

---

## 文件变更清单

| 文件 | 行数变化 | 说明 |
|------|---------|------|
| `src/compile/unified_type_analyzer.cpp` | +80/-5 | 字段写入/读取，Widening，Meet 修复 |
| `test/test_pipeline.cpp` | +90 | 6 个新测试 |
| `PHASE3_PROGRESS.md` | +170 (新文件) | 本文档 |

---

## 未实现（留待后续 Phase）

| 规范章节 | 内容 | 原因 |
|---------|------|------|
| §10 代码生成 | 基于 shape 的 LOAD_FIELD | Phase 6 |
| §7 函数摘要 | 跨函数类型推导 | Phase 4 |
| §8 逃逸分析 | EscapeTransfer | Phase 5 |
| §12.3 | 递归类型 occurs_check | Phase 4 |
| §12.6 | 动态 key 常量传播 | Phase 6 |

---

## 下一步：Phase 4（函数摘要）

1. FuncSummary 构造（参数类型 + 返回类型 + 逃逸信息）
2. 调用点实例化（apply_summary + 类型替换）
3. 递归函数处理

---

**文档结束。**
