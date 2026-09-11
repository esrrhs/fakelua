#pragma once

// native_io_context.h — 每个 State 一份的 Boost.Asio 事件循环
// io_context 不只是一个 epoll/IOCP 句柄，它是事件循环本体加一整套 service，其中 DNS
// 解析线程池是每个 io_context 一条后台线程。所以按连接粒度创建/销毁 context，等于每条
// 连接烧一个 epoll fd 加一条线程；改成每个 State 一份之后，一个 State 内的所有连接
// 共用一套。
// 粒度定在 State 而不是全进程：State 本身就是单线程实体（见 state.h 的线程模型注释），
// 所以下面的计数器都是普通成员，不需要原子操作。反过来全进程一份是错的 —— 不同 State
// 可能跑在不同线程上，一个线程 poll() 时另一个 restart() 是未定义行为。

#include <boost/asio/io_context.hpp>

#include <cstddef>
#include <memory>
#include <utility>

namespace fakelua::native {

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
