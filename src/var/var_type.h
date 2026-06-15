#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

// This enum must be kept in sync with the enum in src/compile/c_runtime_header.h
enum class VarType {
    Nil = 0,
    Bool = 1,
    Int = 2,
    Float = 3,
    String = 4,
    StringId = 5,
    Table = 6,
    Multi = 7,

    Min = Nil,
    Max = Table,
};

inline std::string VarTypeToString(VarType var_type) {
    switch (var_type) {
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
        case VarType::StringId:
            return "StringId";
        case VarType::Table:
            return "Table";
        case VarType::Multi:
            return "Multi";
        default:
            return "UNKNOWN";
    }
}

static_assert(sizeof(VarType) == sizeof(int), "VarType must be an integer type");

}// namespace fakelua
