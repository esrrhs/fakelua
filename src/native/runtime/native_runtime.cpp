#include "native/runtime/native_runtime.h"

#include "native/mysql/native_mysql.h"
#include "native/native_common.h"
#include "native/net/native_net.h"
#include "native/timer/native_timer.h"
#include "var/var.h"

namespace fakelua::runtime {

// runtime.tick() — 驱动本 State 上所有需要周期性推进的 native 模块。各模块自己知道手里
// 有哪些对象（都有一份 per-State 列表），这里只负责按固定顺序把它们串起来，和
// FakeluaDeleteState 里分发 OnStateDeleted 是同一个路子。
// 顺序：定时器 → net → mysql。定时器放最前，让本轮到期的回调能赶上后面的 IO 派发。
static CVar RuntimeTick(State *s, CVar * /*args*/, int /*n*/) {
    timer::TickAll(s);
    net::TickAll(s);
    mysql::TickAll(s);
    return inter::NativeToFakeluaNil(s);
}

void RegisterRuntimeLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "runtime.tick", 0, false, RuntimeTick);
}

}// namespace fakelua::runtime
