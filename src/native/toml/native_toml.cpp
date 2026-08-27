#include "native/toml/native_toml.h"
#include "native/native_common.h"
#include "native/table/native_table.h"

#include <toml++/toml.hpp>

#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

// toml++ lives in ::toml; qualify explicitly to avoid clash with fakelua::toml.

namespace fakelua::toml {

// ── ::toml::node → CVar ──

static CVar node_to_lua(State *s, const ::toml::node &node) {
    if (node.is_table()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        auto *t = node.as_table();
        if (t) {
            for (auto it = t->begin(); it != t->end(); ++it) {
                table::TableHelper::SetTableStrId(s, tbl, std::string(it->first.str()).c_str(), node_to_lua(s, it->second));
            }
        }
        return tbl;
    }
    if (node.is_array()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        auto *arr = node.as_array();
        if (arr) {
            size_t idx = 1;
            for (auto it = arr->begin(); it != arr->end(); ++it, ++idx) {
                table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(idx), node_to_lua(s, *it));
            }
        }
        return tbl;
    }
    if (node.is_string()) {
        auto v = node.value<std::string>();
        if (v) return inter::NativeToFakeluaString(s, *v);
        return inter::NativeToFakeluaString(s, "");
    }
    if (node.is_integer()) {
        auto v = node.value<int64_t>();
        if (v) return inter::NativeToFakeluaLonglong(s, *v);
        return inter::NativeToFakeluaNil(s);
    }
    if (node.is_floating_point()) {
        auto v = node.value<double>();
        if (v) return inter::NativeToFakeluaDouble(s, *v);
        return inter::NativeToFakeluaNil(s);
    }
    if (node.is_boolean()) {
        auto v = node.value<bool>();
        if (v) return inter::NativeToFakeluaBool(s, *v);
        return inter::NativeToFakeluaNil(s);
    }
    // date/time/offsets have no direct Lua equivalent → render as string
    if (node.is_date() || node.is_time() || node.is_date_time()) {
        // toml++ date/time types stream individually
        std::stringstream ss;
        if (node.is_date()) { auto *v = node.as_date(); if (v) ss << *v; }
        else if (node.is_time()) { auto *v = node.as_time(); if (v) ss << *v; }
        else if (node.is_date_time()) { auto *v = node.as_date_time(); if (v) ss << *v; }
        return inter::NativeToFakeluaString(s, ss.str());
    }
    return inter::NativeToFakeluaNil(s);
}

// ── CVar → ::toml::table/array ──

static const int kMaxTomlDepth = 64;

static ::toml::value<double> make_toml_float(double f) {
    return ::toml::value<double>(f);
}

static void lua_to_toml(::toml::table &root, CVar key, CVar val, int depth, std::unordered_set<VarTable *> &visited);

