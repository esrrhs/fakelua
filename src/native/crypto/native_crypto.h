#pragma once

#include "state/state.h"

namespace fakelua::crypto {

// Register crypto library: hashes, encodings, ciphers, crypto.uuid(), crypto.crc32().
void RegisterCryptoLibraryApi(State *s);

}// namespace fakelua::crypto
