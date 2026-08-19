#pragma once

#include "fakelua.h"

namespace fakelua::serialize {

// 注册序列化库到 State：暴露 serialize.encode / serialize.decode
void RegisterSerializeLibraryApi(State *s);

} // namespace fakelua::serialize
