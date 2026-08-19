#include "protobuf_wire.h"

#include <cstring>

#include "var/var.h"

namespace fakelua::protobuf {

// ─── Type → WireType ───

uint8_t WireTypeForScalar(FieldType type) {
    switch (type) {
        case TYPE_INT32:
        case TYPE_INT64:
        case TYPE_UINT32:
        case TYPE_UINT64:
        case TYPE_SINT32:
        case TYPE_SINT64:
        case TYPE_BOOL:
        case TYPE_ENUM:
            return WIRE_VARINT;
        case TYPE_FIXED64:
        case TYPE_SFIXED64:
        case TYPE_DOUBLE:
            return WIRE_64BIT;
        case TYPE_STRING:
        case TYPE_BYTES:
        case TYPE_MESSAGE:
            return WIRE_LEN;
        case TYPE_FIXED32:
        case TYPE_SFIXED32:
        case TYPE_FLOAT:
            return WIRE_32BIT;
        default:
            return WIRE_LEN;  // unknown → length-delimited (safe fallback)
    }
}

bool IsPackable(FieldType type) {
    switch (type) {
        case TYPE_INT32:
        case TYPE_INT64:
        case TYPE_UINT32:
        case TYPE_UINT64:
        case TYPE_SINT32:
        case TYPE_SINT64:
        case TYPE_BOOL:
        case TYPE_ENUM:
        case TYPE_FIXED32:
        case TYPE_FIXED64:
        case TYPE_SFIXED32:
        case TYPE_SFIXED64:
        case TYPE_FLOAT:
        case TYPE_DOUBLE:
            return true;
        default:
            return false;
    }
}

// ─── Varint ───

void write_varint(std::string &out, uint64_t v) {
    while (v >= 0x80) {
        out.push_back(static_cast<char>((v & 0x7f) | 0x80));
        v >>= 7;
    }
    out.push_back(static_cast<char>(v));
}

uint64_t read_varint(const std::string &in, size_t &pos) {
    uint64_t result = 0;
    int shift = 0;
    while (pos < in.size()) {
        uint8_t b = static_cast<uint8_t>(in[pos++]);
        result |= static_cast<uint64_t>(b & 0x7f) << shift;
        if ((b & 0x80) == 0) break;
        shift += 7;
        if (shift >= 64) {
            ThrowFakeluaException("protobuf: varint too long");
        }
    }
    return result;
}

// ─── Fixed-width ───

void write_fixed32(std::string &out, uint32_t v) {
    uint8_t buf[4];
    std::memcpy(buf, &v, 4);
    out.append(reinterpret_cast<char *>(buf), 4);
}

uint32_t read_fixed32(const std::string &in, size_t &pos) {
    if (pos + 4 > in.size()) {
        ThrowFakeluaException("protobuf: truncated fixed32");
    }
    uint32_t v;
    std::memcpy(&v, in.data() + pos, 4);
    pos += 4;
    return v;
}

void write_fixed64(std::string &out, uint64_t v) {
    uint8_t buf[8];
    std::memcpy(buf, &v, 8);
    out.append(reinterpret_cast<char *>(buf), 8);
}

uint64_t read_fixed64(const std::string &in, size_t &pos) {
    if (pos + 8 > in.size()) {
        ThrowFakeluaException("protobuf: truncated fixed64");
    }
    uint64_t v;
    std::memcpy(&v, in.data() + pos, 8);
    pos += 8;
    return v;
}

// ─── Float / Double ───

void write_float(std::string &out, float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, 4);
    write_fixed32(out, bits);
}

float read_float(const std::string &in, size_t &pos) {
    uint32_t bits = read_fixed32(in, pos);
    float v;
    std::memcpy(&v, &bits, 4);
    return v;
}

void write_double(std::string &out, double v) {
    uint64_t bits;
    std::memcpy(&bits, &v, 8);
    write_fixed64(out, bits);
}

double read_double(const std::string &in, size_t &pos) {
    uint64_t bits = read_fixed64(in, pos);
    double v;
    std::memcpy(&v, &bits, 8);
    return v;
}

// ─── Length-delimited ───

std::string read_length_delimited(const std::string &in, size_t &pos) {
    uint64_t len = read_varint(in, pos);
    if (pos + len > in.size()) {
        ThrowFakeluaException("protobuf: truncated length-delimited");
    }
    std::string result(in, pos, len);  // (pos, len) ctor → binary safe
    pos += len;
    return result;
}

// ─── Skip unknown field ───

void skip_value(const std::string &in, size_t &pos, uint8_t wire_type) {
    switch (wire_type) {
        case WIRE_VARINT:
            read_varint(in, pos);
            break;
        case WIRE_64BIT:
            if (pos + 8 > in.size()) ThrowFakeluaException("protobuf: skip truncated 64bit");
            pos += 8;
            break;
        case WIRE_LEN: {
            uint64_t len = read_varint(in, pos);
            if (pos + len > in.size()) ThrowFakeluaException("protobuf: skip truncated len");
            pos += len;
            break;
        }
        case WIRE_32BIT:
            if (pos + 4 > in.size()) ThrowFakeluaException("protobuf: skip truncated 32bit");
            pos += 4;
            break;
        default:
            ThrowFakeluaException("protobuf: skip unknown wire type");
            break;
    }
}

}  // namespace fakelua::protobuf
