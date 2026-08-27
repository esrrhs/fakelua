#pragma once

#include "state/state.h"

namespace fakelua::toml {

// Register TOML library:
//   toml.decode(str) → Lua value
//   toml.encode(value) → TOML string
void RegisterTomlLibraryApi(State *s);

}  // namespace fakelua::toml
