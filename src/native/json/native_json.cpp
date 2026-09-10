#include "native/json/native_json.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "var/var_table.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/json.hpp>

namespace fakelua::json {

namespace bj = boost::json;

static constexpr int kMaxJsonDepth = 64;

// ── Convert boost::json::value to Lua CVar ──
static CVar json_value_to_lua(State *s, const bj::value &v) {
    if (v.is_null()) {
        return inter::NativeToFakeluaNil(s);
    }
    if (v.is_bool()) {
        return inter::NativeToFakeluaBool(s, v.get_bool());
    }
    if (v.is_int64()) {
        return inter::NativeToFakeluaLonglong(s, v.get_int64());
    }
    if (v.is_uint64()) {
        // Lua numbers are signed; but we can still represent up to 2^63-1 as positive.
        // For values > 2^63-1, we could convert to double, but keep simple.
        return inter::NativeToFakeluaLonglong(s, static_cast<int64_t>(v.get_uint64()));
    }
    if (v.is_double()) {
        return inter::NativeToFakeluaDouble(s, v.get_double());
    }
    if (v.is_string()) {
        return inter::NativeToFakeluaString(s, v.get_string().c_str());
    }
    if (v.is_array()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        const bj::array &arr = v.get_array();
        for (size_t i = 0; i < arr.size(); ++i) {
            CVar elem = json_value_to_lua(s, arr[i]);
            table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(i + 1), elem);
        }
        return tbl;
    }
    if (v.is_object()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        const bj::object &obj = v.get_object();
        for (const auto &[key, val] : obj) {
            CVar lua_val = json_value_to_lua(s, val);
            table::TableHelper::SetTableStrId(s, tbl, key.c_str(), lua_val);
        }
        return tbl;
    }
    // Should not reach here
    return inter::NativeToFakeluaNil(s);
}

// ── Convert Lua CVar to boost::json::value ──
static bj::value lua_to_json_value(CVar v, int depth, std::unordered_set<VarTable *> &visited) {
    if (depth > kMaxJsonDepth) {
        ThrowFakeluaException("JSON encode: nesting too deep");
    }
    switch (v.type_) {
    case static_cast<int>(VarType::Nil):
        return nullptr;
    case static_cast<int>(VarType::Bool):
        return AsVar(v).GetBool();
    case static_cast<int>(VarType::Int):
        return static_cast<int64_t>(v.data_.i);
    case static_cast<int>(VarType::Float):
        return v.data_.f;
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId): {
        std::string str = inter::FakeluaToNativeString(nullptr, v);
        return str;
    }
    case static_cast<int>(VarType::Table): {
        auto *t = v.data_.t;
        if (!t) return nullptr;
        if (!visited.insert(t).second) {
            ThrowFakeluaException("JSON encode: cyclic table");
        }

        auto kvs = table::TableHelper::CollectKVPairs(v);
        bool is_array = !kvs.empty();
        int64_t max_idx = 0;
        for (auto &kv : kvs) {
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
            std::sort(kvs.begin(), kvs.end(), [](const table::TableKV &a, const table::TableKV &b) {
                return a.key.data_.i < b.key.data_.i;
            });
            bj::array arr;
            arr.reserve(kvs.size());
            for (auto &kv : kvs) {
                arr.push_back(lua_to_json_value(kv.val, depth + 1, visited));
            }
            visited.erase(t);
            return arr;
        } else {
            bj::object obj;
            for (auto &kv : kvs) {
                obj.emplace(cvar_to_json_key(kv.key), lua_to_json_value(kv.val, depth + 1, visited));
            }
            visited.erase(t);
            return obj;
        }
    }
    default:
        ThrowFakeluaException(std::format("JSON encode: unsupported type {}", VarTypeToString(AsVar(v).Type())));
    }
}

// Helper to convert Lua CVar key to string for JSON object key (same as before)
static std::string cvar_to_json_key(CVar k) {
    switch (k.type_) {
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId):
        return inter::FakeluaToNativeString(nullptr, k);
    case static_cast<int>(VarType::Int):
        return std::to_string(k.data_.i);
    case static_cast<int>(VarType::Float): {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.17g", k.data_.f);
        return buf;
    }
    case static_cast<int>(VarType::Bool):
        return AsVar(k).GetBool() ? "true" : "false";
    default:
        ThrowFakeluaException(std::format("JSON encode: unsupported object key type {}",
                                           VarTypeToString(AsVar(k).Type())));
    }
}

// ── Lua Bindings ——

// json.decode(json_str) → Lua value
static CVar json_decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "json.decode", "json string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string str = inter::FakeluaToNativeString(s, a0);

    try {
        bj::value jv = bj::parse(str);
        return json_value_to_lua(s, jv);
    } catch (const bj::system_error &e) {
        ThrowFakeluaException(std::format("JSON parse error: {}", e.what()));
    }
}

// json.encode(value) → JSON string
static CVar json_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "json.encode", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::unordered_set<VarTable *> visited;
    bj::value jv = lua_to_json_value(a0, 0, visited);
    std::string out = bj::serialize(jv);
    return inter::NativeToFakeluaString(s, out);
}

void RegisterJsonLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "json.decode", 1, false, json_decode);
    RegisterNativeFunction(s, "json.encode", 1, false, json_encode);
}

}  // namespace fakelua::json