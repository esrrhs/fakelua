#pragma once

#include "state/state.h"

namespace fakelua::mysql {

// Register MySQL library: mysql.connect(...) returns a connection object.
// Connection methods: :query(sql), :close().
void RegisterMysqlLibraryApi(State *s);

}  // namespace fakelua::mysql
