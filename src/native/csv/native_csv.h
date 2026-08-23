#pragma once

#include "state/state.h"

namespace fakelua::csv {

// Register CSV library:
//   csv.decode(str, sep?) → table of rows (each row is a table of fields)
//   csv.encode(rows, sep?) → CSV string
void RegisterCsvLibraryApi(State *s);

}  // namespace fakelua::csv
