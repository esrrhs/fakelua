#include "native/serialize/native_serialize.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "var/var.h"
#include "var/var_string.h"
#include "var/var_table.h"

#include <cstring>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fakelua::serialize {

// ─────────────────────────────────────────────────────────────────────────────
// Wire format（类 protobuf 编码）
//
//   每个值 = [type_tag(1 byte)] [payload]
//
//   0x00            nil
//   0x01            false
//   0x02            true
//   0x03 + varint   整数（zigzag 编码：小绝对值 → 小编码）
//   0x04 + 8 bytes  double（小端 memcpy）
//   0x05 + varint(len) + bytes   新字符串，加入字典
//   0x06 + varint(id)            字典中的字符串引用
//   0x07 + varint(count) + N*(key,value)   表
//
//   不支持的类型（闭包等）在表中跳过，顶层编码则抛错。
// ─────────────────────────────────────────────────────────────────────────────

enum Tag : uint8_t {
    TAG_NIL   = 0x00,
    TAG_FALSE = 0x01,
    TAG_TRUE  = 0x02,
    TAG_INT   = 0x03,
    TAG_DOUBLE = 0x04,
    TAG_STR_NEW = 0x05,
    TAG_STR_REF = 0x06,
    TAG_TABLE = 0x07,
};

// ─── 辅助：从 CVar 提取字符串（二进制安全） ───

static std::string cvar_to_string(CVar v) {
    if (v.type_ == static_cast<int>(VarType::String) && v.data_.s) {
        auto sv = v.data_.s->Str();
        return std::string(sv.data(), sv.size());
    }
    if (v.type_ == static_cast<int>(VarType::StringId) && v.data_.i) {
        const char *ptr = reinterpret_cast<const char *>(v.data_.i);
        int sz = *reinterpret_cast<const int *>(ptr);
        return std::string(ptr + 8, sz);
    }
    return {};
}

static std::string_view cvar_to_string_view(CVar v) {
    if (v.type_ == static_cast<int>(VarType::String) && v.data_.s) {
        return v.data_.s->Str();
    }
    if (v.type_ == static_cast<int>(VarType::StringId) && v.data_.i) {
        const char *ptr = reinterpret_cast<const char *>(v.data_.i);
        int sz = *reinterpret_cast<const int *>(ptr);
        return std::string_view(ptr + 8, sz);
    }
    return {};
}

// ─── 类型判断 ───

static bool is_supported(CVar v) {
    switch (v.type_) {
        case static_cast<int>(VarType::Nil):
        case static_cast<int>(VarType::Bool):
        case static_cast<int>(VarType::Int):
        case static_cast<int>(VarType::Float):
        case static_cast<int>(VarType::String):
        case static_cast<int>(VarType::StringId):
        case static_cast<int>(VarType::Table):
            return true;
        default:
            return false;
    }
}

// ─── Varint（LEB128 无符号） ───

static void write_varint(std::string &out, uint64_t v) {
    while (v >= 0x80) {
        out.push_back(static_cast<char>((v & 0x7f) | 0x80));
        v >>= 7;
    }
    out.push_back(static_cast<char>(v));
}

static uint64_t read_varint(const std::string &in, size_t &pos) {
    uint64_t result = 0;
    int shift = 0;
    while (pos < in.size()) {
        uint8_t b = static_cast<uint8_t>(in[pos++]);
        result |= static_cast<uint64_t>(b & 0x7f) << shift;
        if ((b & 0x80) == 0) break;
        shift += 7;
        if (shift >= 64) {
            ThrowFakeluaException("serialize.decode: varint too long");
        }
    }
    return result;
}

// ─── Zigzag（有符号整数 ↔ 无符号） ───

static uint64_t zigzag_encode(int64_t n) {
    return (static_cast<uint64_t>(n) << 1) ^ static_cast<uint64_t>(n >> 63);
}

static int64_t zigzag_decode(uint64_t u) {
    return static_cast<int64_t>((u >> 1) ^ (-(u & 1)));
}

// ─── Double（小端 memcpy） ───

static void write_double(std::string &out, double v) {
    uint8_t buf[8];
    std::memcpy(buf, &v, 8);
    out.append(reinterpret_cast<char *>(buf), 8);
}

static double read_double(const std::string &in, size_t &pos) {
    if (pos + 8 > in.size()) {
        ThrowFakeluaException("serialize.decode: truncated double");
    }
    double v;
    std::memcpy(&v, in.data() + pos, 8);
    pos += 8;
    return v;
}

// ─── 编码 ───

struct EncodeState {
    std::unordered_map<std::string_view, uint32_t> dict;  // 字符串 → 字典 id
};

