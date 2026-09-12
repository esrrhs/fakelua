#pragma once

// native_io_context.h — 每个 State 一份的 Boost.Asio 事件循环
// io_context 不只是一个 epoll/IOCP 句柄，它是事件循环本体加一整套 service，其中 DNS
// 解析线程池是每个 io_context 一条后台线程。所以按连接粒度创建/销毁 context，等于每条
// 连接烧一个 epoll fd 加一条线程；改成每个 State 一份之后，一个 State 内的所有连接
// 共用一套。
// 粒度定在 State 而不是全进程：State 本身就是单线程实体（见 state.h 的线程模型注释），
// 所以下面的计数器都是普通成员，不需要原子操作。反过来全进程一份是错的 —— 不同 State
// 可能跑在不同线程上，一个线程 poll() 时另一个 restart() 是未定义行为。
//
// Windows 约束（根 CMakeLists 全局 BOOST_ASIO_DISABLE_IOCP，必须所有 native IO 遵守）：
//  - 只用本对象的 poll()，不要 io_context::run()。
//  - 不要 socket.non_blocking(true)；不要在 poll() 线程上做同步 read_some/write_some、
//    SSL shutdown、close_statement、windows::object_handle wait。
//  - 不要 Asio pipe（DISABLE_IOCP 之后没有 BOOST_ASIO_HAS_PIPE）。
//  - 不要自建 io_context，concurrency_hint 保持 1。
//  - 关连接（和 net TCP 一样，所有平台）：回调里只打标记；Tick()/Lua 返回后再
//    socket.shutdown + close（不要 socket.cancel()），poll() 收完完成包，然后 destroy。
//    不要 Boost.Redis connection::cancel()、不要 ssl::stream::shutdown()——它们会
//    在这条 poll() 线程上等待。不要 leak。

#include <boost/asio/io_context.hpp>

#include <cstddef>
#include <memory>
#include <utility>

namespace fakelua::native {

#ifdef _WIN32
inline constexpr bool kWindowsAsio = true;
#else
inline constexpr bool kWindowsAsio = false;
#endif

// Abort in-flight I/O then destroy. `abort` must be non-blocking (socket
// shutdown+close), like net TCP. Do not Boost.Redis connection::cancel().
// Caller Poll()s to drain while `p` is still alive. TickDepth deferral is
// separate and required on every platform.
template<class T, class Abort>
void TeardownTransport(std::unique_ptr<T> &p, Abort &&abort) {
    if (!p) return;
    abort(*p);
    p.reset();
}

class IoContext {
public:
    // 1 = 只会被一个线程驱动。这样 asio 不会再给 context 配一条自己的 timer 线程。
    IoContext() : ctx_(1) {
    }

    IoContext(const IoContext &) = delete;
    IoContext &operator=(const IoContext &) = delete;

    boost::asio::io_context &Get() {
        return ctx_;
    }

    // 执行已就绪的回调后立即返回。派发进 Lua 期间是空操作：那时嵌套 poll 会在派发
    // 方还在读自己的状态时把它改写掉。
    std::size_t Poll();

    bool InDispatch() const {
        return dispatch_depth_ > 0;
    }

    // 标记"正在把事件派发进 Lua"。它活着的期间 Poll() 不做事。
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
    boost::asio::io_context ctx_;
    int dispatch_depth_ = 0;
};

// 与在飞回调共享的存活标记。io_context 比使用它的对象活得久，未完成的操作会真的被
// 投递（以前 context 跟着对象一起销毁，这些操作是被直接丢弃的），所以捕获裸宿主
// 指针的回调必须先确认宿主还在。把 token 声明成最后一个成员，它就会最先被销毁。
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

    // 把返回值拷进回调，在回调里先判 Alive()。
    [[nodiscard]] Watch GetWatch() const {
        return Watch(flag_);
    }

private:
    std::shared_ptr<bool> flag_;
};

}// namespace fakelua::native
