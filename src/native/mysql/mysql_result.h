#pragma once

// mysql_result.h — result set structures for MySQL text protocol.

#include "native/mysql/mysql_protocol.h"
#include "var/var.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace fakelua {
class State;
}

namespace fakelua::mysql {

struct ColumnDef {
    std::string catalog;    // usually "def"
    std::string schema;
    std::string table;
    std::string org_table;
    std::string name;
    std::string org_name;
    uint16_t charset = 0;
    uint32_t length = 0;
    ColType type = MYSQL_TYPE_DECIMAL;
    uint16_t flags = 0;
    uint8_t decimals = 0;
};

struct MysqlResult {
    bool is_result_set = false;   // true = has row data, false = OK status only

    // result set mode
    std::vector<ColumnDef> columns;
    // rows: each row is a list of (is_null, value_string) pairs.
    // NULL values have is_null=true. Text protocol returns all values as strings.
    std::vector<std::vector<std::pair<bool, std::string>>> rows;

    // OK status mode (INSERT/UPDATE/DELETE or empty result)
    uint64_t affected_rows = 0;
    uint64_t last_insert_id = 0;

    // Statement ID (for COM_STMT_PREPARE response)
    uint32_t stmt_id = 0;
    uint16_t status_flags = 0;
    std::string info;
};

// Parse a column definition packet (one per column, before rows)
ColumnDef parse_column_def(const std::vector<char> &payload);

// Parse a single row packet (text protocol). Returns one (is_null, value) per column.
std::vector<std::pair<bool, std::string>> parse_row(const std::vector<char> &payload,
                                                    size_t num_columns);

// Parse an OK packet payload into a result (status mode for INSERT/UPDATE/DELETE)
MysqlResult parse_ok_to_result(const std::vector<char> &payload);

// Convert MysqlResult to Lua table (for callback dispatch)
CVar result_to_lua(::fakelua::State *s, const MysqlResult &result);

}  // namespace fakelua::mysql
