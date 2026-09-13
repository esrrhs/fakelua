#include "native/native_io_context.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace fakelua::native {

IoContext::IoContext() {
#ifdef _WIN32
    WSADATA wsa{};
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    base_ = event_base_new();
}

IoContext::~IoContext() {
    if (dns_) {
        evdns_base_free(dns_, 0);
        dns_ = nullptr;
    }
    if (base_) {
        event_base_free(base_);
        base_ = nullptr;
    }
}

std::size_t IoContext::Poll() {
    if (dispatch_depth_ > 0 || !base_) return 0;
    int rc = event_base_loop(base_, EVLOOP_NONBLOCK);
    return rc == 0 ? 1 : 0;
}

}// namespace fakelua::native