static void add_to_toml_table(::toml::table &tbl, const std::string &key, CVar val, int depth, std::unordered_set<VarTable *> &visited) {
    switch (val.type_) {
    case static_cast<int>(VarType::Nil):
        // TOML has no null; omit the key entirely.
        break;
    case static_cast<int>(VarType::Bool):
        tbl.insert(key, AsVar(val).GetBool());
        break;
    case static_cast<int>(VarType::Int):
        tbl.insert(key, val.data_.i);
        break;
    case static_cast<int>(VarType::Float):
        tbl.insert(key, make_toml_float(val.data_.f));
        break;
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId):
        tbl.insert(key, inter::FakeluaToNativeString(nullptr, val));
        break;
    case static_cast<int>(VarType::Table): {
        auto *t = val.data_.t;
        if (!t) break;
        if (!visited.insert(t).second) {
            ThrowFakeluaException("TOML encode: cyclic table");
        }
        auto kvs = table::TableHelper::CollectKVPairs(val);
        bool is_array = !kvs.empty();
        int64_t max_idx = 0;
        for (auto &kv : kvs) {
            if (kv.key.type_ != static_cast<int>(VarType::Int)) { is_array = false; break; }
            int64_t k = kv.key.data_.i;
            if (k < 1 || k > 1000000) { is_array = false; break; }
            if (k > max_idx) max_idx = k;
        }
        if (is_array && (max_idx <= 0 || static_cast<size_t>(max_idx) != kvs.size())) {
            is_array = false;
        }
        if (is_array) {
            auto arr = ::toml::array{};
            std::sort(kvs.begin(), kvs.end(), [](const table::TableKV &a, const table::TableKV &b) {
                return a.key.data_.i < b.key.data_.i;
            });
            for (auto &kv : kvs) {
                switch (kv.val.type_) {
                case static_cast<int>(VarType::Bool):
                    arr.push_back(AsVar(kv.val).GetBool());
                    break;
                case static_cast<int>(VarType::Int):
                    arr.push_back(kv.val.data_.i);
                    break;
                case static_cast<int>(VarType::Float):
                    arr.push_back(make_toml_float(kv.val.data_.f));
                    break;
                case static_cast<int>(VarType::String):
                case static_cast<int>(VarType::StringId):
                    arr.push_back(inter::FakeluaToNativeString(nullptr, kv.val));
                    break;
                case static_cast<int>(VarType::Table): {
                    auto sub = ::toml::table{};
                    auto *st = kv.val.data_.t;
                    if (st) {
                        if (!visited.insert(st).second) {
                            ThrowFakeluaException("TOML encode: cyclic table");
                        }
                        auto skvs = table::TableHelper::CollectKVPairs(kv.val);
                        for (auto &skv : skvs) {
                            lua_to_toml(sub, skv.key, skv.val, depth + 1, visited);
                        }
                        visited.erase(st);
                    }
                    arr.push_back(sub);
                    break;
                }
                default:
                    break;
                }
            }
            tbl.insert(key, arr);
        } else {
            auto sub = ::toml::table{};
            for (auto &kv : kvs) {
                lua_to_toml(sub, kv.key, kv.val, depth + 1, visited);
            }
            tbl.insert(key, sub);
        }
        visited.erase(t);
        break;
    }
    default:
        break;
    }
}

static void lua_to_toml(::toml::table &root, CVar key, CVar val, int depth, std::unordered_set<VarTable *> &visited) {
    if (depth > kMaxTomlDepth) {
        ThrowFakeluaException("TOML encode: nesting too deep");
    }
    std::string k = inter::FakeluaToNativeString(nullptr, key);
    add_to_toml_table(root, k, val, depth, visited);
}

// ── Lua Bindings ──

static CVar toml_decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "toml.decode", "toml string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string str = inter::FakeluaToNativeString(s, a0);
    try {
        auto result = ::toml::parse(str);
        return node_to_lua(s, result);
    } catch (const ::toml::parse_error &e) {
        ThrowFakeluaException(std::format("TOML parse error: {}", e.description()));
    }
}

static CVar toml_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "toml.encode", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    try {
        auto root = ::toml::table{};
        std::unordered_set<VarTable *> visited;
        if (a0.type_ == static_cast<int>(VarType::Table)) {
            auto *t = a0.data_.t;
            if (t) {
                if (!visited.insert(t).second) {
                    ThrowFakeluaException("TOML encode: cyclic table");
                }
                auto kvs = table::TableHelper::CollectKVPairs(a0);
                for (auto &kv : kvs) {
                    lua_to_toml(root, kv.key, kv.val, 0, visited);
                }
                visited.erase(t);
            }
        } else {
            // Top-level scalar/bool: encode as a single value via key "value"
            lua_to_toml(root, inter::NativeToFakeluaString(s, "value"), a0, 0, visited);
        }
        std::stringstream ss;
        ss << root;
        return inter::NativeToFakeluaString(s, ss.str());
    } catch (const ::toml::parse_error &e) {
        ThrowFakeluaException(std::format("TOML encode error: {}", e.description()));
    }
}

void RegisterTomlLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "toml.decode", 1, false, toml_decode);
    RegisterNativeFunction(s, "toml.encode", 1, false, toml_encode);
}

}  // namespace fakelua::toml
