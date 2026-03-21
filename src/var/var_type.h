#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

enum class VarType {
    Nil,
    Bool,
    Int,
    Float,
    String,
    Table,

    Min = Nil,
    Max = Table,
};

inline std::string VarTypeToString(VarType t) {
    switch (t) {
        case VarType::Nil:
            return "Nil";
        case VarType::Bool:
            return "Bool";
        case VarType::Int:
            return "Int";
        case VarType::Float:
            return "Float";
        case VarType::String:
            return "String";
        case VarType::Table:
            return "Table";
        default:
            return "UNKNOWN";
    }
}

static_assert(sizeof(VarType) == sizeof(int), "VarType must be an integer type");

}// namespace fakelua
