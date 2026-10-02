#include "native/serialize/native_serialize.h"
#include "native/serialize/wire_codec.h"
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
#include <format>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fakelua::serialize {

// 紧凑 wire 格式（nil/bool/int/float/string/table）的编解码在 wire_codec 中实现，
// Lua serialize.encode/decode 与内部异步回调绑定参数共用同一格式，这里只做 Lua 层封装。

// 辅助：从 CVar 提取字符串（二进制安全），供 Boost.Serialization 树使用。
static std::string CVarToString(CVar v) {
    if (v.type_ == static_cast<int>(VarType::String) && v.data_.s) {
        return std::string(v.data_.s->Str());
    }
    if (v.type_ == static_cast<int>(VarType::StringId) && v.data_.i) {
        const char *ptr = reinterpret_cast<const char *>(v.data_.i);
        int sz = *reinterpret_cast<const int *>(ptr);
        return std::string(ptr + 8, sz);
    }
    return {};
}

// 原生函数
static CVar SerializeEncode(State *s, CVar *args, int n) {
    CVar v = inter::GetNativeArg(s, args, n, 0);
    if (!WireIsSerializable(v)) {
        LOG_ERROR(s, "serialize", "serialize.encode: unsupported type: {}", VarTypeToString(static_cast<VarType>(v.type_)));
        ThrowFakeluaException("serialize.encode: unsupported type: " + VarTypeToString(static_cast<VarType>(v.type_)));
    }
    std::string out = WireEncode(v);
    LOG_DEBUG(s, "serialize", "serialize.encode: bytes={}", out.size());
    return inter::NativeToFakeluaString(s, out);
}

static CVar SerializeDecode(State *s, CVar *args, int n) {
    std::string in = CVarToString(inter::GetNativeArg(s, args, n, 0));
    CVar result = WireDecode(s, in);
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
                    if (WireIsSerializable(k) && WireIsSerializable(val)) {
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
    if (!WireIsSerializable(v)) {
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
