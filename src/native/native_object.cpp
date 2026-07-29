#include "native/native_object.h"
#include "jit/vm.h"
#include "var/var_multi.h"

#include <algorithm>
#include <cmath>
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
        const VarString *vs = (t == static_cast<int>(VarType::String)) ? v.data_.s : reinterpret_cast<VarString *>(v.data_.i);
        f.s = std::string(vs->Str());
        f.vs_dirty = true;
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

int64_t NativeObjectManager::CreateGroup(int64_t specified_group_id) {
    int64_t gid = (specified_group_id != 0) ? specified_group_id : ++next_auto_group_id_;
    if (!group_objects_.contains(gid)) {
        group_objects_[gid] = {};
    }
    return gid;
}

NativeObject *NativeObjectManager::Create(int64_t group_id, const std::string &type_name, int64_t id) {
    if (group_id == 0) {
        ThrowFakeluaException("NativeObjectManager::Create failed: group_id must be specified and non-zero");
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

        NativeObject::Destroy(obj);
        objects_.erase(it);
        return true;
    }
    return false;
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
            count++;
        }
    }
    return count;
}

void NativeObjectManager::Clear() {
    for (auto &[k, v]: objects_) {
        NativeObject::Destroy(v);
    }
    objects_.clear();
    group_objects_.clear();
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

    if (!obj || !method_ptr || !(*method_ptr)) {
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

CVar NativeSpecGet(VarTable *tbl, CVar k, bool *finish) {
    auto *spec = static_cast<NativeObjectSpec *>(tbl->spec);
    NativeObject *obj = spec->obj;
    State *s = spec->state;

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
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeObject 公开实现
// ─────────────────────────────────────────────────────────────────────────────

NativeObject::NativeObject(std::string type_name) : impl_(new Impl{std::move(type_name)}) {
}

NativeObject::~NativeObject() {
    delete impl_;
}

NativeObject *NativeObject::Create(int64_t group_id, std::string type_name, int64_t id) {
    if (group_id == 0) {
        ThrowFakeluaException("NativeObject::Create failed: group_id must be specified and non-zero. Objects can only be created within a group.");
    }
    auto *obj = new NativeObject(std::move(type_name));
    obj->impl_->id = id;
    obj->impl_->group_id = group_id;
    return obj;
}

void NativeObject::Destroy(NativeObject *obj) {
    delete obj;
}

CVar NativeObject::Wrap(State *s) const {
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
    const size_t n = impl_->kv.size();
    if (n > 0) {
        vtbl->spec_keys = static_cast<CVar *>(alloc.Alloc(sizeof(CVar) * n));
        vtbl->spec_vals = static_cast<CVar *>(alloc.Alloc(sizeof(CVar) * n));
        vtbl->spec_count = static_cast<uint32_t>(n);

        size_t i = 0;
        for (const auto &[k, v]: impl_->kv) {
            // key：在 arena 中分配 VarString
            const size_t klen = k.size();
            auto *vs = static_cast<VarString *>(alloc.Alloc(sizeof(VarString) + klen));
            // 直接写 POD 字段
            *reinterpret_cast<int *>(vs) = static_cast<int>(klen);
            *reinterpret_cast<uint32_t *>(reinterpret_cast<char *>(vs) + sizeof(int)) = 0u;
            if (klen > 0) {
                std::memcpy(reinterpret_cast<char *>(vs) + sizeof(VarString), k.data(), klen);
            }

            vtbl->spec_keys[i].type_ = static_cast<int>(VarType::String);
            vtbl->spec_keys[i].flag_ = 0;
            vtbl->spec_keys[i].data_.s = vs;

            vtbl->spec_vals[i] = NativeFieldToCVar(v, s);
            ++i;
        }
    }

    CVar r{};
    r.type_ = static_cast<int>(VarType::Table);
    r.data_.t = vtbl;
    return r;
}

NativeObject *NativeObject::Unwrap(CVar v) {
    if (v.type_ != static_cast<int>(VarType::Table)) return nullptr;
    const VarTable *tbl = v.data_.t;
    if (!tbl || tbl->spec_get != reinterpret_cast<void *>(NativeSpecGet)) return nullptr;
    return static_cast<NativeObjectSpec *>(tbl->spec)->obj;
}

const std::string &NativeObject::GetTypeName() const {
    return impl_->type_name;
}

bool NativeObject::Has(std::string_view key) const {
    return impl_->kv.count(std::string(key)) > 0;
}

void NativeObject::Del(std::string_view key) {
    impl_->kv.erase(std::string(key));
}

void NativeObject::Clear() {
    impl_->kv.clear();
    impl_->methods.clear();
}

void NativeObject::RegisterMethod(std::string_view name, NativeMethod method) {
    impl_->methods[std::string(name)] = std::move(method);
}

bool NativeObject::HasMethod(std::string_view name) const {
    return impl_->methods.contains(std::string(name));
}

void NativeObject::UnregisterMethod(std::string_view name) {
    impl_->methods.erase(std::string(name));
}

size_t NativeObject::Size() const {
    return impl_->kv.size();
}

// ── Set 系列 ─────────────────────────────────────────────────────────────────
void NativeObject::SetNil(std::string_view key) {
    impl_->kv.erase(std::string(key));
}

void NativeObject::SetInt(std::string_view key, int64_t val) {
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::Int;
    f.i = val;
}

void NativeObject::SetFloat(std::string_view key, double val) {
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::Float;
    f.f = val;
}

void NativeObject::SetBool(std::string_view key, bool val) {
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::Bool;
    f.b = val;
}

void NativeObject::SetString(std::string_view key, std::string_view val) {
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::String;
    f.s = std::string(val);
    f.vs_dirty = true;
}

void NativeObject::SetObject(std::string_view key, NativeObject *obj) {
    auto &f = impl_->kv[std::string(key)];
    f.kind = NativeField::Kind::Object;
    f.obj = obj;
}

void NativeObject::SetFromCVar(std::string_view key, CVar v) {
    impl_->kv[std::string(key)] = CVarToNativeField(v);
}

// ── Get 系列 ─────────────────────────────────────────────────────────────────
int64_t NativeObject::GetInt(std::string_view key, int64_t def) const {
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return def;
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::Int) return f.i;
    if (f.kind == NativeField::Kind::Float) return static_cast<int64_t>(f.f);
    return def;
}

double NativeObject::GetFloat(std::string_view key, double def) const {
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return def;
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::Float) return f.f;
    if (f.kind == NativeField::Kind::Int) return static_cast<double>(f.i);
    return def;
}

bool NativeObject::GetBool(std::string_view key, bool def) const {
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return def;
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::Bool) return f.b;
    return def;
}

