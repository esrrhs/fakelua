#include "native/container/native_container.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "var/var.h"

#include <boost/container/deque.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <cstdint>
#include <format>
#include <string>
#include <utility>

namespace fakelua::container {

namespace {

struct ContainerKey {
    enum class Kind : uint8_t { Nil, Bool, Int, Float, String };
    Kind kind = Kind::Nil;
    bool b = false;
    int64_t i = 0;
    double f = 0;
    std::string s;

    static ContainerKey FromCVar(CVar v) {
        const int t = v.type_;
        if (t == static_cast<int>(VarType::Table) || t == static_cast<int>(VarType::Closure) || t == static_cast<int>(VarType::Multi)) {
            ThrowFakeluaException("container map/set keys must be nil, boolean, number, or string");
        }
        NativeField nf = CVarToNativeField(v);
        ContainerKey k;
        switch (nf.kind) {
            case NativeField::Kind::Nil:
                k.kind = Kind::Nil;
                break;
            case NativeField::Kind::Bool:
                k.kind = Kind::Bool;
                k.b = nf.b;
                break;
            case NativeField::Kind::Int:
                k.kind = Kind::Int;
                k.i = nf.i;
                break;
            case NativeField::Kind::Float: {
                int64_t iv = 0;
                if (DoubleFitsInt64(nf.f, &iv)) {
                    k.kind = Kind::Int;
                    k.i = iv;
                } else {
                    k.kind = Kind::Float;
                    k.f = nf.f;
                }
                break;
            }
            case NativeField::Kind::String:
                k.kind = Kind::String;
                k.s = std::move(nf.s);
                break;
            default:
                ThrowFakeluaException("container map/set keys must be nil, boolean, number, or string");
        }
        return k;
    }

    CVar ToCVar(State *state) const {
        switch (kind) {
            case Kind::Nil:
                return inter::NativeToFakeluaNil(state);
            case Kind::Bool:
                return inter::NativeToFakeluaBool(state, b);
            case Kind::Int:
                return inter::NativeToFakeluaLonglong(state, i);
            case Kind::Float:
                return inter::NativeToFakeluaDouble(state, f);
            case Kind::String:
                return inter::NativeToFakeluaString(state, s);
        }
        return inter::NativeToFakeluaNil(state);
    }

    friend bool operator<(const ContainerKey &a, const ContainerKey &b) {
        if (a.kind != b.kind) {
            return static_cast<uint8_t>(a.kind) < static_cast<uint8_t>(b.kind);
        }
        switch (a.kind) {
            case Kind::Nil:
                return false;
            case Kind::Bool:
                return a.b < b.b;
            case Kind::Int:
                return a.i < b.i;
            case Kind::Float:
                return a.f < b.f;
            case Kind::String:
                return a.s < b.s;
        }
        return false;
    }
};

struct ContainerImpl {
    enum class Kind { Deque, Map, Set } kind = Kind::Deque;
    boost::container::deque<NativeField> deque;
    boost::container::flat_map<ContainerKey, NativeField> map;
    boost::container::flat_set<ContainerKey> set;
};

static constexpr const char *kPtrKey = "__container__";

ContainerImpl *Unwrap(NativeObject *self) {
    if (!self) return nullptr;
    return reinterpret_cast<ContainerImpl *>(self->GetInt(kPtrKey, 0));
}

ContainerImpl *Require(NativeObject *self) {
    ContainerImpl *c = Unwrap(self);
    if (!c) ThrowFakeluaException("container is closed");
    return c;
}

NativeField PersistValue(CVar v, const char *what) {
    const int t = v.type_;
    if (t == static_cast<int>(VarType::Table) && NativeObject::Unwrap(v) == nullptr) {
        ThrowFakeluaException(std::format("{} cannot be a Lua table", what));
    }
    if (t == static_cast<int>(VarType::Closure) || t == static_cast<int>(VarType::Multi)) {
        ThrowFakeluaException(std::format("{} cannot be a function", what));
    }
    return CVarToNativeField(v);
}

void Attach(NativeObject *nat, ContainerImpl *impl) {
    nat->SetInt(kPtrKey, reinterpret_cast<int64_t>(impl));
    nat->SetFinalizer([](NativeObject *self) {
        auto *c = Unwrap(self);
        self->SetInt(kPtrKey, 0);
        delete c;
    });
}

CVar Close(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    if (self) {
        s->GetNativeObjectManager().DestroyGroup(self->GetGroupId());
    }
    return inter::NativeToFakeluaNil(s);
}

CVar SizeOf(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *c = Require(self);
    int64_t n = 0;
    switch (c->kind) {
        case ContainerImpl::Kind::Deque:
            n = static_cast<int64_t>(c->deque.size());
            break;
        case ContainerImpl::Kind::Map:
            n = static_cast<int64_t>(c->map.size());
            break;
        case ContainerImpl::Kind::Set:
            n = static_cast<int64_t>(c->set.size());
            break;
    }
    return inter::NativeToFakeluaLonglong(s, n);
}

CVar EmptyOf(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *c = Require(self);
    bool empty = true;
    switch (c->kind) {
        case ContainerImpl::Kind::Deque:
            empty = c->deque.empty();
            break;
        case ContainerImpl::Kind::Map:
            empty = c->map.empty();
            break;
        case ContainerImpl::Kind::Set:
            empty = c->set.empty();
            break;
    }
    return inter::NativeToFakeluaBool(s, empty);
}

CVar ClearOf(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto *c = Require(self);
    switch (c->kind) {
        case ContainerImpl::Kind::Deque:
            c->deque.clear();
            break;
        case ContainerImpl::Kind::Map:
            c->map.clear();
            break;
        case ContainerImpl::Kind::Set:
            c->set.clear();
            break;
    }
    return inter::NativeToFakeluaNil(s);
}

// --- deque ---

CVar DequePushBack(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "deque:push_back", "value expected");
    Require(self)->deque.push_back(PersistValue(inter::GetNativeArg(s, args, n, 0), "deque:push_back"));
    return inter::NativeToFakeluaNil(s);
}

CVar DequePushFront(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "deque:push_front", "value expected");
    Require(self)->deque.push_front(PersistValue(inter::GetNativeArg(s, args, n, 0), "deque:push_front"));
    return inter::NativeToFakeluaNil(s);
}

