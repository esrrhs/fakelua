#pragma once

#include "protobuf_schema.h"

#include <string>

namespace fakelua {
class State;
struct CVar;
}  // namespace fakelua

namespace fakelua::protobuf {

// ─── 类型化编解码 ───
//
// Schema-driven：按 proto 定义把 Lua table 编成标准 protobuf 二进制，或反之。

// 编码：message 名 + Lua table → 二进制字符串
// 出错时抛 ThrowFakeluaException
std::string EncodeMessage(State *s, const std::string &msg_name, const CVar &table);

// 解码：message 名 + 二进制字符串 → Lua table
// 出错时抛 ThrowFakeluaException
CVar DecodeMessage(State *s, const std::string &msg_name, const std::string &data);

}  // namespace fakelua::protobuf
