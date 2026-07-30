#pragma once

#include "fakelua.h"
#include "state/heap.h"
#include "state/state.h"
#include "util/common.h"
#include "var/var_closure.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "var/var_type.h"

#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────────
// NativeField: 单个字段的值，存活于 C++ 堆，不依赖 fakelua arena
// ─────────────────────────────────────────────────────────────────────────────
struct NativeField {
    enum class Kind { Nil, Int, Float, Bool, String, Object };

    Kind kind = Kind::Nil;
    int64_t i = 0;
    double f = 0.0;
    bool b = false;
    std::string s;              // Kind::String 时的原始字符串
    NativeObject *obj = nullptr;// Kind::Object 时的嵌套对象（不拥有）

    // 惰性构建 VarString 缓存（存在 C++ 堆，供 spec_get 返回 VAR_STRING CVar）
    // 当 s 内容变化时重建；由于 fakelua 是单线程的，此处不加锁。
    mutable std::vector<char> vs_cache;// [sizeof(VarString) + s.size()] 的 buffer
    mutable bool vs_dirty = true;

    // 返回与 VarString 内存布局兼容的指针（data_ 紧跟在结构体后）
    VarString *GetVarString() const {
        if (vs_dirty || vs_cache.size() != sizeof(VarString) + static_cast<size_t>(std::max(0, (int) s.size()))) {
            vs_cache.resize(sizeof(VarString) + s.size());
            // 直接写 POD 字段（避免调用构造函数访问 data_[0]）
            std::memcpy(vs_cache.data(), "\0\0\0\0\0\0\0\0", 8);// size_=0, hash_=0
            *reinterpret_cast<int *>(vs_cache.data()) = static_cast<int>(s.size());
            if (!s.empty()) {
                std::memcpy(vs_cache.data() + sizeof(VarString), s.data(), s.size());
            }
            vs_dirty = false;
        }
        return reinterpret_cast<VarString *>(vs_cache.data());
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// NativeObjectSpec: arena 内的小结构体，被 VarTable.spec 指向
// VarTable 每帧 reset，但 NativeObjectSpec.obj 和 .state 始终有效
// ─────────────────────────────────────────────────────────────────────────────
struct NativeObjectSpec {
    NativeObject *obj;// C++ 堆，跨帧持久
    State *state;     // fakelua 状态（单帧内有效即可）
};

// ─────────────────────────────────────────────────────────────────────────────
// NativeObject: 完整实现（pImpl 的 Impl 部分）
// public API 在 fakelua.h 中声明
// ─────────────────────────────────────────────────────────────────────────────
struct NativeObject::Impl {
    std::string type_name;
    int64_t id = 0;
    int64_t group_id = 0;
    std::unordered_map<std::string, NativeField> kv;
    std::unordered_map<std::string, NativeMethod> methods;
};

// spec_get / spec_set 静态实现
CVar NativeSpecGet(VarTable *tbl, CVar k, bool *finish);
void NativeSpecSet(VarTable *tbl, CVar k, CVar v, bool *finish);

// 从 CVar key 提取字符串视图（VAR_STRINGID / VAR_STRING）
inline std::string_view KeyToStringView(CVar k) {
    const int t = k.type_;
    if (t == static_cast<int>(VarType::StringId)) {
        if (!k.data_.i) return {};
        const char *ptr = reinterpret_cast<const char *>(k.data_.i);
        int sz = *reinterpret_cast<const int *>(ptr);
        return {ptr + 8, static_cast<size_t>(sz)};
    }
    if (t == static_cast<int>(VarType::String)) {
        if (!k.data_.s) return {};
        return k.data_.s->Str();
    }
    return {};
}

// NativeField → CVar（需要 State* 以便嵌套对象 Wrap）
CVar NativeFieldToCVar(const NativeField &field, State *s);

// CVar → NativeField（将 fakelua 值转为 C++ 持久值）
NativeField CVarToNativeField(CVar v);

}// namespace fakelua

#include "native/native_math.h"
#include "native/native_string.h"
#include "native/native_table.h"
#include "state/state.h"
