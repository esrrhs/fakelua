#pragma once

#include "state/state.h"

namespace fakelua::xml {

// Register XML library:
//   xml.decode(str) → Lua value
//   xml.encode(value) → XML string
//
// Decode convention:
//   - Element node → table
//   - Attributes → table["_attr_attrname"] = value
//   - Text content → table["_text"] = value
//   - Child elements → grouped by tag; multiple same-tag siblings → array
//   - Plain text node → returned as string directly
void RegisterXmlLibraryApi(State *s);

}  // namespace fakelua::xml
