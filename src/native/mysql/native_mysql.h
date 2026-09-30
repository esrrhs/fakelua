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
// Pool methods: :acquire(), :release(conn), :close(), :stats().
void RegisterMysqlPoolApi(State *s);

// Shared connection methods (used by both direct connect and pool)
CVar ConnQuery(NativeObject *self, State *s, CVar *args, int n);
CVar ConnStmtPrepare(NativeObject *self, State *s, CVar *args, int n);
CVar ConnStmtExecute(NativeObject *self, State *s, CVar *args, int n);
CVar ConnStmtClose(NativeObject *self, State *s, CVar *args, int n);
// 单个对象的驱动，不是 Lua 可见的方法。
void TickMysqlConnection(NativeObject *self, State *s);
void TickMysqlPool(NativeObject *self);
MysqlConnection *UnwrapConnNative(NativeObject *self);

// Per-State NativeObject registry so FakeluaDeleteState can close sockets.
void RegisterMysqlNativeWrapper(State *s, NativeObject *nat, bool is_pool);
void UnregisterMysqlNativeWrapper(NativeObject *nat);
void OnStateDeleted(State *s);

// 驱动本 State 上所有连接池和连接。由 runtime.tick() 调用。
void TickAll(State *s);

// 单测：pool:with 的 fn 抛错后连接必须回到可再次 Acquire 的状态。
// 归还成功返回 1；fn 没有抛错返回 -1；归还失败（仍被占用）返回 0。
int TestPoolWithFnThrowReturnsConnection(State *s, CVar fn);

}// namespace fakelua::mysql
