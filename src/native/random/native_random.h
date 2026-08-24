#pragma once

#include "state/state.h"

namespace fakelua::random {

// ─────────────────────────────────────────────────────────────────────────────
// Random number generator — PCG-32 algorithm
//
// Why C++ backend: fakelua's Lua subset resolves all function calls at compile
// time. RNG state is mutable and must persist across calls, so it lives in a
// NativeObject backed by C++ (same pattern as timer/net).
//
// API:
//   local rng = random.new(seed)
//   rng:int(min, max)              — integer in [min, max]
//   rng:float(min, max)            — float in [min, max)
//   rng:dice(count, sides)         — sum of `count` dice, each [1, sides]
//   rng:chance(prob)               — true with probability prob (0.0–1.0)
//   rng:weighted(weights)          — pick index by weight table (1-based)
//   rng:get_state()                — get 64-bit internal state as hex string for save
//   rng:set_state(hex_str)         — restore 64-bit internal state from hex string
// ─────────────────────────────────────────────────────────────────────────────

void RegisterRandomLibraryApi(State *s);

}  // namespace fakelua::random
