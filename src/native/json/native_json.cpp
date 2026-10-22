#include "native/json/native_json.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "var/var_table.h"

#include <algorithm>
#include <boost/json.hpp>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_set>

namespace fakelua::json {

namespace bj = boost::json;

static constexpr int kMaxJsonDepth = 64;

// Helper to convert Lua CVar key to string for JSON object key (same as before)
static std::string CVarToJsonKey(CVar k) {
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
            ThrowFakeluaException(std::format("JSON encode: unsupported object key type {}", VarTypeToString(AsVar(k).Type())));
    }
}

// Convert boost::json::value to Lua CVar
static CVar JsonValueToLua(State *s, const bj::value &v) {
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
        const uint64_t u = v.get_uint64();
        // Keep exact integers in int64 range; larger values become double
        // (lossy above 2^53) instead of wrapping to a negative int64.
        if (u <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            return inter::NativeToFakeluaLonglong(s, static_cast<int64_t>(u));
        }
        return inter::NativeToFakeluaDouble(s, static_cast<double>(u));
    }
    if (v.is_double()) {
        return inter::NativeToFakeluaDouble(s, v.get_double());
    }
    if (v.is_string()) {
        const auto &js = v.get_string();
        return inter::NativeToFakeluaStringView(s, std::string_view(js.data(), js.size()));
    }
    if (v.is_array()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        const bj::array &arr = v.get_array();
        for (size_t i = 0; i < arr.size(); ++i) {
            CVar elem = JsonValueToLua(s, arr[i]);
            table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(i + 1), elem);
        }
        return tbl;
    }
    if (v.is_object()) {
        CVar tbl = table::TableHelper::CreateTable(s);
        const bj::object &obj = v.get_object();
        for (const auto &[key, val]: obj) {
            CVar lua_val = JsonValueToLua(s, val);
            table::TableHelper::SetTableStrId(s, tbl, std::string_view(key.data(), key.size()), lua_val);
        }
        return tbl;
    }
    // Should not reach here
    return inter::NativeToFakeluaNil(s);
}

// 纯数组 table 启发式：空 table 一律编码为 []（历史上空 table 产出 {}，
// 客户端按数组解析时直接崩）；非空时要求 key 全部是从 1 起的连续整数。
// 键全是正整数但中间有洞（{[1]=a,[3]=c}）单独算 kHole：以前会退化成
// {"1":"a","3":"c"}，前端收到的形状变了而且不报错，所以改成让 encode 直接失败。
enum class ArrayShape { kObject, kArray, kHole };

struct ArrayShapeInfo {
    ArrayShape shape = ArrayShape::kObject;
    int64_t max_idx = 0;
    // shape == kHole 时是第一个缺元素的下标
    int64_t hole_at = 0;
    size_t count = 0;
};

static ArrayShapeInfo CheckArrayShape(const std::vector<table::TableKV> &kvs) {
    ArrayShapeInfo info;
    info.count = kvs.size();
    if (kvs.empty()) {
        info.shape = ArrayShape::kArray;
        return info;
    }
    // 只要有任何非整数 key，就当作普通 Object
    for (const auto &kv: kvs) {
        if (kv.key.type_ != static_cast<int>(VarType::Int)) {
            return info;
        }
        const int64_t key = kv.key.data_.i;
        if (key > info.max_idx) info.max_idx = key;
    }

    // 全是整数 key 即为数组意图：按抽屉原理，若键从 1 连续递增，则 max_idx == kvs.size()
    // 且不存在 <= 0 的 key。第一个缺失的正整数下标必然在 [1, kvs.size() + 1] 内，
    // 无论键有多大都只需 kvs.size() + 2 槽标记，杜绝大 key 内存溢出或漏检。
    std::vector<bool> seen(kvs.size() + 2, false);
    bool has_non_positive = false;
    for (const auto &kv: kvs) {
        const int64_t key = kv.key.data_.i;
        if (key >= 1 && static_cast<size_t>(key) <= kvs.size() + 1) {
            seen[static_cast<size_t>(key)] = true;
        } else if (key < 1) {
            has_non_positive = true;
        }
    }

    int64_t first_missing = 1;
    for (size_t i = 1; i < seen.size(); ++i) {
        if (!seen[i]) {
            first_missing = static_cast<int64_t>(i);
            break;
        }
    }

    if (!has_non_positive && static_cast<size_t>(info.max_idx) == kvs.size() && first_missing > static_cast<int64_t>(kvs.size())) {
        info.shape = ArrayShape::kArray;
        return info;
    }

    info.shape = ArrayShape::kHole;
    info.hole_at = first_missing;
    return info;
}

