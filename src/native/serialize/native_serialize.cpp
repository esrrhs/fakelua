#include "native/serialize/native_serialize.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "util/logging.h"
#include "var/var.h"
#include "var/var_string.h"
#include "var/var_table.h"

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <cstdint>
#include <cstring>
#include <format>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua::serialize {

// Wire format（类 protobuf 编码）
//   每个值 = [type_tag(1 byte)] [payload]
//   0x00            nil
//   0x01            false
//   0x02            true
//   0x03 + varint   整数（zigzag 编码：小绝对值 → 小编码）
//   0x04 + 8 bytes  double（小端 memcpy）
//   0x05 + varint(len) + bytes   新字符串，加入字典
//   0x06 + varint(id)            字典中的字符串引用
//   0x07 + varint(count) + N*(key,value)   表
//   不支持的类型（闭包等）在表中跳过，顶层编码则抛错。

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

// 辅助：从 CVar 提取字符串（二进制安全）
static std::string CVarToString(CVar v) {
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

static std::string_view CVarToStringView(CVar v) {
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

// 类型判断
static bool IsSupported(CVar v) {
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
static void WriteVarint(std::string &out, uint64_t v) {
    while (v >= 0x80) {
        out.push_back(static_cast<char>((v & 0x7f) | 0x80));
        v >>= 7;
    }
    out.push_back(static_cast<char>(v));
}

static uint64_t ReadVarint(const std::string &in, size_t &pos) {
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
            ThrowFakeluaException("serialize.decode: varint too long");
        }
    }
    if (!terminated) {
        ThrowFakeluaException("serialize.decode: truncated varint");
    }
    return result;
}

// Zigzag（有符号整数 ↔ 无符号）
static uint64_t ZigzagEncode(int64_t n) {
    return (static_cast<uint64_t>(n) << 1) ^ static_cast<uint64_t>(n >> 63);
}

static int64_t ZigzagDecode(uint64_t u) {
    return static_cast<int64_t>((u >> 1) ^ (-(u & 1)));
}

// Double（小端 memcpy）
static void WriteDouble(std::string &out, double v) {
    uint8_t buf[8];
    std::memcpy(buf, &v, 8);
    out.append(reinterpret_cast<char *>(buf), 8);
}

static double ReadDouble(const std::string &in, size_t &pos) {
    if (pos + 8 > in.size()) {
        ThrowFakeluaException("serialize.decode: truncated double");
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

static void EncodeValue(std::string &out, CVar v, EncodeState &state) {
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
            if (state.depth >= 64) {
                ThrowFakeluaException("serialize.encode: nesting too deep");
            }
            if (!state.visited.insert(t).second) {
                ThrowFakeluaException("serialize.encode: cyclic table");
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
            ThrowFakeluaException("serialize.encode: unsupported type: " + VarTypeToString(static_cast<VarType>(v.type_)));
    }
}

// 解码
struct DecodeState {
    std::vector<std::string> dict;// id → 字符串
    int depth = 0;
};

static CVar DecodeValue(const std::string &in, size_t &pos, DecodeState &state, State *s) {
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
            uint64_t u = ReadVarint(in, pos);
            return inter::NativeToFakeluaLonglong(s, ZigzagDecode(u));
        }
        case TAG_DOUBLE:
            return inter::NativeToFakeluaDouble(s, ReadDouble(in, pos));
        case TAG_STR_NEW: {
            uint64_t len = ReadVarint(in, pos);
            if (len > in.size() - pos) {
                ThrowFakeluaException("serialize.decode: truncated string");
            }
            std::string str(in, pos, static_cast<size_t>(len));
            pos += static_cast<size_t>(len);
            uint32_t id = static_cast<uint32_t>(state.dict.size());
            state.dict.push_back(str);
            return inter::NativeToFakeluaString(s, state.dict[id]);
        }
        case TAG_STR_REF: {
            uint64_t id = ReadVarint(in, pos);
            if (id >= state.dict.size()) {
                ThrowFakeluaException("serialize.decode: bad string ref id");
            }
            return inter::NativeToFakeluaString(s, state.dict[id]);
        }
        case TAG_TABLE: {
            if (state.depth >= 64) {
                ThrowFakeluaException("serialize.decode: nesting too deep");
            }
            uint64_t count = ReadVarint(in, pos);
            // Each key/value pair is at least two tags.
            if (count > (in.size() - pos) / 2) {
                ThrowFakeluaException("serialize.decode: table too large");
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
            ThrowFakeluaException("serialize.decode: unknown tag: " + std::to_string(tag));
    }
}

// 原生函数
static CVar SerializeEncode(State *s, CVar *args, int n) {
    CVar v = inter::GetNativeArg(s, args, n, 0);
    if (!IsSupported(v)) {
        LOG_ERROR(s, "serialize", "serialize.encode: unsupported type: {}", VarTypeToString(static_cast<VarType>(v.type_)));
        ThrowFakeluaException("serialize.encode: unsupported type: " + VarTypeToString(static_cast<VarType>(v.type_)));
    }
    std::string out;
    out.reserve(64);
    EncodeState state;
    EncodeValue(out, v, state);
    LOG_DEBUG(s, "serialize", "serialize.encode: bytes={}", out.size());
    return inter::NativeToFakeluaString(s, out);
}

static CVar SerializeDecode(State *s, CVar *args, int n) {
    std::string in = CVarToString(inter::GetNativeArg(s, args, n, 0));
    size_t pos = 0;
    DecodeState state;
    CVar result = DecodeValue(in, pos, state, s);
    if (pos != in.size()) {
        LOG_ERROR(s, "serialize", "serialize.decode: trailing bytes (read={} total={})", pos, in.size());
        ThrowFakeluaException("serialize.decode: trailing bytes");
    }
    LOG_DEBUG(s, "serialize", "serialize.decode: bytes={}", in.size());
    return result;
}

// Boost.Serialization 树：不替换 encode/decode 的紧凑 wire，提供可读的 text/xml 往返。
// kind 用 int，避免 int8_t/signed char 被 Boost 当成字符写出。
struct SerNode {
    int kind = 0;// 0 nil, 1 bool, 2 int, 3 float, 4 string, 5 table
    bool b = false;
    int64_t i = 0;
    double f = 0;
    std::string s;
    // 拆成两个 vector：std::pair<SerNode, SerNode> 要求 SerNode 已完整，Clang+libstdc++ 会在
    // Boost.Serialization resize 时 static_assert 失败。vector<SerNode> 允许递归定义。
    std::vector<SerNode> keys;
    std::vector<SerNode> vals;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar & BOOST_SERIALIZATION_NVP(kind);
        ar & BOOST_SERIALIZATION_NVP(b);
        ar & BOOST_SERIALIZATION_NVP(i);
        ar & BOOST_SERIALIZATION_NVP(f);
        ar & BOOST_SERIALIZATION_NVP(s);
        ar & BOOST_SERIALIZATION_NVP(keys);
        ar & BOOST_SERIALIZATION_NVP(vals);
    }
};

static constexpr int kSerMaxDepth = 64;

static SerNode CVarToSerNode(CVar v, std::unordered_set<VarTable *> &visited, int depth, const char *op) {
    SerNode n;
    switch (v.type_) {
        case static_cast<int>(VarType::Nil):
            n.kind = 0;
            return n;
        case static_cast<int>(VarType::Bool):
            n.kind = 1;
            n.b = AsVar(v).GetBool();
            return n;
        case static_cast<int>(VarType::Int):
            n.kind = 2;
            n.i = v.data_.i;
            return n;
        case static_cast<int>(VarType::Float):
            n.kind = 3;
            n.f = v.data_.f;
            return n;
        case static_cast<int>(VarType::String):
        case static_cast<int>(VarType::StringId):
            n.kind = 4;
            n.s = CVarToString(v);
            return n;
        case static_cast<int>(VarType::Table): {
            if (depth >= kSerMaxDepth) {
                ThrowFakeluaException(std::string(op) + ": nesting too deep");
            }
            VarTable *t = v.data_.t;
            if (t && !visited.insert(t).second) {
                ThrowFakeluaException(std::string(op) + ": cyclic table");
            }
            n.kind = 5;
            if (t) {
                table::TableHelper::ForEachKV(v, [&](CVar k, CVar val) {
                    if (IsSupported(k) && IsSupported(val)) {
                        n.keys.push_back(CVarToSerNode(k, visited, depth + 1, op));
                        n.vals.push_back(CVarToSerNode(val, visited, depth + 1, op));
                    }
                });
                visited.erase(t);
            }
            return n;
        }
        default:
            ThrowFakeluaException(std::string(op) + ": unsupported type: " + VarTypeToString(static_cast<VarType>(v.type_)));
    }
}

static CVar SerNodeToCVar(const SerNode &n, State *s, int depth) {
    if (depth >= kSerMaxDepth) {
        ThrowFakeluaException("serialize.decode: nesting too deep");
    }
    switch (n.kind) {
        case 0:
            return inter::NativeToFakeluaNil(s);
        case 1:
            return inter::NativeToFakeluaBool(s, n.b);
        case 2:
            return inter::NativeToFakeluaLonglong(s, n.i);
        case 3:
            return inter::NativeToFakeluaDouble(s, n.f);
        case 4:
            return inter::NativeToFakeluaString(s, n.s);
        case 5: {
            CVar tbl = table::TableHelper::CreateTable(s);
            const size_t nkv = n.keys.size() < n.vals.size() ? n.keys.size() : n.vals.size();
            for (size_t i = 0; i < nkv; ++i) {
                table::TableHelper::SetTable(s, tbl, SerNodeToCVar(n.keys[i], s, depth + 1), SerNodeToCVar(n.vals[i], s, depth + 1));
            }
            return tbl;
        }
        default:
            ThrowFakeluaException("serialize.decode: unknown kind: " + std::to_string(n.kind));
    }
}

static SerNode RequireSerNode(State *s, CVar *args, int n, const char *op) {
    CVar v = inter::GetNativeArg(s, args, n, 0);
    if (!IsSupported(v)) {
        ThrowFakeluaException(std::string(op) + ": unsupported type: " + VarTypeToString(static_cast<VarType>(v.type_)));
    }
    std::unordered_set<VarTable *> visited;
    return CVarToSerNode(v, visited, 0, op);
}

static CVar SerializeTextEncode(State *s, CVar *args, int n) {
    SerNode root = RequireSerNode(s, args, n, "serialize.text_encode");
    try {
        std::ostringstream oss;
        {
            boost::archive::text_oarchive oa(oss);
            oa << root;
        }
        return inter::NativeToFakeluaString(s, oss.str());
    } catch (const boost::archive::archive_exception &e) {
        ThrowFakeluaException(std::format("serialize.text_encode: {}", e.what()));
    }
}

static CVar SerializeTextDecode(State *s, CVar *args, int n) {
    std::string in = CVarToString(inter::GetNativeArg(s, args, n, 0));
    try {
        std::istringstream iss(in);
        SerNode root;
        {
            boost::archive::text_iarchive ia(iss);
            ia >> root;
        }
        return SerNodeToCVar(root, s, 0);
    } catch (const boost::archive::archive_exception &e) {
        ThrowFakeluaException(std::format("serialize.text_decode: {}", e.what()));
    }
}

static CVar SerializeXmlEncode(State *s, CVar *args, int n) {
    SerNode root = RequireSerNode(s, args, n, "serialize.xml_encode");
    try {
        std::ostringstream oss;
        {
            boost::archive::xml_oarchive oa(oss);
            oa << boost::serialization::make_nvp("value", root);
        }
        return inter::NativeToFakeluaString(s, oss.str());
    } catch (const boost::archive::archive_exception &e) {
        ThrowFakeluaException(std::format("serialize.xml_encode: {}", e.what()));
    }
}

static CVar SerializeXmlDecode(State *s, CVar *args, int n) {
    std::string in = CVarToString(inter::GetNativeArg(s, args, n, 0));
    try {
        std::istringstream iss(in);
        SerNode root;
        {
            boost::archive::xml_iarchive ia(iss);
            ia >> boost::serialization::make_nvp("value", root);
        }
        return SerNodeToCVar(root, s, 0);
    } catch (const boost::archive::archive_exception &e) {
        ThrowFakeluaException(std::format("serialize.xml_decode: {}", e.what()));
    }
}

// 注册
void RegisterSerializeLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "serialize.encode", 1, false, SerializeEncode);
    RegisterNativeFunction(s, "serialize.decode", 1, false, SerializeDecode);
    RegisterNativeFunction(s, "serialize.text_encode", 1, false, SerializeTextEncode);
    RegisterNativeFunction(s, "serialize.text_decode", 1, false, SerializeTextDecode);
    RegisterNativeFunction(s, "serialize.xml_encode", 1, false, SerializeXmlEncode);
    RegisterNativeFunction(s, "serialize.xml_decode", 1, false, SerializeXmlDecode);
}

}// namespace fakelua::serialize
