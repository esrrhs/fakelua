#pragma once

// wire_codec.h — CVar 紧凑 wire 编解码（内部模块，非 Lua API）。
//
// 单值格式与 Lua serialize.encode/decode 完全一致（见 native_serialize.cpp），
// 供两类使用者复用：
//   1. serialize 原生库（Lua API 层）；
//   2. 异步原生回调（mysql）：把"函数名 + 绑定参数"中的数据参数序列化成自有的
//      字节串暂存，跨多次顶层 Call / State::Reset() 保存，结果回来时在派发帧内
//      decode 回临时 arena。字节串是普通堆内存，回调结束即释放，不占用 const arena。

#include "fakelua.h"

#include <string>
#include <string_view>
#include <vector>

namespace fakelua::serialize {

// 值是否可被 wire 编码：nil/bool/int/float/string/stringid/table，其余（闭包/native 对象等）不行。
bool WireIsSerializable(CVar v);

// 单个 CVar → wire 字节。表内遇到不可序列化字段会跳过（与 serialize.encode 历史行为一致）；
// 顶层值不可序列化时抛错。
std::string WireEncode(CVar v);

// wire 字节 → 单个 CVar（分配在 State 临时 arena 上，仅当前帧有效）。
// 字节必须被完整消费，尾随字节抛错。
CVar WireDecode(State *s, std::string_view in);

// 回调绑定参数元组：[varint count][value]...
// 与 WireEncode 不同，这里对参数做【严格】校验：闭包/native 对象等不可序列化类型
// 出现在任意嵌套位置（含表的键值）都抛 bad argument，不允许静默丢字段。
// fname/argno_base 用于错误信息（bad argument #argno_base+i to 'fname'）。
std::string WireEncodeCallbackArgs(State *s, CVar *args, int first, int n, const char *fname, int argno_base);

// 绑定参数元组解码。产物在临时 arena 上；DecodedArgs 同时持有字符串字典等后备内存，
// 调用方必须让它活到回调函数返回之后。
struct DecodedArgs {
    std::vector<CVar> vars;
    std::vector<std::string> str_dict;
};

DecodedArgs WireDecodeCallbackArgs(State *s, std::string_view blob);

}// namespace fakelua::serialize
