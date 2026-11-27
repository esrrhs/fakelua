#include "native/event/native_event.h"
#include "native/native_common.h"
#include "util/logging.h"
#include "var/var.h"
#include "var/var_string.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua::event {

// 每个 State 一份事件状态（避免跨 VM 串数据）

struct ListenerEntry {
    uint64_t id = 0;
    std::string func_name;
    JITType cb_jit = JIT_TCC;
};

struct EventState {
    uint64_t next_id = 0;
    // event_name → 有序回调列表
    std::unordered_map<std::string, std::vector<ListenerEntry>> listeners;
    // event_name → 一次性回调列表（触发后自动移除）
    std::unordered_map<std::string, std::vector<ListenerEntry>> once_listeners;
};

// 每个 State 一份，随 State 销毁。放在 State 上而不是这里的 static map：后者是全进程
// 一份，多个线程各跑自己的 State 时会并发改同一个容器。
static EventState &GetEventState(State *s) {
    return s->GetModuleState<EventState>();
}

// 辅助：从 CVar 提取字符串

static std::string CVarToString(CVar v) {
    if (v.type_ == static_cast<int>(VarType::String) && v.data_.s) {
        return std::string(v.data_.s->Str());
    }
    if (v.type_ == static_cast<int>(VarType::StringId) && v.data_.i) {
        const char *ptr = reinterpret_cast<const char *>(v.data_.i);
        int sz = *reinterpret_cast<const int *>(ptr);
        return std::string(ptr + 8, sz);
    }
    return {};
}

// C++ → Lua 回调派发（与 timer/net 同款机制）

static void DispatchEvent(State *state, const ListenerEntry &entry, const CVar *args, int arg_count) {
    if (!state || entry.func_name.empty()) return;
    DispatchNamedLuaCallback(state, entry.func_name, args, arg_count, entry.cb_jit);
}

// 原生函数实现

// event.on(event_name, func_name) — 订阅事件
static CVar EventOn(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "event.on", "event_name and func_name expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string event_name = CVarToString(a0);
    if (event_name.empty()) {
        ThrowBadArgument(1, "event.on", "event_name must be a non-empty string");
    }

    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string func_name = CVarToString(a1);
    if (func_name.empty()) {
        ThrowBadArgument(2, "event.on", "func_name must be a non-empty string");
    }

    LOG_DEBUG(s, "event", "event.on: event={} func={}", event_name, func_name);
    auto &es = GetEventState(s);
    es.listeners[event_name].push_back(ListenerEntry{++es.next_id, std::move(func_name), s->CurrentJit()});
    return inter::NativeToFakeluaNil(s);
}

// event.once(event_name, func_name) — 一次性订阅（触发后自动移除）
static CVar EventOnce(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "event.once", "event_name and func_name expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string event_name = CVarToString(a0);
    if (event_name.empty()) {
        ThrowBadArgument(1, "event.once", "event_name must be a non-empty string");
    }

    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string func_name = CVarToString(a1);
    if (func_name.empty()) {
        ThrowBadArgument(2, "event.once", "func_name must be a non-empty string");
    }

    auto &es = GetEventState(s);
    es.once_listeners[event_name].push_back(ListenerEntry{++es.next_id, std::move(func_name), s->CurrentJit()});
    return inter::NativeToFakeluaNil(s);
}

// event.off(event_name, func_name) — 取消订阅
static CVar EventOff(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "event.off", "event_name and func_name expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string event_name = CVarToString(a0);

    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string func_name = CVarToString(a1);

    auto &es = GetEventState(s);
    // 从 listeners 中移除
    auto it = es.listeners.find(event_name);
    if (it != es.listeners.end()) {
        auto &vec = it->second;
        for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
            if (vit->func_name == func_name) {
                vec.erase(vit);
                break;
            }
        }
        if (vec.empty()) {
            es.listeners.erase(it);
        }
    }
    LOG_DEBUG(s, "event", "event.off: event={} func={}", event_name, func_name);

    // 从 once_listeners 中移除
    auto it2 = es.once_listeners.find(event_name);
    if (it2 != es.once_listeners.end()) {
        auto &vec = it2->second;
        for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
            if (vit->func_name == func_name) {
                vec.erase(vit);
                break;
            }
        }
        if (vec.empty()) {
            es.once_listeners.erase(it2);
        }
    }

    return inter::NativeToFakeluaNil(s);
}

// event.emit(event_name, ...) — 触发事件（vararg，最多 4 个参数）
static CVar EventEmit(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "event.emit", "event_name expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string event_name = CVarToString(a0);
    if (event_name.empty()) {
        return inter::NativeToFakeluaNil(s);
    }

    // 提取事件参数（最多 4 个，跳过第一个 event_name）
    CVar event_args[4];
    int event_arg_count = 0;
    for (int i = 1; i < n && event_arg_count < 4; i++) {
        event_args[event_arg_count++] = inter::GetNativeArg(s, args, n, i);
    }

    // 在派发任何回调前同时快照 listeners 和 once_listeners，防止回调内注册的
    // event.once 在同一次 emit 中立即触发，也防止提前整表擦除导致异常中断时丢监听器。
    auto &es = GetEventState(s);
    std::vector<ListenerEntry> listeners_snapshot;
    if (auto it = es.listeners.find(event_name); it != es.listeners.end()) {
        listeners_snapshot = it->second;
    }
    std::vector<ListenerEntry> once_snapshot;
    if (auto it2 = es.once_listeners.find(event_name); it2 != es.once_listeners.end()) {
        once_snapshot = it2->second;
    }

    if (!listeners_snapshot.empty()) {
        LOG_DEBUG(s, "event", "event.emit: event={} listeners={} args={}", event_name, listeners_snapshot.size(), event_arg_count);
        for (const auto &entry: listeners_snapshot) {
            DispatchEvent(s, entry, event_args, event_arg_count);
        }
    }

    for (const auto &entry: once_snapshot) {
        auto &cur_es = GetEventState(s);
        auto it2 = cur_es.once_listeners.find(event_name);
        if (it2 == cur_es.once_listeners.end()) continue;
        auto &vec = it2->second;
        bool still_present = false;
        for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
            if (vit->id == entry.id) {
                vec.erase(vit);
                still_present = true;
                break;
            }
        }
        if (vec.empty()) {
            cur_es.once_listeners.erase(it2);
        }
        if (still_present) {
            DispatchEvent(s, entry, event_args, event_arg_count);
        }
    }

    return inter::NativeToFakeluaNil(s);
}

// event.clear(event_name) — 清除指定事件的所有回调
static CVar EventClear(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "event.clear", "event_name expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string event_name = CVarToString(a0);

    GetEventState(s).listeners.erase(event_name);
    GetEventState(s).once_listeners.erase(event_name);
    return inter::NativeToFakeluaNil(s);
}

// event.clear_all() — 清除所有事件的所有回调
static CVar EventClearAll(State *s, CVar * /*args*/, int /*n*/) {
    GetEventState(s).listeners.clear();
    GetEventState(s).once_listeners.clear();
    return inter::NativeToFakeluaNil(s);
}

// 注册

void RegisterEventLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "event.on", 2, false, EventOn);
    RegisterNativeFunction(s, "event.once", 2, false, EventOnce);
    RegisterNativeFunction(s, "event.off", 2, false, EventOff);
    RegisterNativeFunction(s, "event.emit", 1, true, EventEmit);
    RegisterNativeFunction(s, "event.clear", 1, false, EventClear);
    RegisterNativeFunction(s, "event.clear_all", 0, false, EventClearAll);
}

}// namespace fakelua::event
