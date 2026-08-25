#include "native/object/native_object.h"
#include "jit/vm.h"
#include "native/basic/native_basic.h"
#include "native/io/native_io.h"
#include "native/native_common.h"
#include "native/os/native_os.h"
#include "native/utf8/native_utf8.h"
#include "var/var_multi.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <format>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────────
// CVar ↔ NativeField 转换
// ─────────────────────────────────────────────────────────────────────────────

CVar NativeFieldToCVar(const NativeField &field, State *s) {
    CVar r{};
    switch (field.kind) {
        case NativeField::Kind::Nil:
            r.type_ = static_cast<int>(VarType::Nil);
            break;
        case NativeField::Kind::Int:
            r.type_ = static_cast<int>(VarType::Int);
            r.data_.i = field.i;
            break;
        case NativeField::Kind::Float:
            r.type_ = static_cast<int>(VarType::Float);
            r.data_.f = field.f;
            break;
        case NativeField::Kind::Bool:
            r.type_ = static_cast<int>(VarType::Bool);
            r.data_.b = field.b;
            break;
        case NativeField::Kind::String:
            // 返回指向 C++ 堆 VarString 缓存的指针；
            // fakelua 不会释放它（arena 仅管理自己的分配），安全。
            r.type_ = static_cast<int>(VarType::String);
            r.data_.s = field.GetVarString();
            break;
        case NativeField::Kind::Object:
            if (field.obj != nullptr && s != nullptr) {
                r = field.obj->Wrap(s);
            } else {
                r.type_ = static_cast<int>(VarType::Nil);
            }
            break;
    }
    return r;
}

