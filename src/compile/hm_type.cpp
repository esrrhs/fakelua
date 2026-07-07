#include "compile/hm_type.h"
#include <cstdlib>
#include <cstring>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────
// TypeArena
// ─────────────────────────────────────────────────────────────────────────

void TypeArena::Reset() {
    for (void *blk : blocks_) std::free(blk);
    blocks_.clear();
    offset_ = 0;
}

void *TypeArena::Alloc(size_t n) {
    // Align to 8 bytes for comfort with pointer-heavy Type structs.
    n = (n + 7) & ~size_t(7);
    if (blocks_.empty() || offset_ + n > kBlockSize) {
        void *blk = std::malloc(kBlockSize);
        blocks_.push_back(blk);
        offset_ = 0;
    }
    void *p = static_cast<char *>(blocks_.back()) + offset_;
    offset_ += n;
    return p;
}

const char *TypeArena::AllocString(const std::string &s) {
    char *p = static_cast<char *>(Alloc(s.size() + 1));
    std::memcpy(p, s.data(), s.size());
    p[s.size()] = '\0';
    return p;
}

void *TypeArena::AllocCopy(const void *src, size_t n) {
    void *p = Alloc(n);
    std::memcpy(p, src, n);
    return p;
}

// ─────────────────────────────────────────────────────────────────────────
// Primitive singletons (one per type kind)
// ─────────────────────────────────────────────────────────────────────────
namespace {
Type *MakePrimStatic(TypeArena &arena, TypeKind k) {
    Type *t = arena.Alloc<Type>();
    t->kind = k;
    t->var_id = -1;
    t->bound = nullptr;
    t->nparams = 0;
    t->ret = nullptr;
    t->elem = nullptr;
    t->is_open = false;
    t->layout_id = -1;
    return t;
}
}  // namespace

// We cache the primitive singletons so that literally every int literal can
// share one Type* (important for Unify's same-pointer fast path).
namespace {
struct PrimCache {
    Type *int_ty = nullptr;
    Type *float_ty = nullptr;
    Type *string_ty = nullptr;
    Type *bool_ty = nullptr;
    Type *nil_ty = nullptr;
    Type *dynamic_ty = nullptr;
};
thread_local PrimCache g_prims;

void EnsurePrims(TypeArena &arena) {
    if (!g_prims.int_ty) {
        g_prims.int_ty        = MakePrimStatic(arena, TypeKind::TY_INT);
        g_prims.float_ty      = MakePrimStatic(arena, TypeKind::TY_FLOAT);
        g_prims.string_ty     = MakePrimStatic(arena, TypeKind::TY_STRING);
        g_prims.bool_ty       = MakePrimStatic(arena, TypeKind::TY_BOOL);
        g_prims.nil_ty        = MakePrimStatic(arena, TypeKind::TY_NIL);
        g_prims.dynamic_ty    = MakePrimStatic(arena, TypeKind::TY_DYNAMIC);
    }
}
}  // namespace

// ─────────────────────────────────────────────────────────────────────────
// HmType constructors
// ─────────────────────────────────────────────────────────────────────────
namespace HmType {

Type *MakeVar(TypeArena &arena, int id) {
    Type *t = arena.Alloc<Type>();
    t->kind = TypeKind::TY_VAR;
    t->var_id = id;
    t->bound = nullptr;
    t->nparams = 0;
    t->ret = nullptr;
    t->elem = nullptr;
    t->is_open = false;
    t->layout_id = -1;
    return t;
}

Type *MakePrim(TypeArena &arena, TypeKind k) {
    EnsurePrims(arena);
    switch (k) {
        case TypeKind::TY_INT:          return g_prims.int_ty;
        case TypeKind::TY_FLOAT:        return g_prims.float_ty;
        case TypeKind::TY_STRING:       return g_prims.string_ty;
        case TypeKind::TY_BOOL:         return g_prims.bool_ty;
        case TypeKind::TY_NIL:          return g_prims.nil_ty;
        case TypeKind::TY_DYNAMIC:      return g_prims.dynamic_ty;
        default:                        return g_prims.dynamic_ty;
    }
}

Type *MakeFun(TypeArena &arena, std::vector<Type *> params, Type *ret) {
    Type *t = arena.Alloc<Type>();
    t->kind = TypeKind::TY_FUN;
    t->nparams = 0;
    t->ret = ret;
    t->params = std::move(params);
    return t;
}

Type *MakeFunN(TypeArena &arena, int nparams, Type *ret) {
    Type *t = arena.Alloc<Type>();
    t->kind = TypeKind::TY_FUN;
    t->nparams = nparams;
    t->ret = ret;
    return t;
}

Type *MakeRecord(TypeArena &arena, bool open) {
    Type *t = arena.Alloc<Type>();
    t->kind = open ? TypeKind::TY_RECORD_OPEN : TypeKind::TY_RECORD;
    t->is_open = open;
    t->layout_id = -1;
    return t;
}

Type *MakeArray(TypeArena &arena, Type *elem) {
    Type *t = arena.Alloc<Type>();
    t->kind = TypeKind::TY_ARRAY;
    t->elem = elem;
    return t;
}

Type *MakeUnion(TypeArena &arena, std::vector<Type *> members) {
    Type *t = arena.Alloc<Type>();
    t->kind = TypeKind::TY_UNION;
    t->members = std::move(members);
    return t;
}

}  // namespace HmType

