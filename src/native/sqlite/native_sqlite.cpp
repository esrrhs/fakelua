#include "native/sqlite/native_sqlite.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "native/object/native_object.h"
#include "var/var_table.h"

#include <sqlite3.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fakelua::sqlite {

// ── Object structs ──

struct DbObject {
    sqlite3 *db = nullptr;
};

struct StmtObject {
    sqlite3 *db = nullptr;       // for error messages
    sqlite3_stmt *stmt = nullptr;
};

// ── Helpers ──

static DbObject *unwrap_db(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<DbObject *>(self->GetInt("__sqlite_db__", 0));
}

static StmtObject *unwrap_stmt(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<StmtObject *>(self->GetInt("__sqlite_stmt__", 0));
}

static std::unordered_map<State *, std::vector<NativeObject *>> g_sqlite_wrappers;

static void register_sqlite_wrapper(State *s, NativeObject *nat) {
    if (!s || !nat) return;
    nat->SetInt("__sqlite_state__", reinterpret_cast<int64_t>(s));
    g_sqlite_wrappers[s].push_back(nat);
}

static void unregister_sqlite_wrapper(NativeObject *nat) {
    if (!nat) return;
    auto *st = reinterpret_cast<State *>(nat->GetInt("__sqlite_state__", 0));
    nat->SetInt("__sqlite_state__", 0);
    if (!st) return;
    auto it = g_sqlite_wrappers.find(st);
    if (it == g_sqlite_wrappers.end()) return;
    auto &v = it->second;
    v.erase(std::remove(v.begin(), v.end(), nat), v.end());
    if (v.empty()) g_sqlite_wrappers.erase(it);
}

void OnStateDeleted(State *s) {
    if (!s) return;
    auto it = g_sqlite_wrappers.find(s);
    if (it == g_sqlite_wrappers.end()) return;
    auto wrappers = std::move(it->second);
    g_sqlite_wrappers.erase(it);
    for (auto *nat : wrappers) {
        if (!nat) continue;
        nat->SetInt("__sqlite_state__", 0);
        NativeObjectManager::Instance().DestroyGroup(nat->GetGroupId());
    }
}

[[noreturn]] static void error(const std::string &msg) {
    ThrowFakeluaException("sqlite: " + msg);
}

static CVar sqlite_column_to_cvar(State *s, sqlite3_stmt *stmt, int i) {
    switch (sqlite3_column_type(stmt, i)) {
    case SQLITE_INTEGER:
        return inter::NativeToFakeluaLonglong(s, sqlite3_column_int64(stmt, i));
    case SQLITE_FLOAT:
        return inter::NativeToFakeluaDouble(s, sqlite3_column_double(stmt, i));
    case SQLITE_TEXT: {
        const unsigned char *p = sqlite3_column_text(stmt, i);
        int n = sqlite3_column_bytes(stmt, i);
        if (!p || n <= 0) return inter::NativeToFakeluaString(s, "");
        return inter::NativeToFakeluaStringView(
            s, std::string_view(reinterpret_cast<const char *>(p), static_cast<size_t>(n)));
    }
    case SQLITE_BLOB: {
        const void *p = sqlite3_column_blob(stmt, i);
        int n = sqlite3_column_bytes(stmt, i);
        if (!p || n <= 0) return inter::NativeToFakeluaString(s, "");
        return inter::NativeToFakeluaStringView(
            s, std::string_view(static_cast<const char *>(p), static_cast<size_t>(n)));
    }
    case SQLITE_NULL:
    default:
        return inter::NativeToFakeluaNil(s);
    }
}

// ── Forward declarations for stmt methods (used in db_prepare) ──

CVar stmt_bind(NativeObject *self, State *s, CVar *args, int n);
CVar stmt_step(NativeObject *self, State *s, CVar *args, int n);
CVar stmt_reset(NativeObject *self, State *s, CVar *args, int n);
CVar stmt_columns(NativeObject *self, State *s, CVar *args, int n);
CVar stmt_close(NativeObject *self, State *s, CVar *args, int n);

// ── db:exec(sql) → table or nil ──
// Uses sqlite3_prepare_v2 so we can distinguish empty SELECT from non-SELECT.