NativeField CVarToNativeField(CVar v) {
    NativeField f;
    const int t = v.type_;

    if (t == static_cast<int>(VarType::Nil)) {
        f.kind = NativeField::Kind::Nil;
    } else if (t == static_cast<int>(VarType::Bool)) {
        f.kind = NativeField::Kind::Bool;
        f.b = v.data_.b;
    } else if (t == static_cast<int>(VarType::Int)) {
        f.kind = NativeField::Kind::Int;
        f.i = v.data_.i;
    } else if (t == static_cast<int>(VarType::Float)) {
        f.kind = NativeField::Kind::Float;
        f.f = v.data_.f;
    } else if (t == static_cast<int>(VarType::String) || t == static_cast<int>(VarType::StringId)) {
        f.kind = NativeField::Kind::String;
        const VarString *vs = (t == static_cast<int>(VarType::String)) ? v.data_.s : reinterpret_cast<const VarString *>(v.data_.i);
        if (vs) {
            f.s = std::string(vs->Str());
            f.vs_dirty = true;
        } else {
            f.kind = NativeField::Kind::Nil;
        }
    } else if (t == static_cast<int>(VarType::Table)) {
        // 如果是 NativeObject 的 wrapper，记录引用；否则视为 nil
        NativeObject *nested = NativeObject::Unwrap(v);
        if (nested) {
            f.kind = NativeField::Kind::Object;
            f.obj = nested;
        }
        // 纯 lua table 不做深拷贝，直接忽略
    }
    // Multi / Closure 类型不做转换，保持 Nil
    return f;
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeObjectManager 实现
// ─────────────────────────────────────────────────────────────────────────────

NativeObjectManager &NativeObjectManager::Instance() {
    static NativeObjectManager instance;
    return instance;
}

int64_t NativeObjectManager::CreateGroup() {
    int64_t gid = ++next_auto_group_id_;
    if (!group_objects_.contains(gid)) {
        group_objects_[gid] = {};
    }
    return gid;
}

NativeObject *NativeObjectManager::Create(int64_t group_id, const std::string &type_name, int64_t id) {
    if (group_id == 0) {
        ThrowFakeluaException("NativeObjectManager::Create failed: group_id must be specified and non-zero");
    }
    if (id == 0) {
        id = --next_auto_obj_id_;
    }
    auto key = std::make_pair(type_name, id);
    auto it = objects_.find(key);
    if (it != objects_.end()) {
        return it->second;
    }
    auto *obj = NativeObject::Create(group_id, type_name, id);
    objects_[key] = obj;
    group_objects_[group_id].push_back(obj);
    return obj;
}

NativeObject *NativeObjectManager::Get(const std::string &type_name, int64_t id) const {
    auto key = std::make_pair(type_name, id);
    auto it = objects_.find(key);
    return (it != objects_.end()) ? it->second : nullptr;
}

bool NativeObjectManager::DestroySingle(const std::string &type_name, int64_t id) {
    auto key = std::make_pair(type_name, id);
    auto it = objects_.find(key);
    if (it != objects_.end()) {
        NativeObject *obj = it->second;
        int64_t gid = obj->GetGroupId();

        auto git = group_objects_.find(gid);
        if (git != group_objects_.end()) {
            std::erase(git->second, obj);
            if (git->second.empty()) {
                group_objects_.erase(git);
            }
        }

        // 全局对象也可能走 DestroySingle；必须在 Destroy 之前清索引
        for (auto gobj = global_objects_.begin(); gobj != global_objects_.end(); ) {
            if (gobj->second == obj) {
                gobj = global_objects_.erase(gobj);
            } else {
                ++gobj;
            }
        }

        NativeObject::Destroy(obj);
        zombies_.push_back(obj);
        objects_.erase(it);
        return true;
    }
    return false;
}

// ── 全局对象（group_id == 0，通过 string key 索引）─────────────────────────

NativeObject *NativeObjectManager::GlobalCreate(const std::string &key, const std::string &type_name, int64_t id) {
    auto it = global_objects_.find(key);
    if (it != global_objects_.end()) {
        // key 已存在，直接返回已有对象（与 Create 的幂等语义一致）
        return it->second;
    }
    if (id == 0) {
        id = --next_auto_obj_id_;
    }
    // 全局对象 group_id = 0，不加入 group_objects_ 索引
    auto *obj = NativeObject::Create(0, type_name, id);
    objects_[std::make_pair(type_name, id)] = obj;
    global_objects_[key] = obj;
    return obj;
}

NativeObject *NativeObjectManager::GlobalGet(const std::string &key) const {
    auto it = global_objects_.find(key);
    return (it != global_objects_.end()) ? it->second : nullptr;
}

bool NativeObjectManager::GlobalDestroy(const std::string &key) {
    auto it = global_objects_.find(key);
    if (it == global_objects_.end()) {
        return false;
    }
    NativeObject *obj = it->second;
    auto obj_key = std::make_pair(obj->GetTypeName(), obj->GetId());
    objects_.erase(obj_key);
    global_objects_.erase(it);
    NativeObject::Destroy(obj);
    zombies_.push_back(obj);
    return true;
}

size_t NativeObjectManager::DestroyGroup(int64_t group_id) {
    auto git = group_objects_.find(group_id);
    if (git == group_objects_.end()) {
        return 0;
    }

    std::vector<NativeObject *> to_destroy = std::move(git->second);
    group_objects_.erase(git);

    size_t count = 0;
    for (auto *obj: to_destroy) {
        if (obj) {
            auto key = std::make_pair(obj->GetTypeName(), obj->GetId());
            objects_.erase(key);
            NativeObject::Destroy(obj);
            zombies_.push_back(obj);
            count++;
        }
    }
    return count;
}

void NativeObjectManager::Clear() {
    // objects_ 中的对象仍然"存活"（impl_ 非空），必须先调 Destroy 触发 finalizer 再 delete。
    // zombies_ 中的对象已经由 DestroySingle/GlobalDestroy/DestroyGroup 调用过 Destroy()，
    // impl_ 已被置 nullptr，只剩一个空壳，只需 delete 释放堆内存即可。
    // 两个集合保证互不重叠（erase 之后才 push_back），因此不存在 double-free。
    for (auto &[k, v]: objects_) {
        NativeObject::Destroy(v);
        delete v;
    }
    objects_.clear();
    group_objects_.clear();
    global_objects_.clear();
    for (auto *z: zombies_) {
        // impl_ 已为 nullptr（Destroy 已清空），直接释放对象本身
        delete z;
    }
    zombies_.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeMethodBridge — 宿主 C++ 成员回调的方法派发桥接
// ─────────────────────────────────────────────────────────────────────────────

CVar NativeMethodBridge(VarClosure *cl, CVar vararg_cvar) {
    if (!cl || cl->upvalue_count < 3) {
        ThrowFakeluaException("NativeMethodBridge failed: invalid VarClosure metadata");
    }

    auto *obj = reinterpret_cast<NativeObject *>(cl->upvalues[0]->data_.i);
    auto *state = reinterpret_cast<State *>(cl->upvalues[1]->data_.i);
    auto *method_ptr = reinterpret_cast<NativeMethod *>(cl->upvalues[2]->data_.i);

    if (!obj || !obj->impl_) {
        ThrowFakeluaException("native object is destroyed");
    }
    if (!method_ptr || !(*method_ptr)) {
        ThrowFakeluaException("NativeMethodBridge failed: obj or method is null");
    }

    CVar *all_args = nullptr;
    int total_arg_count = 0;
    if (vararg_cvar.type_ == static_cast<int>(VarType::Multi) && vararg_cvar.data_.m) {
        total_arg_count = static_cast<int>(vararg_cvar.data_.m->GetCount());
        all_args = vararg_cvar.data_.m->GetVars();
    }

    // 自动判断 self 传递 (冒号 player:method(...) / 点号 player.method(player, ...) vs fn(...))
    NativeObject *passed_self = (total_arg_count > 0) ? NativeObject::Unwrap(all_args[0]) : nullptr;
    NativeObject *actual_self = (passed_self != nullptr) ? passed_self : obj;

    CVar *call_args = nullptr;
    int call_n = 0;

    if (passed_self != nullptr) {
        call_args = all_args + 1;
        call_n = total_arg_count - 1;
    } else {
        call_args = all_args;
        call_n = total_arg_count;
    }

    return (*method_ptr)(actual_self, state, call_args, call_n);
}

// ─────────────────────────────────────────────────────────────────────────────
// spec_get / spec_set 实现
// ─────────────────────────────────────────────────────────────────────────────

// 依据 obj->impl_->kv 的当前内容，重建 tbl 的 spec_keys / spec_vals 快照数组，
// 使 pairs()/next() 迭代结果能反映 NativeSpecSet 写入后的最新字段集合。
// 该函数在 Wrap() 和 NativeSpecSet() 之后都会被调用，以保持快照与实际字段同步。
void RefreshSpecKeys(VarTable *tbl, const NativeObject *obj, State *s) {
    auto &alloc = s->GetHeap().GetAllocator(false /* is_const */);
    const auto &kv = obj->impl_->kv;
    const size_t n = kv.size();

    if (n == 0) {
        tbl->spec_keys = nullptr;
        tbl->spec_vals = nullptr;
        tbl->spec_count = 0;
        return;
    }

    tbl->spec_keys = static_cast<CVar *>(alloc.Alloc(sizeof(CVar) * n));
    tbl->spec_vals = static_cast<CVar *>(alloc.Alloc(sizeof(CVar) * n));
    tbl->spec_count = static_cast<uint32_t>(n);

    size_t i = 0;
    for (const auto &[k, v]: kv) {
        // key：在 arena 中分配 VarString
        const size_t klen = k.size();
        auto *vs = static_cast<VarString *>(alloc.Alloc(sizeof(VarString) + klen + 1));
        *reinterpret_cast<int *>(vs) = static_cast<int>(klen);
        *reinterpret_cast<uint32_t *>(reinterpret_cast<char *>(vs) + sizeof(int)) = 0u;
        char *buf = reinterpret_cast<char *>(vs) + sizeof(VarString);
        if (klen > 0) {
            std::memcpy(buf, k.data(), klen);
        }
        buf[klen] = '\0';

        tbl->spec_keys[i].type_ = static_cast<int>(VarType::String);
        tbl->spec_keys[i].flag_ = 0;
        tbl->spec_keys[i].data_.s = vs;

        tbl->spec_vals[i] = NativeFieldToCVar(v, s);
        ++i;
    }
}

CVar NativeSpecGet(VarTable *tbl, CVar k, bool *finish) {
    auto *spec = static_cast<NativeObjectSpec *>(tbl->spec);
    NativeObject *obj = spec->obj;
    State *s = spec->state;

    if (!obj || !obj->impl_) {
        *finish = true;
        return {};
    }

    const std::string_view key = KeyToStringView(k);
    if (key.empty()) {
        // key 类型不是字符串，回退到 VarTable 哈希表
        *finish = false;
        return {};
    }

    *finish = true;
    const std::string skey(key);

    // 1. 优先查找属性 KV 字段
    const auto &kv = obj->impl_->kv;
    const auto it = kv.find(skey);
    if (it != kv.end()) {
        return NativeFieldToCVar(it->second, s);
    }

    // 2. 查找绑定的 C++ 成员方法 Methods
    const auto &methods = obj->impl_->methods;
    const auto mit = methods.find(skey);
    if (mit != methods.end()) {
        auto &alloc = s->GetHeap().GetAllocator(false /* temp */);

        auto *uv0 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv0->type_ = static_cast<int>(VarType::Int);
        uv0->data_.i = reinterpret_cast<int64_t>(obj);

        auto *uv1 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv1->type_ = static_cast<int>(VarType::Int);
        uv1->data_.i = reinterpret_cast<int64_t>(s);

        auto *uv2 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv2->type_ = static_cast<int>(VarType::Int);
        uv2->data_.i = reinterpret_cast<int64_t>(&mit->second);

        auto *cl = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure) + 3 * sizeof(CVar *)));
        cl->func_ptr = reinterpret_cast<void *>(NativeMethodBridge);
        cl->upvalue_count = 3;
        cl->expected_arg_count = 1;
        cl->is_vararg = true;
        cl->upvalues[0] = uv0;
        cl->upvalues[1] = uv1;
        cl->upvalues[2] = uv2;

        CVar res{};
        res.type_ = static_cast<int>(VarType::Closure);
        res.data_.cl = cl;
        return res;
    }

    // 3. 字段与方法均未定义，默认返回 nil
    CVar nil_cvar{};
    nil_cvar.type_ = static_cast<int>(VarType::Nil);
    return nil_cvar;
}

