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

struct EventState {
    // event_name → 有序回调函数名列表
    std::unordered_map<std::string, std::vector<std::string>> listeners;
    // event_name → 一次性回调函数名列表（触发后自动移除）
    std::unordered_map<std::string, std::vector<std::string>> once_listeners;
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

static void DispatchEvent(State *state, const std::string &func_name, CVar *args, int arg_count) {
    if (func_name.empty()) return;

    auto func = state->GetVM().GetFunction(func_name);
    if (func.Empty()) return;

    void *addr = func.GetAddr(JIT_TCC);
    JITType jit_type = JIT_TCC;
    if (!addr) {
        addr = func.GetAddr(JIT_GCC);
        jit_type = JIT_GCC;
    }

    if (addr) {
        inter::DispatchCall(state, addr, args, arg_count, jit_type);
    }
    // 不存在的函数静默跳过（可能已被卸载）
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

    GetEventState(s).listeners[event_name].push_back(std::move(func_name));
    LOG_DEBUG(s, "event", "event.on: event={} func={}", event_name, func_name);
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

    GetEventState(s).once_listeners[event_name].push_back(std::move(func_name));
    return inter::NativeToFakeluaNil(s);
}

// event.off(event_name, func_name) — 取消订阅
static CVar EventOff(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "event.off", "event_name and func_name expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string event_name = CVarToString(a0);

    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    std::string func_name = CVarToString(a1);

    // 从 listeners 中移除
    auto it = GetEventState(s).listeners.find(event_name);
    if (it != GetEventState(s).listeners.end()) {
        auto &vec = it->second;
        for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
            if (*vit == func_name) {
                vec.erase(vit);
                break;
            }
        }
        if (vec.empty()) {
            GetEventState(s).listeners.erase(it);
        }
    }
    LOG_DEBUG(s, "event", "event.off: event={} func={}", event_name, func_name);

    // 从 once_listeners 中移除
    auto it2 = GetEventState(s).once_listeners.find(event_name);
    if (it2 != GetEventState(s).once_listeners.end()) {
        auto &vec = it2->second;
        for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
            if (*vit == func_name) {
                vec.erase(vit);
                break;
            }
        }
        if (vec.empty()) {
            GetEventState(s).once_listeners.erase(it2);
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

    // 快照 listeners（防止回调中修改列表导致迭代器失效）
    auto it = GetEventState(s).listeners.find(event_name);
    if (it != GetEventState(s).listeners.end()) {
        std::vector<std::string> snapshot = it->second;
        LOG_DEBUG(s, "event", "event.emit: event={} listeners={} args={}", event_name, snapshot.size(), event_arg_count);
        for (const auto &func_name: snapshot) {
            DispatchEvent(s, func_name, event_args, event_arg_count);
        }
    }

    // 一次性回调：触发后清除
    auto it2 = GetEventState(s).once_listeners.find(event_name);
    if (it2 != GetEventState(s).once_listeners.end()) {
        std::vector<std::string> snapshot = it2->second;
        GetEventState(s).once_listeners.erase(it2);
        for (const auto &func_name: snapshot) {
            DispatchEvent(s, func_name, event_args, event_arg_count);
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