CVar DequePopBack(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto &d = Require(self)->deque;
    if (d.empty()) return inter::NativeToFakeluaNil(s);
    NativeField v = std::move(d.back());
    d.pop_back();
    return NativeFieldToCVar(v, s);
}

CVar DequePopFront(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto &d = Require(self)->deque;
    if (d.empty()) return inter::NativeToFakeluaNil(s);
    NativeField v = std::move(d.front());
    d.pop_front();
    return NativeFieldToCVar(v, s);
}

CVar DequeFront(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto &d = Require(self)->deque;
    if (d.empty()) return inter::NativeToFakeluaNil(s);
    return NativeFieldToCVar(d.front(), s);
}

CVar DequeBack(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto &d = Require(self)->deque;
    if (d.empty()) return inter::NativeToFakeluaNil(s);
    return NativeFieldToCVar(d.back(), s);
}

CVar DequeAt(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "deque:at", "index expected");
    auto &d = Require(self)->deque;
    int64_t i = CheckIntegerArg(inter::GetNativeArg(s, args, n, 0), 1, "deque:at");
    if (i < 1 || static_cast<size_t>(i) > d.size()) return inter::NativeToFakeluaNil(s);
    return NativeFieldToCVar(d[static_cast<size_t>(i - 1)], s);
}

CVar DequeSet(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "deque:set", "index and value expected");
    auto &d = Require(self)->deque;
    int64_t i = CheckIntegerArg(inter::GetNativeArg(s, args, n, 0), 1, "deque:set");
    if (i < 1 || static_cast<size_t>(i) > d.size()) {
        ThrowBadArgument(1, "deque:set", "index out of range");
    }
    d[static_cast<size_t>(i - 1)] = PersistValue(inter::GetNativeArg(s, args, n, 1), "deque:set");
    return inter::NativeToFakeluaNil(s);
}

CVar DequeToTable(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto &d = Require(self)->deque;
    CVar tbl = table::TableHelper::CreateTable(s);
    for (size_t i = 0; i < d.size(); ++i) {
        table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(i + 1), NativeFieldToCVar(d[i], s));
    }
    return tbl;
}

// --- map (flat_map) ---

CVar MapSet(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "map:set", "key and value expected");
    auto &m = Require(self)->map;
    ContainerKey k = ContainerKey::FromCVar(inter::GetNativeArg(s, args, n, 0));
    m[std::move(k)] = PersistValue(inter::GetNativeArg(s, args, n, 1), "map:set");
    return inter::NativeToFakeluaNil(s);
}

CVar MapGet(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "map:get", "key expected");
    auto &m = Require(self)->map;
    auto it = m.find(ContainerKey::FromCVar(inter::GetNativeArg(s, args, n, 0)));
    if (it == m.end()) return inter::NativeToFakeluaNil(s);
    return NativeFieldToCVar(it->second, s);
}

