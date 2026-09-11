#include "protobuf_codec.h"

#include "native/table/native_table.h"
#include "var/var.h"
#include "var/var_string.h"
#include "var/var_table.h"

#include <cstring>
#include <format>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace fakelua::protobuf {

using table::TableHelper;

// 辅助：从 Lua table 取值
static CVar GetField(State *s, const CVar &tbl, const std::string &name) {
    return TableHelper::GetTableStrId(s, tbl, name.c_str());
}

// 辅助：VarType 转 int（避免 static_cast<int>(VarType::Float>) 的解析歧义）
static int VarTypeToInt(VarType t) {
    return static_cast<int>(t);
}

static bool IsVarType(const CVar &v, VarType t) {
    return v.type_ == VarTypeToInt(t);
}

// 辅助：从 CVar 提取字符串（二进制安全）
static std::string CVarToString(const CVar &v) {
    if (IsVarType(v, VarType::String) && v.data_.s) {
        auto sv = v.data_.s->Str();
        return std::string(sv.data(), sv.size());
    }
    if (IsVarType(v, VarType::StringId) && v.data_.i) {
        const char *ptr = reinterpret_cast<const char *>(v.data_.i);
        int sz = *reinterpret_cast<const int *>(ptr);
        return std::string(ptr + 8, sz);
    }
    return {};
}

// 辅助：判断标量值是否为零值
static bool IsScalarZero(const CVar &v) {
    if (IsVarType(v, VarType::Nil)) return true;
    if (IsVarType(v, VarType::Int)) return v.data_.i == 0;
    if (IsVarType(v, VarType::Float)) return v.data_.f == 0.0;
    if (IsVarType(v, VarType::Bool)) return !v.data_.b;
    if (IsVarType(v, VarType::String) || IsVarType(v, VarType::StringId)) {
        return CVarToString(v).empty();
    }
    return false;
}

// 辅助：从 CVar 提取 int64
static int64_t CVarToInt(const CVar &v) {
    if (IsVarType(v, VarType::Bool)) return v.data_.b ? 1 : 0;
    return inter::CVarToInteger(v, 0);
}

// 辅助：从 CVar 提取 double
static double CVarToDouble(const CVar &v) {
    if (v.type_ == static_cast<int>(VarType::Float)) return v.data_.f;
    if (v.type_ == static_cast<int>(VarType::Int)) return static_cast<double>(v.data_.i);
    return 0.0;
}

// 辅助：遍历 Lua table 的所有键值对
struct KVPair {
    CVar key;
    CVar val;
};

static std::vector<KVPair> CollectKVPairs(const CVar &tbl) {
    std::vector<KVPair> result;
    TableHelper::ForEachKV(tbl, [&](CVar k, CVar val) {
        if (k.type_ != static_cast<int>(VarType::Nil)) {
            result.push_back({k, val});
        }
    });
    return result;
}

// 辅助：判断 Lua table 是数组（连续整数键 1..N）还是 map
static bool IsArrayTable(const CVar &tbl, size_t &out_len) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
        out_len = 0;
        return false;
    }
    int64_t seq_len = TableHelper::GetTableLen(tbl);
    if (seq_len <= 0) {
        out_len = 0;
        return false;
    }
    size_t kv_count = 0;
    bool ok = true;
    TableHelper::ForEachKV(tbl, [&](CVar k, CVar /*val*/) {
        ++kv_count;
        if (k.type_ != static_cast<int>(VarType::Int) || k.data_.i < 1 || k.data_.i > seq_len) {
            ok = false;
        }
    });
    if (!ok || kv_count != static_cast<size_t>(seq_len)) {
        out_len = 0;
        return false;
    }
    out_len = static_cast<size_t>(seq_len);
    return true;
}

