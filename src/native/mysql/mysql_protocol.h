#pragma once

// mysql_protocol.h — MySQL wire protocol primitives (zero external dependencies)
// Implements packet framing, length-encoded types, SHA1, and the
// mysql_native_password handshake. Self-contained: no OpenSSL, no libmysqlclient.

#include "native/crypto/hash.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::mysql {

// ─────────────────────────────────────────────────────────────────────────────
// Packet type bytes
// ─────────────────────────────────────────────────────────────────────────────

static constexpr uint8_t PACKET_OK  = 0x00;
static constexpr uint8_t PACKET_EOF = 0xFE;
static constexpr uint8_t PACKET_ERR = 0xFF;
static constexpr uint8_t COM_QUERY  = 0x03;
static constexpr uint8_t COM_QUIT   = 0x01;
static constexpr uint32_t MAX_PACKET_SIZE = 0xFFFFFF;  // 16MB - 1

// ─────────────────────────────────────────────────────────────────────────────
// Capability flags (from MySQL mysql_com.h — authoritative values)
// ─────────────────────────────────────────────────────────────────────────────

enum Capability : uint32_t {
    CLIENT_LONG_PASSWORD      = 0x00000001,
    CLIENT_FOUND_ROWS         = 0x00000002,
    CLIENT_LONG_FLAG          = 0x00000004,
    CLIENT_CONNECT_WITH_DB    = 0x00000008,
    CLIENT_NO_SCHEMA          = 0x00000010,
    CLIENT_COMPRESS           = 0x00000020,
    CLIENT_ODBC               = 0x00000040,
    CLIENT_LOCAL_FILES        = 0x00000080,
    CLIENT_IGNORE_SPACE       = 0x00000100,
    CLIENT_PROTOCOL_41        = 0x00000200,
    CLIENT_INTERACTIVE        = 0x00000400,
    CLIENT_SSL                = 0x00000800,
    CLIENT_IGNORE_SIGPIPE     = 0x00001000,
    CLIENT_TRANSACTIONS       = 0x00002000,
    CLIENT_RESERVED           = 0x00004000,
    CLIENT_SECURE_CONNECTION  = 0x00008000,
    CLIENT_MULTI_STATEMENTS   = 0x00010000,
    CLIENT_MULTI_RESULTS      = 0x00020000,
    CLIENT_PS_MULTI_RESULTS   = 0x00040000,
    CLIENT_PLUGIN_AUTH        = 0x00080000,
    CLIENT_CONNECT_ATTRS      = 0x00100000,
    CLIENT_PLUGIN_AUTH_LENENC = 0x00200000,
    CLIENT_CAN_HANDLE_EXPIRED = 0x00400000,
    CLIENT_SESSION_TRACK      = 0x00800000,
    CLIENT_DEPRECATE_EOF      = 0x01000000,
};

// Our declared capability set (no SSL — internal network only)
constexpr uint32_t kMyCapabilities =
    CLIENT_PROTOCOL_41 | CLIENT_SECURE_CONNECTION | CLIENT_PLUGIN_AUTH |
    CLIENT_CONNECT_WITH_DB | CLIENT_LONG_PASSWORD | CLIENT_TRANSACTIONS |
    CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS;

// ─────────────────────────────────────────────────────────────────────────────
// Column type codes (text protocol returns all values as strings; these are
// used only for column metadata)
// ─────────────────────────────────────────────────────────────────────────────

enum ColType : uint8_t {
    MYSQL_TYPE_DECIMAL     = 0,
    MYSQL_TYPE_TINY        = 1,
    MYSQL_TYPE_SHORT       = 2,
    MYSQL_TYPE_LONG        = 3,
    MYSQL_TYPE_FLOAT       = 4,
    MYSQL_TYPE_DOUBLE      = 5,
    MYSQL_TYPE_NULL        = 6,
    MYSQL_TYPE_TIMESTAMP   = 7,
    MYSQL_TYPE_LONGLONG    = 8,
    MYSQL_TYPE_INT24       = 9,
    MYSQL_TYPE_DATE        = 10,
    MYSQL_TYPE_TIME        = 11,
    MYSQL_TYPE_DATETIME    = 12,
    MYSQL_TYPE_YEAR        = 13,
    MYSQL_TYPE_VARCHAR     = 15,
    MYSQL_TYPE_BIT         = 16,
    MYSQL_TYPE_JSON        = 152,
    MYSQL_TYPE_NEWDECIMAL  = 246,
    MYSQL_TYPE_ENUM        = 247,
    MYSQL_TYPE_SET         = 248,
    MYSQL_TYPE_TINY_BLOB   = 249,
    MYSQL_TYPE_MEDIUM_BLOB = 250,
    MYSQL_TYPE_LONG_BLOB   = 251,
    MYSQL_TYPE_BLOB        = 252,
    MYSQL_TYPE_VAR_STRING  = 253,
    MYSQL_TYPE_STRING      = 254,
    MYSQL_TYPE_GEOMETRY    = 255,
};

// ─────────────────────────────────────────────────────────────────────────────
// Read primitives (little-endian)
// ─────────────────────────────────────────────────────────────────────────────

bool read_bytes(const std::vector<char> &buf, size_t &pos, char *out, size_t len);

