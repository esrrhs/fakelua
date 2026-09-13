#pragma once

#include "state/state.h"

namespace fakelua::container {

// Boost.Container 持久容器（NativeObject，跨帧存活；Lua table 在 arena reset 后会失效）。
//   container.deque / vector / small_vector / list / map / set
void RegisterContainerLibraryApi(State *s);

}// namespace fakelua::container
