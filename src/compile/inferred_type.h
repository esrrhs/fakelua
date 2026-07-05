#pragma once

namespace fakelua {

enum InferredType {
    T_UNKNOWN = 0,
    T_NIL,
    T_BOOL,
    T_INT,
    T_FLOAT,
    T_STRING,
    T_RECORD,        // 封闭 record（结构体）
    T_RECORD_OPEN,   // 开放 record（可能还有未知字段）
    T_DYNAMIC,       // 完全未知，走 CVar
};

inline bool IsNumericInferredType(InferredType type) {
    return type == T_INT || type == T_FLOAT;
}

inline bool IsRecordInferredType(InferredType type) {
    return type == T_RECORD || type == T_RECORD_OPEN;
}

}// namespace fakelua
