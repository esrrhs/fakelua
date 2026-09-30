#include "native/arena_pin.h"

#include "interp/func_proto.h"
#include "native/table/native_table.h"
#include "state/state.h"
#include "var/var_closure.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include "var/var_table.h"

#include <cstring>
#include <new>
#include <string>
#include <unordered_map>

namespace fakelua {

namespace {

struct PinSeen {
    std::unordered_map<const VarTable *, CVar> tables;
    std::unordered_map<const VarClosure *, VarClosure *> closures;
    std::unordered_map<const VarString *, VarString *> strings;
};

CVar PinCVar(State *s, CVar v, PinSeen &seen);

VarString *PinString(State *s, const VarString *src, PinSeen &seen) {
    if (!src) return nullptr;
    if (s->GetHeap().OwnsConst(src)) return const_cast<VarString *>(src);
    if (const auto it = seen.strings.find(src); it != seen.strings.end()) return it->second;
    auto &alloc = s->GetHeap().GetAllocator(true);
    auto *dst = static_cast<VarString *>(alloc.Alloc(sizeof(VarString) + src->Size()));
    new (dst) VarString(src->Str());
    seen.strings.emplace(src, dst);
    return dst;
}

const char *PinCodeStr(State *s, const char *code) {
    if (!code || code == kInterpClosureMagic) return code;
    const size_t n = std::strlen(code);
    auto &alloc = s->GetHeap().GetAllocator(true);
    auto *dst = static_cast<char *>(alloc.Alloc(n + 1));
    std::memcpy(dst, code, n + 1);
    return dst;
}

VarClosure *PinClosure(State *s, VarClosure *cl, PinSeen &seen) {
    if (!cl) return nullptr;
    // 已经在 const arena 上的闭包（上次登记钉过）直接复用，避免再复制一份捕获表。
    if (s->GetHeap().OwnsConst(cl)) return cl;
    if (const auto it = seen.closures.find(cl); it != seen.closures.end()) return it->second;

    const int nup = cl->upvalue_count > 0 ? cl->upvalue_count : 0;
    auto &alloc = s->GetHeap().GetAllocator(true);
    auto *out = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure) + static_cast<size_t>(nup) * sizeof(CVar *)));
    out->func_ptr = cl->func_ptr;
    out->upvalue_count = nup;
    out->expected_arg_count = cl->expected_arg_count;
    out->is_vararg = cl->is_vararg;
    out->code_str = PinCodeStr(s, cl->code_str);
    seen.closures.emplace(cl, out);

    for (int i = 0; i < nup; ++i) {
        CVar *src = cl->upvalues[i];
        if (!src) {
            out->upvalues[i] = nullptr;
            continue;
        }
        CVar pinned = PinCVar(s, *src, seen);
        *src = pinned;
        auto *box = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        *box = pinned;
        out->upvalues[i] = box;
    }
    return out;
}