CVar db_exec(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "db:exec", "sql expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string sql = inter::FakeluaToNativeString(s, a0);

    auto *obj = unwrap_db(self);
    if (!obj || !obj->db) error("db:exec: database is closed");

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(obj->db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char *err = sqlite3_errmsg(obj->db);
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
                CVar val = sqlite_column_to_cvar(s, stmt, i);
                table::TableHelper::SetTableStrId(s, row_tbl, col_names[i].c_str(), val);
            }
            table::TableHelper::SetTableInt(s, tbl, row_idx++, row_tbl);
        }
        if (rc != SQLITE_DONE) {
            const char *err = sqlite3_errmsg(obj->db);
            sqlite3_finalize(stmt);
            error("db:exec: " + std::string(err ? err : "unknown error"));
        }
    } else {
        // Non-SELECT: execute and check for errors
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            const char *err = sqlite3_errmsg(obj->db);
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

// ── db:prepare(sql) → stmt object ──

CVar db_prepare(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "db:prepare", "sql expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string sql = inter::FakeluaToNativeString(s, a0);

    auto *obj = unwrap_db(self);
    if (!obj || !obj->db) error("db:prepare: database is closed");

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(obj->db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char *err = sqlite3_errmsg(obj->db);
        if (stmt) sqlite3_finalize(stmt);
        error("db:prepare: " + std::string(err ? err : "unknown error"));
    }

    // Create stmt NativeObject
    int64_t gid = NativeObjectManager::Instance().CreateGroup();
    auto *nat = NativeObjectManager::Instance().Create(gid, "sqlite_stmt");
    auto *stmt_obj = new StmtObject();
    stmt_obj->db = obj->db;
    stmt_obj->stmt = stmt;
    nat->SetFinalizer([](NativeObject *self) {
        unregister_sqlite_wrapper(self);
        auto *obj = unwrap_stmt(self);
        if (obj) {
            if (obj->stmt) {
                sqlite3_finalize(obj->stmt);
            }
            delete obj;
            self->SetInt("__sqlite_stmt__", 0);
        }
    });
    nat->RegisterMethod("bind", stmt_bind);
    nat->RegisterMethod("step", stmt_step);
    nat->RegisterMethod("reset", stmt_reset);
    nat->RegisterMethod("columns", stmt_columns);
    nat->RegisterMethod("close", stmt_close);
    nat->SetInt("__sqlite_stmt__", reinterpret_cast<int64_t>(stmt_obj));
    register_sqlite_wrapper(s, nat);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// ── stmt:bind(...) — bind positional parameters ──

CVar stmt_bind(NativeObject *self, State *s, CVar *args, int n) {
    auto *obj = unwrap_stmt(self);
    if (!obj || !obj->stmt) error("stmt:bind: statement is closed");

    // Clear existing bindings
    sqlite3_clear_bindings(obj->stmt);

    // Bind each argument positionally
    for (int i = 0; i < n; i++) {
        CVar arg = inter::GetNativeArg(s, args, n, i);
        int rc;
        if (arg.type_ == static_cast<int>(VarType::Nil)) {
            rc = sqlite3_bind_null(obj->stmt, i + 1);
        } else if (arg.type_ == static_cast<int>(VarType::Int)) {
            rc = sqlite3_bind_int64(obj->stmt, i + 1, arg.data_.i);
        } else if (arg.type_ == static_cast<int>(VarType::Float)) {
            rc = sqlite3_bind_double(obj->stmt, i + 1, arg.data_.f);
        } else if (arg.type_ == static_cast<int>(VarType::Bool)) {
            rc = sqlite3_bind_int(obj->stmt, i + 1, arg.data_.b ? 1 : 0);
        } else if (arg.type_ == static_cast<int>(VarType::String) || arg.type_ == static_cast<int>(VarType::StringId)) {
            std::string str = inter::FakeluaToNativeString(s, arg);
            if (str.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
                error("stmt:bind: string too large at position " + std::to_string(i + 1));
            }
            rc = sqlite3_bind_text(obj->stmt, i + 1, str.data(), static_cast<int>(str.size()), SQLITE_TRANSIENT);
        } else {
            error("stmt:bind: unsupported type at position " + std::to_string(i + 1));
        }
        if (rc != SQLITE_OK) {
            error("stmt:bind: " + std::string(sqlite3_errmsg(obj->db)));
        }
    }

    return inter::NativeToFakeluaNil(s);
}

// ── stmt:step() → row table or nil ──

CVar stmt_step(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap_stmt(self);
    if (!obj || !obj->stmt) error("stmt:step: statement is closed");

    int rc = sqlite3_step(obj->stmt);
    if (rc == SQLITE_ROW) {
        int col_count = sqlite3_column_count(obj->stmt);
        CVar row = table::TableHelper::CreateTable(s);
        for (int i = 0; i < col_count; i++) {
            const char *name = sqlite3_column_name(obj->stmt, i);
            CVar val = sqlite_column_to_cvar(s, obj->stmt, i);
            table::TableHelper::SetTableStrId(s, row, name ? name : "", val);
        }
        return row;
    } else if (rc == SQLITE_DONE) {
        return inter::NativeToFakeluaNil(s);
    } else {
        error("stmt:step: " + std::string(sqlite3_errmsg(obj->db)));
    }
}

// ── stmt:reset() — reset for re-execution ──

CVar stmt_reset(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap_stmt(self);
    if (!obj || !obj->stmt) error("stmt:reset: statement is closed");

    int rc = sqlite3_reset(obj->stmt);
    if (rc != SQLITE_OK) {
        error("stmt:reset: " + std::string(sqlite3_errmsg(obj->db)));
    }
    return inter::NativeToFakeluaNil(s);
}

// ── stmt:columns() → column names table ──

CVar stmt_columns(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap_stmt(self);
    if (!obj || !obj->stmt) error("stmt:columns: statement is closed");

    int col_count = sqlite3_column_count(obj->stmt);
    CVar tbl = table::TableHelper::CreateTable(s);
    for (int i = 0; i < col_count; i++) {
        const char *name = sqlite3_column_name(obj->stmt, i);
        table::TableHelper::SetTableInt(s, tbl, i + 1,
            inter::NativeToFakeluaString(s, name ? name : ""));
    }
    return tbl;
}

// ── stmt:close() — finalize the statement ──

CVar stmt_close(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap_stmt(self);
    if (!obj || !obj->stmt) return inter::NativeToFakeluaNil(s);

    int rc = sqlite3_finalize(obj->stmt);
    obj->stmt = nullptr;
    obj->db = nullptr;
    delete obj;
    self->SetInt("__sqlite_stmt__", 0);
    if (rc != SQLITE_OK) {
        error(std::string("stmt:close: ") + sqlite3_errstr(rc));
    }
    return inter::NativeToFakeluaNil(s);
}

// ── db:last_insert_rowid() ──

CVar db_last_insert_rowid(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap_db(self);
    if (!obj || !obj->db) error("db:last_insert_rowid: database is closed");
    return inter::NativeToFakeluaLonglong(s, sqlite3_last_insert_rowid(obj->db));
}

// ── db:changes() ──

CVar db_changes(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap_db(self);
    if (!obj || !obj->db) error("db:changes: database is closed");
    return inter::NativeToFakeluaLonglong(s, sqlite3_changes64(obj->db));
}

// ── db:close() ──

CVar db_close(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *obj = unwrap_db(self);
    if (obj) {
        if (obj->db) {
            sqlite3_close_v2(obj->db);
            obj->db = nullptr;
        }
        delete obj;
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
    static int init_rc = sqlite3_initialize();
    if (init_rc != SQLITE_OK) {
        error(std::format("sqlite.open: sqlite3_initialize failed: {}", init_rc));
    }

    sqlite3 *db = nullptr;
    int rc = sqlite3_open_v2(filename.c_str(), &db,
                              SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        std::string err = db ? sqlite3_errmsg(db) : "unknown error";
        if (db) sqlite3_close(db);
        error("sqlite.open: cannot open database: " + err);
    }

    // Create NativeObject wrapper
    int64_t gid = NativeObjectManager::Instance().CreateGroup();
    auto *nat = NativeObjectManager::Instance().Create(gid, "sqlite_db");
    auto *db_obj = new DbObject();
    db_obj->db = db;
    nat->SetFinalizer([](NativeObject *self) {
        unregister_sqlite_wrapper(self);
        auto *obj = unwrap_db(self);
        if (obj) {
            if (obj->db) {
                sqlite3_close_v2(obj->db);
            }
            delete obj;
            self->SetInt("__sqlite_db__", 0);
        }
    });
    nat->RegisterMethod("exec", db_exec);
    nat->RegisterMethod("prepare", db_prepare);
    nat->RegisterMethod("last_insert_rowid", db_last_insert_rowid);
    nat->RegisterMethod("changes", db_changes);
    nat->RegisterMethod("close", db_close);
    nat->SetInt("__sqlite_db__", reinterpret_cast<int64_t>(db_obj));
    register_sqlite_wrapper(s, nat);

    return inter::NativeToFakeluaNativeObject(s, nat);
}

// ── Registration ──

void RegisterSqliteLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "sqlite.open", 1, false, sqlite_open);
}

}  // namespace fakelua::sqlite