std::string NativeObject::GetString(std::string_view key, std::string_view def) const {
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return std::string(def);
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::String) return f.s;
    return std::string(def);
}

NativeObject *NativeObject::GetObject(std::string_view key) const {
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) return nullptr;
    const auto &f = it->second;
    if (f.kind == NativeField::Kind::Object) return f.obj;
    return nullptr;
}

CVar NativeObject::GetAsCVar(std::string_view key, State *s) const {
    const auto it = impl_->kv.find(std::string(key));
    if (it == impl_->kv.end()) {
        CVar r{};
        r.type_ = static_cast<int>(VarType::Nil);
        return r;
    }
    return NativeFieldToCVar(it->second, s);
}

int64_t NativeObject::GetId() const {
    return impl_->id;
}

void NativeObject::SetId(int64_t id) {
    impl_->id = id;
}

int64_t NativeObject::GetGroupId() const {
    return impl_->group_id;
}

void NativeObject::SetGroupId(int64_t group_id) {
    impl_->group_id = group_id;
}

// ── Iterate（只读快照）──────────────────────────────────────────────────────
void NativeObject::ForEach(const std::function<void(std::string_view, NativeObject::FieldKind)> &fn) const {
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
    // new_native_group([group_id]) -> group_id
    RegisterNativeFunction(s, "new_native_group", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar arg0 = inter::GetNativeArg(state, args, n, 0);
        int64_t specified_gid = (arg0.type_ != static_cast<int>(VarType::Nil)) ? inter::FakeluaToNative<int64_t>(state, arg0) : 0;
        int64_t gid = NativeObjectManager::Instance().CreateGroup(specified_gid);
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
}

void RegisterMathLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "math.abs", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Int)) return inter::NativeToFakeluaInt(state, std::abs(a0.data_.i));
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::abs(a0.data_.f));
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.floor", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::floor(a0.data_.f));
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.ceil", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::ceil(a0.data_.f));
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.max", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
        return (v0 >= v1) ? a0 : a1;
    });

    RegisterNativeFunction(s, "math.min", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
        return (v0 <= v1) ? a0 : a1;
    });

    RegisterNativeFunction(s, "math.sqrt", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::sqrt(v0));
    });

    RegisterNativeFunction(s, "math.sin", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::sin(v0));
    });

    RegisterNativeFunction(s, "math.cos", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::cos(v0));
    });

    RegisterNativeFunction(s, "math.tan", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::tan(v0));
    });

    RegisterNativeFunction(s, "math.pow", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::pow(v0, v1));
    });

    RegisterNativeFunction(s, "math.asin", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::asin(v0));
    });

    RegisterNativeFunction(s, "math.acos", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::acos(v0));
    });

    RegisterNativeFunction(s, "math.atan", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
            return inter::NativeToFakeluaFloat(state, std::atan2(v0, v1));
        }
        return inter::NativeToFakeluaFloat(state, std::atan(v0));
    });

    RegisterNativeFunction(s, "math.exp", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::exp(v0));
    });

    RegisterNativeFunction(s, "math.log", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
            return inter::NativeToFakeluaFloat(state, std::log(v0) / std::log(v1));
        }
        return inter::NativeToFakeluaFloat(state, std::log(v0));
    });

    RegisterNativeFunction(s, "math.log10", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::log10(v0));
    });

    RegisterNativeFunction(s, "math.sinh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::sinh(v0));
    });

    RegisterNativeFunction(s, "math.cosh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::cosh(v0));
    });

    RegisterNativeFunction(s, "math.tanh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::tanh(v0));
    });

    RegisterNativeFunction(s, "math.fmod", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::fmod(v0, v1));
    });

    RegisterNativeFunction(s, "math.ldexp", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        int v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? static_cast<int>(a1.data_.f) : 0);
        return inter::NativeToFakeluaFloat(state, std::ldexp(v0, v1));
    });

    RegisterNativeFunction(s, "math.type", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Int)) return inter::NativeToFakeluaStringView(state, "integer");
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaStringView(state, "float");
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.tointeger", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        if (a0.type_ == static_cast<int>(VarType::Float)) {
            double f = a0.data_.f;
            if (static_cast<double>(static_cast<int64_t>(f)) == f) {
                return inter::NativeToFakeluaInt(state, static_cast<int64_t>(f));
            }
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.ult", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        uint64_t u0 = (a0.type_ == static_cast<int>(VarType::Int)) ? static_cast<uint64_t>(a0.data_.i) : 0;
        uint64_t u1 = (a1.type_ == static_cast<int>(VarType::Int)) ? static_cast<uint64_t>(a1.data_.i) : 0;
        return inter::NativeToFakeluaBool(state, u0 < u1);
    });
}

