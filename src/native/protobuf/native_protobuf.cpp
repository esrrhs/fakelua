#include "native_protobuf.h"

#include "protobuf_codec.h"
#include "protobuf_parser.h"
#include "protobuf_schema.h"

#include "native/native_common.h"
#include "native/table/native_table.h"
#include "util/logging.h"
#include "var/var_string.h"

#include <format>
#include <string>
#include <string_view>
#include <vector>

namespace fakelua::protobuf {

using table::TableHelper;

// ─── 辅助：从 CVar 提取字符串（二进制安全）───

static std::string CVarToString(const CVar &v) {
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

// ─── protobuf.load(proto_text) ───
// 解析 .proto 文本，注册所有 message/enum。成功返回 "ok"，失败返回错误信息字符串。

static CVar pb_load(State *s, CVar *args, int n) {
    if (n < 1) {
        return inter::NativeToFakeluaString(s, "protobuf.load: missing argument");
    }
    std::string text = CVarToString(args[0]);
    std::string err = ParseProto(text);
    if (!err.empty()) {
        LOG_ERROR("protobuf", "protobuf.load failed: {}", err);
        return inter::NativeToFakeluaString(s, err);
    }
    LOG_DEBUG("protobuf", "protobuf.load: ok (text_len={})", text.size());
    return inter::NativeToFakeluaString(s, "ok");
}

// ─── protobuf.encode(message_name, table) → binary ───

static CVar pb_encode(State *s, CVar *args, int n) {
    if (n < 2) {
        ThrowFakeluaException("protobuf.encode: requires message_name and table");
    }
    std::string msg_name = CVarToString(args[0]);
    CVar table = args[1];

    std::string bin = EncodeMessage(s, msg_name, table);
    LOG_DEBUG("protobuf", "protobuf.encode: msg={} bytes={}", msg_name, bin.size());
    return inter::NativeToFakeluaString(s, bin);
}

// ─── protobuf.decode(message_name, binary) → table ───

static CVar pb_decode(State *s, CVar *args, int n) {
    if (n < 2) {
        ThrowFakeluaException("protobuf.decode: requires message_name and binary");
    }
    std::string msg_name = CVarToString(args[0]);
    std::string data = CVarToString(args[1]);

    LOG_DEBUG("protobuf", "protobuf.decode: msg={} bytes={}", msg_name, data.size());
    return DecodeMessage(s, msg_name, data);
}

// ─── protobuf.types() → 已注册消息名列表 ───

static CVar pb_types(State *s, CVar *args, int n) {
    auto names = ProtobufState::Instance().MessageNames();
    CVar tbl = TableHelper::CreateTable(s);
    for (size_t i = 0; i < names.size(); ++i) {
        TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(i + 1),
                                 inter::NativeToFakeluaString(s, names[i]));
    }
    return tbl;
}

// ─── protobuf.fields(message_name) → 字段信息表 ───

static CVar pb_fields(State *s, CVar *args, int n) {
    if (n < 1) {
        ThrowFakeluaException("protobuf.fields: requires message_name");
    }
    std::string msg_name = CVarToString(args[0]);
    const MessageDef *msg = ProtobufState::Instance().FindMessage(msg_name);
    if (!msg) {
        ThrowFakeluaException(std::format("protobuf.fields: unknown message '{}'", msg_name));
    }

    CVar tbl = TableHelper::CreateTable(s);
    int64_t idx = 0;
    for (const auto &field : msg->fields) {
        idx++;
        CVar field_tbl = TableHelper::CreateTable(s);
        TableHelper::SetTableStrId(s, field_tbl, "name", inter::NativeToFakeluaString(s, field.name));
        TableHelper::SetTableStrId(s, field_tbl, "number", inter::NativeToFakeluaInt(s, field.number));
        TableHelper::SetTableStrId(s, field_tbl, "type", inter::NativeToFakeluaString(s, [field]() {
            switch (field.type) {
                case TYPE_DOUBLE: return "double";
                case TYPE_FLOAT: return "float";
                case TYPE_INT64: return "int64";
                case TYPE_UINT64: return "uint64";
                case TYPE_INT32: return "int32";
                case TYPE_FIXED64: return "fixed64";
                case TYPE_FIXED32: return "fixed32";
                case TYPE_BOOL: return "bool";
                case TYPE_STRING: return "string";
                case TYPE_BYTES: return "bytes";
                case TYPE_UINT32: return "uint32";
                case TYPE_ENUM: return "enum";
                case TYPE_SFIXED32: return "sfixed32";
                case TYPE_SFIXED64: return "sfixed64";
                case TYPE_SINT32: return "sint32";
                case TYPE_SINT64: return "sint64";
                case TYPE_MESSAGE: return "message";
                default: return "unknown";
            }
        }()));
        if (!field.type_name.empty()) {
            TableHelper::SetTableStrId(s, field_tbl, "type_name", inter::NativeToFakeluaString(s, field.type_name));
        }
        TableHelper::SetTableStrId(s, field_tbl, "label", inter::NativeToFakeluaString(s, [field]() {
            if (field.is_map) return "map";
            if (field.repeated) return "repeated";
            if (field.optional) return "optional";
            return "singular";
        }()));
        TableHelper::SetTableInt(s, tbl, idx, field_tbl);
    }
    return tbl;
}

// ─── 注册 ───

void RegisterProtobufLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "protobuf.load", 1, false, pb_load);
    RegisterNativeFunction(s, "protobuf.encode", 2, false, pb_encode);
    RegisterNativeFunction(s, "protobuf.decode", 2, false, pb_decode);
    RegisterNativeFunction(s, "protobuf.types", 0, false, pb_types);
    RegisterNativeFunction(s, "protobuf.fields", 1, false, pb_fields);
}

}  // namespace fakelua::protobuf
