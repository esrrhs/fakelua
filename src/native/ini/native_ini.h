#pragma once

#include "state/state.h"

namespace fakelua::ini {

// Register INI library:
//   ini.decode(str) → Lua value
//   ini.encode(value) → INI string
//
// INI structure: sections containing key=value pairs.
//   ini.decode → table[section][key] = value
//   ini.encode → each top-level sub-table is a [section], its entries are key=value
// Values are auto-typed: numbers, booleans, or strings.
void RegisterIniLibraryApi(State *s);

}  // namespace fakelua::ini
