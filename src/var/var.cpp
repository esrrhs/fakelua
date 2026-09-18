#include "var.h"
#include "fakelua.h"
#include "state/const_string.h"
#include "state/state.h"
#include "util/common.h"
#include "var_multi.h"
#include "var_string.h"

namespace fakelua {

Var const_null_var;

VarString *Var::GetString() const {
    DEBUG_ASSERT(type_ == static_cast<int>(VarType::String) || type_ == static_cast<int>(VarType::StringId));
    if (LIKELY(type_ == static_cast<int>(VarType::String))) {
        return data_.s;
    } else /*if (type_ == static_cast<int>(VarType::StringId))*/ {
        return ConstString::GetVarString(data_.i);
    }
}

void Var::SetTempString(State *state, const std::string_view &val) {
    type_ = static_cast<int>(VarType::String);
    data_.s = VarString::AllocTemp(state, val);
}

void Var::SetConstString(State *state, const std::string_view &val) {
    type_ = static_cast<int>(VarType::StringId);
    data_.i = state->GetConstString().Alloc(val);
}

bool Var::TestTrue() const {
    switch (Type()) {
        case VarType::Nil:
            return false;
        case VarType::Bool:
            return GetBool();
        case VarType::Multi:
            return true;
        default:
            return true;
    }
}

std::string Var::ToString(bool has_quote, bool has_postfix) const {
    std::string ret;
    DEBUG_ASSERT(Type() >= VarType::Min && (Type() <= VarType::Max || Type() == VarType::Multi));
    switch (Type()) {
        case VarType::Nil:
            ret = "nil";
            break;
        case VarType::Bool:
            ret = GetBool() ? "true" : "false";
            break;
        case VarType::Int:
            ret = std::to_string(data_.i);
            break;
        case VarType::Float: {
            ret = std::format("{}", data_.f);
            break;
        }
        case VarType::String:
        case VarType::StringId:
            ret = has_quote ? std::format("\"{}\"", GetString()->Str()) : std::string(GetString()->Str());
            break;
        case VarType::Table:
            ret = std::format("table({})", static_cast<void *>(data_.t));
            break;
        case VarType::Closure:
            ret = std::format("function({})", static_cast<void *>(data_.cl));
            break;
        case VarType::Multi: {
            VarMulti *m = data_.m;
            ret = "multi(";
            for (uint32_t i = 0; i < m->GetCount(); ++i) {
                if (i > 0) {
                    ret += ", ";
                }
                ret += static_cast<const Var &>(m->GetVars()[i]).ToString(has_quote, has_postfix);
            }
            ret += ")";
            break;
        }
    }

    return ret;
}

size_t Var::Hash() const {
    switch (Type()) {
        case VarType::Nil: {
            return 0;
        }
        case VarType::Bool: {
            return data_.b ? 1 : 0;
        }
        case VarType::Int: {
            return static_cast<uint32_t>(data_.i ^ (data_.i >> 32));
        }
        case VarType::Float: {
            union {
                double d;
                uint64_t u;
            } bits;

            bits.d = data_.f;
            return static_cast<uint32_t>(bits.u ^ (bits.u >> 32));
        }
        case VarType::String:
        case VarType::StringId: {
            return GetString()->Hash();
        }
        case VarType::Table: {
            return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data_.t) ^ (reinterpret_cast<uintptr_t>(data_.t) >> 32));
        }
        case VarType::Closure: {
            return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data_.cl) ^ (reinterpret_cast<uintptr_t>(data_.cl) >> 32));
        }
        case VarType::Multi: {
            return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data_.m) ^ (reinterpret_cast<uintptr_t>(data_.m) >> 32));
        }
        default: {
            return 0;
        }
    }
}

bool Var::Equal(const Var &rhs) const {
    if (Type() != rhs.Type()) {
        // String and StringId are interchangeable for comparison
        if ((Type() == VarType::String && rhs.Type() == VarType::StringId) || (Type() == VarType::StringId && rhs.Type() == VarType::String)) {
            return GetString()->Str() == rhs.GetString()->Str();
        }
        // Int and Float with the same mathematical value are equal (Lua semantics).
        if (Type() == VarType::Int && rhs.Type() == VarType::Float) {
            return static_cast<double>(data_.i) == rhs.data_.f;
        }
        if (Type() == VarType::Float && rhs.Type() == VarType::Int) {
            return data_.f == static_cast<double>(rhs.data_.i);
        }
        return false;
    }

    switch (Type()) {
        case VarType::Nil:
            return true;
        case VarType::Bool:
            return data_.b == rhs.data_.b;
        case VarType::Int:
            return data_.i == rhs.data_.i;
        case VarType::Float:
            return data_.f == rhs.data_.f;
        case VarType::String:
            return data_.s->Str() == rhs.data_.s->Str();
        case VarType::StringId:
            return data_.i == rhs.data_.i;
        case VarType::Table:
            return data_.t == rhs.data_.t;
        case VarType::Closure:
            return data_.cl == rhs.data_.cl;
        case VarType::Multi: {
            VarMulti *m1 = data_.m;
            VarMulti *m2 = rhs.data_.m;
            if (m1->GetCount() != m2->GetCount()) {
                return false;
            }
            for (uint32_t i = 0; i < m1->GetCount(); ++i) {
                if (!(static_cast<const Var &>(m1->GetVars()[i]) == static_cast<const Var &>(m2->GetVars()[i]))) {
                    return false;
                }
            }
            return true;
        }
        default:
            return false;
    }
}

}// namespace fakelua
