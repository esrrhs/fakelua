#include "native/mysql/mysql_result.h"
#include "native/table/native_table.h"
#include "native/native_common.h"
#include "var/var.h"

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

// ─────────────────────────────────────────────────────────────────────────────
// Convert MysqlResult to Lua table
// ─────────────────────────────────────────────────────────────────────────────

CVar result_to_lua(::fakelua::State *s, const MysqlResult &result) {
    CVar tbl = table::TableHelper::CreateTable(s);

    if (result.is_result_set) {
        // is_result_set = true at key 1
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, true));

        // columns at key 2
        CVar cols_tbl = table::TableHelper::CreateTable(s);
        for (size_t i = 0; i < result.columns.size(); ++i) {
            const auto &col = result.columns[i];
            CVar col_tbl = table::TableHelper::CreateTable(s);
            table::TableHelper::SetTableInt(s, col_tbl, 1,
                inter::NativeToFakeluaString(s, col.name));
            table::TableHelper::SetTableInt(s, col_tbl, 2,
                inter::NativeToFakeluaInt(s, static_cast<int64_t>(col.type)));
            table::TableHelper::SetTableInt(s, cols_tbl,
                static_cast<int64_t>(i + 1), col_tbl);
        }
        table::TableHelper::SetTableInt(s, tbl, 2, cols_tbl);

        // rows at key 3
        CVar rows_tbl = table::TableHelper::CreateTable(s);
        for (size_t i = 0; i < result.rows.size(); ++i) {
            const auto &row = result.rows[i];
            CVar row_tbl = table::TableHelper::CreateTable(s);
            for (size_t j = 0; j < row.size(); ++j) {
                if (row[j].first) {
                    table::TableHelper::SetTableInt(s, row_tbl,
                        static_cast<int64_t>(j + 1), inter::NativeToFakeluaNil(s));
                } else {
                    table::TableHelper::SetTableInt(s, row_tbl,
                        static_cast<int64_t>(j + 1),
                        inter::NativeToFakeluaString(s, row[j].second));
                }
            }
            table::TableHelper::SetTableInt(s, rows_tbl,
                static_cast<int64_t>(i + 1), row_tbl);
        }
        table::TableHelper::SetTableInt(s, tbl, 3, rows_tbl);
    } else {
        // Status mode
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, false));
        table::TableHelper::SetTableInt(s, tbl, 4,
            inter::NativeToFakeluaLonglong(s, static_cast<long long>(result.affected_rows)));
        table::TableHelper::SetTableInt(s, tbl, 5,
            inter::NativeToFakeluaLonglong(s, static_cast<long long>(result.last_insert_id)));
        table::TableHelper::SetTableInt(s, tbl, 6,
            inter::NativeToFakeluaString(s, result.info));
    }

    return tbl;
}

}  // namespace fakelua::mysql