void NativeSpecSet(VarTable *tbl, CVar k, CVar v, bool *finish) {
    auto *spec = static_cast<NativeObjectSpec *>(tbl->spec);
    NativeObject *obj = spec->obj;
    if (!obj || !obj->impl_) {
        *finish = true;
        return;
    }

    const std::string_view key = KeyToStringView(k);
    if (key.empty()) {
        *finish = false;
        return;
    }

    *finish = true;
    const std::string skey(key);

    if (v.type_ == static_cast<int>(VarType::Nil)) {
        obj->impl_->kv.erase(skey);
    } else {
        obj->impl_->kv[skey] = CVarToNativeField(v);
    }

    // 保持 spec_keys/spec_vals 快照与最新字段集合同步，使 pairs()/next() 迭代
    // 能够看到 Wrap() 之后通过 spec_set 写入/删除的字段。
    RefreshSpecKeys(tbl, obj, spec->state);
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeObject 公开实现
// ─────────────────────────────────────────────────────────────────────────────

NativeObject::NativeObject(std::string type_name) : impl_(new Impl{std::move(type_name)}) {
}

NativeObject::~NativeObject() {
    // 触发销毁回调，让拥有者释放关联的 C++ 资源（在 impl_ 仍有效时调用）。
    if (impl_ && impl_->finalizer) impl_->finalizer(this);
    delete impl_;
    impl_ = nullptr;
}

bool NativeObject::Alive() const {
    return impl_ != nullptr;
}

void NativeObject::SetFinalizer(const std::function<void(NativeObject *self)> &fn) {
    if (!impl_) return;
    impl_->finalizer = fn;
}

NativeObject *NativeObject::Create(int64_t group_id, std::string type_name, int64_t id) {
    // group_id == 0 表示全局对象（不属于任何 group），由 NativeObjectManager::GlobalCreate 使用。
    auto *obj = new NativeObject(std::move(type_name));
    obj->impl_->id = id;
    obj->impl_->group_id = group_id;
    return obj;
}

void NativeObject::Destroy(NativeObject *obj) {
    if (!obj || !obj->impl_) return;
    if (obj->impl_->finalizer) {
        auto fn = std::move(obj->impl_->finalizer);
        obj->impl_->finalizer = nullptr;
        fn(obj);
    }
    delete obj->impl_;
    obj->impl_ = nullptr;
}

CVar NativeObject::Wrap(State *s) const {
    if (!impl_ || !s) {
        return inter::NativeToFakeluaNil(s);
    }
    auto &alloc = s->GetHeap().GetAllocator(false /* temp */);

    // ── 分发 VarTable 壳（arena，帧内有效）──────────────────────────────────
    auto *vtbl = static_cast<VarTable *>(alloc.Alloc(sizeof(VarTable)));
    *vtbl = VarTable{};
    for (auto &qd: vtbl->quick_data_) {
        qd.key.type_ = static_cast<int>(VarType::Nil);
        qd.val.type_ = static_cast<int>(VarType::Nil);
    }
    vtbl->free_list_idx_ = VarTable::INVALID_INDEX;

    // ── 分发 NativeObjectSpec（arena，帧内有效）──────────────────────────────
    auto *spec = static_cast<NativeObjectSpec *>(alloc.Alloc(sizeof(NativeObjectSpec)));
    spec->obj = const_cast<NativeObject *>(this);// C++ 堆，跨帧持久
    spec->state = s;

    vtbl->spec = spec;
    vtbl->spec_get = reinterpret_cast<void *>(NativeSpecGet);
    vtbl->spec_set = reinterpret_cast<void *>(NativeSpecSet);

    // ── 填充 spec_keys / spec_vals（供 pairs() 迭代）─────────────────────────
    RefreshSpecKeys(vtbl, this, s);

    CVar r{};
    r.type_ = static_cast<int>(VarType::Table);
    r.data_.t = vtbl;
    return r;
}

NativeObject *NativeObject::Unwrap(CVar v) {
    if (v.type_ != static_cast<int>(VarType::Table)) return nullptr;
    const VarTable *tbl = v.data_.t;
    if (!tbl || tbl->spec_get != reinterpret_cast<void *>(NativeSpecGet)) return nullptr;
    auto *obj = static_cast<NativeObjectSpec *>(tbl->spec)->obj;
    if (!obj || !obj->impl_) return nullptr;
    return obj;
}

const std::string &NativeObject::GetTypeName() const {
    static const std::string kDead;
    if (!impl_) return kDead;
    return impl_->type_name;
}

bool NativeObject::Has(std::string_view key) const {
    if (!impl_) return false;
    return impl_->kv.count(std::string(key)) > 0;
}

void NativeObject::Del(std::string_view key) {
    if (!impl_) return;
    impl_->kv.erase(std::string(key));
}

void NativeObject::Clear() {
    if (!impl_) return;
    impl_->kv.clear();
    impl_->methods.clear();
}

void NativeObject::RegisterMethod(std::string_view name, NativeMethod method) {
    if (!impl_) return;
    impl_->methods[std::string(name)] = std::move(method);
}

bool NativeObject::HasMethod(std::string_view name) const {
    if (!impl_) return false;
    return impl_->methods.contains(std::string(name));
}

void NativeObject::UnregisterMethod(std::string_view name) {
    if (!impl_) return;
    impl_->methods.erase(std::string(name));
}

size_t NativeObject::Size() const {
    return impl_ ? impl_->kv.size() : 0;
}

// ── Set 系列 ─────────────────────────────────────────────────────────────────
void NativeObject::SetNil(std::string_view key) {
    if (!impl_) return;
    impl_->kv.erase(std::string(key));
}

void NativeObject::SetInt(std::string_view key, int64_t val) {
    if (!impl_) return;
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::Int;
    f.i = val;
}

void NativeObject::SetFloat(std::string_view key, double val) {
    if (!impl_) return;
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::Float;
    f.f = val;
}

void NativeObject::SetBool(std::string_view key, bool val) {
    if (!impl_) return;
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::Bool;
    f.b = val;
}

void NativeObject::SetString(std::string_view key, std::string_view val) {
    if (!impl_) return;
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::String;
    f.s = std::string(val);
    f.vs_dirty = true;
}

void NativeObject::SetObject(std::string_view key, NativeObject *obj) {
    if (!impl_) return;
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::Object;
    f.obj = obj;
}

void NativeObject::SetFromCVar(std::string_view key, CVar v) {
    if (!impl_) return;
    impl_->kv[std::string(key)] = CVarToNativeField(v);
}

// ── Get 系列 ─────────────────────────────────────────────────────────────────
int64_t NativeObject::GetInt(std::string_view key, int64_t def) const {
    if (!impl_) return def;
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return def;
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::Int) return f.i;
    if (f.kind == NativeField::Kind::Float) {
        int64_t iv = 0;
        if (DoubleFitsInt64(f.f, &iv)) return iv;
        if (!std::isfinite(f.f) || f.f < static_cast<double>(INT64_MIN) ||
            f.f >= static_cast<double>(INT64_MAX) + 1.0) {
            return def;
        }
        return static_cast<int64_t>(f.f);
    }
    return def;
}

