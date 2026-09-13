#pragma once

#include <cstdint>
#include <string>

namespace fakelua::protobuf {

// Wire types
// 0  VARINT   int32, int64, uint32, uint64, sint32, sint64, bool, enum
//   1  I64      fixed64, sfixed64, double
//   2  LEN      string, bytes, message, packed repeated
//   5  I32      fixed32, sfixed32, float

enum WireType : uint8_t {
    WIRE_VARINT = 0,
    WIRE_64BIT = 1,
    WIRE_LEN = 2,
    WIRE_32BIT = 5,
};

// Field types（对应 FieldDescriptorProto::Type 枚举值 1-18）
enum FieldType : uint8_t {
    TYPE_DOUBLE = 1,
    TYPE_FLOAT = 2,
    TYPE_INT64 = 3,
    TYPE_UINT64 = 4,
    TYPE_INT32 = 5,
    TYPE_FIXED64 = 6,
    TYPE_FIXED32 = 7,
    TYPE_BOOL = 8,
    TYPE_STRING = 9,
    TYPE_GROUP = 10,// deprecated，不支持
    TYPE_MESSAGE = 11,
    TYPE_BYTES = 12,
    TYPE_UINT32 = 13,
    TYPE_ENUM = 14,
    TYPE_SFIXED32 = 15,
    TYPE_SFIXED64 = 16,
    TYPE_SINT32 = 17,
    TYPE_SINT64 = 18,
};

// Type → WireType 映射
uint8_t WireTypeForScalar(FieldType type);

// 判断标量类型是否可 packed
bool IsPackable(FieldType type);

// Varint（LEB128 无符号） （提前声明，供 write_tag 内联使用）

void WriteVarint(std::string &out, uint64_t v);

// Tag
// tag = (field_number << 3) | wire_type

inline void WriteTag(std::string &out, uint32_t field_number, uint8_t wire_type) {
    WriteVarint(out, (static_cast<uint64_t>(field_number) << 3) | wire_type);
}

inline uint32_t TagFieldNumber(uint64_t tag) {
    return static_cast<uint32_t>(tag >> 3);
}

inline uint8_t TagWireType(uint64_t tag) {
    return static_cast<uint8_t>(tag & 0x7);
}

// Varint 读取
uint64_t ReadVarint(const std::string &in, size_t &pos);

// Zigzag（有符号整数 ↔ 无符号）
inline void WriteVarintZigzag(std::string &out, int64_t v) {
    WriteVarint(out, (static_cast<uint64_t>(v) << 1) ^ static_cast<uint64_t>(v >> 63));
}

inline int64_t ReadVarintZigzag(const std::string &in, size_t &pos) {
    uint64_t u = ReadVarint(in, pos);
    return static_cast<int64_t>((u >> 1) ^ (-(u & 1)));
}

// Fixed-width（小端 memcpy）
void WriteFixed32(std::string &out, uint32_t v);
uint32_t ReadFixed32(const std::string &in, size_t &pos);
void WriteFixed64(std::string &out, uint64_t v);
uint64_t ReadFixed64(const std::string &in, size_t &pos);

// Float / Double（IEEE 754 bit-cast）
void WriteFloat(std::string &out, float v);
float ReadFloat(const std::string &in, size_t &pos);
void WriteDouble(std::string &out, double v);
double ReadDouble(const std::string &in, size_t &pos);

// Length-delimited
inline void WriteLengthDelimited(std::string &out, const char *data, size_t len) {
    WriteVarint(out, len);
    out.append(data, len);
}

std::string ReadLengthDelimited(const std::string &in, size_t &pos);

// Skip unknown field（按 wire type 跳过）
void SkipValue(const std::string &in, size_t &pos, uint8_t wire_type);

}// namespace fakelua::protobuf