uint8_t  read_uint8 (const std::vector<char> &buf, size_t &pos);
uint16_t read_uint16(const std::vector<char> &buf, size_t &pos);
uint32_t read_uint24(const std::vector<char> &buf, size_t &pos);
uint32_t read_uint32(const std::vector<char> &buf, size_t &pos);
uint64_t read_uint64(const std::vector<char> &buf, size_t &pos);

// ─────────────────────────────────────────────────────────────────────────────
// Write primitives (little-endian)
// ─────────────────────────────────────────────────────────────────────────────

void write_uint16(std::string &out, uint16_t v);
void write_uint24(std::string &out, uint32_t v);
void write_uint32(std::string &out, uint32_t v);
void write_uint64(std::string &out, uint64_t v);

// ─────────────────────────────────────────────────────────────────────────────
// Length-encoded integer / string
// ─────────────────────────────────────────────────────────────────────────────

uint64_t read_lenenc_int(const std::vector<char> &buf, size_t &pos);
void     write_lenenc_int(std::string &out, uint64_t v);

std::string read_lenenc_str(const std::vector<char> &buf, size_t &pos);
void        write_lenenc_str(std::string &out, const char *data, size_t len);

std::string read_nul_str(const std::vector<char> &buf, size_t &pos);

// ─────────────────────────────────────────────────────────────────────────────
// Packet framing
// ─────────────────────────────────────────────────────────────────────────────

// Build a MySQL packet: 3-byte LE length + 1-byte sequence + payload
std::string make_packet(uint8_t seq, const char *payload, size_t len);

// ─────────────────────────────────────────────────────────────────────────────
// Authentication (mysql_native_password + caching_sha2_password)
// ─────────────────────────────────────────────────────────────────────────────

// mysql_native_password: XOR(SHA1(pwd), SHA1(scramble + SHA1(SHA1(pwd))))
// scramble = 20 bytes from server auth_plugin_data; returns 20-byte response.
std::array<uint8_t, 20> native_password_hash(const std::string &password, const std::string &scramble);

// caching_sha2_password: XOR(SHA256(pwd), SHA256(scramble + SHA256(SHA256(pwd))))
// scramble = 20 bytes; returns 32-byte response. Used by MySQL 8+.
std::vector<uint8_t> caching_sha2_password_hash(const std::string &password, const std::string &scramble);

// ─────────────────────────────────────────────────────────────────────────────
// Prepared statements (binary protocol)
// ─────────────────────────────────────────────────────────────────────────────

static constexpr uint8_t COM_STMT_PREPARE  = 0x16;
static constexpr uint8_t COM_STMT_EXECUTE  = 0x17;
static constexpr uint8_t COM_STMT_CLOSE    = 0x19;
static constexpr uint8_t COM_STMT_RESET    = 0x1A;
static constexpr uint8_t COM_STMT_FETCH    = 0x1C;

// Parse COM_STMT_PREPARE response
struct PrepareResult {
    uint32_t statement_id = 0;
    uint16_t num_columns = 0;
    uint16_t num_params = 0;
    uint16_t num_warnings = 0;
    bool valid = false;
};
PrepareResult parse_prepare_response(const std::vector<char> &payload);

// Build COM_STMT_EXECUTE packet payload
std::string build_stmt_execute(uint32_t statement_id,
                              const std::vector<std::string> &params);

// Parse binary result set row (prepared statement execute result)
std::vector<std::pair<bool, std::string>> parse_binary_row(const std::vector<char> &payload,
                                                           size_t num_columns);

// ─────────────────────────────────────────────────────────────────────────────
// Handshake parsing
// ─────────────────────────────────────────────────────────────────────────────

struct HandshakeInfo {
    std::string server_version;
    uint32_t connection_id = 0;
    std::string scramble_part1;   // 8 bytes
    uint16_t capabilities_low = 0;
    uint8_t charset = 0;
    uint16_t status = 0;
    uint16_t capabilities_high = 0;
    uint8_t scramble_len = 0;
    std::string scramble_part2;   // 12 bytes (total scramble = part1 + part2 = 20)
    std::string auth_plugin_name;
    uint32_t capabilities = 0;    // full 32-bit capability set
};

// Parse server handshake packet payload (without 4-byte header)
HandshakeInfo parse_handshake(const std::vector<char> &payload);

// Build client handshake response payload (without 4-byte header)
std::string build_handshake_response(const HandshakeInfo &info,
                                     const std::string &username,
                                     const std::string &password,
                                     const std::string &database);

// ─────────────────────────────────────────────────────────────────────────────
// Response packet parsing
// ─────────────────────────────────────────────────────────────────────────────

struct OkResponse {
    uint64_t affected_rows = 0;
    uint64_t last_insert_id = 0;
    uint16_t status_flags = 0;
    uint16_t warnings = 0;
    std::string info;
};

struct ErrResponse {
    uint16_t error_code = 0;
    std::string sql_state;   // 5 bytes
    std::string message;
};

enum class ResponseType { Ok, Err, Eof, ResultSet };

ResponseType peek_type(uint8_t first_byte);

OkResponse  parse_ok(const std::vector<char> &payload);
ErrResponse parse_err(const std::vector<char> &payload);

// Parse EOF payload: warnings(2) + status(2). Returns true on success.
bool parse_eof(const std::vector<char> &payload, uint16_t &warnings, uint16_t &status);

}  // namespace fakelua::mysql
