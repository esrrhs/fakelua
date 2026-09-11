#pragma once

#include "state/state.h"

namespace fakelua::runtime {

// 注册 runtime 库：runtime.tick()
void RegisterRuntimeLibraryApi(State *s);

}// namespace fakelua::runtime
