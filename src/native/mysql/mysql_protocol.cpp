#include "native/mysql/mysql_protocol.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <stdexcept>

namespace fakelua::mysql {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

[[noreturn]] static void protocol_error(const std::string &msg) {
    throw std::runtime_error("mysql protocol: " + msg);
}

static void ensure(const std::vector<char> &buf, size_t pos, size_t need) {
    if (pos + need > buf.size()) {
        protocol_error("unexpected end of packet");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Read primitives (little-endian)
// ─────────────────────────────────────────────────────────────────────────────

bool read_bytes(const std::vector<char> &buf, size_t &pos, char *out, size_t len) {
    if (pos + len > buf.size()) return false;
    std::memcpy(out, buf.data() + pos, len);
    pos += len;
    return true;
}

uint8_t read_uint8(const std::vector<char> &buf, size_t &pos) {
    ensure(buf, pos, 1);
    uint8_t v = static_cast<uint8_t>(buf[pos]);
    pos += 1;
    return v;
}

uint16_t read_uint16(const std::vector<char> &buf, size_t &pos) {
    ensure(buf, pos, 2);
    uint16_t v = static_cast<uint8_t>(buf[pos]) |
                 (static_cast<uint8_t>(buf[pos + 1]) << 8);
    pos += 2;
    return v;
}

uint32_t read_uint24(const std::vector<char> &buf, size_t &pos) {
    ensure(buf, pos, 3);
    uint32_t v = static_cast<uint8_t>(buf[pos]) |
                 (static_cast<uint8_t>(buf[pos + 1]) << 8) |
                 (static_cast<uint8_t>(buf[pos + 2]) << 16);
    pos += 3;
    return v;
}

uint32_t read_uint32(const std::vector<char> &buf, size_t &pos) {
    ensure(buf, pos, 4);
    uint32_t v = static_cast<uint8_t>(buf[pos]) |
                 (static_cast<uint8_t>(buf[pos + 1]) << 8) |
                 (static_cast<uint8_t>(buf[pos + 2]) << 16) |
                 (static_cast<uint8_t>(buf[pos + 3]) << 24);
    pos += 4;
    return v;
}

uint64_t read_uint64(const std::vector<char> &buf, size_t &pos) {
    ensure(buf, pos, 8);
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
        v |= static_cast<uint64_t>(static_cast<uint8_t>(buf[pos + i])) << (8 * i);
    }
    pos += 8;
    return v;
}

// ─────────────────────────────────────────────────────────────────────────────
// Write primitives (little-endian)
// ─────────────────────────────────────────────────────────────────────────────

void write_uint16(std::string &out, uint16_t v) {
    out.push_back(static_cast<char>(v & 0xFF));
    out.push_back(static_cast<char>((v >> 8) & 0xFF));
}

void write_uint24(std::string &out, uint32_t v) {
    out.push_back(static_cast<char>(v & 0xFF));
    out.push_back(static_cast<char>((v >> 8) & 0xFF));
    out.push_back(static_cast<char>((v >> 16) & 0xFF));
}

void write_uint32(std::string &out, uint32_t v) {
    out.push_back(static_cast<char>(v & 0xFF));
    out.push_back(static_cast<char>((v >> 8) & 0xFF));
    out.push_back(static_cast<char>((v >> 16) & 0xFF));
    out.push_back(static_cast<char>((v >> 24) & 0xFF));
}

void write_uint64(std::string &out, uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        out.push_back(static_cast<char>((v >> (8 * i)) & 0xFF));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Length-encoded integer
// ─────────────────────────────────────────────────────────────────────────────

uint64_t read_lenenc_int(const std::vector<char> &buf, size_t &pos) {
    ensure(buf, pos, 1);
    uint8_t first = static_cast<uint8_t>(buf[pos]);
    if (first < 0xFB) {
        pos += 1;
        return first;
    }
    if (first == 0xFB) {
        // NULL value marker
        protocol_error("unexpected NULL (0xFB) where integer expected");
    }
    if (first == 0xFC) {
        ensure(buf, pos, 3);
        pos += 1;
        return read_uint16(buf, pos);
    }
    if (first == 0xFD) {
        ensure(buf, pos, 4);
        pos += 1;
        return read_uint24(buf, pos);
    }
    // 0xFE → 8 bytes
    ensure(buf, pos, 9);
    pos += 1;
    return read_uint64(buf, pos);
}

void write_lenenc_int(std::string &out, uint64_t v) {
    if (v < 0xFB) {
        out.push_back(static_cast<char>(v & 0xFF));
    } else if (v < 0x10000) {
        out.push_back(static_cast<char>(0xFC));
        write_uint16(out, static_cast<uint16_t>(v));
    } else if (v < 0x1000000) {
        out.push_back(static_cast<char>(0xFD));
        write_uint24(out, static_cast<uint32_t>(v));
    } else {
        out.push_back(static_cast<char>(0xFE));
        write_uint64(out, v);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Length-encoded string
// ─────────────────────────────────────────────────────────────────────────────

std::string read_lenenc_str(const std::vector<char> &buf, size_t &pos) {
    uint64_t len = read_lenenc_int(buf, pos);
    if (len > buf.size() - pos) {
        protocol_error("lenenc string exceeds packet bounds");
    }
    std::string s(buf.data() + pos, static_cast<size_t>(len));
    pos += static_cast<size_t>(len);
    return s;
}

void write_lenenc_str(std::string &out, const char *data, size_t len) {
    write_lenenc_int(out, len);
    out.append(data, len);
}

// ─────────────────────────────────────────────────────────────────────────────
// NUL-terminated string
// ─────────────────────────────────────────────────────────────────────────────

std::string read_nul_str(const std::vector<char> &buf, size_t &pos) {
    size_t start = pos;
    while (pos < buf.size() && buf[pos] != '\0') {
        ++pos;
    }
    std::string s(buf.data() + start, pos - start);
    if (pos < buf.size()) ++pos;  // skip the NUL
    return s;
}

// ─────────────────────────────────────────────────────────────────────────────
// Packet framing
// ─────────────────────────────────────────────────────────────────────────────

std::string make_packet(uint8_t seq, const char *payload, size_t len) {
    std::string pkt;
    pkt.reserve(4 + len);
    write_uint24(pkt, static_cast<uint32_t>(len));
    pkt.push_back(static_cast<char>(seq));
    pkt.append(payload, len);
    return pkt;
}

// ─────────────────────────────────────────────────────────────────────────────
// mysql_native_password authentication (delegates to crypto::sha1)
// ─────────────────────────────────────────────────────────────────────────────

std::array<uint8_t, 20> native_password_hash(const std::string &password, const std::string &scramble) {
    if (scramble.size() != 20) {
        protocol_error("mysql_native_password: scramble must be 20 bytes");
    }

    // stage1 = SHA1(password)
    auto stage1 = crypto::sha1(password);

    // stage2 = SHA1(stage1)
    auto stage2 = crypto::sha1(stage1.data(), stage1.size());

    // hash = SHA1(scramble + stage2)
    std::string combined;
    combined.reserve(scramble.size() + stage2.size());
    combined.append(scramble);
    combined.append(reinterpret_cast<const char *>(stage2.data()), stage2.size());
    auto hash = crypto::sha1(combined);

    // response = stage1 XOR hash
    std::array<uint8_t, 20> response{};
    for (int i = 0; i < 20; ++i) {
        response[i] = stage1[i] ^ hash[i];
    }
    return response;
}

// ─────────────────────────────────────────────────────────────────────────────
// Handshake parsing
// ─────────────────────────────────────────────────────────────────────────────

HandshakeInfo parse_handshake(const std::vector<char> &payload) {
    HandshakeInfo info;
    size_t pos = 0;

    // Protocol version (should be 10)
    uint8_t proto = read_uint8(payload, pos);
    if (proto != 10) {
        protocol_error(std::format("unsupported protocol version {}", proto));
    }

    // Server version (NUL-terminated string)
    info.server_version = read_nul_str(payload, pos);

    // Connection ID (4 bytes)
    info.connection_id = read_uint32(payload, pos);

    // auth_plugin_data_part_1 (8 bytes)
    info.scramble_part1.assign(payload.data() + pos, 8);
    pos += 8;

    // filler 1 (0x00)
    pos += 1;

    // capability_flags_lower (2 bytes)
    info.capabilities_low = read_uint16(payload, pos);

    if (pos >= payload.size()) {
        // Minimal handshake (pre-4.1) — not supported
        protocol_error("handshake too short (pre-4.1 server?)");
    }

    // character_set (1 byte)
    info.charset = read_uint8(payload, pos);

    // status_flags (2 bytes)
    info.status = read_uint16(payload, pos);

    // capability_flags_upper (2 bytes)
    info.capabilities_high = read_uint16(payload, pos);

    // Combine capabilities
    info.capabilities = static_cast<uint32_t>(info.capabilities_low) |
                        (static_cast<uint32_t>(info.capabilities_high) << 16);

    // auth_plugin_data_len (1 byte) — length of part_2 (should be 21 for part1+part2=20+1 NUL)
    info.scramble_len = read_uint8(payload, pos);

    // reserved (10 bytes of 0x00)
    pos += 10;

    // auth_plugin_data_part_2 (at least 12 bytes; total scramble = part1(8) + part2(12) = 20)
    // Length = scramble_len - 1 (last byte is NUL). We need 12 bytes.
    size_t part2_len = 0;
    if (info.scramble_len > 0) {
        part2_len = info.scramble_len - 1;  // subtract trailing NUL
    }
    if (part2_len < 12) {
        protocol_error("handshake scramble part2 too short");
    }
    ensure(payload, pos, 12);
    info.scramble_part2.assign(payload.data() + pos, 12);
    pos += part2_len;  // skip remaining (including NUL)

    // auth_plugin_name (NUL-terminated) — present if CLIENT_PLUGIN_AUTH capability
    if (info.capabilities & CLIENT_PLUGIN_AUTH) {
        info.auth_plugin_name = read_nul_str(payload, pos);
    }

    return info;
}

// ─────────────────────────────────────────────────────────────────────────────
// Build client handshake response
// ─────────────────────────────────────────────────────────────────────────────

std::string build_handshake_response(const HandshakeInfo &info,
                                     const std::string &username,
                                     const std::string &password,
                                     const std::string &database) {
    std::string payload;
    payload.reserve(256);

    // capability flags (4 bytes) — our declared capabilities, intersected with server's
    uint32_t caps = kMyCapabilities & info.capabilities;
    write_uint32(payload, caps);

    // max packet size (4 bytes) — 16MB
    write_uint32(payload, MAX_PACKET_SIZE);

    // charset (1 byte) — use server's charset
    payload.push_back(static_cast<char>(info.charset));

    // reserved (23 bytes of 0x00)
    payload.append(23, '\0');

    // username (NUL-terminated)
    payload.append(username);
    payload.push_back('\0');

    // auth response — always compute mysql_native_password hash for the initial response.
    // If the server requires caching_sha2_password, it will send an Auth Switch Request (0xFE)
    // and we handle it in handle_handshake_packet().
    std::string scramble = info.scramble_part1 + info.scramble_part2;
    auto hash = native_password_hash(password, scramble);
    std::vector<uint8_t> auth(hash.begin(), hash.end());

    if (caps & CLIENT_PLUGIN_AUTH_LENENC) {
        // length-encoded auth response
        write_lenenc_str(payload, reinterpret_cast<const char *>(auth.data()), auth.size());
    } else {
        // 1-byte length prefix + fixed-length response
        payload.push_back(static_cast<char>(auth.size()));
        payload.append(reinterpret_cast<const char *>(auth.data()), auth.size());
    }

    // database (NUL-terminated) — if CLIENT_CONNECT_WITH_DB
    if ((caps & CLIENT_CONNECT_WITH_DB) && !database.empty()) {
        payload.append(database);
        payload.push_back('\0');
    }

    // auth plugin name (NUL-terminated) — if CLIENT_PLUGIN_AUTH
    // Always request mysql_native_password to avoid caching_sha2_password issues.
    // MySQL 8.0 defaults to caching_sha2_password which requires SSL for full auth
    // and has been observed to fail silently (fast auth → no OK) over plain TCP.
    // If the server insists on caching_sha2_password, it sends an Auth Switch Request
    // (0xFE) which we handle in handle_handshake_packet().
    if ((caps & CLIENT_PLUGIN_AUTH)) {
        payload.append("mysql_native_password");
        payload.push_back('\0');
    }

    return payload;
}

// ─────────────────────────────────────────────────────────────────────────────
// Response packet parsing
// ─────────────────────────────────────────────────────────────────────────────

ResponseType peek_type(uint8_t first_byte) {
    if (first_byte == PACKET_OK) return ResponseType::Ok;
    if (first_byte == PACKET_ERR) return ResponseType::Err;
    if (first_byte == PACKET_EOF) return ResponseType::Eof;
    return ResponseType::ResultSet;
}

OkResponse parse_ok(const std::vector<char> &payload) {
    OkResponse r;
    size_t pos = 0;
    // first byte is 0x00 (OK) or 0xFE (EOF-as-OK under DEPRECATE_EOF)
    if (pos >= payload.size()) protocol_error("empty OK packet");
    ++pos;  // skip header byte
    r.affected_rows = read_lenenc_int(payload, pos);
    r.last_insert_id = read_lenenc_int(payload, pos);
    r.status_flags = read_uint16(payload, pos);
    r.warnings = read_uint16(payload, pos);
    // remaining is info string
    if (pos < payload.size()) {
        r.info.assign(payload.data() + pos, payload.size() - pos);
    }
    return r;
}

ErrResponse parse_err(const std::vector<char> &payload) {
    ErrResponse r;
    size_t pos = 0;
    if (pos >= payload.size() || payload[pos] != PACKET_ERR) {
        protocol_error("not an ERR packet");
    }
    ++pos;  // skip 0xFF
    r.error_code = read_uint16(payload, pos);
    // SQL state marker '#' (0x23) + 5-byte state, if CLIENT_PROTOCOL_41
    if (pos < payload.size() && payload[pos] == '#') {
        ++pos;
        if (pos + 5 > payload.size()) protocol_error("ERR packet SQL state truncated");
        r.sql_state.assign(payload.data() + pos, 5);
        pos += 5;
    }
    // remaining is error message
    if (pos < payload.size()) {
        r.message.assign(payload.data() + pos, payload.size() - pos);
    }
    return r;
}

bool parse_eof(const std::vector<char> &payload, uint16_t &warnings, uint16_t &status) {
    size_t pos = 0;
    if (pos >= payload.size()) return false;
    uint8_t header = static_cast<uint8_t>(payload[pos]);
    if (header != PACKET_EOF) return false;
    ++pos;
    warnings = read_uint16(payload, pos);
    status = read_uint16(payload, pos);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// caching_sha2_password authentication (MySQL 8+ default)
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> caching_sha2_password_hash(const std::string &password, const std::string &scramble) {
    if (scramble.size() != 20) {
        protocol_error("caching_sha2_password: scramble must be 20 bytes");
    }

    // SHA256(password)
    auto stage1 = crypto::sha256(password);

    // SHA256(SHA256(password))
    auto stage2 = crypto::sha256(stage1.data(), stage1.size());

    // SHA256(scramble + SHA256(SHA256(password)))
    std::string combined;
    combined.reserve(scramble.size() + stage2.size());
    combined.append(scramble);
    combined.append(reinterpret_cast<const char *>(stage2.data()), stage2.size());
    auto hash = crypto::sha256(combined);

    // response = SHA256(password) XOR hash
    std::vector<uint8_t> response(32);
    for (size_t i = 0; i < 32; ++i) {
        response[i] = stage1[i] ^ hash[i];
    }
    return response;
}

// ─────────────────────────────────────────────────────────────────────────────
// Prepared statements (binary protocol)
// ─────────────────────────────────────────────────────────────────────────────

PrepareResult parse_prepare_response(const std::vector<char> &payload) {
    PrepareResult result;
    size_t pos = 0;

    if (payload.empty()) return result;

    // status byte: 0x00 = OK
    uint8_t status = read_uint8(payload, pos);
    if (status != 0x00) return result;

    // statement_id (4 bytes)
    result.statement_id = read_uint32(payload, pos);

    // num_columns (2 bytes)
    result.num_columns = read_uint16(payload, pos);

    // num_params (2 bytes)
    result.num_params = read_uint16(payload, pos);

    // filler 1 byte
    pos += 1;

    // num_warnings (2 bytes)
    result.num_warnings = read_uint16(payload, pos);

    result.valid = true;
    return result;
}

std::string build_stmt_execute(uint32_t statement_id,
                              const std::vector<std::string> &params) {
    std::string payload;
    payload.reserve(128);

    // command byte
    payload.push_back(static_cast<char>(COM_STMT_EXECUTE));

    // statement_id (4 bytes)
    write_uint32(payload, statement_id);

    // flags: 0 = CURSOR_TYPE_NO_CURSOR
    payload.push_back(0x00);

    // iteration_count (4 bytes, always 1)
    write_uint32(payload, 1);

    if (params.empty()) {
        return payload;
    }

    // null bitmap: ceil(num_params / 8) bytes
    size_t bitmap_size = (params.size() + 7) / 8;
    payload.append(bitmap_size, '\0');

    // new_params_bind_flag: 1 = bind types
    payload.push_back(0x01);

    // parameter types: 2 bytes each (MYSQL_TYPE_STRING = 0xfd for simplicity)
    for (size_t i = 0; i < params.size(); ++i) {
        write_uint16(payload, 0xfd);  // MYSQL_TYPE_VARCHAR
    }

    // parameter values: length-encoded strings
    for (const auto &p : params) {
        write_lenenc_str(payload, p.data(), p.size());
    }

    return payload;
}

std::vector<std::pair<bool, std::string>> parse_binary_row(const std::vector<char> &payload,
                                                           size_t num_columns) {
    std::vector<std::pair<bool, std::string>> row;
    row.reserve(num_columns);

    if (payload.empty()) return row;

    size_t pos = 0;

    // packet header (1 byte, should be 0x00 or length)
    uint8_t header = read_uint8(payload, pos);
    if (header != 0x00) {
        // Not a valid binary row packet
        return row;
    }

    // null bitmap: ceil(num_columns / 8) bytes, but bits are stored differently
    // Bit i (from LSB of byte i/8) is set if column i is NULL
    size_t bitmap_size = (num_columns + 7 + 2) / 8;  // +2 for offset
    if (pos + bitmap_size > payload.size()) return row;

    std::vector<uint8_t> bitmap(bitmap_size);
    for (size_t i = 0; i < bitmap_size; ++i) {
        bitmap[i] = static_cast<uint8_t>(payload[pos + i]);
    }
    pos += bitmap_size;

    for (size_t col = 0; col < num_columns; ++col) {
        // Check null bitmap (bit position col+2 because of 2-bit offset)
        size_t bit_pos = col + 2;
        size_t byte_idx = bit_pos / 8;
        size_t bit_idx = bit_pos % 8;
        bool is_null = (byte_idx < bitmap.size()) && (bitmap[byte_idx] & (1 << bit_idx));

        if (is_null) {
            row.emplace_back(true, "");
        } else {
            // Read length-encoded string
            row.emplace_back(false, read_lenenc_str(payload, pos));
        }
    }

    return row;
}

}  // namespace fakelua::mysql
