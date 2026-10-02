#include "native/serialize/wire_codec.h"

#include "native/native_common.h"
#include "native/table/native_table.h"
#include "var/var.h"
#include "var/var_string.h"
#include "var/var_table.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua::serialize {

// Wire format（类 protobuf 编码）——与 native_serialize.cpp 文档保持一致：
//   每个值 = [type_tag(1 byte)] [payload]
//   0x00            nil
//   0x01            false
//   0x02            true
//   0x03 + varint   整数（zigzag 编码：小绝对值 → 小编码）
//   0x04 + 8 bytes  double（小端 memcpy）
//   0x05 + varint(len) + bytes   新字符串，加入字典
//   0x06 + varint(id)            字典中的字符串引用
//   0x07 + varint(count) + N*(key,value)   表

namespace {

enum Tag : uint8_t {
    TAG_NIL = 0x00,
    TAG_FALSE = 0x01,
    TAG_TRUE = 0x02,
    TAG_INT = 0x03,
    TAG_DOUBLE = 0x04,
    TAG_STR_NEW = 0x05,
    TAG_STR_REF = 0x06,
    TAG_TABLE = 0x07,
};

constexpr int kWireMaxDepth = 64;

// 辅助：从 CVar 提取字符串（二进制安全）
std::string CVarToString(CVar v) {
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

std::string_view CVarToStringView(CVar v) {
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

bool IsSupported(CVar v) {
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

// Varint（LEB128 无符号）
void WriteVarint(std::string &out, uint64_t v) {
    while (v >= 0x80) {
        out.push_back(static_cast<char>((v & 0x7f) | 0x80));
        v >>= 7;
    }
    out.push_back(static_cast<char>(v));
}

uint64_t ReadVarint(std::string_view in, size_t &pos) {
    uint64_t result = 0;
    int shift = 0;
    bool terminated = false;
    while (pos < in.size()) {
        uint8_t b = static_cast<uint8_t>(in[pos++]);
        result |= static_cast<uint64_t>(b & 0x7f) << shift;
        if ((b & 0x80) == 0) {
            terminated = true;
            break;
        }
        shift += 7;
        if (shift >= 64) {
            ThrowFakeluaException("wire.decode: varint too long");
        }
    }
    if (!terminated) {
        ThrowFakeluaException("wire.decode: truncated varint");
    }
    return result;
}

// Zigzag（有符号整数 ↔ 无符号）
uint64_t ZigzagEncode(int64_t n) {
    return (static_cast<uint64_t>(n) << 1) ^ static_cast<uint64_t>(n >> 63);
}

int64_t ZigzagDecode(uint64_t u) {
    return static_cast<int64_t>((u >> 1) ^ (-(u & 1)));
}

void WriteDouble(std::string &out, double v) {
    uint8_t buf[8];
    std::memcpy(buf, &v, 8);
    out.append(reinterpret_cast<char *>(buf), 8);
}

double ReadDouble(std::string_view in, size_t &pos) {
    if (pos + 8 > in.size()) {
        ThrowFakeluaException("wire.decode: truncated double");
    }
    double v;
    std::memcpy(&v, in.data() + pos, 8);
    pos += 8;
    return v;
}

// 编码
struct EncodeState {
    std::unordered_map<std::string_view, uint32_t> dict;// 字符串 → 字典 id
    std::unordered_set<VarTable *> visited;
    int depth = 0;
};

void EncodeValue(std::string &out, CVar v, EncodeState &state) {
    switch (v.type_) {
        case static_cast<int>(VarType::Nil):
            out.push_back(TAG_NIL);
            return;
        case static_cast<int>(VarType::Bool):
            out.push_back(AsVar(v).GetBool() ? TAG_TRUE : TAG_FALSE);
            return;
        case static_cast<int>(VarType::Int):
            out.push_back(TAG_INT);
            WriteVarint(out, ZigzagEncode(v.data_.i));
            return;
        case static_cast<int>(VarType::Float):
            out.push_back(TAG_DOUBLE);
            WriteDouble(out, v.data_.f);
            return;
        case static_cast<int>(VarType::String):
        case static_cast<int>(VarType::StringId): {
            auto sv = CVarToStringView(v);
            auto it = state.dict.find(sv);
            if (it != state.dict.end()) {
                out.push_back(TAG_STR_REF);
                WriteVarint(out, it->second);
            } else {
                uint32_t id = static_cast<uint32_t>(state.dict.size());
                state.dict.emplace(sv, id);
                out.push_back(TAG_STR_NEW);
                WriteVarint(out, static_cast<uint64_t>(sv.size()));
                if (!sv.empty()) {
                    out.append(sv.data(), sv.size());
                }
            }
            return;
        }
        case static_cast<int>(VarType::Table): {
            VarTable *t = v.data_.t;
            if (!t) {
                out.push_back(TAG_TABLE);
                WriteVarint(out, 0);
                return;
            }
            if (state.depth >= kWireMaxDepth) {
                ThrowFakeluaException("wire.encode: nesting too deep");
            }
            if (!state.visited.insert(t).second) {
                ThrowFakeluaException("wire.encode: cyclic table");
            }
            state.depth++;
            std::vector<CVar> keys;
            std::vector<CVar> vals;
            try {
                table::TableHelper::ForEachKV(v, [&](CVar k, CVar val) {
                    if (IsSupported(k) && IsSupported(val)) {
                        keys.push_back(k);
                        vals.push_back(val);
                    }
                });
                out.push_back(TAG_TABLE);
                WriteVarint(out, static_cast<uint64_t>(keys.size()));
                for (size_t i = 0; i < keys.size(); ++i) {
                    EncodeValue(out, keys[i], state);
                    EncodeValue(out, vals[i], state);
                }
            } catch (...) {
                state.depth--;
                state.visited.erase(t);
                throw;
            }
            state.depth--;
            state.visited.erase(t);
            return;
        }
        default:
            ThrowFakeluaException("wire.encode: unsupported type: " + VarTypeToString(static_cast<VarType>(v.type_)));
    }
}

// 解码
struct DecodeState {
    std::vector<std::string> dict;// id → 字符串
    int depth = 0;
};

CVar DecodeValue(std::string_view in, size_t &pos, DecodeState &state, State *s) {
    if (pos >= in.size()) {
        ThrowFakeluaException("wire.decode: unexpected end of input");
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
            uint64_t u = ReadVarint(in, pos);
            return inter::NativeToFakeluaLonglong(s, ZigzagDecode(u));
        }
        case TAG_DOUBLE:
            return inter::NativeToFakeluaDouble(s, ReadDouble(in, pos));
        case TAG_STR_NEW: {
            uint64_t len = ReadVarint(in, pos);
            if (len > in.size() - pos) {
                ThrowFakeluaException("wire.decode: truncated string");
            }
            std::string str(in.substr(pos, static_cast<size_t>(len)));
            pos += static_cast<size_t>(len);
            uint32_t id = static_cast<uint32_t>(state.dict.size());
            state.dict.push_back(std::move(str));
            return inter::NativeToFakeluaString(s, state.dict[id]);
        }
        case TAG_STR_REF: {
            uint64_t id = ReadVarint(in, pos);
            if (id >= state.dict.size()) {
                ThrowFakeluaException("wire.decode: bad string ref id");
            }
            return inter::NativeToFakeluaString(s, state.dict[id]);
        }
        case TAG_TABLE: {
            if (state.depth >= kWireMaxDepth) {
                ThrowFakeluaException("wire.decode: nesting too deep");
            }
            uint64_t count = ReadVarint(in, pos);
            // Each key/value pair is at least two tags.
            if (count > (in.size() - pos) / 2) {
                ThrowFakeluaException("wire.decode: table too large");
            }
            CVar tbl = table::TableHelper::CreateTable(s);
            state.depth++;
            try {
                for (uint64_t i = 0; i < count; ++i) {
                    CVar key = DecodeValue(in, pos, state, s);
                    CVar val = DecodeValue(in, pos, state, s);
                    table::TableHelper::SetTable(s, tbl, key, val);
                }
            } catch (...) {
                state.depth--;
                throw;
            }
            state.depth--;
            return tbl;
        }
        default:
            ThrowFakeluaException("wire.decode: unknown tag: " + std::to_string(tag));
    }
}

// 严格校验：回调绑定参数不允许静默丢字段。任意位置出现不可序列化类型都响亮报错。
void ValidateStrict(CVar v, int depth, std::unordered_set<VarTable *> &visited, const char *fname, int argno) {
    if (!IsSupported(v)) {
        ThrowBadArgument(argno, fname,
                         "callback args must be nil, boolean, number, string or a table thereof (closures and native objects are not allowed)");
    }
    if (v.type_ != static_cast<int>(VarType::Table) || !v.data_.t) return;
    VarTable *t = v.data_.t;
    if (depth >= kWireMaxDepth) {
        ThrowFakeluaException("callback args: nesting too deep");
    }
    if (!visited.insert(t).second) {
        ThrowFakeluaException("callback args: cyclic table");
    }
    table::TableHelper::ForEachKV(v, [&](CVar k, CVar val) {
        ValidateStrict(k, depth + 1, visited, fname, argno);
        ValidateStrict(val, depth + 1, visited, fname, argno);
    });
    visited.erase(t);
}

}// namespace

bool WireIsSerializable(CVar v) {
    return IsSupported(v);
}

std::string WireEncode(CVar v) {
    std::string out;
    out.reserve(64);
    EncodeState state;
    EncodeValue(out, v, state);
    return out;
}

CVar WireDecode(State *s, std::string_view in) {
    size_t pos = 0;
    DecodeState state;
    CVar result = DecodeValue(in, pos, state, s);
    if (pos != in.size()) {
        ThrowFakeluaException("wire.decode: trailing bytes");
    }
    return result;
}

std::string WireEncodeCallbackArgs(State * /*s*/, CVar *args, int first, int n, const char *fname, int argno_base) {
    std::string out;
    out.reserve(32);
    const int count = n > first ? n - first : 0;
    WriteVarint(out, static_cast<uint64_t>(count));
    if (count == 0) return out;

    // 先整体严格校验，再编码：不允许走到编码器的"跳过不支持字段"静默分支。
    std::unordered_set<VarTable *> visited;
    for (int i = 0; i < count; ++i) {
        ValidateStrict(args[first + i], 0, visited, fname, argno_base + i);
    }

    EncodeState state;
    for (int i = 0; i < count; ++i) {
        EncodeValue(out, args[first + i], state);
    }
    return out;
}

DecodedArgs WireDecodeCallbackArgs(State *s, std::string_view blob) {
    DecodedArgs out;
    if (blob.empty()) return out;
    size_t pos = 0;
    uint64_t count = ReadVarint(blob, pos);
    out.vars.reserve(static_cast<size_t>(count));
    DecodeState state;
    for (uint64_t i = 0; i < count; ++i) {
        out.vars.push_back(DecodeValue(blob, pos, state, s));
    }
    if (pos != blob.size()) {
        ThrowFakeluaException("wire.decode: trailing bytes in callback args");
    }
    // 解码出的字符串可能引用 state.dict 里 std::string 的内容，把后备内存随 DecodedArgs
    // 一起带走，调用方持有 DecodedArgs 到回调返回即可保证有效。
    out.str_dict = std::move(state.dict);
    return out;
}

}// namespace fakelua::serialize
