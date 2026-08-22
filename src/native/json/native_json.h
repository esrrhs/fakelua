#pragma once

#include "state/state.h"

namespace fakelua::json {

// Register JSON library:
//   json.encode(value) → JSON string
//   json.decode(str)   → Lua value
void RegisterJsonLibraryApi(State *s);

}  // namespace fakelua::json
