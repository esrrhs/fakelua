#pragma once

#include "fakelua.h"

namespace fakelua::net {

void RegisterNetLibraryApi(State *s);

// State 销毁时关掉该 VM 上的 socket，避免 fd 泄漏和 custom parser 里悬挂的 State*。
void OnStateDeleted(State *s);

}// namespace fakelua::net