// 编码单个标量值（不含 tag）
static void EncodeScalar(std::string &out, FieldType type, const CVar &v) {
    switch (type) {
        case TYPE_DOUBLE:
            WriteDouble(out, CVarToDouble(v));
            break;
        case TYPE_FLOAT:
            WriteFloat(out, static_cast<float>(CVarToDouble(v)));
            break;
        case TYPE_INT64:
        case TYPE_UINT64:
            WriteVarint(out, static_cast<uint64_t>(CVarToInt(v)));
            break;
        case TYPE_INT32:
        case TYPE_UINT32:
            WriteVarint(out, static_cast<uint64_t>(CVarToInt(v)));
            break;
        case TYPE_SINT32:
        case TYPE_SINT64:
            WriteVarintZigzag(out, CVarToInt(v));
            break;
        case TYPE_FIXED32:
            WriteFixed32(out, static_cast<uint32_t>(CVarToInt(v)));
            break;
        case TYPE_FIXED64:
            WriteFixed64(out, static_cast<uint64_t>(CVarToInt(v)));
            break;
        case TYPE_SFIXED32:
            WriteFixed32(out, static_cast<uint32_t>(CVarToInt(v)));
            break;
        case TYPE_SFIXED64:
            WriteFixed64(out, static_cast<uint64_t>(CVarToInt(v)));
            break;
        case TYPE_BOOL:
            WriteVarint(out, CVarToInt(v) != 0 ? 1 : 0);
            break;
        case TYPE_ENUM:
            WriteVarint(out, static_cast<uint64_t>(CVarToInt(v)));
            break;
        case TYPE_STRING:
        case TYPE_BYTES: {
            std::string s = CVarToString(v);
            WriteLengthDelimited(out, s.data(), s.size());
            break;
        }
        case TYPE_MESSAGE: {
            // 嵌套 message：递归编码后写入 length-delimited
            // 注意：type_name 在调用方处理，这里不直接支持
            break;
        }
        default:
            break;
    }
}

// 编码一个字段（含 tag）
static constexpr int kMaxProtoDepth = 64;

static std::string EncodeMessageImpl(State *s, const std::string &msg_name, const CVar &table, std::unordered_set<VarTable *> &visited, int depth);

static void EncodeField(std::string &out, const FieldDef &field, const CVar &v, State *s, std::unordered_set<VarTable *> &visited, int depth) {
    uint8_t wire = WireTypeForScalar(field.type);

    if (field.is_map) {
        // map → repeated {key, value} entry message
        auto kvs = CollectKVPairs(v);
        for (auto &[key, val]: kvs) {
            // 编码 entry sub-message 到临时缓冲区
            std::string entry_buf;
            // key = field 1（需要 tag + value）
            WriteTag(entry_buf, 1, WireTypeForScalar(field.map_key_type));
            EncodeScalar(entry_buf, field.map_key_type, key);
            // value = field 2
            if (field.map_value_type == TYPE_MESSAGE) {
                const MessageDef *msg = GetProtobufState(s).FindMessage(field.map_value_type_name);
                if (!msg) {
                    ThrowFakeluaException(std::format("protobuf.encode: unknown map value type '{}'", field.map_value_type_name));
                }
                std::string sub = EncodeMessageImpl(s, field.map_value_type_name, val, visited, depth + 1);
                WriteTag(entry_buf, 2, WIRE_LEN);
                WriteLengthDelimited(entry_buf, sub.data(), sub.size());
            } else {
                std::string val_buf;
                EncodeScalar(val_buf, field.map_value_type, val);
                WriteTag(entry_buf, 2, WireTypeForScalar(field.map_value_type));
                entry_buf += val_buf;
            }
            WriteTag(out, field.number, WIRE_LEN);
            WriteLengthDelimited(out, entry_buf.data(), entry_buf.size());
        }
        return;
    }

    if (field.repeated) {
        size_t arr_len = 0;
        bool is_arr = IsArrayTable(v, arr_len);

        if (is_arr && IsPackable(field.type)) {
            // packed repeated：一个 WIRE_LEN 记录
            std::string packed;
            for (size_t i = 0; i < arr_len; ++i) {
                CVar elem = TableHelper::GetTableInt(s, v, static_cast<int64_t>(i + 1));
                EncodeScalar(packed, field.type, elem);
            }
            WriteTag(out, field.number, WIRE_LEN);
            WriteLengthDelimited(out, packed.data(), packed.size());
        } else {
            // unpacked：每个值一个记录（或 map 值不是数组）
            if (is_arr) {
                for (size_t i = 0; i < arr_len; ++i) {
                    CVar elem = TableHelper::GetTableInt(s, v, static_cast<int64_t>(i + 1));
                    WriteTag(out, field.number, wire);
                    if (field.type == TYPE_MESSAGE) {
                        const MessageDef *msg = GetProtobufState(s).FindMessage(field.type_name);
                        if (!msg) {
                            ThrowFakeluaException(std::format("protobuf.encode: unknown message type '{}'", field.type_name));
                        }
                        std::string sub = EncodeMessageImpl(s, field.type_name, elem, visited, depth + 1);
                        WriteLengthDelimited(out, sub.data(), sub.size());
                    } else {
                        EncodeScalar(out, field.type, elem);
                    }
                }
            } else {
                if (IsVarType(v, VarType::Nil) || (IsVarType(v, VarType::Table) && CollectKVPairs(v).empty())) {
                    return;
                }
                WriteTag(out, field.number, wire);
                if (field.type == TYPE_MESSAGE) {
                    const MessageDef *msg = GetProtobufState(s).FindMessage(field.type_name);
                    if (!msg) {
                        ThrowFakeluaException(std::format("protobuf.encode: unknown message type '{}'", field.type_name));
                    }
                    std::string sub = EncodeMessageImpl(s, field.type_name, v, visited, depth + 1);
                    WriteLengthDelimited(out, sub.data(), sub.size());
                } else {
                    EncodeScalar(out, field.type, v);
                }
            }
        }
        return;
    }

    // 普通标量字段
    WriteTag(out, field.number, wire);
    if (field.type == TYPE_MESSAGE) {
        const MessageDef *msg = GetProtobufState(s).FindMessage(field.type_name);
        if (!msg) {
            ThrowFakeluaException(std::format("protobuf.encode: unknown message type '{}'", field.type_name));
        }
        std::string sub = EncodeMessageImpl(s, field.type_name, v, visited, depth + 1);
        WriteLengthDelimited(out, sub.data(), sub.size());
    } else {
        EncodeScalar(out, field.type, v);
    }
}

