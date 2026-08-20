#pragma once

#include "state/state.h"

namespace fakelua::crypto {

// Register crypto library: crypto.md5(str), crypto.sha1(str), crypto.sha256(str)
// Each returns the hex-encoded digest string.
void RegisterCryptoLibraryApi(State *s);

}  // namespace fakelua::crypto