bool TableHelper::VarKeyEqualInt(CVar k, int64_t idx) {
    if (k.type_ == static_cast<int>(VarType::Int)) {
        return k.data_.i == idx;
    }
    if (k.type_ == static_cast<int>(VarType::Float)) {
        return k.data_.f == static_cast<double>(idx);
    }
    return false;
}

int64_t TableHelper::GetTableLen(CVar tbl) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return 0;
    VarTable *t = tbl.data_.t;
    int64_t max_idx = static_cast<int64_t>(t->spec_count);
    for (const auto &qd: t->quick_data_) {
        if (qd.key.type_ == static_cast<int>(VarType::Int)) {
            if (qd.key.data_.i > max_idx) max_idx = qd.key.data_.i;
        }
    }
    if (t->nodes_ && t->bucket_count_ > 0) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            uint32_t node_idx = t->active_list_[i];
            const auto &entry = t->nodes_[node_idx].entry;
            if (entry.key.type_ == static_cast<int>(VarType::Int)) {
                if (entry.key.data_.i > max_idx) max_idx = entry.key.data_.i;
            }
        }
    }
    return max_idx;
}

CVar TableHelper::GetTableInt(State *s, CVar tbl, int64_t idx) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return CVar{static_cast<int>(VarType::Nil)};
    VarTable *t = tbl.data_.t;

    if (t->spec_count > 0 && t->spec_vals && t->spec_keys) {
        for (uint32_t i = 0; i < t->spec_count; ++i) {
            if (VarKeyEqualInt(t->spec_keys[i], idx)) {
                return t->spec_vals[i];
            }
        }
    }

    for (const auto &qd: t->quick_data_) {
        if (VarKeyEqualInt(qd.key, idx)) {
            return qd.val;
        }
    }

    if (t->nodes_ && t->bucket_count_ > 0) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            uint32_t node_idx = t->active_list_[i];
            const auto &entry = t->nodes_[node_idx].entry;
            if (VarKeyEqualInt(entry.key, idx)) {
                return entry.val;
            }
        }
    }
    return CVar{static_cast<int>(VarType::Nil)};
}

