#pragma once

#include "fakelua.h"

namespace fakelua {

class VarClosure;

// 把闭包（以及它捕获的表、字符串、嵌套闭包）复制到 const arena。
// fakelua 没有 GC。运行期值默认落在临时 arena 上，每次顶层 Call 开头的
// State::Reset() 会把这块 arena 的水位清零，裸指针跨帧就会悬空。
// const arena 不参与 Reset，和 State 同寿，因此异步回调可以一直持有这里返回的指针。
// 不做引用计数：State 销毁时整块 arena 一起释放。
// 同时把捕获盒子里的值改写成复制后的对象，这样同一帧内调用方和回调看到的是同一份表。
VarClosure *PinClosureForAsync(State *s, VarClosure *cl);

}// namespace fakelua
