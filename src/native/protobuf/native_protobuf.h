#pragma once

#include "fakelua.h"

namespace fakelua::protobuf {

// 注册 protobuf 库到 State
// 暴露：protobuf.load / encode / decode / types / fields
void RegisterProtobufLibraryApi(State *s);

}  // namespace fakelua::protobuf