// 编码 message
static std::string EncodeMessageImpl(State *s, const std::string &msg_name, const CVar &table, std::unordered_set<VarTable *> &visited, int depth) {
    if (depth > kMaxProtoDepth) {
        ThrowFakeluaException("protobuf.encode: nesting too deep");
    }
    const MessageDef *msg = GetProtobufState(s).FindMessage(msg_name);
    if (!msg) {
        ThrowFakeluaException(std::format("protobuf.encode: unknown message type '{}'", msg_name));
    }

    VarTable *t = (IsVarType(table, VarType::Table) && table.data_.t) ? table.data_.t : nullptr;
    if (t && !visited.insert(t).second) {
        ThrowFakeluaException("protobuf.encode: cyclic table");
    }

    std::string out;
    try {
        for (const auto &field: msg->fields) {
            CVar value = GetField(s, table, field.name);

            if (!field.repeated && !field.is_map && IsVarType(value, VarType::Nil)) {
                continue;
            }
            if (field.repeated || field.is_map) {
                if (IsVarType(value, VarType::Nil)) continue;
                if (IsVarType(value, VarType::Table) && CollectKVPairs(value).empty()) continue;
            } else if (!field.optional && IsScalarZero(value)) {
                continue;
            }

            EncodeField(out, field, value, s, visited, depth);
        }
    } catch (...) {
        if (t) visited.erase(t);
        throw;
    }
    if (t) visited.erase(t);
    return out;
}

std::string EncodeMessage(State *s, const std::string &msg_name, const CVar &table) {
    std::unordered_set<VarTable *> visited;
    return EncodeMessageImpl(s, msg_name, table, visited, 0);
}

