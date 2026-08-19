#pragma once

#include "protobuf_schema.h"

#include <string>

namespace fakelua::protobuf {

// ─── .proto 文件解析器 ───
//
// 解析 proto3 .proto 文本，注册所有 message/enum 到 ProtobufState 单例。
// 支持：message（含嵌套）、enum、map<K,V>、oneof、repeated、optional、
//       全部 18 种 scalar type、import（多文件合并）、package 限定名。
// 不支持：service/RPC、extensions、groups、proto2 required、自定义 options。

// 解析 .proto 文本，返回空字符串表示成功，否则返回错误信息
std::string ParseProto(const std::string &text, const std::string &filename = "");

}  // namespace fakelua::protobuf
