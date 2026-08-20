#include "native/mysql/native_mysql.h"
#include "native/mysql/mysql_connection.h"
#include "native/mysql/mysql_result.h"
#include "native/mysql/mysql_protocol.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "var/var.h"

#include <cstring>
#include <memory>
#include <string>

namespace fakelua::mysql {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

[[noreturn]] static void error(const std::string &msg) {
    ThrowFakeluaException("mysql: " + msg);
}

// Extract string from CVar (handles both String and StringId)
static std::string cvar_to_string(CVar v) {
    if (v.type_ == static_cast<int>(VarType::String) && v.data_.s) {
        return std::string(v.data_.s->Str());
    }
    if (v.type_ == static_cast<int>(VarType::StringId) && v.data_.i) {
        const char *ptr = reinterpret_cast<const char *>(v.data_.i);
        int sz = *reinterpret_cast<const int *>(ptr);
        return std::string(ptr + 8, sz);
    }
    return {};
}

// Retrieve MysqlConnection* from NativeObject (stored as int64 __mysql_conn__)
static MysqlConnection *unwrap_conn(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<MysqlConnection *>(self->GetInt("__mysql_conn__", 0));
}

// ─────────────────────────────────────────────────────────────────────────────
// Convert MysqlResult to Lua table
// ─────────────────────────────────────────────────────────────────────────────

static CVar result_to_lua(State *s, const MysqlResult &result) {
    CVar tbl = table::TableHelper::CreateTable(s);

    if (result.is_result_set) {
        // Set is_result_set = true
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, true));

        // Build columns array (1-based)
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
        // columns at key 2
        table::TableHelper::SetTableInt(s, tbl, 2, cols_tbl);

        // Build rows array (1-based)
        CVar rows_tbl = table::TableHelper::CreateTable(s);
        for (size_t i = 0; i < result.rows.size(); ++i) {
            const auto &row = result.rows[i];
            CVar row_tbl = table::TableHelper::CreateTable(s);
            for (size_t j = 0; j < row.size(); ++j) {
                if (row[j].first) {
                    // NULL → nil
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
        // rows at key 3
        table::TableHelper::SetTableInt(s, tbl, 3, rows_tbl);

    } else {
        // Status mode: is_result_set = false, affected_rows, last_insert_id, info
        table::TableHelper::SetTableInt(s, tbl, 1, inter::NativeToFakeluaBool(s, false));
        table::TableHelper::SetTableInt(s, tbl, 4,
            inter::NativeToFakeluaInt(s, static_cast<int64_t>(result.affected_rows)));
        table::TableHelper::SetTableInt(s, tbl, 5,
            inter::NativeToFakeluaInt(s, static_cast<int64_t>(result.last_insert_id)));
        table::TableHelper::SetTableInt(s, tbl, 6,
            inter::NativeToFakeluaString(s, result.info));
    }

    return tbl;
}

// ─────────────────────────────────────────────────────────────────────────────
// Forward declarations (used as NativeObject methods)
// ─────────────────────────────────────────────────────────────────────────────

static CVar conn_query(NativeObject *self, State *s, CVar *args, int n);
static CVar conn_close(NativeObject *self, State *s, CVar *args, int n);

// ─────────────────────────────────────────────────────────────────────────────
// mysql.connect(config) → connection object
// ─────────────────────────────────────────────────────────────────────────────

static CVar mysql_connect(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "mysql.connect", "config table expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);

    // Read config fields from the table
    std::string host = "127.0.0.1";
    uint16_t port = 3306;
    std::string user;
    std::string password;
    std::string database;

    if (a0.type_ == static_cast<int>(VarType::Table) && a0.data_.t) {
        // Read config fields via table helper (string-keyed lookup)
        CVar host_var = table::TableHelper::GetTableStrId(s, a0, "host");
        if (host_var.type_ != static_cast<int>(VarType::Nil)) host = cvar_to_string(host_var);

        CVar port_var = table::TableHelper::GetTableStrId(s, a0, "port");
        if (port_var.type_ != static_cast<int>(VarType::Nil)) {
            port = static_cast<uint16_t>(inter::CVarToInteger(port_var, 3306));
        }

        CVar user_var = table::TableHelper::GetTableStrId(s, a0, "user");
        if (user_var.type_ != static_cast<int>(VarType::Nil)) user = cvar_to_string(user_var);

        CVar pass_var = table::TableHelper::GetTableStrId(s, a0, "password");
        if (pass_var.type_ != static_cast<int>(VarType::Nil)) password = cvar_to_string(pass_var);

        CVar db_var = table::TableHelper::GetTableStrId(s, a0, "db");
        if (db_var.type_ != static_cast<int>(VarType::Nil)) database = cvar_to_string(db_var);
    } else {
        ThrowBadArgument(1, "mysql.connect", "config must be a table");
    }

    if (user.empty()) ThrowBadArgument(1, "mysql.connect", "user required");

    // Create connection
    auto *conn = std::make_unique<MysqlConnection>().release();
    try {
        conn->connect(host, port, user, password, database);
    } catch (const std::exception &e) {
        delete conn;
        error(std::format("connect failed: {}", e.what()));
    }

    // Wrap in NativeObject
    auto *nat = NativeObjectManager::Instance().Create(0, "mysql_connection");
    nat->SetInt("__mysql_conn__", reinterpret_cast<int64_t>(conn));
    nat->SetFinalizer([](NativeObject *self) {
        auto *c = unwrap_conn(self);
        if (c) {
            delete c;
            self->SetInt("__mysql_conn__", 0);
        }
    });
    nat->RegisterMethod("query", conn_query);
    nat->RegisterMethod("close", conn_close);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:query(sql) → result table
// ─────────────────────────────────────────────────────────────────────────────

static CVar conn_query(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "conn:query", "sql expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string sql = cvar_to_string(a0);

    auto *conn = unwrap_conn(self);
    if (!conn || !conn->connected()) error("conn:query: connection is closed");

    try {
        MysqlResult result = conn->query(sql);
        return result_to_lua(s, result);
    } catch (const std::exception &e) {
        error(std::format("query failed: {}", e.what()));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// conn:close()
// ─────────────────────────────────────────────────────────────────────────────

static CVar conn_close(NativeObject *self, State *s, CVar *args, int n) {
    auto *conn = unwrap_conn(self);
    if (conn) {
        conn->close();
        delete conn;
        self->SetInt("__mysql_conn__", 0);
    }
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// Registration
// ─────────────────────────────────────────────────────────────────────────────

void RegisterMysqlLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "mysql.connect", 1, false, mysql_connect);
    // conn:query and conn:close are registered per-connection as NativeObject methods.
}

}  // namespace fakelua::mysql
