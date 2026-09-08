#pragma once

// native_tick.h — State 级的 tick 注册表
//
// 所有需要被周期性驱动的 native 对象（net 的 server/client、mysql 的连接和连接池、
// 定时器）在创建时把自己注册进来，销毁时注销，由 runtime.tick() 统一驱动。这样 Lua
// 侧不必逐个对象 tick，也不会漏掉某个对象。
//
// 每个 State 一份，而 State 是单线程实体（见 state.h 的线程模型注释），所以这里不加锁。

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace fakelua::native {

class TickRegistry {
public:
    using Handle = uint64_t;

    // 返回值用于 Remove。按注册顺序驱动，所以顺序是确定的。
    Handle Add(std::function<void()> fn);

    // handle 为 0 或已注销时是 no-op，调用方不必判空。
    void Remove(Handle handle);

    // 依次驱动所有已注册项。
    //
    // 允许回调里注册和注销：本轮驱动的是进入时的快照，期间新注册的要等下一轮，被注销
    // 的会被跳过。嵌套调用（回调进了 Lua，Lua 又调 runtime.tick()）直接返回，与各模块
    // 原有的 tick 重入守卫一致。
    void TickAll();

private:
    struct Entry {
        Handle handle = 0;
        std::function<void()> fn;
        // 供本轮快照识别"回调里已被注销"的项：此时它已经从 entries_ 里移除了，但快照
        // 还持有一份 shared_ptr。
        bool removed = false;
    };

    std::vector<std::shared_ptr<Entry>> entries_;
    Handle next_handle_ = 1;
    bool ticking_ = false;
};

}  // namespace fakelua::native