void TableHelper::SetTableInt(State *s, CVar tbl, int64_t idx, CVar val) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return;
    VarTable *t = tbl.data_.t;
    CVar key{static_cast<int>(VarType::Int)};
    key.data_.i = idx;
    uint32_t hash = static_cast<uint32_t>(idx ^ (idx >> 32));

    if (t->spec_count > 0 && t->spec_vals && t->spec_keys) {
        for (uint32_t i = 0; i < t->spec_count; ++i) {
            if (VarKeyEqualInt(t->spec_keys[i], idx)) {
                t->spec_vals[i] = val;
                return;
            }
        }
    }

    for (auto &qd: t->quick_data_) {
        if (VarKeyEqualInt(qd.key, idx)) {
            static_cast<CVar &>(qd.val) = val;
            qd.hash = hash;
            return;
        }
    }

    for (auto &qd: t->quick_data_) {
        if (qd.key.type_ == static_cast<int>(VarType::Nil)) {
            static_cast<CVar &>(qd.key) = key;
            static_cast<CVar &>(qd.val) = val;
            qd.hash = hash;
            t->count_++;
            return;
        }
    }
}

void TableHelper::SetTableStrId(State *s, CVar tbl, const char *str_key, CVar val) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return;
    VarTable *t = tbl.data_.t;
    int64_t id = s->GetConstString().Alloc(str_key);
    auto *vs = reinterpret_cast<const VarString *>(id);
    uint32_t hash = vs->Hash();

    CVar key{static_cast<int>(VarType::StringId)};
    key.data_.i = id;

    for (auto &qd: t->quick_data_) {
        if (qd.key.type_ == static_cast<int>(VarType::StringId) && qd.key.data_.i == id) {
            static_cast<CVar &>(qd.val) = val;
            return;
        }
    }

    for (auto &qd: t->quick_data_) {
        if (qd.key.type_ == static_cast<int>(VarType::Nil)) {
            static_cast<CVar &>(qd.key) = key;
            static_cast<CVar &>(qd.val) = val;
            qd.hash = hash;
            t->count_++;
            return;
        }
    }
}

void RegisterTableLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "table.insert", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        int64_t len = TableHelper::GetTableLen(tbl);

        if (n == 1) return inter::NativeToFakeluaNil(state);
        if (n == 2) {
            CVar val = inter::GetNativeArg(state, args, n, 1);
            TableHelper::SetTableInt(state, tbl, len + 1, val);
        } else {
            CVar pos_var = inter::GetNativeArg(state, args, n, 1);
            CVar val = inter::GetNativeArg(state, args, n, 2);
            int64_t pos = (pos_var.type_ == static_cast<int>(VarType::Int)) ? pos_var.data_.i : 1;
            for (int64_t i = len; i >= pos; --i) {
                CVar item = TableHelper::GetTableInt(state, tbl, i);
                TableHelper::SetTableInt(state, tbl, i + 1, item);
            }
            TableHelper::SetTableInt(state, tbl, pos, val);
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "table.remove", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        int64_t len = TableHelper::GetTableLen(tbl);
        int64_t pos = len;
        if (n >= 2) {
            CVar pos_var = inter::GetNativeArg(state, args, n, 1);
            if (pos_var.type_ == static_cast<int>(VarType::Int)) pos = pos_var.data_.i;
        }
        if (pos < 1 || pos > len) return inter::NativeToFakeluaNil(state);

        CVar removed = TableHelper::GetTableInt(state, tbl, pos);
        for (int64_t i = pos; i < len; ++i) {
            CVar next_val = TableHelper::GetTableInt(state, tbl, i + 1);
            TableHelper::SetTableInt(state, tbl, i, next_val);
        }
        TableHelper::SetTableInt(state, tbl, len, inter::NativeToFakeluaNil(state));
        return removed;
    });

    RegisterNativeFunction(s, "table.concat", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        std::string sep = "";
        if (n >= 2) {
            CVar sep_var = inter::GetNativeArg(state, args, n, 1);
            auto sv = KeyToStringView(sep_var);
            if (!sv.empty()) sep = std::string(sv);
        }
        int64_t start_i = 1;
        if (n >= 3) {
            CVar i_var = inter::GetNativeArg(state, args, n, 2);
            if (i_var.type_ == static_cast<int>(VarType::Int)) start_i = i_var.data_.i;
        }
        int64_t end_j = TableHelper::GetTableLen(tbl);
        if (n >= 4) {
            CVar j_var = inter::GetNativeArg(state, args, n, 3);
            if (j_var.type_ == static_cast<int>(VarType::Int)) end_j = j_var.data_.i;
        }

        std::string res;
        for (int64_t idx = start_i; idx <= end_j; ++idx) {
            if (idx > start_i) res += sep;
            CVar item = TableHelper::GetTableInt(state, tbl, idx);
            if (item.type_ == static_cast<int>(VarType::Int)) {
                res += std::to_string(item.data_.i);
            } else if (item.type_ == static_cast<int>(VarType::Float)) {
                res += std::to_string(item.data_.f);
            } else {
                auto sv = KeyToStringView(item);
                if (!sv.empty()) res += std::string(sv);
            }
        }
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "table.unpack", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        int64_t start_i = 1;
        if (n >= 2) {
            CVar i_var = inter::GetNativeArg(state, args, n, 1);
            if (i_var.type_ == static_cast<int>(VarType::Int)) start_i = i_var.data_.i;
        }
        int64_t end_j = TableHelper::GetTableLen(tbl);
        if (n >= 3) {
            CVar j_var = inter::GetNativeArg(state, args, n, 2);
            if (j_var.type_ == static_cast<int>(VarType::Int)) end_j = j_var.data_.i;
        }

        if (start_i > end_j) return inter::NativeToFakeluaNil(state);
        int count = static_cast<int>(end_j - start_i + 1);
        CVar multi = inter::AllocMultiCVar(state, count);
        for (int i = 0; i < count; ++i) {
            CVar item = TableHelper::GetTableInt(state, tbl, start_i + i);
            inter::SetMultiCVarElement(multi, i, item);
        }
        return multi;
    });

    RegisterNativeFunction(s, "table.pack", 0, true, [](State *state, CVar *args, int n) -> CVar {
        VarTable *vtbl = static_cast<VarTable *>(FakeluaAlloc(state, sizeof(VarTable), false));
        *vtbl = VarTable{};
        for (auto &qd: vtbl->quick_data_) {
            qd.key.type_ = static_cast<int>(VarType::Nil);
            qd.val.type_ = static_cast<int>(VarType::Nil);
        }
        vtbl->free_list_idx_ = VarTable::INVALID_INDEX;

        CVar tbl_cvar{};
        tbl_cvar.type_ = static_cast<int>(VarType::Table);
        tbl_cvar.data_.t = vtbl;

        for (int i = 0; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            TableHelper::SetTableInt(state, tbl_cvar, i + 1, arg_i);
        }

        TableHelper::SetTableStrId(state, tbl_cvar, "n", inter::NativeToFakeluaInt(state, n));
        return tbl_cvar;
    });

    RegisterNativeFunction(s, "table.move", 4, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 4) return inter::NativeToFakeluaNil(state);
        CVar a1 = inter::GetNativeArg(state, args, n, 0);
        CVar f_var = inter::GetNativeArg(state, args, n, 1);
        CVar e_var = inter::GetNativeArg(state, args, n, 2);
        CVar t_var = inter::GetNativeArg(state, args, n, 3);
        CVar a2 = (n >= 5) ? inter::GetNativeArg(state, args, n, 4) : a1;

        int64_t f = (f_var.type_ == static_cast<int>(VarType::Int)) ? f_var.data_.i : 1;
        int64_t e = (e_var.type_ == static_cast<int>(VarType::Int)) ? e_var.data_.i : 0;
        int64_t t = (t_var.type_ == static_cast<int>(VarType::Int)) ? t_var.data_.i : 1;

        if (e >= f) {
            int64_t count = e - f + 1;
            if (t <= f || t > e) {
                for (int64_t i = 0; i < count; ++i) {
                    CVar val = TableHelper::GetTableInt(state, a1, f + i);
                    TableHelper::SetTableInt(state, a2, t + i, val);
                }
            } else {
                for (int64_t i = count - 1; i >= 0; --i) {
                    CVar val = TableHelper::GetTableInt(state, a1, f + i);
                    TableHelper::SetTableInt(state, a2, t + i, val);
                }
            }
        }
        return a2;
    });
}

}// namespace fakelua
