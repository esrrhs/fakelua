#pragma once

namespace fakelua {

// 简单类型枚举 —— 推断器给"单值"贴的静态类型
//
// 这里的"简单"是相对于 hm_type.h 的 HM 类型系统而言的：
//   - 该枚举只区分"值看起来是什么"（int/float/dynamic/record），
//   - 它**不**表达任何关于类型变量、shape、record 字段的细节。
//
// InferredType 与 HM 类型在项目里的用途分工（见 SSA_PIPELINE_STATUS.md / ssa-pipeline-refactoring.md）：
//
//   InferredType（简单类型）：
//     - 用于 shape_type.h 的 FieldDef.type，表达一个字段的"大概"类型
//     - 快速路径：当一个值的静态结构足够简单、无需打开 HM 推导就能判断时，
//       直接贴这个枚举值
//     - 当 HM 推断失败或需要降级时，最终也会收敛到 T_DYNAMIC
//
//   HM 类型（hm_type.h）：
//     - 表达含类型变量（TY_VAR）、函数、record、array、union 的任意类型表达式
//     - 驱动统一化（unify）和 OccursCheck 等经典 HM 算法
//     - 用于需要精确追踪"未知类型如何相互约束"的场景（函数返回值、递归等）
//
// 简单地说：InferredType 是"近似标签"，HM 类型是"精确推导结构"。
// 推断流程里先用快速路径尽量打简单标签，必要时再走 HM。
enum InferredType {
    T_UNKNOWN = 0,// 尚未被推断触碰过的初始状态（不应在正常流程中长期保留）
    T_NIL,        // Lua nil
    T_BOOL,       // boolean
    T_INT,        // 整数
    T_FLOAT,      // 浮点
    T_STRING,     // 字符串
    T_RECORD,     // 封闭 record（结构体）——字段固定，走偏移访问
    T_RECORD_OPEN,// 开放 record —— 可能存在未知字段，已知字段仍走偏移
    T_DYNAMIC,    // 完全未知，运行时用 CVar 不透明表示，不做静态布局
};

// 判断是否为数值类型（int 或 float）
// 用于数值运算的快速路径：只要两边都是数值，结果就是数值（具体 int/float 由 MeetType 决定）
inline bool IsNumericInferredType(InferredType type) {
    return type == T_INT || type == T_FLOAT;
}

// 判断是否为 record 类型（封闭或开放）
// 用于决定是否可以走"字段偏移"访问路径
inline bool IsRecordInferredType(InferredType type) {
    return type == T_RECORD || type == T_RECORD_OPEN;
}

}// namespace fakelua
