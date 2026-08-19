#include "native/timer/native_timer.h"
#include "native/timer/heap_timer.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "var/var.h"
#include "var/var_string.h"

#include <string>
#include <unordered_map>

namespace fakelua::timer {

// ─────────────────────────────────────────────────────────────────────────────
// 全局单例定时器状态（fakelua 单线程模型，全局状态即可）
// ─────────────────────────────────────────────────────────────────────────────

struct TimerState {
    HeapTimer heap;
    // timer id → Lua 回调函数名（定时器到期时按函数名派发，不存闭包）
    std::unordered_map<HeapTimer::TimerId, std::string> callbacks;

    // 心跳（周期性，永不自动删除）
    std::string heartbeat_cb;                  // 心跳回调函数名
    HeapTimer::TimePoint heartbeat_next{};     // 下次触发时间
    uint32_t heartbeat_interval_ms = 0;        // 心跳间隔
    bool heartbeat_active = false;             // 是否已注册心跳
};

static TimerState g_state;

// ─────────────────────────────────────────────────────────────────────────────
// 辅助：从 CVar 提取字符串
// ─────────────────────────────────────────────────────────────────────────────

static std::string cvar_to_string(CVar v) {
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

// ─────────────────────────────────────────────────────────────────────────────
// NativeObject 辅助方法注册（供测试在回调中读写状态）
// ─────────────────────────────────────────────────────────────────────────────

static CVar obj_get_int(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "get_int", "key expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string key = cvar_to_string(a0);
    return inter::NativeToFakeluaInt(s, self->GetInt(key, 0));
}

static CVar obj_set_int(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "set_int", "key and value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string key = cvar_to_string(a0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    int64_t val = inter::CVarToInteger(a1, 0);
    self->SetInt(key, val);
    return inter::NativeToFakeluaNil(s);
}

// timer.register_obj_methods(obj) — 为已有 NativeObject 注册 get_int/set_int
static CVar timer_register_obj_methods(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "timer.register_obj_methods", "NativeObject expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    NativeObject *obj = NativeObject::Unwrap(a0);
    if (!obj) {
        ThrowFakeluaException("timer.register_obj_methods: argument is not a NativeObject");
    }
    obj->RegisterMethod("get_int", obj_get_int);
    obj->RegisterMethod("set_int", obj_set_int);
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// C++ → Lua 回调派发（核心：按函数名查找，不存闭包）
// 与 net 模块 call_lua_event 同款机制，type = "timer"
// ─────────────────────────────────────────────────────────────────────────────

static CVar call_lua_timer_event(State *state, const std::string &func_name, HeapTimer::TimerId id) {
    if (func_name.empty()) return CVar{static_cast<int>(VarType::Nil)};

    // 优先查找 JIT 编译的 Lua 函数
    auto func = state->GetVM().GetFunction(func_name);
    void *addr = nullptr;
    JITType jit_type = JIT_TCC;
    if (!func.Empty()) {
        addr = func.GetAddr(JIT_TCC);
        if (!addr) {
            addr = func.GetAddr(JIT_GCC);
            jit_type = JIT_GCC;
        }
    }

    if (addr) {
        // 构造参数: (type="timer", timer_id)
        CVar args[2];
        args[0] = inter::NativeToFakeluaString(state, "timer");
        args[1] = inter::NativeToFakeluaInt(state, static_cast<int64_t>(id));
        return inter::DispatchCall(addr, args, 2, jit_type);
    } else {
        // 回退：尝试原生函数
        auto *entry = state->GetVM().FindNativeFunction(func_name);
        if (entry && entry->callback) {
            CVar args[2];
            args[0] = inter::NativeToFakeluaString(state, "timer");
            args[1] = inter::NativeToFakeluaInt(state, static_cast<int64_t>(id));
            return entry->callback(state, args, 2);
        }
    }
    return CVar{static_cast<int>(VarType::Nil)};
}

// ─────────────────────────────────────────────────────────────────────────────
// 原生函数实现
// ─────────────────────────────────────────────────────────────────────────────

// timer.set(delay_ms, func_name) — 设置一次性定时器，返回 timer id（失败返回 nil）
static CVar timer_set(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "timer.set", "delay_ms and func_name expected");

    // 第 1 参数：delay_ms（整数）
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    int64_t delay_val = inter::CVarToInteger(a0, -1);
    if (delay_val < 0) {
        ThrowBadArgument(1, "timer.set", "delay_ms must be a non-negative integer");
    }
    if (delay_val > std::numeric_limits<uint32_t>::max()) {
        ThrowBadArgument(1, "timer.set", "delay_ms too large");
    }

    // 第 2 参数：func_name（字符串）
    std::string fname;
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    if (a1.type_ == static_cast<int>(VarType::String) && a1.data_.s) {
        fname = std::string(a1.data_.s->Str());
    } else if (a1.type_ == static_cast<int>(VarType::StringId)) {
        if (a1.data_.i) {
            const char *ptr = reinterpret_cast<const char *>(a1.data_.i);
            int sz = *reinterpret_cast<const int *>(ptr);
            fname.assign(ptr + 8, sz);
        }
    } else {
        ThrowBadArgument(2, "timer.set", "func_name must be a string");
    }

    auto id = g_state.heap.Add(static_cast<uint32_t>(delay_val));
    if (id == 0) {
        // 堆已满（达到 int32_max），返回 nil
        return inter::NativeToFakeluaNil(s);
    }
    g_state.callbacks[id] = std::move(fname);
    return inter::NativeToFakeluaInt(s, static_cast<int64_t>(id));
}

// timer.del(id) — 删除一次性定时器，返回是否成功
static CVar timer_del(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "timer.del", "timer id expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    int64_t id_val = inter::CVarToInteger(a0, -1);
    if (id_val < 0) {
        ThrowBadArgument(1, "timer.del", "timer id must be a non-negative integer");
    }

    auto id = static_cast<HeapTimer::TimerId>(id_val);
    bool ok = g_state.heap.Del(id);
    if (ok) {
        g_state.callbacks.erase(id);
    }
    return inter::NativeToFakeluaBool(s, ok);
}

// timer.tick() — 驱动定时器：触发到期的一次性定时器和心跳
static CVar timer_tick(State *s, CVar * /*args*/, int /*n*/) {
    auto now = HeapTimer::Clock::now();

    // 1) 一次性定时器
    auto expired = g_state.heap.Update();
    for (auto id : expired) {
        auto it = g_state.callbacks.find(id);
        if (it != g_state.callbacks.end()) {
            call_lua_timer_event(s, it->second, id);
            g_state.callbacks.erase(it);
        }
    }

    // 2) 心跳：到期则触发，并重新调度下一跳（永不删除）
    if (g_state.heartbeat_active && now >= g_state.heartbeat_next) {
        call_lua_timer_event(s, g_state.heartbeat_cb, 0);
        // 推进到下一个未来的触发点（防止追 catch-up 导致死循环）
        g_state.heartbeat_next += std::chrono::milliseconds(g_state.heartbeat_interval_ms);
        if (g_state.heartbeat_next < now) {
            g_state.heartbeat_next = now + std::chrono::milliseconds(g_state.heartbeat_interval_ms);
        }
    }

    return inter::NativeToFakeluaNil(s);
}

// timer.set_heartbeat(interval_ms, func_name) — 注册周期性心跳，永不自动删除
// 重复调用会覆盖之前的心跳（只保留一个全局心跳）
static CVar timer_set_heartbeat(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "timer.set_heartbeat", "interval_ms and func_name expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    int64_t interval_val = inter::CVarToInteger(a0, -1);
    if (interval_val <= 0) {
        ThrowBadArgument(1, "timer.set_heartbeat", "interval_ms must be a positive integer");
    }
    if (interval_val > std::numeric_limits<uint32_t>::max()) {
        ThrowBadArgument(1, "timer.set_heartbeat", "interval_ms too large");
    }

    std::string fname;
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    if (a1.type_ == static_cast<int>(VarType::String) && a1.data_.s) {
        fname = std::string(a1.data_.s->Str());
    } else if (a1.type_ == static_cast<int>(VarType::StringId)) {
        if (a1.data_.i) {
            const char *ptr = reinterpret_cast<const char *>(a1.data_.i);
            int sz = *reinterpret_cast<const int *>(ptr);
            fname.assign(ptr + 8, sz);
        }
    } else {
        ThrowBadArgument(2, "timer.set_heartbeat", "func_name must be a string");
    }

    g_state.heartbeat_cb = std::move(fname);
    g_state.heartbeat_interval_ms = static_cast<uint32_t>(interval_val);
    g_state.heartbeat_next = HeapTimer::Clock::now() + std::chrono::milliseconds(g_state.heartbeat_interval_ms);
    g_state.heartbeat_active = true;

    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// 注册
// ─────────────────────────────────────────────────────────────────────────────

void RegisterTimerLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "timer.set", 2, false, timer_set);
    RegisterNativeFunction(s, "timer.del", 1, false, timer_del);
    RegisterNativeFunction(s, "timer.tick", 0, false, timer_tick);
    RegisterNativeFunction(s, "timer.set_heartbeat", 2, false, timer_set_heartbeat);
    RegisterNativeFunction(s, "timer.register_obj_methods", 1, false, timer_register_obj_methods);
}

} // namespace fakelua::timer