double NativeObject::GetFloat(std::string_view key, double def) const {
    if (!impl_) return def;
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return def;
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::Float) return f.f;
    if (f.kind == NativeField::Kind::Int) return static_cast<double>(f.i);
    return def;
}

bool NativeObject::GetBool(std::string_view key, bool def) const {
    if (!impl_) return def;
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return def;
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::Bool) return f.b;
    return def;
}

std::string NativeObject::GetString(std::string_view key, std::string_view def) const {
    if (!impl_) return std::string(def);
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return std::string(def);
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::String) return f.s;
    return std::string(def);
}

NativeObject *NativeObject::GetObject(std::string_view key) const {
    if (!impl_) return nullptr;
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return nullptr;
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::Object) return f.obj;
    return nullptr;
}

CVar NativeObject::GetAsCVar(std::string_view key, State *s) const {
    if (!impl_) {
        CVar r{};
        r.type_ = static_cast<int>(VarType::Nil);
        return r;
    }
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) {
        CVar r{};
        r.type_ = static_cast<int>(VarType::Nil);
        return r;
    }
    return NativeFieldToCVar(it->second, s);
}

int64_t NativeObject::GetId() const {
    return impl_ ? impl_->id : 0;
}

void NativeObject::SetId(int64_t id) {
    if (!impl_) return;
    impl_->id = id;
}