static std::string HoleMessage(const std::string &who, const ArrayShapeInfo &info) {
    return std::format("{}: array table has a hole at index {} ({} entries, max index {}); nil in an array is not encodable",
                       who, info.hole_at, info.count, info.max_idx);
}

// Convert Lua CVar to boost::json::value
static bj::value LuaToJsonValue(CVar v, int depth, std::unordered_set<VarTable *> &visited) {
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
            return bj::value(str);
        }
        case static_cast<int>(VarType::Table): {
            auto *t = v.data_.t;
            if (!t) return nullptr;
            if (!visited.insert(t).second) {
                ThrowFakeluaException("JSON encode: cyclic table");
            }

            auto kvs = table::TableHelper::CollectKVPairs(v);
            const ArrayShapeInfo shape = CheckArrayShape(kvs);
            if (shape.shape == ArrayShape::kHole) {
                ThrowFakeluaException(HoleMessage("JSON encode", shape));
            }

            if (shape.shape == ArrayShape::kArray) {
                std::sort(kvs.begin(), kvs.end(), [](const table::TableKV &a, const table::TableKV &b) { return a.key.data_.i < b.key.data_.i; });
                bj::array arr;
                arr.reserve(kvs.size());
                for (auto &kv: kvs) {
                    arr.push_back(LuaToJsonValue(kv.val, depth + 1, visited));
                }
                visited.erase(t);
                return arr;
            } else {
                bj::object obj;
                for (auto &kv: kvs) {
                    obj.emplace(CVarToJsonKey(kv.key), LuaToJsonValue(kv.val, depth + 1, visited));
                }
                visited.erase(t);
                return obj;
            }
        }
        default:
            ThrowFakeluaException(std::format("JSON encode: unsupported type {}", VarTypeToString(AsVar(v).Type())));
    }
}

// Lua Bindings ——

// json.decode(json_str) → Lua value
static CVar JsonDecode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "json.decode", "json string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string str = inter::FakeluaToNativeString(s, a0);

    bj::parse_options opt;
    opt.max_depth = static_cast<std::size_t>(kMaxJsonDepth);
    boost::system::error_code ec;
    bj::value jv = bj::parse(str, ec, {}, opt);
    if (ec) {
        ThrowFakeluaException(std::format("JSON parse error: {}", ec.message()));
    }
    return JsonValueToLua(s, jv);
}

// json.encode(value) → JSON string
static CVar JsonEncode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "json.encode", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::unordered_set<VarTable *> visited;
    bj::value jv = LuaToJsonValue(a0, 0, visited);
    std::string out = bj::serialize(jv);
    return inter::NativeToFakeluaString(s, out);
}

// json.encode_array(value) → JSON string
// 严格数组编码通道：顶层必须是数组形 table（空 table → []），否则报错。
// 用于协议字段必须为数组的场景，避免 encode 的对象/数组启发式歧义。
static CVar JsonEncodeArray(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "json.encode_array", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    const char *const who = "json.encode_array";
    ArrayShapeInfo shape;
    if (a0.type_ == static_cast<int>(VarType::Table) && a0.data_.t) {
        shape = CheckArrayShape(table::TableHelper::CollectKVPairs(a0));
    }
    if (shape.shape == ArrayShape::kHole) {
        ThrowFakeluaException(HoleMessage(who, shape));
    }
    if (shape.shape != ArrayShape::kArray) {
        ThrowFakeluaException(std::format("{}: value is not an array-like table", who));
    }
    std::unordered_set<VarTable *> visited;
    bj::value jv = LuaToJsonValue(a0, 0, visited);
    std::string out = bj::serialize(jv);
    return inter::NativeToFakeluaString(s, out);
}

void RegisterJsonLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "json.decode", 1, false, JsonDecode);
    RegisterNativeFunction(s, "json.encode", 1, false, JsonEncode);
    RegisterNativeFunction(s, "json.encode_array", 1, false, JsonEncodeArray);
}

}// namespace fakelua::json
