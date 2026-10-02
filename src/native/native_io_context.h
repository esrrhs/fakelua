#pragma once

// native_io_context.h — 每个 State 一份 libevent event_base。
// 粒度定在 State：State 是单线程实体，runtime.tick() 里 EVLOOP_NONBLOCK 驱动。
// 不要另开线程跑 event_base_dispatch / loop(0)。

#include <event2/dns.h>
#include <event2/event.h>

#include <cstddef>
#include <memory>
#include <utility>

namespace fakelua::native {

#ifdef _WIN32
inline constexpr bool kWindowsNative = true;
#else
inline constexpr bool kWindowsNative = false;
#endif

class IoContext {
public:
    IoContext();
    ~IoContext();

    IoContext(const IoContext &) = delete;
    IoContext &operator=(const IoContext &) = delete;

    event_base *Get() {
        return base_;
    }

    evdns_base *Dns() {
        if (!dns_ && base_) dns_ = evdns_base_new(base_, 1);
        return dns_;
    }

    std::size_t Poll();

    // C++→Lua 回调嵌套深度。DispatchScope 在各派发入口 +1，析构时 -1。
    // 纯计数器，不依赖平台：Linux / macOS / Windows 行为一致。
    // 当前包住回调的位置：
    //   net    TcpServer / TcpClient / UdpSocket::DrainEventsWith
    //   mysql  MysqlConnection::DispatchConnect / DispatchCallbackWithResult
    //   http   CallNamed
    //   redis  CallNamed，以及连接/命令结果派发
    // net 的 send 看到 InDispatch() 为真时改为入队，由本轮 tick 在 scope 结束后泵出。
    bool InDispatch() const {
        return dispatch_depth_ > 0;
    }

    class DispatchScope {
    public:
        explicit DispatchScope(IoContext &owner) : owner_(owner) {
            ++owner_.dispatch_depth_;
        }

        ~DispatchScope() {
            --owner_.dispatch_depth_;
        }

        DispatchScope(const DispatchScope &) = delete;
        DispatchScope &operator=(const DispatchScope &) = delete;

    private:
        IoContext &owner_;
    };

private:
    event_base *base_ = nullptr;
    evdns_base *dns_ = nullptr;
    int dispatch_depth_ = 0;
};

class LifeToken {
public:
    class Watch {
    public:
        explicit Watch(std::shared_ptr<const bool> flag) : flag_(std::move(flag)) {
        }

        [[nodiscard]] bool Alive() const {
            return flag_ && *flag_;
        }

    private:
        std::shared_ptr<const bool> flag_;
    };

    LifeToken() : flag_(std::make_shared<bool>(true)) {
    }

    ~LifeToken() {
        *flag_ = false;
    }

    LifeToken(const LifeToken &) = delete;
    LifeToken &operator=(const LifeToken &) = delete;

    [[nodiscard]] Watch GetWatch() const {
        return Watch(flag_);
    }

private:
    std::shared_ptr<bool> flag_;
};

}// namespace fakelua::native
