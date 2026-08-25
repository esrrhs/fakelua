#pragma once

#include "state/state.h"

namespace fakelua {
class NativeObject;
}

namespace fakelua::mysql {

class MysqlConnection;

// Register MySQL library: mysql.connect(...) returns a connection object.
// Connection methods: :query(sql), :close().
void RegisterMysqlLibraryApi(State *s);

// Register MySQL pool library: mysql_pool.create(config) returns a pool object.
// Pool methods: :acquire(), :release(conn), :tick(), :close(), :stats().
void RegisterMysqlPoolApi(State *s);

// Shared connection methods (used by both direct connect and pool)
CVar conn_query(NativeObject *self, State *s, CVar *args, int n);
CVar conn_stmt_prepare(NativeObject *self, State *s, CVar *args, int n);
CVar conn_stmt_execute(NativeObject *self, State *s, CVar *args, int n);
CVar conn_stmt_close(NativeObject *self, State *s, CVar *args, int n);
CVar conn_tick(NativeObject *self, State *s, CVar *args, int n);
MysqlConnection *unwrap_conn_native(NativeObject *self);

// Per-State NativeObject registry so FakeluaDeleteState can close sockets.
void RegisterMysqlNativeWrapper(State *s, NativeObject *nat, bool is_pool);
void UnregisterMysqlNativeWrapper(NativeObject *nat);
void OnStateDeleted(State *s);

}  // namespace fakelua::mysql
