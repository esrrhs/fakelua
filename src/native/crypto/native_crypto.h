#pragma once

#include "state/state.h"

namespace fakelua::crypto {

// Register crypto library: hashes, encodings, ciphers, crypto.uuid(), crypto.crc32(), crypto.xxhash().
void RegisterCryptoLibraryApi(State *s);

}// namespace fakelua::crypto
