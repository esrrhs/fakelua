#pragma once

#include "fakelua.h"

namespace fakelua::serialize {

// 注册序列化库：serialize.encode/decode（紧凑 wire）以及 text/xml（Boost.Serialization）。
void RegisterSerializeLibraryApi(State *s);

}// namespace fakelua::serialize
