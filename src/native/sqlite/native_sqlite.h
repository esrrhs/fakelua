#pragma once

#include "state/state.h"

namespace fakelua::sqlite {

// Register SQLite library:
//   sqlite.open(filename) → db object
//   db:exec(sql) → result table or nil
//   db:close()
void RegisterSqliteLibraryApi(State *s);

}  // namespace fakelua::sqlite
