#include "native/sqlite/native_sqlite.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "native/object/native_object.h"
#include "var/var_table.h"

#include <sqlite3.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace fakelua::sqlite {

// ── Helpers ──

static sqlite3 *unwrap_db(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<sqlite3 *>(self->GetInt("__sqlite_db__", 0));
}

[[noreturn]] static void error(const std::string &msg) {
    ThrowFakeluaException("sqlite: " + msg);
}

// ── db:exec(sql) → table or nil ──
// Uses sqlite3_prepare_v2 so we can distinguish empty SELECT from non-SELECT.

CVar db_exec(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "db:exec", "sql expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string sql = inter::FakeluaToNativeString(s, a0);

    auto *db = unwrap_db(self);
    if (!db) error("db:exec: database is closed");

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char *err = sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        error("db:exec: " + std::string(err ? err : "unknown error"));
    }

    int col_count = sqlite3_column_count(stmt);
    CVar tbl = table::TableHelper::CreateTable(s);

    if (col_count > 0) {
        // Store column names
        std::vector<std::string> col_names;
        col_names.reserve(col_count);
        for (int i = 0; i < col_count; i++) {
            const char *name = sqlite3_column_name(stmt, i);
            col_names.push_back(name ? name : "");
        }

        // Fetch rows
        int64_t row_idx = 1;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            CVar row_tbl = table::TableHelper::CreateTable(s);
            for (int i = 0; i < col_count; i++) {
                CVar val;
                int type = sqlite3_column_type(stmt, i);
                switch (type) {
                case SQLITE_INTEGER:
                    val = inter::NativeToFakeluaInt(s, sqlite3_column_int64(stmt, i));
                    break;
                case SQLITE_FLOAT:
                    val = inter::NativeToFakeluaFloat(s, sqlite3_column_double(stmt, i));
                    break;
                case SQLITE_TEXT:
                    val = inter::NativeToFakeluaString(s, reinterpret_cast<const char *>(sqlite3_column_text(stmt, i)));
                    break;
                case SQLITE_BLOB:
                    val = inter::NativeToFakeluaString(s, "blob");
                    break;
                case SQLITE_NULL:
                default:
                    val = inter::NativeToFakeluaNil(s);
                    break;
                }
                table::TableHelper::SetTableStrId(s, row_tbl, col_names[i].c_str(), val);
            }
            table::TableHelper::SetTableInt(s, tbl, row_idx++, row_tbl);
        }
    } else {
        // Non-SELECT: execute and check for errors
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            const char *err = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            error("db:exec: " + std::string(err ? err : "unknown error"));
        }
    }

    sqlite3_finalize(stmt);

    if (col_count > 0) {
        return tbl;
    }
    return inter::NativeToFakeluaNil(s);
}

// ── db:close() ──

CVar db_close(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *db = unwrap_db(self);
    if (db) {
        sqlite3_close(db);
        self->SetInt("__sqlite_db__", 0);
    }
    return inter::NativeToFakeluaNil(s);
}

// ── sqlite.open(filename) → db object ──

static CVar sqlite_open(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "sqlite.open", "filename expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string filename = inter::FakeluaToNativeString(s, a0);

    // Ensure SQLite is initialized (needed on some platforms)
    static bool initialized = []() {
        return sqlite3_initialize() == SQLITE_OK;
    }();
    (void)initialized;

    sqlite3 *db = nullptr;
    int rc = sqlite3_open_v2(filename.c_str(), &db,
                              SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        if (db) sqlite3_close(db);
        error("sqlite.open: cannot open database: " + std::string(sqlite3_errmsg(db)));
    }

    // Create NativeObject wrapper
    int64_t gid = NativeObjectManager::Instance().CreateGroup();
    auto *nat = NativeObjectManager::Instance().Create(gid, "sqlite_db");
    nat->SetFinalizer([](NativeObject *self) {
        auto *db = unwrap_db(self);
        if (db) {
            sqlite3_close(db);
            self->SetInt("__sqlite_db__", 0);
        }
    });
    nat->RegisterMethod("exec", db_exec);
    nat->RegisterMethod("close", db_close);
    nat->SetInt("__sqlite_db__", reinterpret_cast<int64_t>(db));

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// ── Registration ──

void RegisterSqliteLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "sqlite.open", 1, false, sqlite_open);
}

}  // namespace fakelua::sqlite
