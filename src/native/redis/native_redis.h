#pragma once

#include "state/state.h"

namespace fakelua::redis {

void RegisterRedisLibraryApi(State *s);

void OnStateDeleted(State *s);

void TickAll(State *s);

}// namespace fakelua::redis
