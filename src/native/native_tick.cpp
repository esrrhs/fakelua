#include "native/native_tick.h"

#include <utility>

namespace fakelua::native {

TickRegistry::Handle TickRegistry::Add(std::function<void()> fn) {
    if (!fn) return 0;
    auto entry = std::make_shared<Entry>();
    entry->handle = next_handle_++;
    entry->fn = std::move(fn);
    entries_.push_back(std::move(entry));
    return entries_.back()->handle;
}

void TickRegistry::Remove(Handle handle) {
    if (handle == 0) return;
    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if ((*it)->handle != handle) continue;
        // 先打标记再移除：TickAll 的快照靠这个标记跳过本轮已注销的项。
        (*it)->removed = true;
        entries_.erase(it);
        return;
    }
}

void TickRegistry::TickAll() {
    if (ticking_) return;

    struct TickingGuard {
        bool &flag;
        explicit TickingGuard(bool &f) : flag(f) { flag = true; }
        // Lua 回调可能抛异常，标记必须无条件复位。
        ~TickingGuard() { flag = false; }
    } guard(ticking_);

    auto snapshot = entries_;
    for (const auto &entry : snapshot) {
        if (entry->removed) continue;
        entry->fn();
    }
}

}  // namespace fakelua::native
