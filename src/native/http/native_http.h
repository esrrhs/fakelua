#pragma once

#include "state/state.h"

namespace fakelua::http {

void RegisterHttpLibraryApi(State *s);

void OnStateDeleted(State *s);

void TickAll(State *s);

}// namespace fakelua::http