// ─────────────────────────────────────────────────────────────────────────
// TypeVarTable
// ─────────────────────────────────────────────────────────────────────────

Type *TypeVarTable::NewVar() {
    if (next_var_id_ >= MAX_VARS) {
        // Out of variable slots — return a fresh "dummy" dynamic so inference
        // degrades gracefully rather than crashing.
        return HmType::MakePrim(arena_, TypeKind::TY_DYNAMIC);
    }
    Type *v = &vars_[next_var_id_];
    v->kind = TypeKind::TY_VAR;
    v->var_id = next_var_id_;
    v->bound = nullptr;
    v->nparams = 0;
    v->ret = nullptr;
    v->elem = nullptr;
    v->is_open = false;
    v->layout_id = -1;
    next_var_id_++;
    return v;
}

void TypeVarTable::Reset() {
    next_var_id_ = 0;
}

Type *TypeVarTable::Prune(Type *t) {
    while (t && t->kind == TypeKind::TY_VAR && t->bound) {
        t = t->bound;
    }
    return t;
}

// ── Occurs check ──────────────────────────────────────────────────────────
// We guard against infinite recursion by tracking visited variable ids.
namespace {
struct OccursState {
    int var_id;
    // Ring-buffer style dedup: linear scan is fine for typical type depth.
    std::vector<int> visited;
};

bool OccursWalk(Type *t, OccursState &st) {
    t = TypeVarTable::Prune(t);
    if (!t) return false;
    if (t->kind == TypeKind::TY_VAR) {
        if (t->var_id == st.var_id) return true;
        // Already visited?  Skip to avoid infinite walk on shared DAGs.
        for (int v : st.visited) if (v == t->var_id) return false;
        st.visited.push_back(t->var_id);
        // Also walk the bound (in case of forward pointers).
        return OccursWalk(t->bound, st);
    }
    switch (t->kind) {
        case TypeKind::TY_INT:
        case TypeKind::TY_FLOAT:
        case TypeKind::TY_STRING:
        case TypeKind::TY_BOOL:
        case TypeKind::TY_NIL:
        case TypeKind::TY_DYNAMIC:
            return false;
        case TypeKind::TY_FUN:
            for (Type *p : t->params) if (OccursWalk(p, st)) return true;
            return OccursWalk(t->ret, st);
        case TypeKind::TY_RECORD:
        case TypeKind::TY_RECORD_OPEN: {
            for (const auto &f : t->fields) if (OccursWalk(f.type, st)) return true;
            return false;
        }
        case TypeKind::TY_ARRAY:
            return OccursWalk(t->elem, st);
        case TypeKind::TY_UNION:
            for (Type *m : t->members) if (OccursWalk(m, st)) return true;
            return false;
        case TypeKind::TY_VAR:
            return false;  // handled by Prune above
    }
    return false;
}
}  // namespace

bool TypeVarTable::OccursCheck(Type *var, Type *t) {
    var = Prune(var);
    if (!var || var->kind != TypeKind::TY_VAR) return false;
    OccursState st;
    st.var_id = var->var_id;
    return OccursWalk(t, st);
}

