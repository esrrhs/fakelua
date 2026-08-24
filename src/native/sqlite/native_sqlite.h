#pragma once

#include "state/state.h"

namespace fakelua::sqlite {

// Register SQLite library:
//   sqlite.open(filename) → db object
//   db:exec(sql) → result table or nil
//   db:prepare(sql) → stmt object
//   db:last_insert_rowid() → int64
//   db:changes() → int64
//   db:close()
//   stmt:bind(...) — bind positional parameters (nil/int/float/string/bool)
//   stmt:step() → row table or nil
//   stmt:reset() — reset for re-execution
//   stmt:columns() → column names table
//   stmt:close()
void RegisterSqliteLibraryApi(State *s);

}  // namespace fakelua::sqlite
