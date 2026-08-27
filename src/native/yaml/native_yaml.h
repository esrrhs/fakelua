#pragma once

#include "state/state.h"

namespace fakelua::yaml {

// Register YAML library:
//   yaml.decode(str) → Lua value
//   yaml.encode(value) → YAML string
void RegisterYamlLibraryApi(State *s);

}  // namespace fakelua::yaml
