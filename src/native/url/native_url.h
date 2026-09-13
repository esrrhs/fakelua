#pragma once

#include "state/state.h"

namespace fakelua::url {

// url.parse / url.format / url.encode / url.decode / url.encode_query / url.decode_query
void RegisterUrlLibraryApi(State *s);

}// namespace fakelua::url