CVar PinTable(State *s, CVar src, PinSeen &seen) {
    VarTable *t = src.data_.t;
    if (!t) return inter::NativeToFakeluaNil(s);
    // 同一帧里第二个闭包再钉时，表已经在 const arena 上，必须返回同一份，
    // 否则连接回调和查询回调会各改各的副本。
    if (s->GetHeap().OwnsConst(t)) return src;
    if (const auto it = seen.tables.find(t); it != seen.tables.end()) return it->second;

    // CreateTable 在外层 ConstAllocScope 里走 const arena。
    CVar result = table::TableHelper::CreateTable(s);
    result.flag_ = 0;
    seen.tables.emplace(t, result);
    VarTable *out = result.data_.t;
    out->spec_get = t->spec_get;
    out->spec_set = t->spec_set;
    out->spec_count = t->spec_count;
    out->spec_bytes = t->spec_bytes;
    out->spec_cvars = t->spec_cvars;

    constexpr uint32_t kMaxSpecBytes = 1u << 20;
    if (t->spec && t->spec_bytes > 0 && t->spec_bytes <= kMaxSpecBytes) {
        auto &alloc = s->GetHeap().GetAllocator(true);
        auto *blob = alloc.Alloc(t->spec_bytes);
        std::memcpy(blob, t->spec, t->spec_bytes);
        out->spec = blob;
        if (t->spec_cvars > 0) {
            if (t->spec_bytes != t->spec_cvars * sizeof(CVar)) {
                ThrowFakeluaException("pin closure: spec table layout does not match CVar array");
            }
            auto *fields = static_cast<CVar *>(blob);
            for (uint32_t i = 0; i < t->spec_cvars; ++i) {
                fields[i] = PinCVar(s, fields[i], seen);
            }
        }
    } else {
        out->spec = t->spec;
    }

    if (t->spec_count > 0 && t->spec_keys && t->spec_vals) {
        auto &alloc = s->GetHeap().GetAllocator(true);
        out->spec_keys = static_cast<CVar *>(alloc.Alloc(sizeof(CVar) * t->spec_count));
        out->spec_vals = static_cast<CVar *>(alloc.Alloc(sizeof(CVar) * t->spec_count));
        for (uint32_t i = 0; i < t->spec_count; ++i) {
            out->spec_keys[i] = PinCVar(s, t->spec_keys[i], seen);
            if (t->spec_cvars > 0 && out->spec) {
                out->spec_vals[i] = static_cast<CVar *>(out->spec)[i];
            } else {
                out->spec_vals[i] = PinCVar(s, t->spec_vals[i], seen);
            }
        }
    }

    // 特化表和普通表：按键重放到新表上。spec_set 会写回上面的 const spec 块。
    // 不透明 spec（NativeObject）的字段不在哈希部分，避免再走 spec_set。
    const bool rebuild_hash = t->spec_cvars > 0 || t->spec == nullptr;
    if (rebuild_hash) {
        table::TableHelper::ForEachKV(src, [&](CVar key, CVar val) {
            CVar pk = PinCVar(s, key, seen);
            CVar pv = PinCVar(s, val, seen);
            table::TableHelper::SetTable(s, result, pk, pv);
        });
    } else if (t->bucket_count_ == 0) {
        out->count_ = t->count_;
        for (uint32_t i = 0; i < t->count_ && i < VarTable::QUICK_DATA_SIZE; ++i) {
            static_cast<CVar &>(out->quick_data_[i].key) = PinCVar(s, t->quick_data_[i].key, seen);
            static_cast<CVar &>(out->quick_data_[i].val) = PinCVar(s, t->quick_data_[i].val, seen);
            out->quick_data_[i].hash = t->quick_data_[i].hash;
        }
    }

    result.flag_ = src.flag_;
    seen.tables[t] = result;
    return result;
}

CVar PinCVar(State *s, CVar v, PinSeen &seen) {
    switch (static_cast<VarType>(v.type_)) {
        case VarType::String:
            v.data_.s = PinString(s, v.data_.s, seen);
            return v;
        case VarType::Table:
            return PinTable(s, v, seen);
        case VarType::Closure: {
            VarClosure *pinned = PinClosure(s, v.data_.cl, seen);
            v.data_.cl = pinned;
            return v;
        }
        case VarType::Multi: {
            VarMulti *m = v.data_.m;
            if (!m) return v;
            const int n = static_cast<int>(m->GetCount());
            CVar out = inter::AllocMultiCVar(s, n);
            for (int i = 0; i < n; ++i) {
                inter::SetMultiCVarElement(out, i, PinCVar(s, m->GetVars()[i], seen));
            }
            return out;
        }
        default:
            return v;
    }
}

}// namespace

VarClosure *PinClosureForAsync(State *s, VarClosure *cl) {
    if (!s || !cl) return nullptr;
    // 嵌套的表重哈希 / Multi 分配都走 GetValueAllocator，这里切到 const arena。
    State::ConstAllocScope const_alloc(s);
    PinSeen seen;
    return PinClosure(s, cl, seen);
}

}// namespace fakelua
