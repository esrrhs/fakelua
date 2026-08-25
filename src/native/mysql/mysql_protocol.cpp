#include "native/mysql/mysql_protocol.h"

#include <algorithm>
#include <cstdio>
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
    if (len > 0 && !payload) protocol_error("make_packet: null payload");
    std::string pkt;
    size_t off = 0;
    uint8_t s = seq;
    uint32_t chunk;
    do {
        size_t remaining = len - off;
        chunk = remaining > MAX_PACKET_SIZE ? MAX_PACKET_SIZE : static_cast<uint32_t>(remaining);
        pkt.reserve(pkt.size() + 4 + chunk);
        write_uint24(pkt, chunk);
        pkt.push_back(static_cast<char>(s++));
        if (chunk > 0) {
            pkt.append(payload + off, chunk);
        }
        off += chunk;
    } while (chunk == MAX_PACKET_SIZE);
    return pkt;
}

bool consume_logical_packet(const uint8_t *buf, size_t buf_len, size_t &consumed,
                            std::vector<uint8_t> &out_payload, uint8_t &seq) {
    if (!buf) return false;
    size_t offset = 0;
    out_payload.clear();
    for (;;) {
        if (buf_len < offset + 4) return false;
        uint32_t payload_len = static_cast<uint32_t>(buf[offset]) |
                               (static_cast<uint32_t>(buf[offset + 1]) << 8) |
                               (static_cast<uint32_t>(buf[offset + 2]) << 16);
        uint8_t pkt_seq = buf[offset + 3];
        if (buf_len < offset + 4 + payload_len) return false;
        out_payload.insert(out_payload.end(), buf + offset + 4, buf + offset + 4 + payload_len);
        offset += 4 + payload_len;
        seq = pkt_seq;
        if (payload_len < MAX_PACKET_SIZE) {
            consumed = offset;
            return true;
        }
    }
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

    // auth_plugin_data_part_1 (8 bytes) + filler
    ensure(payload, pos, 9);
    info.scramble_part1.assign(payload.data() + pos, 8);
    pos += 9;

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

    std::string scramble = info.scramble_part1 + info.scramble_part2;
    if (password.empty()) {
        if (caps & CLIENT_PLUGIN_AUTH_LENENC) {
            write_lenenc_str(payload, "", 0);
        } else {
            payload.push_back('\0');
        }
    } else {
        auto hash = native_password_hash(password, scramble);
        std::vector<uint8_t> auth(hash.begin(), hash.end());
        if (caps & CLIENT_PLUGIN_AUTH_LENENC) {
            write_lenenc_str(payload, reinterpret_cast<const char *>(auth.data()), auth.size());
        } else {
            payload.push_back(static_cast<char>(auth.size()));
            payload.append(reinterpret_cast<const char *>(auth.data()), auth.size());
        }
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
    if (pos >= payload.size() || static_cast<uint8_t>(payload[pos]) != PACKET_ERR) {
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

std::string build_stmt_execute(uint32_t statement_id, const std::vector<StmtParam> &params) {
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

    // null bitmap: ceil(num_params / 8) bytes；bit i 对应第 i 个参数（不是结果行的 +2）
    size_t bitmap_off = payload.size();
    size_t bitmap_size = (params.size() + 7) / 8;
    payload.append(bitmap_size, '\0');
    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i].is_null) {
            payload[bitmap_off + i / 8] = static_cast<char>(
                static_cast<uint8_t>(payload[bitmap_off + i / 8]) | (1u << (i % 8)));
        }
    }

    // new_params_bind_flag: 1 = bind types
    payload.push_back(0x01);

    // parameter types: 2 bytes each
    // Use MYSQL_TYPE_STRING (0xfe) for string parameters as required by MySQL 8.0
    for (size_t i = 0; i < params.size(); ++i) {
        write_uint16(payload, 0xfe);  // MYSQL_TYPE_STRING
    }

    // parameter values: length-encoded strings（NULL 参数不写值）
    for (const auto &p : params) {
        if (p.is_null) continue;
        write_lenenc_str(payload, p.value.data(), p.value.size());
    }

    return payload;
}

std::string build_stmt_execute(uint32_t statement_id, const std::vector<std::string> &params) {
    std::vector<StmtParam> typed;
    typed.reserve(params.size());
    for (const auto &p : params) {
        typed.push_back(StmtParam{false, p});
    }
    return build_stmt_execute(statement_id, typed);
}

static std::string binary_int_to_string(uint64_t v, bool is_signed) {
    if (is_signed) return std::to_string(static_cast<int64_t>(v));
    return std::to_string(v);
}

static bool col_unsigned(const std::vector<uint16_t> &flags, size_t col) {
    return col < flags.size() && (flags[col] & UNSIGNED_FLAG) != 0;
}