CVar MapHas(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "map:has", "key expected");
    auto &m = Require(self)->map;
    return inter::NativeToFakeluaBool(s, m.find(ContainerKey::FromCVar(inter::GetNativeArg(s, args, n, 0))) != m.end());
}

CVar MapErase(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "map:erase", "key expected");
    auto &m = Require(self)->map;
    bool ok = m.erase(ContainerKey::FromCVar(inter::GetNativeArg(s, args, n, 0))) > 0;
    return inter::NativeToFakeluaBool(s, ok);
}

CVar MapKeys(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto &m = Require(self)->map;
    CVar tbl = table::TableHelper::CreateTable(s);
    int64_t i = 1;
    for (const auto &kv: m) {
        table::TableHelper::SetTableInt(s, tbl, i++, kv.first.ToCVar(s));
    }
    return tbl;
}

CVar MapToTable(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto &m = Require(self)->map;
    CVar tbl = table::TableHelper::CreateTable(s);
    for (const auto &kv: m) {
        table::TableHelper::SetTable(s, tbl, kv.first.ToCVar(s), NativeFieldToCVar(kv.second, s));
    }
    return tbl;
}

// --- set (flat_set) ---

CVar SetInsert(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "set:insert", "value expected");
    auto &st = Require(self)->set;
    auto r = st.insert(ContainerKey::FromCVar(inter::GetNativeArg(s, args, n, 0)));
    return inter::NativeToFakeluaBool(s, r.second);
}

CVar SetHas(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "set:has", "value expected");
    auto &st = Require(self)->set;
    return inter::NativeToFakeluaBool(s, st.find(ContainerKey::FromCVar(inter::GetNativeArg(s, args, n, 0))) != st.end());
}

CVar SetErase(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "set:erase", "value expected");
    auto &st = Require(self)->set;
    bool ok = st.erase(ContainerKey::FromCVar(inter::GetNativeArg(s, args, n, 0))) > 0;
    return inter::NativeToFakeluaBool(s, ok);
}

CVar SetValues(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    auto &st = Require(self)->set;
    CVar tbl = table::TableHelper::CreateTable(s);
    int64_t i = 1;
    for (const auto &v: st) {
        table::TableHelper::SetTableInt(s, tbl, i++, v.ToCVar(s));
    }
    return tbl;
}

NativeObject *Make(State *state, ContainerImpl::Kind kind, const char *type_name) {
    int64_t gid = state->GetNativeObjectManager().CreateGroup();
    NativeObject *nat = state->GetNativeObjectManager().Create(gid, type_name, 0);
    auto *impl = new ContainerImpl();
    impl->kind = kind;
    Attach(nat, impl);
    nat->RegisterMethod("size", SizeOf);
    nat->RegisterMethod("empty", EmptyOf);
    nat->RegisterMethod("clear", ClearOf);
    nat->RegisterMethod("close", Close);
    return nat;
}

}// namespace

void RegisterContainerLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "container.deque", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
        NativeObject *nat = Make(state, ContainerImpl::Kind::Deque, "container_deque");
        nat->RegisterMethod("push_back", DequePushBack);
        nat->RegisterMethod("push_front", DequePushFront);
        nat->RegisterMethod("pop_back", DequePopBack);
        nat->RegisterMethod("pop_front", DequePopFront);
        nat->RegisterMethod("front", DequeFront);
        nat->RegisterMethod("back", DequeBack);
        nat->RegisterMethod("at", DequeAt);
        nat->RegisterMethod("set", DequeSet);
        nat->RegisterMethod("to_table", DequeToTable);
        return nat->Wrap(state);
    });

    RegisterNativeFunction(s, "container.map", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
        NativeObject *nat = Make(state, ContainerImpl::Kind::Map, "container_map");
        nat->RegisterMethod("set", MapSet);
        nat->RegisterMethod("get", MapGet);
        nat->RegisterMethod("has", MapHas);
        nat->RegisterMethod("erase", MapErase);
        nat->RegisterMethod("keys", MapKeys);
        nat->RegisterMethod("to_table", MapToTable);
        return nat->Wrap(state);
    });

    RegisterNativeFunction(s, "container.set", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
        NativeObject *nat = Make(state, ContainerImpl::Kind::Set, "container_set");
        nat->RegisterMethod("insert", SetInsert);
        nat->RegisterMethod("has", SetHas);
        nat->RegisterMethod("erase", SetErase);
        nat->RegisterMethod("values", SetValues);
        return nat->Wrap(state);
    });
}

}// namespace fakelua::container