// ── Unify ─────────────────────────────────────────────────────────────────
namespace {
// Forward declaration for recursive helpers.
bool UnifyImpl(TypeVarTable *table, Type *a, Type *b);

bool BindTo(Type *var, Type *target) {
    // var must already be pruned to unbound TY_VAR by the caller.
    var->bound = target;
    return true;
}

// Helper to walk fields and unify.  Returns false on mismatch.
bool UnifyRecordFields(Type *ra, Type *rb) {
    // ra and rb are both record kinds here.
    // Match fields by name.
    for (auto &fa : ra->fields) {
        RecordField *fb = nullptr;
        for (auto &f : rb->fields) {
            if (f.name == fa.name) { fb = &f; break; }
        }
        if (fb) {
            if (!UnifyImpl(nullptr, fa.type, fb->type)) return false;
        } else {
            // Field only in ra.
            if (!rb->is_open) {
                // Closed record lacks this field → mismatch.
                return false;
            }
            // Open record: OK, the absent field just isn't required.
        }
    }
    for (auto &fb : rb->fields) {
        RecordField *fa = nullptr;
        for (auto &f : ra->fields) {
            if (f.name == fb.name) { fa = &f; break; }
        }
        if (!fa) {
            if (!ra->is_open) return false;
        }
    }
    return true;
}

bool UnifyImpl(TypeVarTable *table, Type *a, Type *b) {
    if (a == b) return true;

    // Prune both.
    while (a && a->kind == TypeKind::TY_VAR && a->bound) a = a->bound;
    while (b && b->kind == TypeKind::TY_VAR && b->bound) b = b->bound;

    if (a == b) return true;

    // Variable binding.
    if (a && a->kind == TypeKind::TY_VAR) {
        // Occurs check: a must not appear in b (unless b is the same var,
        // which we already handled via a == b above).
        if (TypeVarTable::OccursCheck(a, b)) {
            // Recursive type — degrade gracefully to dynamic if available,
            // but here we just signal failure so callers can decide.
            return false;
        }
        if (table) {
            // Bind via the table (no extra state needed).
            a->bound = b;
        } else {
            a->bound = b;
        }
        return true;
    }
    if (b && b->kind == TypeKind::TY_VAR) {
        if (TypeVarTable::OccursCheck(b, a)) return false;
        b->bound = a;
        return true;
    }

    if (!a || !b) return false;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
        case TypeKind::TY_INT:
        case TypeKind::TY_FLOAT:
        case TypeKind::TY_STRING:
        case TypeKind::TY_BOOL:
        case TypeKind::TY_NIL:
        case TypeKind::TY_DYNAMIC:
            return true;  // same kind ⇒ equal
        case TypeKind::TY_FUN: {
            // Params count: explicit params vector or nparams marker.
            int an = a->params.empty() ? a->nparams : (int)a->params.size();
            int bn = b->params.empty() ? b->nparams : (int)b->params.size();
            if (an != bn) return false;
            if (!UnifyImpl(table, a->ret, b->ret)) return false;
            // Unify declared params if both have explicit vectors.
            if (!a->params.empty() && !b->params.empty()) {
                for (size_t i = 0; i < a->params.size(); ++i) {
                    if (!UnifyImpl(table, a->params[i], b->params[i])) return false;
                }
            }
            return true;
        }
        case TypeKind::TY_RECORD:
        case TypeKind::TY_RECORD_OPEN:
            return UnifyRecordFields(a, b);
        case TypeKind::TY_ARRAY:
            return UnifyImpl(table, a->elem, b->elem);
        case TypeKind::TY_UNION:
            if (a->members.size() != b->members.size()) return false;
            for (size_t i = 0; i < a->members.size(); ++i) {
                if (!UnifyImpl(table, a->members[i], b->members[i])) return false;
            }
            return true;
        case TypeKind::TY_VAR:
            // Both pruned already, different vars, can't bind (handled above).
            return a == b;
    }
    return false;
}
}  // namespace

bool TypeVarTable::Unify(Type *a, Type *b) {
    return UnifyImpl(this, a, b);
}

bool TypeVarTable::UnifySoft(Type *a, Type *b) {
    // Like Unify, but when we'd return false we instead degrade by binding
    // the mismatched side to TY_DYNAMIC and returning true.
    if (Unify(a, b)) return true;

    // Attempt to rescue by downgrading the most specific side to dynamic.
    a = Prune(a);
    b = Prune(b);
    if (!a || !b) return false;

    if (a->kind == TypeKind::TY_VAR) {
        a->bound = HmType::MakePrim(arena_, TypeKind::TY_DYNAMIC);
        return true;
    }
    if (b->kind == TypeKind::TY_VAR) {
        b->bound = HmType::MakePrim(arena_, TypeKind::TY_DYNAMIC);
        return true;
    }

    if (a->kind != b->kind) {
        return false;
    }

    // Both same kind but structurally incompatible — recurse into the more
    // flexible side (open records / arrays).
    switch (a->kind) {
        case TypeKind::TY_RECORD:
        case TypeKind::TY_RECORD_OPEN:
            return UnifyRecordFields(a, b);
        default:
            return false;
    }
}

void TypeVarTable::BindVar(Type *var, Type *target) {
    var = Prune(var);
    if (!var || var->kind != TypeKind::TY_VAR) return;
    var->bound = target;
}

bool TypeVarTable::BindTo(Type *var, Type *target) {
    var = Prune(var);
    if (!var || var->kind != TypeKind::TY_VAR) return false;
    var->bound = target;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Global convenience
// ─────────────────────────────────────────────────────────────────────────
namespace {
// We keep the global state in thread-local storage so that tests don't
// interfere.  Init/Shutdown manage lifetime.
struct GlobalHMState {
    TypeArena arena;
    TypeVarTable table;
    GlobalHMState() : table(arena) {}
};

thread_local GlobalHMState *g_state = nullptr;
}  // namespace

TypeVarTable *g_type_table = nullptr;
TypeArena *g_type_arena = nullptr;

void InitTypeTable() {
    if (!g_state) {
        g_state = new GlobalHMState();
    }
    g_type_table = &g_state->table;
    g_type_arena = &g_state->arena;
}

void ShutdownTypeTable() {
    delete g_state;
    g_state = nullptr;
    g_type_table = nullptr;
    g_type_arena = nullptr;
}

}  // namespace fakelua
