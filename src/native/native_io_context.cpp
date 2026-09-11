#include "native/native_io_context.h"

namespace fakelua::native {

std::size_t IoContext::Poll() {
    if (dispatch_depth_ > 0) return 0;
    // 工作队列排空后 asio 会把 context 标记成 stopped，此后 poll() 直接返回。
    if (ctx_.stopped()) {
        ctx_.restart();
    }
    return ctx_.poll();
}

}// namespace fakelua::native
