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

// Detect whether a Lua table's kv pairs form an array (contiguous 1..N integer keys).
// If so, sorts kvs in-place by key ascending.
static bool detect_and_sort_array(std::vector<table::TableKV> &kvs) {
    if (kvs.empty()) return false;
    int64_t max_idx = 0;
    for (auto &kv : kvs) {
        if (kv.key.type_ != static_cast<int>(VarType::Int)) return false;
        int64_t k = kv.key.data_.i;
        if (k < 1 || k > 1000000) return false;
        if (k > max_idx) max_idx = k;
    }
    if (max_idx <= 0 || static_cast<size_t>(max_idx) != kvs.size()) return false;
    std::sort(kvs.begin(), kvs.end(), [](const table::TableKV &a, const table::TableKV &b) {
        return a.key.data_.i < b.key.data_.i;
    });
    return true;
}

// Recursive CVar → toml::node conversion. Returns a heap-allocated toml::node
// (unique_ptr) so callers can insert into either a toml::table or toml::array.
// Returns nullptr for Nil (caller should skip).
static std::unique_ptr<::toml::node> lua_to_toml_node(CVar val, int depth, std::unordered_set<VarTable *> &visited) {
    if (depth > kMaxTomlDepth) {
        ThrowFakeluaException("TOML encode: nesting too deep");
    }
    switch (val.type_) {
    case static_cast<int>(VarType::Nil):
        return nullptr;
    case static_cast<int>(VarType::Bool):
        return std::make_unique<::toml::value<bool>>(AsVar(val).GetBool());
    case static_cast<int>(VarType::Int):
        return std::make_unique<::toml::value<int64_t>>(val.data_.i);
    case static_cast<int>(VarType::Float):
        return std::make_unique<::toml::value<double>>(val.data_.f);
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId):
        return std::make_unique<::toml::value<std::string>>(inter::FakeluaToNativeString(nullptr, val));
    case static_cast<int>(VarType::Table): {
        auto *t = val.data_.t;
        if (!t) return std::make_unique<::toml::table>();
        if (!visited.insert(t).second) {
            ThrowFakeluaException("TOML encode: cyclic table");
        }
        auto kvs = table::TableHelper::CollectKVPairs(val);
        std::unique_ptr<::toml::node> result;
        if (detect_and_sort_array(kvs)) {
            auto arr = std::make_unique<::toml::array>();
            for (auto &kv : kvs) {
                auto node = lua_to_toml_node(kv.val, depth + 1, visited);
                if (node) arr->push_back(std::move(*node));
            }
            result = std::move(arr);
        } else {
            auto tbl = std::make_unique<::toml::table>();
            for (auto &kv : kvs) {
                if (kv.key.type_ != static_cast<int>(VarType::Int) &&
                    kv.key.type_ != static_cast<int>(VarType::String) &&
                    kv.key.type_ != static_cast<int>(VarType::StringId)) {
                    continue;
                }
                std::string k = inter::FakeluaToNativeString(nullptr, kv.key);
                auto node = lua_to_toml_node(kv.val, depth + 1, visited);
                if (node) tbl->insert(k, std::move(*node));
            }
            result = std::move(tbl);
        }
        visited.erase(t);
        return result;
    }
    default:
        return nullptr;
    }
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
                    if (kv.key.type_ != static_cast<int>(VarType::Int) &&
                        kv.key.type_ != static_cast<int>(VarType::String) &&
                        kv.key.type_ != static_cast<int>(VarType::StringId)) {
                        continue;
                    }
                    std::string k = inter::FakeluaToNativeString(nullptr, kv.key);
                    auto node = lua_to_toml_node(kv.val, 0, visited);
                    if (node) root.insert(k, std::move(*node));
                }
                visited.erase(t);
            }
        } else {
            // Top-level scalar/bool: encode as a single value via key "value"
            auto node = lua_to_toml_node(a0, 0, visited);
            if (node) root.insert("value", std::move(*node));
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