static void encode_value(std::string &out, CVar v, EncodeState &state) {
    switch (v.type_) {
        case static_cast<int>(VarType::Nil):
            out.push_back(TAG_NIL);
            return;
        case static_cast<int>(VarType::Bool):
            out.push_back(v.data_.i ? TAG_TRUE : TAG_FALSE);
            return;
        case static_cast<int>(VarType::Int):
            out.push_back(TAG_INT);
            write_varint(out, zigzag_encode(v.data_.i));
            return;
        case static_cast<int>(VarType::Float):
            out.push_back(TAG_DOUBLE);
            write_double(out, v.data_.f);
            return;
        case static_cast<int>(VarType::String):
        case static_cast<int>(VarType::StringId): {
            auto sv = cvar_to_string_view(v);
            auto it = state.dict.find(sv);
            if (it != state.dict.end()) {
                out.push_back(TAG_STR_REF);
                write_varint(out, it->second);
            } else {
                uint32_t id = static_cast<uint32_t>(state.dict.size());
                state.dict.emplace(sv, id);
                out.push_back(TAG_STR_NEW);
                write_varint(out, static_cast<uint64_t>(sv.size()));
                out.append(sv.data(), sv.size());
            }
            return;
        }
        case static_cast<int>(VarType::Table): {
            VarTable *t = v.data_.t;
            // 收集所有键值对（跳过不支持的类型）
            std::vector<CVar> keys;
            std::vector<CVar> vals;
            auto add_kv = [&](CVar k, CVar val) {
                if (is_supported(k) && is_supported(val)) {
                    keys.push_back(k);
                    vals.push_back(val);
                }
            };
            if (t->spec_keys && t->spec_vals && t->spec_count > 0) {
                for (uint32_t i = 0; i < t->spec_count; ++i) {
                    if (t->spec_keys[i].type_ == static_cast<int>(VarType::Nil)) continue;
                    add_kv(t->spec_keys[i], t->spec_vals[i]);
                }
            }
            for (uint32_t i = 0; i < VarTable::QUICK_DATA_SIZE; ++i) {
                if (t->quick_data_[i].key.type_ == static_cast<int>(VarType::Nil)) continue;
                add_kv(t->quick_data_[i].key, t->quick_data_[i].val);
            }
            if (t->nodes_ && t->bucket_count_ > 0 && t->active_list_) {
                for (uint32_t i = 0; i < t->count_; ++i) {
                    uint32_t node_idx = t->active_list_[i];
                    const auto &entry = t->nodes_[node_idx].entry;
                    if (entry.key.type_ == static_cast<int>(VarType::Nil)) continue;
                    add_kv(entry.key, entry.val);
                }
            }
            out.push_back(TAG_TABLE);
            write_varint(out, static_cast<uint64_t>(keys.size()));
            for (size_t i = 0; i < keys.size(); ++i) {
                encode_value(out, keys[i], state);
                encode_value(out, vals[i], state);
            }
            return;
        }
        default:
            ThrowFakeluaException("serialize.encode: unsupported type: " + VarTypeToString(static_cast<VarType>(v.type_)));
    }
}

// ─── 解码 ───

struct DecodeState {
    std::vector<std::string> dict;  // id → 字符串
};

static CVar decode_value(const std::string &in, size_t &pos, DecodeState &state, State *s) {
    if (pos >= in.size()) {
        ThrowFakeluaException("serialize.decode: unexpected end of input");
    }
    uint8_t tag = static_cast<uint8_t>(in[pos++]);
    switch (tag) {
        case TAG_NIL:
            return inter::NativeToFakeluaNil(s);
        case TAG_FALSE:
            return inter::NativeToFakeluaBool(s, false);
        case TAG_TRUE:
            return inter::NativeToFakeluaBool(s, true);
        case TAG_INT: {
            uint64_t u = read_varint(in, pos);
            return inter::NativeToFakeluaLonglong(s, zigzag_decode(u));
        }
        case TAG_DOUBLE:
            return inter::NativeToFakeluaDouble(s, read_double(in, pos));
        case TAG_STR_NEW: {
            uint64_t len = read_varint(in, pos);
            if (pos + len > in.size()) {
                ThrowFakeluaException("serialize.decode: truncated string");
            }
            std::string str(in, pos, len);
            pos += len;
            uint32_t id = static_cast<uint32_t>(state.dict.size());
            state.dict.push_back(str);
            return inter::NativeToFakeluaString(s, state.dict[id]);
        }
        case TAG_STR_REF: {
            uint64_t id = read_varint(in, pos);
            if (id >= state.dict.size()) {
                ThrowFakeluaException("serialize.decode: bad string ref id");
            }
            return inter::NativeToFakeluaString(s, state.dict[id]);
        }
        case TAG_TABLE: {
            uint64_t count = read_varint(in, pos);
            CVar tbl = table::TableHelper::CreateTable(s);
            for (uint64_t i = 0; i < count; ++i) {
                CVar key = decode_value(in, pos, state, s);
                CVar val = decode_value(in, pos, state, s);
                table::TableHelper::SetTable(s, tbl, key, val);
            }
            return tbl;
        }
        default:
            ThrowFakeluaException("serialize.decode: unknown tag: " + std::to_string(tag));
    }
}

// ─── 原生函数 ───

static CVar serialize_encode(State *s, CVar *args, int n) {
    CVar v = inter::GetNativeArg(s, args, n, 0);
    if (!is_supported(v)) {
        ThrowFakeluaException("serialize.encode: unsupported type: " + VarTypeToString(static_cast<VarType>(v.type_)));
    }
    std::string out;
    out.reserve(64);
    EncodeState state;
    encode_value(out, v, state);
    return inter::NativeToFakeluaString(s, out);
}

static CVar serialize_decode(State *s, CVar *args, int n) {
    std::string in = cvar_to_string(inter::GetNativeArg(s, args, n, 0));
    size_t pos = 0;
    DecodeState state;
    CVar result = decode_value(in, pos, state, s);
    return result;
}

// ─── 注册 ───

void RegisterSerializeLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "serialize.encode", 1, false, serialize_encode);
    RegisterNativeFunction(s, "serialize.decode", 1, false, serialize_decode);
}

} // namespace fakelua::serialize
