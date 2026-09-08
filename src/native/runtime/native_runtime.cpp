#include "native/runtime/native_runtime.h"

#include "native/native_common.h"
#include "native/native_tick.h"
#include "var/var.h"

namespace fakelua::runtime {

// runtime.tick() — 驱动本 State 上所有注册过的 native 对象：定时器、net 的
// server/client、mysql 的连接和连接池。谁注册了就驱动谁，调用方不必知道有哪些对象。
static CVar runtime_tick(State *s, CVar * /*args*/, int /*n*/) {
    s->GetTickRegistry().TickAll();
    return inter::NativeToFakeluaNil(s);
}

void RegisterRuntimeLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "runtime.tick", 0, false, runtime_tick);
}

}  // namespace fakelua::runtime
