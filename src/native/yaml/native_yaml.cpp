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

// Scalar type inference
// YAML scalars arrive as strings; recover the most specific Lua type.
// Order matters: bool → integer → float → string.

static CVar ScalarToLua(State *s, const std::string &str) {
    if (str.empty()) return inter::NativeToFakeluaString(s, str);

    // Bool: YAML 1.1/1.2 common forms. Only exact matches, never guess from string content.
    if (str == "true" || str == "True" || str == "TRUE" || str == "yes" || str == "Yes" || str == "YES" || str == "on" || str == "On" || str == "ON") {
        return inter::NativeToFakeluaBool(s, true);
    }
    if (str == "false" || str == "False" || str == "FALSE" || str == "no" || str == "No" || str == "NO" || str == "off" || str == "Off" || str == "OFF") {
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
        if (*q < '0' || *q > '9') {
            all_digits = false;
            break;
        }
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
    for (char c: str) {
        if (c == '.' || c == 'e' || c == 'E') {
            might_float = true;
            break;
        }
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

static constexpr int kMaxYamlDepth = 64;
static constexpr size_t kMaxYamlNodes = 100000;

static void CheckYamlTextNesting(const std::string &str) {
    int flow_depth = 0;
    bool in_single = false;
    bool in_double = false;
    bool at_node_start = true;
    size_t col = 0;
    std::vector<size_t> block_indents;
    block_indents.reserve(static_cast<size_t>(kMaxYamlDepth) + 4);

    auto push_block_indent = [&](size_t c_pos) {
        while (!block_indents.empty() && block_indents.back() >= c_pos) {
            block_indents.pop_back();
        }
        block_indents.push_back(c_pos);
        if (block_indents.size() + static_cast<size_t>(flow_depth) > static_cast<size_t>(kMaxYamlDepth * 2)) {
            ThrowFakeluaException("yaml.decode: nesting too deep");
        }
    };

    for (size_t i = 0; i < str.size(); ++i) {
        const char c = str[i];
        if (in_single) {
            if (c == '\n') {
                col = 0;
            } else if (c == '\'') {
                if (i + 1 < str.size() && str[i + 1] == '\'') {
                    ++i;
                    col += 2;
                } else {
                    in_single = false;
                    ++col;
                }
            } else if (c != '\r') {
                ++col;
            }
            continue;
        }
        if (in_double) {
            if (c == '\n') {
                col = 0;
                continue;
            }
            if (c == '\\' && i + 1 < str.size()) {
                ++i;
                col += 2;
                continue;
            }
            if (c == '"') {
                in_double = false;
            }
            if (c != '\r') {
                ++col;
            }
            continue;
        }
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            col = 0;
            if (flow_depth == 0) {
                at_node_start = true;
            }
            continue;
        }
        if (c == '#') {
            while (i + 1 < str.size() && str[i + 1] != '\n') {
                ++i;
            }
            continue;
        }
        if (flow_depth == 0 && at_node_start) {
            if (c == ' ' || c == '\t') {
                ++col;
                continue;
            }
            if ((c == '-' || c == '?') &&
                (i + 1 == str.size() || str[i + 1] == ' ' || str[i + 1] == '\t' || str[i + 1] == '\r' || str[i + 1] == '\n')) {
                push_block_indent(col);
                ++col;
                continue;
            }
            if (c == '&' || c == '!') {
                while (i + 1 < str.size() && str[i + 1] != ' ' && str[i + 1] != '\t' && str[i + 1] != '\r' && str[i + 1] != '\n' &&
                       str[i + 1] != '[' && str[i + 1] != '{' && str[i + 1] != ',') {
                    ++i;
                    ++col;
                }
                ++col;
                continue;
            }
            push_block_indent(col);
            at_node_start = false;
        }
        if (c == '\'') {
            in_single = true;
            ++col;
            continue;
        }
        if (c == '"') {
            in_double = true;
            ++col;
            continue;
        }
        if (flow_depth == 0 && c == ':' &&
            (i + 1 == str.size() || str[i + 1] == ' ' || str[i + 1] == '\t' || str[i + 1] == '\r' || str[i + 1] == '\n')) {
            at_node_start = true;
            ++col;
            continue;
        }
        if (c == '[' || c == '{') {
            ++flow_depth;
            if (flow_depth > kMaxYamlDepth) {
                ThrowFakeluaException("yaml.decode: nesting too deep");
            }
        } else if ((c == ']' || c == '}') && flow_depth > 0) {
            --flow_depth;
        }
        ++col;
    }
}

// YAML::Node → CVar
static CVar NodeToLua(State *s, const YAML::Node &node, int depth, std::vector<YAML::Node> &stack, size_t &visited_nodes) {
    if (depth > kMaxYamlDepth) {
        ThrowFakeluaException("yaml.decode: nesting too deep");
    }
    if (++visited_nodes > kMaxYamlNodes) {
        ThrowFakeluaException("yaml.decode: node limit exceeded");
    }
    if (!node.IsDefined() || node.IsNull()) {
        return inter::NativeToFakeluaNil(s);
    }
    if (node.IsScalar()) {
        return ScalarToLua(s, node.as<std::string>());
    }
    for (const auto &seen: stack) {
        if (node.is(seen)) {
            ThrowFakeluaException("yaml.decode: cyclic alias");
        }
    }
    stack.push_back(node);

    struct PopGuard {
        std::vector<YAML::Node> &stack;

        ~PopGuard() {
            stack.pop_back();
        }
    } guard{stack};

    if (node.IsSequence()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        size_t idx = 1;
        for (auto it = node.begin(); it != node.end(); ++it, ++idx) {
            table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(idx), NodeToLua(s, *it, depth + 1, stack, visited_nodes));
        }
        return tbl;
    }
    if (node.IsMap()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        for (auto it = node.begin(); it != node.end(); ++it) {
            if (++visited_nodes > kMaxYamlNodes) {
                ThrowFakeluaException("yaml.decode: node limit exceeded");
            }
            std::string key = it->first.as<std::string>();
            table::TableHelper::SetTableTempStr(s, tbl, key, NodeToLua(s, it->second, depth + 1, stack, visited_nodes));
        }
        return tbl;
    }
    return inter::NativeToFakeluaNil(s);
}

// CVar → YAML::Emitter
static void LuaToEmitter(YAML::Emitter &out, CVar v, int depth, std::unordered_set<VarTable *> &visited) {
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
            if (!t) {
                out << YAML::Null;
                break;
            }
            if (!visited.insert(t).second) {
                ThrowFakeluaException("YAML encode: cyclic table");
            }
            auto kvs = table::TableHelper::CollectKVPairs(v);
            // array detection: contiguous 1..N integer keys
            bool is_array = !kvs.empty();
            int64_t max_idx = 0;
            for (auto &kv: kvs) {
                if (kv.key.type_ != static_cast<int>(VarType::Int)) {
                    is_array = false;
                    break;
                }
                int64_t key = kv.key.data_.i;
                if (key < 1 || key > 1000000) {
                    is_array = false;
                    break;
                }
                if (key > max_idx) max_idx = key;
            }
            if (is_array && (max_idx <= 0 || static_cast<size_t>(max_idx) != kvs.size())) {
                is_array = false;
            }
            if (is_array) {
                std::sort(kvs.begin(), kvs.end(), [](const table::TableKV &a, const table::TableKV &b) { return a.key.data_.i < b.key.data_.i; });
                out << YAML::Flow << YAML::BeginSeq;
                for (auto &kv: kvs) {
                    LuaToEmitter(out, kv.val, depth + 1, visited);
                }
                out << YAML::EndSeq;
            } else {
                out << YAML::BeginMap;
                for (auto &kv: kvs) {
                    std::string key = inter::FakeluaToNativeString(nullptr, kv.key);
                    out << YAML::Key << key << YAML::Value;
                    LuaToEmitter(out, kv.val, depth + 1, visited);
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

// Lua Bindings
static CVar YamlDecode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "yaml.decode", "yaml string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string str = inter::FakeluaToNativeString(s, a0);
    try {
        CheckYamlTextNesting(str);
        YAML::Node root = YAML::Load(str);
        std::vector<YAML::Node> stack;
        size_t visited_nodes = 0;
        return NodeToLua(s, root, 0, stack, visited_nodes);
    } catch (const YAML::Exception &e) {
        ThrowFakeluaException(std::format("YAML parse error: {}", e.what()));
    }
}

static CVar YamlEncode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "yaml.encode", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    try {
        YAML::Emitter out;
        std::unordered_set<VarTable *> visited;
        LuaToEmitter(out, a0, 0, visited);
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
    RegisterNativeFunction(s, "yaml.decode", 1, false, YamlDecode);
    RegisterNativeFunction(s, "yaml.encode", 1, false, YamlEncode);
}

}// namespace fakelua::yaml