// 解码单个标量值
static CVar DecodeScalar(const std::string &data, size_t &pos, FieldType type, State *s) {
    switch (type) {
        case TYPE_DOUBLE:
            return inter::NativeToFakeluaDouble(s, ReadDouble(data, pos));
        case TYPE_FLOAT:
            return inter::NativeToFakeluaDouble(s, ReadFloat(data, pos));
        case TYPE_INT64:
        case TYPE_UINT64:
            return inter::NativeToFakeluaLonglong(s, static_cast<long long>(ReadVarint(data, pos)));
        case TYPE_INT32:
            return inter::NativeToFakeluaInt(s, static_cast<int>(ReadVarint(data, pos)));
        case TYPE_UINT32:
            return inter::NativeToFakeluaLonglong(s, static_cast<long long>(ReadVarint(data, pos) & 0xFFFFFFFFULL));
        case TYPE_SINT32:
            return inter::NativeToFakeluaInt(s, static_cast<int>(ReadVarintZigzag(data, pos)));
        case TYPE_SINT64:
            return inter::NativeToFakeluaLonglong(s, ReadVarintZigzag(data, pos));
        case TYPE_FIXED32:
            return inter::NativeToFakeluaLonglong(s, static_cast<long long>(ReadFixed32(data, pos)));
        case TYPE_SFIXED32:
            return inter::NativeToFakeluaInt(s, static_cast<int>(ReadFixed32(data, pos)));
        case TYPE_FIXED64:
        case TYPE_SFIXED64:
            return inter::NativeToFakeluaLonglong(s, static_cast<long long>(ReadFixed64(data, pos)));
        case TYPE_BOOL:
            return inter::NativeToFakeluaBool(s, ReadVarint(data, pos) != 0);
        case TYPE_ENUM:
            return inter::NativeToFakeluaInt(s, static_cast<int>(ReadVarint(data, pos)));
        case TYPE_STRING: {
            std::string str = ReadLengthDelimited(data, pos);
            return inter::NativeToFakeluaString(s, str);
        }
        case TYPE_BYTES: {
            std::string str = ReadLengthDelimited(data, pos);
            return inter::NativeToFakeluaString(s, str);
        }
        case TYPE_MESSAGE: {
            // 嵌套 message：length-delimited 后递归解码
            // 注意：type_name 在调用方处理
            return inter::NativeToFakeluaNil(s);
        }
        default:
            return inter::NativeToFakeluaNil(s);
    }
}

static bool FieldWireOk(const FieldDef *field, uint8_t wire_type) {
    if (field->is_map || field->type == TYPE_MESSAGE) {
        return wire_type == WIRE_LEN;
    }
    if (wire_type == WireTypeForScalar(field->type)) return true;
    // proto3 packed repeated scalars arrive as length-delimited
    if (field->repeated && wire_type == WIRE_LEN && IsPackable(field->type)) return true;
    return false;
}

// 解码 message
static CVar DecodeMessageImpl(State *s, const std::string &msg_name, const std::string &data, int depth);

CVar DecodeMessage(State *s, const std::string &msg_name, const std::string &data) {
    return DecodeMessageImpl(s, msg_name, data, 0);
}

