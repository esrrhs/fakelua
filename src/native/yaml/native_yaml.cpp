#include "native/yaml/native_yaml.h"
#include "native/native_common.h"
#include "native/table/native_table.h"

#include <yaml-cpp/yaml.h>

#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

namespace fakelua::yaml {

// ── Scalar type inference ──
// YAML scalars arrive as strings; recover the most specific Lua type.
// Order matters: bool → integer → float → string.

static CVar scalar_to_lua(State *s, const std::string &str) {
    if (str.empty()) return inter::NativeToFakeluaString(s, str);

    // Bool: YAML 1.1/1.2 common forms. Only exact matches, never guess from string content.
    if (str == "true" || str == "True" || str == "TRUE" ||
        str == "yes" || str == "Yes" || str == "YES" ||
        str == "on" || str == "On" || str == "ON") {
        return inter::NativeToFakeluaBool(s, true);
    }
    if (str == "false" || str == "False" || str == "FALSE" ||
        str == "no" || str == "No" || str == "NO" ||
        str == "off" || str == "Off" || str == "OFF") {
        return inter::NativeToFakeluaBool(s, false);
    }

    // null
    if (str == "~" || str == "null" || str == "Null" || str == "NULL") {
        return inter::NativeToFakeluaNil(s);
    }

    // Integer: optional sign + digits only. Reject "01" style (leading zero non-zero int stays string).
    const char *start = str.c_str();
    const char *p = start;
    if (*p == '+' || *p == '-') ++p;
    if (*p == '\0') return inter::NativeToFakeluaString(s, str);
    bool all_digits = true;
    for (const char *q = p; *q; ++q) {
        if (*q < '0' || *q > '9') { all_digits = false; break; }
    }
    if (all_digits) {
        // leading zero on multi-digit → keep as string
        if (str.size() > 1 && str[0] == '0') return inter::NativeToFakeluaString(s, str);
        if (str.size() > 2 && (str[0] == '+' || str[0] == '-') && str[1] == '0') return inter::NativeToFakeluaString(s, str);
        try {
            size_t pos = 0;
            long long v = std::stoll(start, &pos, 10);
            if (pos == str.size()) return inter::NativeToFakeluaLonglong(s, v);
        } catch (...) {
        }
    }

    // Float: must contain '.', 'e', or 'E' to avoid retrying integer parse.
    bool might_float = false;
    for (char c : str) {
        if (c == '.' || c == 'e' || c == 'E') { might_float = true; break; }
    }
    if (might_float) {
        try {
            size_t pos = 0;
            double d = std::stod(start, &pos);
            if (pos == str.size() && std::isfinite(d)) {
                return inter::NativeToFakeluaDouble(s, d);
            }
        } catch (...) {
        }
    }

    return inter::NativeToFakeluaString(s, str);
}

// ── YAML::Node → CVar ──

static CVar node_to_lua(State *s, const YAML::Node &node) {
    if (!node.IsDefined() || node.IsNull()) {
        return inter::NativeToFakeluaNil(s);
    }
    if (node.IsScalar()) {
        return scalar_to_lua(s, node.as<std::string>());
    }
    if (node.IsSequence()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        size_t idx = 1;
        for (auto it = node.begin(); it != node.end(); ++it, ++idx) {
            table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(idx), node_to_lua(s, *it));
        }
        return tbl;
    }
    if (node.IsMap()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        for (auto it = node.begin(); it != node.end(); ++it) {
            std::string key = it->first.as<std::string>();
            table::TableHelper::SetTableStrId(s, tbl, key.c_str(), node_to_lua(s, it->second));
        }
        return tbl;
    }
    return inter::NativeToFakeluaNil(s);
}

// ── CVar → YAML::Emitter ──

static int kMaxYamlDepth = 64;

static void lua_to_emitter(YAML::Emitter &out, CVar v, int depth, std::unordered_set<VarTable *> &visited) {
    if (depth > kMaxYamlDepth) {
        ThrowFakeluaException("YAML encode: nesting too deep");
    }
    switch (v.type_) {
    case static_cast<int>(VarType::Nil):
        out << YAML::Null;
        break;
    case static_cast<int>(VarType::Bool):
        out << (AsVar(v).GetBool() ? true : false);
        break;
    case static_cast<int>(VarType::Int):
        out << v.data_.i;
        break;
    case static_cast<int>(VarType::Float): {
        double f = v.data_.f;
        if (!std::isfinite(f)) {
            out << YAML::Null;
        } else {
            // yaml-cpp handles formatting; use precise output
            out << YAML::Precision(17) << f;
        }
        break;
    }
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId):
        out << inter::FakeluaToNativeString(nullptr, v);
        break;
    case static_cast<int>(VarType::Table): {
        auto *t = v.data_.t;
        if (!t) { out << YAML::Null; break; }
        if (!visited.insert(t).second) {
            ThrowFakeluaException("YAML encode: cyclic table");
        }
        auto kvs = table::TableHelper::CollectKVPairs(v);
        // array detection: contiguous 1..N integer keys
        bool is_array = !kvs.empty();
        int64_t max_idx = 0;
        for (auto &kv : kvs) {
            if (kv.key.type_ != static_cast<int>(VarType::Int)) { is_array = false; break; }
            int64_t key = kv.key.data_.i;
            if (key < 1 || key > 1000000) { is_array = false; break; }
            if (key > max_idx) max_idx = key;
        }
        if (is_array && (max_idx <= 0 || static_cast<size_t>(max_idx) != kvs.size())) {
            is_array = false;
        }
        if (is_array) {
            std::sort(kvs.begin(), kvs.end(), [](const table::TableKV &a, const table::TableKV &b) {
                return a.key.data_.i < b.key.data_.i;
            });
            out << YAML::Flow << YAML::BeginSeq;
            for (auto &kv : kvs) {
                lua_to_emitter(out, kv.val, depth + 1, visited);
            }
            out << YAML::EndSeq;
        } else {
            out << YAML::BeginMap;
            for (auto &kv : kvs) {
                std::string key = inter::FakeluaToNativeString(nullptr, kv.key);
                out << YAML::Key << key << YAML::Value;
                lua_to_emitter(out, kv.val, depth + 1, visited);
            }
            out << YAML::EndMap;
        }
        visited.erase(t);
        break;
    }
    default:
        ThrowFakeluaException(std::format("YAML encode: unsupported type {}", VarTypeToString(AsVar(v).Type())));
    }
}

// ── Lua Bindings ──

static CVar yaml_decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "yaml.decode", "yaml string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string str = inter::FakeluaToNativeString(s, a0);
    try {
        YAML::Node root = YAML::Load(str);
        return node_to_lua(s, root);
    } catch (const YAML::Exception &e) {
        ThrowFakeluaException(std::format("YAML parse error: {}", e.what()));
    }
}

static CVar yaml_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "yaml.encode", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    try {
        YAML::Emitter out;
        std::unordered_set<VarTable *> visited;
        lua_to_emitter(out, a0, 0, visited);
        if (!out.good()) {
            ThrowFakeluaException(std::format("YAML encode error: {}", out.GetLastError()));
        }
        return inter::NativeToFakeluaString(s, std::string(out.c_str()));
    } catch (const YAML::Exception &e) {
        ThrowFakeluaException(std::format("YAML encode error: {}", e.what()));
    }
}

void RegisterYamlLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "yaml.decode", 1, false, yaml_decode);
    RegisterNativeFunction(s, "yaml.encode", 1, false, yaml_encode);
}

}  // namespace fakelua::yaml