static std::string parse_binary_time(const std::vector<char> &payload, size_t &pos) {
    uint8_t len = read_uint8(payload, pos);
    if (len == 0) return "00:00:00";
    uint8_t neg = read_uint8(payload, pos);
    uint32_t days = read_uint32(payload, pos);
    uint8_t hour = read_uint8(payload, pos);
    uint8_t min = read_uint8(payload, pos);
    uint8_t sec = read_uint8(payload, pos);
    uint32_t micro = 0;
    if (len >= 12) micro = read_uint32(payload, pos);
    int64_t total_hour = static_cast<int64_t>(days) * 24 + hour;
    char buf[40];
    if (micro) {
        std::snprintf(buf, sizeof(buf), "%s%lld:%02u:%02u.%06u",
                      neg ? "-" : "", static_cast<long long>(total_hour), min, sec, micro);
    } else {
        std::snprintf(buf, sizeof(buf), "%s%lld:%02u:%02u",
                      neg ? "-" : "", static_cast<long long>(total_hour), min, sec);
    }
    return buf;
}

static std::string parse_binary_datetime(const std::vector<char> &payload, size_t &pos, bool date_only) {
    uint8_t len = read_uint8(payload, pos);
    if (len == 0) return date_only ? "0000-00-00" : "0000-00-00 00:00:00";
    uint16_t year = read_uint16(payload, pos);
    uint8_t month = read_uint8(payload, pos);
    uint8_t day = read_uint8(payload, pos);
    if (date_only || len == 4) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u", year, month, day);
        return buf;
    }
    uint8_t hour = 0, min = 0, sec = 0;
    uint32_t micro = 0;
    if (len >= 7) {
        hour = read_uint8(payload, pos);
        min = read_uint8(payload, pos);
        sec = read_uint8(payload, pos);
    }
    if (len >= 11) micro = read_uint32(payload, pos);
    char buf[40];
    if (micro) {
        std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u.%06u",
                      year, month, day, hour, min, sec, micro);
    } else {
        std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
                      year, month, day, hour, min, sec);
    }
    return buf;
}

std::vector<std::pair<bool, std::string>> parse_binary_row(
    const std::vector<char> &payload, const std::vector<ColType> &types,
    const std::vector<uint16_t> &flags) {
    std::vector<std::pair<bool, std::string>> row;
    size_t num_columns = types.size();
    row.reserve(num_columns);
    if (payload.empty() || num_columns == 0) return row;

    size_t pos = 0;
    uint8_t header = read_uint8(payload, pos);
    if (header != 0x00) return row;

    size_t bitmap_size = (num_columns + 7 + 2) / 8;
    if (pos + bitmap_size > payload.size()) return row;
    std::vector<uint8_t> bitmap(bitmap_size);
    for (size_t i = 0; i < bitmap_size; ++i) {
        bitmap[i] = static_cast<uint8_t>(payload[pos + i]);
    }
    pos += bitmap_size;

    auto is_null = [&](size_t col) {
        size_t bit_pos = col + 2;
        return (bitmap[bit_pos / 8] & (1u << (bit_pos % 8))) != 0;
    };

    for (size_t col = 0; col < num_columns; ++col) {
        if (is_null(col)) {
            row.emplace_back(true, "");
            continue;
        }
        ColType t = types[col];
        bool uns = col_unsigned(flags, col);
        switch (t) {
        case MYSQL_TYPE_TINY: {
            uint8_t v = read_uint8(payload, pos);
            if (uns) row.emplace_back(false, std::to_string(static_cast<unsigned>(v)));
            else row.emplace_back(false, std::to_string(static_cast<int>(static_cast<int8_t>(v))));
            break;
        }
        case MYSQL_TYPE_SHORT: {
            uint16_t v = read_uint16(payload, pos);
            if (uns) row.emplace_back(false, std::to_string(v));
            else row.emplace_back(false, std::to_string(static_cast<int>(static_cast<int16_t>(v))));
            break;
        }
        case MYSQL_TYPE_YEAR: {
            uint16_t v = read_uint16(payload, pos);
            row.emplace_back(false, std::to_string(v));
            break;
        }
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_INT24: {
            uint32_t v = read_uint32(payload, pos);
            if (uns) row.emplace_back(false, std::to_string(v));
            else row.emplace_back(false, std::to_string(static_cast<int32_t>(v)));
            break;
        }
        case MYSQL_TYPE_LONGLONG: {
            uint64_t v = read_uint64(payload, pos);
            row.emplace_back(false, binary_int_to_string(v, !uns));
            break;
        }
        case MYSQL_TYPE_FLOAT: {
            uint32_t bits = read_uint32(payload, pos);
            float f;
            std::memcpy(&f, &bits, 4);
            row.emplace_back(false, std::to_string(f));
            break;
        }
        case MYSQL_TYPE_DOUBLE: {
            uint64_t bits = read_uint64(payload, pos);
            double d;
            std::memcpy(&d, &bits, 8);
            row.emplace_back(false, std::to_string(d));
            break;
        }
        case MYSQL_TYPE_DATE:
            row.emplace_back(false, parse_binary_datetime(payload, pos, true));
            break;
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
            row.emplace_back(false, parse_binary_datetime(payload, pos, false));
            break;
        case MYSQL_TYPE_TIME:
            row.emplace_back(false, parse_binary_time(payload, pos));
            break;
        default:
            row.emplace_back(false, read_lenenc_str(payload, pos));
            break;
        }
    }
    return row;
}

std::vector<std::pair<bool, std::string>> parse_binary_row(const std::vector<char> &payload,
                                                           size_t num_columns) {
    std::vector<ColType> types(num_columns, MYSQL_TYPE_VAR_STRING);
    return parse_binary_row(payload, types);
}

}  // namespace fakelua::mysql