static CVar DecodeMessageImpl(State *s, const std::string &msg_name, const std::string &data, int depth) {
    if (depth > kMaxProtoDepth) {
        ThrowFakeluaException("protobuf.decode: nesting too deep");
    }
    const MessageDef *msg = GetProtobufState(s).FindMessage(msg_name);
    if (!msg) {
        ThrowFakeluaException(std::format("protobuf.decode: unknown message type '{}'", msg_name));
    }

    CVar tbl = TableHelper::CreateTable(s);
    size_t pos = 0;

    while (pos < data.size()) {
        uint64_t tag = ReadVarint(data, pos);
        uint32_t field_number = TagFieldNumber(tag);
        uint8_t wire_type = TagWireType(tag);

        const FieldDef *field = nullptr;
        auto it = msg->number_to_field.find(field_number);
        if (it != msg->number_to_field.end()) field = it->second;

        if (!field) {
            // 未知字段 → 跳过
            SkipValue(data, pos, wire_type);
            continue;
        }

        if (!FieldWireOk(field, wire_type)) {
            SkipValue(data, pos, wire_type);
            continue;
        }

        if (field->is_map) {
            // map → 获取或创建子表，读取 entry sub-message 填入
            CVar map_tbl = TableHelper::GetTableStrId(s, tbl, field->name.c_str());
            if (map_tbl.type_ == static_cast<int>(VarType::Nil)) {
                map_tbl = TableHelper::CreateTable(s);
                TableHelper::SetTableStrId(s, tbl, field->name.c_str(), map_tbl);
            }

            if (wire_type == WIRE_LEN) {
                std::string entry_data = ReadLengthDelimited(data, pos);
                size_t epos = 0;
                CVar key;
                CVar val;
                bool has_key = false, has_val = false;

                while (epos < entry_data.size()) {
                    uint64_t etag = ReadVarint(entry_data, epos);
                    uint32_t enum_field = TagFieldNumber(etag);
                    uint8_t ewire = TagWireType(etag);

                    if (enum_field == 1) {
                        if (ewire != WireTypeForScalar(field->map_key_type)) {
                            SkipValue(entry_data, epos, ewire);
                            continue;
                        }
                        key = DecodeScalar(entry_data, epos, field->map_key_type, s);
                        has_key = true;
                    } else if (enum_field == 2) {
                        uint8_t expected = field->map_value_type == TYPE_MESSAGE ? static_cast<uint8_t>(WIRE_LEN) : WireTypeForScalar(field->map_value_type);
                        if (ewire != expected) {
                            SkipValue(entry_data, epos, ewire);
                            continue;
                        }
                        if (field->map_value_type == TYPE_MESSAGE) {
                            std::string sub = ReadLengthDelimited(entry_data, epos);
                            val = DecodeMessageImpl(s, field->map_value_type_name, sub, depth + 1);
                        } else {
                            val = DecodeScalar(entry_data, epos, field->map_value_type, s);
                        }
                        has_val = true;
                    } else {
                        SkipValue(entry_data, epos, ewire);
                    }
                }

                if (has_key && has_val) {
                    TableHelper::SetTable(s, map_tbl, key, val);
                }
            } else {
                SkipValue(data, pos, wire_type);
            }
            continue;
        }

        if (field->repeated) {
            // 获取或创建 repeated 字段的子表
            CVar arr = TableHelper::GetTableStrId(s, tbl, field->name.c_str());
            if (arr.type_ == static_cast<int>(VarType::Nil)) {
                arr = TableHelper::CreateTable(s);
                TableHelper::SetTableStrId(s, tbl, field->name.c_str(), arr);
            }
            size_t base_len = TableHelper::GetTableLen(arr);
            size_t idx = base_len;

            if (wire_type == WIRE_LEN && IsPackable(field->type)) {
                // packed repeated
                std::string packed = ReadLengthDelimited(data, pos);
                size_t ppos = 0;
                while (ppos < packed.size()) {
                    CVar elem = DecodeScalar(packed, ppos, field->type, s);
                    idx++;
                    TableHelper::SetTableInt(s, arr, static_cast<int64_t>(idx), elem);
                }
            } else {
                // unpacked：追加一个值
                idx++;
                if (field->type == TYPE_MESSAGE) {
                    std::string sub = ReadLengthDelimited(data, pos);
                    CVar elem = DecodeMessageImpl(s, field->type_name, sub, depth + 1);
                    TableHelper::SetTableInt(s, arr, static_cast<int64_t>(idx), elem);
                } else {
                    CVar elem = DecodeScalar(data, pos, field->type, s);
                    TableHelper::SetTableInt(s, arr, static_cast<int64_t>(idx), elem);
                }
            }
            continue;
        }

        // 普通字段
        if (field->type == TYPE_MESSAGE) {
            std::string sub = ReadLengthDelimited(data, pos);
            CVar val = DecodeMessageImpl(s, field->type_name, sub, depth + 1);
            TableHelper::SetTableStrId(s, tbl, field->name.c_str(), val);
        } else {
            CVar val = DecodeScalar(data, pos, field->type, s);
            TableHelper::SetTableStrId(s, tbl, field->name.c_str(), val);
        }
    }

    return tbl;
}

}// namespace fakelua::protobuf