int64_t NativeObject::GetGroupId() const {
    return impl_ ? impl_->group_id : 0;
}

void NativeObject::SetGroupId(int64_t group_id) {
    if (!impl_) return;
    impl_->group_id = group_id;
}

// ── Iterate（只读快照）──────────────────────────────────────────────────────
void NativeObject::ForEach(const std::function<void(std::string_view, NativeObject::FieldKind)> &fn) const {
    if (!impl_) return;
    for (const auto &[k, v]: impl_->kv) {
        fn(k, static_cast<NativeObject::FieldKind>(static_cast<int>(v.kind)));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// RegisterNativeObjectApi — 自动注册内置原生对象 API：
//   - new_native_obj(type, id, [group_id]) -> NativeObject (Wrap 壳)
//   - get_native_obj(type, id) -> NativeObject (Wrap 壳) 或 nil
//   - del_native_obj(type, id) -> bool
//   - del_native_group(group_id) -> count (批处理销毁整个组空间的所有对象)
// ─────────────────────────────────────────────────────────────────────────────

void RegisterNativeObjectApi(State *s) {
    // new_native_group() -> group_id (由系统统一自增分配唯一 group_id)
    RegisterNativeFunction(s, "new_native_group", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
        int64_t gid = NativeObjectManager::Instance().CreateGroup();
        return inter::NativeToFakeluaInt(state, gid);
    });

    // new_native_obj(group_id, type, id) -> NativeObject (Wrap 壳)
    RegisterNativeFunction(s, "new_native_obj", 3, false, [](State *state, CVar *args, int n) -> CVar {
        CVar arg0 = inter::GetNativeArg(state, args, n, 0);
        CVar arg1 = inter::GetNativeArg(state, args, n, 1);
        CVar arg2 = inter::GetNativeArg(state, args, n, 2);

        if (arg0.type_ == static_cast<int>(VarType::Nil)) {
            ThrowFakeluaException("new_native_obj failed: group_id is required");
        }

        int64_t group_id = inter::FakeluaToNative<int64_t>(state, arg0);
        std::string type_name = inter::FakeluaToNative<std::string>(state, arg1);
        int64_t id = inter::FakeluaToNative<int64_t>(state, arg2);

        NativeObject *obj = NativeObjectManager::Instance().Create(group_id, type_name, id);
        return obj->Wrap(state);
    });

    // get_native_obj(type, id) -> NativeObject (Wrap 壳) 或 nil
    RegisterNativeFunction(s, "get_native_obj", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar arg0 = inter::GetNativeArg(state, args, n, 0);
        CVar arg1 = inter::GetNativeArg(state, args, n, 1);

        if (arg0.type_ == static_cast<int>(VarType::Nil) || arg1.type_ == static_cast<int>(VarType::Nil)) {
            return inter::NativeToFakeluaNil(state);
        }

        std::string type_name = inter::FakeluaToNative<std::string>(state, arg0);
        int64_t id = inter::FakeluaToNative<int64_t>(state, arg1);

        NativeObject *obj = NativeObjectManager::Instance().Get(type_name, id);
        if (!obj) {
            return inter::NativeToFakeluaNil(state);
        }
        return obj->Wrap(state);
    });

    // del_native_group(group_id) -> count (批处理一口气注销释放整个 group_id 下的所有对象)
    RegisterNativeFunction(s, "del_native_group", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar arg0 = inter::GetNativeArg(state, args, n, 0);
        int64_t group_id = (arg0.type_ != static_cast<int>(VarType::Nil)) ? inter::FakeluaToNative<int64_t>(state, arg0) : 0;

        size_t count = NativeObjectManager::Instance().DestroyGroup(group_id);
        return inter::NativeToFakeluaInt(state, static_cast<int64_t>(count));
    });

    // new_global_obj(key, type) -> NativeObject (通过 string key 创建全局对象，无需 group_id)
    RegisterNativeFunction(s, "new_global_obj", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar arg0 = inter::GetNativeArg(state, args, n, 0);
        CVar arg1 = inter::GetNativeArg(state, args, n, 1);

        std::string key = inter::FakeluaToNative<std::string>(state, arg0);
        std::string type_name = inter::FakeluaToNative<std::string>(state, arg1);

        NativeObject *obj = NativeObjectManager::Instance().GlobalCreate(key, type_name);
        return obj->Wrap(state);
    });

    // get_global_obj(key) -> NativeObject (Wrap 壳) 或 nil (按 string key 查找全局对象)
    RegisterNativeFunction(s, "get_global_obj", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar arg0 = inter::GetNativeArg(state, args, n, 0);
        std::string key = inter::FakeluaToNative<std::string>(state, arg0);

        NativeObject *obj = NativeObjectManager::Instance().GlobalGet(key);
        if (!obj) {
            return inter::NativeToFakeluaNil(state);
        }
        return obj->Wrap(state);
    });

    // del_global_obj(key) -> bool (按 string key 销毁单个全局对象)
    RegisterNativeFunction(s, "del_global_obj", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar arg0 = inter::GetNativeArg(state, args, n, 0);
        std::string key = inter::FakeluaToNative<std::string>(state, arg0);

        bool ok = NativeObjectManager::Instance().GlobalDestroy(key);
        return inter::NativeToFakeluaBool(state, ok);
    });

    // 自动注册内置标准库 API
    math::RegisterMathLibraryApi(s);
    table::RegisterTableLibraryApi(s);
    string::RegisterStringLibraryApi(s);
    basic::RegisterBasicLibraryApi(s);
    os::RegisterOsLibraryApi(s);
    utf8::RegisterUtf8LibraryApi(s);
    io::RegisterIoLibraryApi(s);
}

}// namespace fakelua
