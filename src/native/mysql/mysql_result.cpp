#include "native/mysql/mysql_result.h"

#include <stdexcept>

namespace fakelua::mysql {

// ─────────────────────────────────────────────────────────────────────────────
// Parse a column definition packet
// ─────────────────────────────────────────────────────────────────────────────

ColumnDef parse_column_def(const std::vector<char> &payload) {
    ColumnDef col;
    size_t pos = 0;

    col.catalog   = read_lenenc_str(payload, pos);
    col.schema    = read_lenenc_str(payload, pos);
    col.table     = read_lenenc_str(payload, pos);
    col.org_table = read_lenenc_str(payload, pos);
    col.name      = read_lenenc_str(payload, pos);
    col.org_name  = read_lenenc_str(payload, pos);

    // filler 0x0C (1 byte) — length-of-fields marker, always 12
    if (pos < payload.size()) {
        pos += 1;  // skip filler
    }

    col.charset  = read_uint16(payload, pos);
    col.length   = read_uint32(payload, pos);
    col.type     = static_cast<ColType>(read_uint8(payload, pos));
    col.flags    = read_uint16(payload, pos);
    col.decimals = read_uint8(payload, pos);

    // 2-byte filler (0x0000) — skip if present
    if (pos + 2 <= payload.size()) {
        pos += 2;
    }

    return col;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parse a single row packet (text protocol)
// ─────────────────────────────────────────────────────────────────────────────

std::vector<std::pair<bool, std::string>> parse_row(const std::vector<char> &payload,
                                                    size_t num_columns) {
    std::vector<std::pair<bool, std::string>> row;
    row.reserve(num_columns);
    size_t pos = 0;

    for (size_t i = 0; i < num_columns; ++i) {
        if (pos >= payload.size()) {
            // Missing columns — pad with NULL
            row.emplace_back(true, "");
            continue;
        }
        uint8_t first = static_cast<uint8_t>(payload[pos]);
        if (first == 0xFB) {
            // NULL value
            pos += 1;
            row.emplace_back(true, "");
        } else {
            row.emplace_back(false, read_lenenc_str(payload, pos));
        }
    }

    return row;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parse an OK packet into a result (status mode)
// ─────────────────────────────────────────────────────────────────────────────

MysqlResult parse_ok_to_result(const std::vector<char> &payload) {
    MysqlResult result;
    result.is_result_set = false;
    if (payload.empty()) return result;

    size_t pos = 1;  // skip header byte (0x00)
    result.affected_rows = read_lenenc_int(payload, pos);
    result.last_insert_id = read_lenenc_int(payload, pos);
    result.status_flags = read_uint16(payload, pos);
    // warnings (2 bytes) — skip
    if (pos + 2 <= payload.size()) pos += 2;
    // remaining is info
    if (pos < payload.size()) {
        result.info.assign(payload.data() + pos, payload.size() - pos);
    }
    return result;
}

}  // namespace fakelua::mysql
