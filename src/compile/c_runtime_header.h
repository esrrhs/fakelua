#pragma once

// C runtime header for generated JIT code.
// This file contains the C type definitions, macros, and runtime functions
// that are injected into every generated C compilation unit.
// Extracted from c_gen.cpp for maintainability.

namespace fakelua {

// The C runtime header content as a raw string literal.
// The _S (State pointer) is prepended by CGen::GenerateHeader() before this content.
inline constexpr const char *kCRuntimeHeader = R"(
typedef struct VarString {
    int size_;
    uint32_t hash_;
    char data_[0];
} VarString;

typedef struct VarTable VarTable;
typedef struct VarMulti VarMulti;

typedef struct CVar {
    int type_;
    int flag_;
    union {
        bool b;
        int64_t i;
        double f;
        VarString *s;
        VarTable *t;
        VarMulti *m;
    } data_;
} CVar;

// 编译期断言：确保 CVar 大小为 16 字节
typedef char check_cvar_size[sizeof(CVar) == 16 ? 1 : -1];

typedef struct VarEntry {
    CVar key;
    CVar val;
    uint32_t hash;
} VarEntry;

typedef struct TableNode {
    VarEntry entry;
    uint32_t next;
    uint32_t active_pos;
} TableNode;

struct VarTable;

typedef CVar (*SpecGetFn)(VarTable *tbl, CVar k, bool *finish);
typedef void (*SpecSetFn)(VarTable *tbl, CVar k, CVar v, bool *finish);

struct VarTable {
    uint32_t count_;
    uint32_t bucket_count_;
    TableNode *nodes_;
    uint32_t *active_list_;
    VarEntry quick_data_[8];
    uint32_t free_list_idx_;
    void *spec;
    SpecGetFn spec_get;
    SpecSetFn spec_set;
    CVar *spec_keys;
    CVar *spec_vals;
    uint32_t spec_count;
};

typedef struct State State;

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define STR_SIZE(s) ((s)->size_)
#define STR_DATA(s) ((s)->data_)
#define TABLE_SIZE(t) ((t)->count_ + (t)->spec_count)

enum {
    VAR_NIL = 0,
    VAR_BOOL = 1,
    VAR_INT = 2,
    VAR_FLOAT = 3,
    VAR_STRING = 4,
    VAR_STRINGID = 5,
    VAR_TABLE = 6,
    VAR_MULTI = 7,
};

typedef struct VarMulti {
    uint32_t count;
    CVar vars[0];
} VarMulti;

extern void* FakeluaAlloc(State *s, size_t size, bool is_const);

static inline CVar FlMakeMulti(State *state, uint32_t count, ...) {
    VarMulti *m = (VarMulti *)FakeluaAlloc(state, sizeof(VarMulti) + count * sizeof(CVar), !__fakelua_init_flag__);
    m->count = count;
    va_list args_list;
    va_start(args_list, count);
    for (uint32_t i = 0; i < count; ++i) {
        m->vars[i] = va_arg(args_list, CVar);
    }
    va_end(args_list);
    CVar res;
    res.type_ = VAR_MULTI;
    res.flag_ = 0;
    res.data_.m = m;
    return res;
}

static inline CVar FlCombineMulti(State *state, uint32_t prefix_count, const CVar *prefix_vars, CVar last) {
    uint32_t last_count = 1;
    const CVar *last_vars = &last;
    if (last.type_ == VAR_MULTI) {
        VarMulti *m = last.data_.m;
        last_count = m->count;
        last_vars = m->vars;
    }

    uint32_t total_count = prefix_count + last_count;
    VarMulti *m = (VarMulti *)FakeluaAlloc(state, sizeof(VarMulti) + total_count * sizeof(CVar), !__fakelua_init_flag__);
    m->count = total_count;

    for (uint32_t i = 0; i < prefix_count; ++i) {
        if (prefix_vars[i].type_ == VAR_MULTI) {
            m->vars[i] = prefix_vars[i].data_.m->count > 0 ? prefix_vars[i].data_.m->vars[0] : (CVar){VAR_NIL};
        } else {
            m->vars[i] = prefix_vars[i];
        }
    }
    for (uint32_t i = 0; i < last_count; ++i) {
        m->vars[prefix_count + i] = last_vars[i];
    }

    CVar res;
    res.type_ = VAR_MULTI;
    res.flag_ = 0;
    res.data_.m = m;
    return res;
}


static inline CVar FlUnboxMulti(CVar v, uint32_t idx) {
    if (LIKELY(v.type_ != VAR_MULTI)) {
        return (idx == 0) ? v : (CVar){VAR_NIL};
    }
    VarMulti *m = v.data_.m;
    return (idx < m->count) ? m->vars[idx] : (CVar){VAR_NIL};
}

static inline CVar FlSliceMulti(State *state, CVar v, uint32_t start_idx) {
    if (v.type_ != VAR_MULTI) {
        return (start_idx == 0) ? v : (CVar){VAR_NIL};
    }
    VarMulti *m = v.data_.m;
    if (start_idx >= m->count) {
        return (CVar){VAR_NIL};
    }
    uint32_t count = m->count - start_idx;
    VarMulti *nm = (VarMulti *)FakeluaAlloc(state, sizeof(VarMulti) + count * sizeof(CVar), !__fakelua_init_flag__);
    nm->count = count;
    for (uint32_t i = 0; i < count; ++i) {
        nm->vars[i] = m->vars[start_idx + i];
    }
    CVar res;
    res.type_ = VAR_MULTI;
    res.flag_ = 0;
    res.data_.m = nm;
    return res;
}

#define SET_NIL(v) do { (v).type_ = VAR_NIL; } while(0)
#define SET_BOOL(v, val) do { (v).type_ = VAR_BOOL; (v).data_.b = (val); } while(0)
#define SET_INT(v, val) do { (v).type_ = VAR_INT; (v).data_.i = (val); } while(0)
#define SET_FLOAT(v, val) do { \
    double __f = (val); \
    (v).type_ = VAR_FLOAT; \
    (v).data_.f = __f; \
} while(0)

#define NORMALIZE_TABLE_KEY(key) ({ \
    CVar __k = (key); \
    if (LIKELY(__k.type_ == VAR_FLOAT)) { \
        double __d = __k.data_.f; \
        if (isfinite(__d)) { \
            double __ip; \
            if (modf(__d, &__ip) == 0.0 && __ip >= (double)INT64_MIN && __ip <= (double)INT64_MAX) { \
                __k.type_ = VAR_INT; \
                __k.data_.i = (int64_t)__ip; \
            } \
        } \
    } \
    __k; \
})

extern void* FakeluaAlloc(State *s, size_t size, bool is_const);
extern void FakeluaThrowError(State *s, const char *msg);
extern CVar FakeluaCallByName(State *s, int jit_type, const char *name, int arg_num, ...);

static inline bool FlIsTrue(CVar v) {
    return LIKELY(v.type_ != VAR_NIL) && (v.type_ != VAR_BOOL || v.data_.b);
}

#ifndef FAKELUA_JIT_TYPE
#define FAKELUA_JIT_TYPE 0
#endif

static inline uint32_t FlHashString(const char *str, int len) {
    uint32_t hash = 5381;
    int i;
    for (i = 0; i < len; ++i) {
        hash = ((hash << 5) + hash) + (uint8_t)str[i];
    }
    if (hash == 0) {
        hash = 1;
    }
    return hash;
}

#define SET_STRING(v, str, len) do { \
    int __len = (len); \
    const char *__s_ptr = (str); \
    VarString *__vs = (VarString *)FakeluaAlloc(_S, sizeof(VarString) + __len, !__fakelua_init_flag__); \
    __vs->size_ = __len; \
    __vs->hash_ = 0; \
    memcpy(__vs->data_, __s_ptr, __len); \
    (v).type_ = VAR_STRING; \
    (v).data_.s = __vs; \
} while(0)

#define SET_TABLE(v) do { \
    VarTable *__t = (VarTable *)FakeluaAlloc(_S, sizeof(VarTable), !__fakelua_init_flag__); \
    __t->count_ = 0; \
    __t->bucket_count_ = 0; \
    __t->nodes_ = NULL; \
    __t->active_list_ = NULL; \
    __t->free_list_idx_ = 0xFFFFFFFF; \
    __t->spec = NULL; \
    __t->spec_get = NULL; \
    __t->spec_set = NULL; \
    __t->spec_keys = NULL; \
    __t->spec_vals = NULL; \
    __t->spec_count = 0; \
    assert(sizeof(__t->quick_data_) == 8 * sizeof(VarEntry)); \
    { int __i; for (__i = 0; __i < 8; ++__i) { \
        __t->quick_data_[__i].key.type_ = VAR_NIL; \
        __t->quick_data_[__i].val.type_ = VAR_NIL; \
        __t->quick_data_[__i].hash = 0; \
    } } \
    (v).type_ = VAR_TABLE; \
    (v).data_.t = __t; \
} while(0)

#define SET_TABLE_SPEC(v, SpecType, get_fn, set_fn, field_count) do { \
    SET_TABLE(v); \
    SpecType *__sp = (SpecType *)FakeluaAlloc(_S, sizeof(SpecType), !__fakelua_init_flag__); \
    (v).data_.t->spec = __sp; \
    (v).data_.t->spec_get = (get_fn); \
    (v).data_.t->spec_set = (set_fn); \
    (v).data_.t->spec_keys = (CVar *)FakeluaAlloc(_S, sizeof(CVar) * (field_count), !__fakelua_init_flag__); \
    (v).data_.t->spec_vals = (CVar *)FakeluaAlloc(_S, sizeof(CVar) * (field_count), !__fakelua_init_flag__); \
    (v).data_.t->spec_count = (field_count); \
} while(0)

#define FL_SPEC(SpecType, v, field) (((SpecType *)(v).data_.t->spec)->field)
#define FL_SET_SPEC(SpecType, v, field, index, val) do { \
    CVar __flsv = (val); \
    FL_SPEC(SpecType, (v), field) = __flsv; \
    (v).data_.t->spec_vals[(index)] = __flsv; \
} while(0)

#define IsTrue(v, result) do { \
    CVar __tv = (v); \
    (result) = (LIKELY(__tv.type_ != VAR_NIL) && (__tv.type_ != VAR_BOOL || __tv.data_.b)); \
} while(0)

#define VarEqual(_fa, _fb, result) do { \
    CVar __a = (_fa), __b = (_fb); \
    if (__a.type_ != __b.type_) { \
        if ((__a.type_ == VAR_STRING || __a.type_ == VAR_STRINGID) && \
            (__b.type_ == VAR_STRING || __b.type_ == VAR_STRINGID)) { \
            VarString *__sa = (__a.type_ == VAR_STRING) ? __a.data_.s : (VarString *)__a.data_.i; \
            VarString *__sb = (__b.type_ == VAR_STRING) ? __b.data_.s : (VarString *)__b.data_.i; \
            if (__sa == __sb) { (result) = true; break; } \
            (result) = (__sa->size_ == __sb->size_ && memcmp(__sa->data_, __sb->data_, __sa->size_) == 0); \
        } else if (__a.type_ == VAR_INT && __b.type_ == VAR_FLOAT) { \
            (result) = ((double)__a.data_.i == __b.data_.f); \
        } else if (__a.type_ == VAR_FLOAT && __b.type_ == VAR_INT) { \
            (result) = (__a.data_.f == (double)__b.data_.i); \
        } else { \
            (result) = false; \
        } \
        break; \
    } \
    switch (__a.type_) { \
        case VAR_NIL: (result) = true; break; \
        case VAR_BOOL: (result) = (__a.data_.b == __b.data_.b); break; \
        case VAR_INT: (result) = (__a.data_.i == __b.data_.i); break; \
        case VAR_FLOAT: (result) = (__a.data_.f == __b.data_.f); break; \
        case VAR_STRING: { \
            if (__a.data_.s == __b.data_.s) { (result) = true; break; } \
            (result) = (__a.data_.s->size_ == __b.data_.s->size_ && memcmp(__a.data_.s->data_, __b.data_.s->data_, __a.data_.s->size_) == 0); \
            break; \
        } \
        case VAR_STRINGID: (result) = (__a.data_.i == __b.data_.i); break; \
        case VAR_TABLE: (result) = (__a.data_.t == __b.data_.t); break; \
        default: (result) = false; break; \
    } \
} while(0)

#define VarHash(v, hash) do { \
    CVar __v = (v); \
    switch (__v.type_) { \
        case VAR_NIL: (hash) = 0; break; \
        case VAR_BOOL: (hash) = (__v.data_.b ? 1 : 0); break; \
        case VAR_INT: (hash) = (uint32_t)(__v.data_.i ^ (__v.data_.i >> 32)); break; \
        case VAR_FLOAT: { union { double __d; uint64_t __u; } __fu; __fu.__d = __v.data_.f; (hash) = (uint32_t)(__fu.__u ^ (__fu.__u >> 32)); break; } \
        case VAR_STRING: { if (__v.data_.s->hash_ == 0) { __v.data_.s->hash_ = FlHashString(__v.data_.s->data_, __v.data_.s->size_); } (hash) = __v.data_.s->hash_; break; } \
        case VAR_STRINGID: { VarString *__vs = (VarString *)__v.data_.i; if (__vs->hash_ == 0) { __vs->hash_ = FlHashString(__vs->data_, __vs->size_); } (hash) = __vs->hash_; break; } \
        case VAR_TABLE: (hash) = (uint32_t)((uintptr_t)__v.data_.t ^ ((uintptr_t)__v.data_.t >> 32)); break; \
        default: (hash) = 0; break; \
    } \
} while(0)

static inline CVar FlGetTable(CVar t, CVar k) {
    k = NORMALIZE_TABLE_KEY(k);
    if (UNLIKELY(t.type_ != VAR_TABLE)) { FakeluaThrowError(_S, "attempt to index a non-table value"); }
    if (UNLIKELY(k.type_ == VAR_NIL)) { FakeluaThrowError(_S, "table index is nil"); }
    VarTable *tbl = t.data_.t;
    if (tbl->spec_get) {
        bool __finish = false;
        CVar __r = tbl->spec_get(tbl, k, &__finish);
        if (__finish) return __r;
    }
    if (UNLIKELY(tbl->count_ == 0)) { return (CVar){VAR_NIL}; }
    uint32_t h; VarHash(k, h);
    if (LIKELY(tbl->bucket_count_ == 0)) {
        bool __eq;
        if (tbl->quick_data_[0].hash == h) { VarEqual(tbl->quick_data_[0].key, k, __eq); if (__eq) { return tbl->quick_data_[0].val; } }
        if (tbl->count_ > 1 && tbl->quick_data_[1].hash == h) { VarEqual(tbl->quick_data_[1].key, k, __eq); if (__eq) { return tbl->quick_data_[1].val; } }
        if (tbl->count_ > 2 && tbl->quick_data_[2].hash == h) { VarEqual(tbl->quick_data_[2].key, k, __eq); if (__eq) { return tbl->quick_data_[2].val; } }
        if (tbl->count_ > 3 && tbl->quick_data_[3].hash == h) { VarEqual(tbl->quick_data_[3].key, k, __eq); if (__eq) { return tbl->quick_data_[3].val; } }
        if (tbl->count_ > 4 && tbl->quick_data_[4].hash == h) { VarEqual(tbl->quick_data_[4].key, k, __eq); if (__eq) { return tbl->quick_data_[4].val; } }
        if (tbl->count_ > 5 && tbl->quick_data_[5].hash == h) { VarEqual(tbl->quick_data_[5].key, k, __eq); if (__eq) { return tbl->quick_data_[5].val; } }
        if (tbl->count_ > 6 && tbl->quick_data_[6].hash == h) { VarEqual(tbl->quick_data_[6].key, k, __eq); if (__eq) { return tbl->quick_data_[6].val; } }
        if (tbl->count_ > 7 && tbl->quick_data_[7].hash == h) { VarEqual(tbl->quick_data_[7].key, k, __eq); if (__eq) { return tbl->quick_data_[7].val; } }
    } else {
        uint32_t mask = tbl->bucket_count_ - 1;
        uint32_t idx = h & mask;
        TableNode *curr = &tbl->nodes_[idx];
        if (UNLIKELY(curr->entry.key.type_ == VAR_NIL)) { return (CVar){VAR_NIL}; }
        while (1) {
            if (curr->entry.hash == h) { bool __eq; VarEqual(curr->entry.key, k, __eq); if (__eq) { return curr->entry.val; } }
            uint32_t next = curr->next;
            if (next == 0xFFFFFFFF) { break; }
            curr = &tbl->nodes_[next];
        }
    }
    return (CVar){VAR_NIL};
}

static inline uint32_t NextPowerOfTwo(uint32_t v) {
    v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++;
    return v;
}

static inline bool FlTableInsertRaw(VarTable *tbl, CVar key, CVar val, uint32_t hash) {
    uint32_t mask = tbl->bucket_count_ - 1;
    uint32_t total_nodes = tbl->bucket_count_ + tbl->bucket_count_ / 2;
    assert(tbl->count_ < total_nodes);
    uint32_t idx = hash & mask;
    TableNode *main_node = &tbl->nodes_[idx];
    if (LIKELY(main_node->entry.key.type_ == VAR_NIL)) {
        main_node->entry.key = key; main_node->entry.val = val; main_node->entry.hash = hash; main_node->next = 0xFFFFFFFF;
        main_node->active_pos = tbl->count_; tbl->active_list_[tbl->count_] = idx;
        tbl->count_++; return true;
    }
    uint32_t curr_idx = idx;
    while (1) {
        TableNode *curr = &tbl->nodes_[curr_idx];
        if (curr->entry.hash == hash) { bool __eq; VarEqual(curr->entry.key, key, __eq); if (__eq) { curr->entry.val = val; return true; } }
        if (curr->next == 0xFFFFFFFF) { break; }
        curr_idx = curr->next;
    }
    if (UNLIKELY(tbl->free_list_idx_ == 0xFFFFFFFF)) { return false; }
    uint32_t new_node_idx = tbl->free_list_idx_;
    TableNode *new_node = &tbl->nodes_[new_node_idx];
    tbl->free_list_idx_ = new_node->next;
    new_node->entry.key = key; new_node->entry.val = val; new_node->entry.hash = hash;
    new_node->next = main_node->next; main_node->next = new_node_idx;
    new_node->active_pos = tbl->count_; tbl->active_list_[tbl->count_] = new_node_idx;
    tbl->count_++; return true;
}

static inline void FlTableRehash(VarTable *tbl) {
    uint32_t old_count = tbl->count_;
    uint32_t old_bucket_count = tbl->bucket_count_;
    TableNode *old_nodes = tbl->nodes_;
    uint32_t new_bucket_count = NextPowerOfTwo(old_count + 1);
    if (new_bucket_count <= old_bucket_count) { new_bucket_count = old_bucket_count * 2; }
    while (1) {
        uint32_t overflow_count = new_bucket_count / 2;
        uint32_t total_nodes = new_bucket_count + overflow_count;
        size_t nodes_size = total_nodes * sizeof(TableNode);
        size_t active_list_size = total_nodes * sizeof(uint32_t);
        char *buffer = (char *)FakeluaAlloc(_S, nodes_size + active_list_size, !__fakelua_init_flag__);
        TableNode *new_nodes = (TableNode *)buffer;
        uint32_t *new_active_list = (uint32_t *)(buffer + nodes_size);
        { uint32_t i; for (i = 0; i < total_nodes; ++i) { new_nodes[i].entry.key.type_ = VAR_NIL; new_nodes[i].next = 0xFFFFFFFF; new_nodes[i].active_pos = 0xFFFFFFFF; } }
        TableNode *prev_nodes = tbl->nodes_; uint32_t *prev_active_list = tbl->active_list_; uint32_t prev_bucket_count = tbl->bucket_count_; uint32_t prev_count = tbl->count_; uint32_t prev_free_list_idx = tbl->free_list_idx_;
        tbl->nodes_ = new_nodes; tbl->active_list_ = new_active_list; tbl->bucket_count_ = new_bucket_count; tbl->count_ = 0;
        if (LIKELY(overflow_count > 0)) {
            { uint32_t i; for (i = 0; i < overflow_count - 1; ++i) { tbl->nodes_[new_bucket_count + i].next = new_bucket_count + i + 1; } }
            tbl->nodes_[new_bucket_count + overflow_count - 1].next = 0xFFFFFFFF;
            tbl->free_list_idx_ = new_bucket_count;
        } else {
            tbl->free_list_idx_ = 0xFFFFFFFF;
        }
        bool success = true;
        if (LIKELY(old_bucket_count == 0)) {
            { uint32_t i; for (i = 0; i < old_count; ++i) { if (!FlTableInsertRaw(tbl, tbl->quick_data_[i].key, tbl->quick_data_[i].val, tbl->quick_data_[i].hash)) { success = false; break; } } }
        } else {
            { uint32_t i; for (i = 0; i < old_bucket_count; ++i) {
                uint32_t curr_idx = i;
                while (curr_idx != 0xFFFFFFFF) {
                    TableNode *old_node = &old_nodes[curr_idx];
                    if (LIKELY(old_node->entry.key.type_ != VAR_NIL)) { if (!FlTableInsertRaw(tbl, old_node->entry.key, old_node->entry.val, old_node->entry.hash)) { success = false; break; } }
                    curr_idx = old_node->next;
                }
                if (!success) { break; }
            } }
        }
        if (LIKELY(success)) { break; } else { tbl->nodes_ = prev_nodes; tbl->active_list_ = prev_active_list; tbl->bucket_count_ = prev_bucket_count; tbl->count_ = prev_count; tbl->free_list_idx_ = prev_free_list_idx; new_bucket_count *= 2; }
    }
}

static inline void FlSetTable(CVar t, CVar k, CVar v) {
    k = NORMALIZE_TABLE_KEY(k);
    if (UNLIKELY(t.type_ != VAR_TABLE)) { FakeluaThrowError(_S, "attempt to index a non-table value"); }
    if (UNLIKELY(k.type_ == VAR_NIL)) { FakeluaThrowError(_S, "table index is nil"); }
    VarTable *tbl = t.data_.t;
    if (tbl->spec_set) {
        bool __finish = false;
        tbl->spec_set(tbl, k, v, &__finish);
        if (__finish) return;
    }
    uint32_t h; VarHash(k, h);
    if (UNLIKELY(v.type_ == VAR_NIL)) {
        if (UNLIKELY(tbl->count_ == 0)) { return; }
        if (LIKELY(tbl->bucket_count_ == 0)) {
            bool __eq;
            if (tbl->quick_data_[0].hash == h) { VarEqual(tbl->quick_data_[0].key, k, __eq); if (__eq) { if (tbl->count_ > 1) { tbl->quick_data_[0] = tbl->quick_data_[tbl->count_ - 1]; } tbl->count_--; return; } }
            if (tbl->count_ > 1 && tbl->quick_data_[1].hash == h) { VarEqual(tbl->quick_data_[1].key, k, __eq); if (__eq) { if (tbl->count_ > 2) { tbl->quick_data_[1] = tbl->quick_data_[tbl->count_ - 1]; } tbl->count_--; return; } }
            if (tbl->count_ > 2 && tbl->quick_data_[2].hash == h) { VarEqual(tbl->quick_data_[2].key, k, __eq); if (__eq) { if (tbl->count_ > 3) { tbl->quick_data_[2] = tbl->quick_data_[tbl->count_ - 1]; } tbl->count_--; return; } }
            if (tbl->count_ > 3 && tbl->quick_data_[3].hash == h) { VarEqual(tbl->quick_data_[3].key, k, __eq); if (__eq) { if (tbl->count_ > 4) { tbl->quick_data_[3] = tbl->quick_data_[tbl->count_ - 1]; } tbl->count_--; return; } }
            if (tbl->count_ > 4 && tbl->quick_data_[4].hash == h) { VarEqual(tbl->quick_data_[4].key, k, __eq); if (__eq) { if (tbl->count_ > 5) { tbl->quick_data_[4] = tbl->quick_data_[tbl->count_ - 1]; } tbl->count_--; return; } }
            if (tbl->count_ > 5 && tbl->quick_data_[5].hash == h) { VarEqual(tbl->quick_data_[5].key, k, __eq); if (__eq) { if (tbl->count_ > 6) { tbl->quick_data_[5] = tbl->quick_data_[tbl->count_ - 1]; } tbl->count_--; return; } }
            if (tbl->count_ > 6 && tbl->quick_data_[6].hash == h) { VarEqual(tbl->quick_data_[6].key, k, __eq); if (__eq) { if (tbl->count_ > 7) { tbl->quick_data_[6] = tbl->quick_data_[tbl->count_ - 1]; } tbl->count_--; return; } }
            if (tbl->count_ > 7 && tbl->quick_data_[7].hash == h) { VarEqual(tbl->quick_data_[7].key, k, __eq); if (__eq) { tbl->count_--; return; } }
        } else {
            uint32_t mask = tbl->bucket_count_ - 1; uint32_t idx = h & mask; TableNode *curr = &tbl->nodes_[idx];
            if (UNLIKELY(curr->entry.key.type_ == VAR_NIL)) { return; }
            { bool __eq; VarEqual(curr->entry.key, k, __eq); if (curr->entry.hash == h && __eq) {
                if (curr->next != 0xFFFFFFFF) {
                    uint32_t next_idx = curr->next; TableNode *next_node = &tbl->nodes_[next_idx];
                    uint32_t pos = next_node->active_pos; uint32_t last_node_idx = tbl->active_list_[tbl->count_ - 1];
                    if (next_idx != last_node_idx) { tbl->active_list_[pos] = last_node_idx; tbl->nodes_[last_node_idx].active_pos = pos; }
                    next_node->active_pos = 0xFFFFFFFF; curr->entry = next_node->entry; curr->next = next_node->next; next_node->next = tbl->free_list_idx_; tbl->free_list_idx_ = next_idx;
                } else {
                    uint32_t pos = curr->active_pos; uint32_t last_node_idx = tbl->active_list_[tbl->count_ - 1];
                    if (idx != last_node_idx) { tbl->active_list_[pos] = last_node_idx; tbl->nodes_[last_node_idx].active_pos = pos; }
                    curr->active_pos = 0xFFFFFFFF; curr->entry.key.type_ = VAR_NIL;
                }
                tbl->count_--; return;
            } }
            uint32_t prev_idx = idx; uint32_t curr_idx = curr->next;
            while (curr_idx != 0xFFFFFFFF) {
                TableNode *node = &tbl->nodes_[curr_idx];
                { bool __eq; VarEqual(node->entry.key, k, __eq); if (node->entry.hash == h && __eq) {
                    uint32_t pos = node->active_pos; uint32_t last_node_idx = tbl->active_list_[tbl->count_ - 1];
                    if (curr_idx != last_node_idx) { tbl->active_list_[pos] = last_node_idx; tbl->nodes_[last_node_idx].active_pos = pos; }
                    node->active_pos = 0xFFFFFFFF; tbl->nodes_[prev_idx].next = node->next; node->next = tbl->free_list_idx_; tbl->free_list_idx_ = curr_idx; tbl->count_--; return;
                } }
                prev_idx = curr_idx; curr_idx = node->next;
            }
        }
        return;
    }
    if (LIKELY(tbl->bucket_count_ == 0)) {
        bool __eq;
        if (tbl->count_ > 0 && tbl->quick_data_[0].hash == h) { VarEqual(tbl->quick_data_[0].key, k, __eq); if (__eq) { tbl->quick_data_[0].val = v; return; } }
        if (tbl->count_ > 1 && tbl->quick_data_[1].hash == h) { VarEqual(tbl->quick_data_[1].key, k, __eq); if (__eq) { tbl->quick_data_[1].val = v; return; } }
        if (tbl->count_ > 2 && tbl->quick_data_[2].hash == h) { VarEqual(tbl->quick_data_[2].key, k, __eq); if (__eq) { tbl->quick_data_[2].val = v; return; } }
        if (tbl->count_ > 3 && tbl->quick_data_[3].hash == h) { VarEqual(tbl->quick_data_[3].key, k, __eq); if (__eq) { tbl->quick_data_[3].val = v; return; } }
        if (tbl->count_ > 4 && tbl->quick_data_[4].hash == h) { VarEqual(tbl->quick_data_[4].key, k, __eq); if (__eq) { tbl->quick_data_[4].val = v; return; } }
        if (tbl->count_ > 5 && tbl->quick_data_[5].hash == h) { VarEqual(tbl->quick_data_[5].key, k, __eq); if (__eq) { tbl->quick_data_[5].val = v; return; } }
        if (tbl->count_ > 6 && tbl->quick_data_[6].hash == h) { VarEqual(tbl->quick_data_[6].key, k, __eq); if (__eq) { tbl->quick_data_[6].val = v; return; } }
        if (tbl->count_ > 7 && tbl->quick_data_[7].hash == h) { VarEqual(tbl->quick_data_[7].key, k, __eq); if (__eq) { tbl->quick_data_[7].val = v; return; } }
        if (tbl->count_ < 8) { tbl->quick_data_[tbl->count_].key = k; tbl->quick_data_[tbl->count_].val = v; tbl->quick_data_[tbl->count_].hash = h; tbl->count_++; return; }
        FlTableRehash(tbl);
    }
    if (UNLIKELY(tbl->count_ >= tbl->bucket_count_ || tbl->free_list_idx_ == 0xFFFFFFFF)) { FlTableRehash(tbl); }
    FlTableInsertRaw(tbl, k, v, h);
}

static inline CVar FlGetTableInt(CVar t, int64_t k) {
    if (UNLIKELY(t.type_ != VAR_TABLE)) { FakeluaThrowError(_S, "attempt to index a non-table value"); }
    VarTable *tbl = t.data_.t;
    if (tbl->spec_get) {
        CVar key_cvar; key_cvar.type_ = VAR_INT; key_cvar.data_.i = k;
        bool __finish = false;
        CVar __r = tbl->spec_get(tbl, key_cvar, &__finish);
        if (__finish) return __r;
    }
    if (UNLIKELY(tbl->count_ == 0)) { return (CVar){VAR_NIL}; }
    uint32_t h = (uint32_t)(k ^ (k >> 32));
    if (LIKELY(tbl->bucket_count_ == 0)) {
        for (uint32_t i = 0; i < tbl->count_; ++i) {
            if (tbl->quick_data_[i].hash == h && tbl->quick_data_[i].key.type_ == VAR_INT && tbl->quick_data_[i].key.data_.i == k) {
                return tbl->quick_data_[i].val;
            }
        }
    } else {
        uint32_t mask = tbl->bucket_count_ - 1;
        uint32_t idx = h & mask;
        TableNode *curr = &tbl->nodes_[idx];
        if (UNLIKELY(curr->entry.key.type_ == VAR_NIL)) { return (CVar){VAR_NIL}; }
        while (1) {
            if (curr->entry.hash == h && curr->entry.key.type_ == VAR_INT && curr->entry.key.data_.i == k) {
                return curr->entry.val;
            }
            uint32_t next = curr->next;
            if (next == 0xFFFFFFFF) { break; }
            curr = &tbl->nodes_[next];
        }
    }
    return (CVar){VAR_NIL};
}

static inline void FlSetTableInt(CVar t, int64_t k, CVar v) {
    if (UNLIKELY(t.type_ != VAR_TABLE)) { FakeluaThrowError(_S, "attempt to index a non-table value"); }
    VarTable *tbl = t.data_.t;
    if (tbl->spec_set) {
        CVar key_cvar; key_cvar.type_ = VAR_INT; key_cvar.data_.i = k;
        bool __finish = false;
        tbl->spec_set(tbl, key_cvar, v, &__finish);
        if (__finish) return;
    }
    uint32_t h = (uint32_t)(k ^ (k >> 32));
    CVar key_cvar; key_cvar.type_ = VAR_INT; key_cvar.data_.i = k;
    if (UNLIKELY(v.type_ == VAR_NIL)) {
        FlSetTable(t, key_cvar, v);
        return;
    }
    if (LIKELY(tbl->bucket_count_ == 0)) {
        for (uint32_t i = 0; i < tbl->count_; ++i) {
            if (tbl->quick_data_[i].hash == h && tbl->quick_data_[i].key.type_ == VAR_INT && tbl->quick_data_[i].key.data_.i == k) {
                tbl->quick_data_[i].val = v; return;
            }
        }
        if (tbl->count_ < 8) {
            tbl->quick_data_[tbl->count_].key = key_cvar;
            tbl->quick_data_[tbl->count_].val = v;
            tbl->quick_data_[tbl->count_].hash = h;
            tbl->count_++; return;
        }
        FlTableRehash(tbl);
    }
    if (UNLIKELY(tbl->count_ >= tbl->bucket_count_ || tbl->free_list_idx_ == 0xFFFFFFFF)) { FlTableRehash(tbl); }
    FlTableInsertRaw(tbl, key_cvar, v, h);
}

static inline CVar FlGetTableStrId(CVar t, int64_t str_id) {
    if (UNLIKELY(t.type_ != VAR_TABLE)) { FakeluaThrowError(_S, "attempt to index a non-table value"); }
    VarTable *tbl = t.data_.t;
    if (tbl->spec_get) {
        CVar k; k.type_ = VAR_STRINGID; k.data_.i = str_id;
        bool __finish = false;
        CVar __r = tbl->spec_get(tbl, k, &__finish);
        if (__finish) return __r;
    }
    // hash fallback
    if (UNLIKELY(tbl->count_ == 0)) { return (CVar){VAR_NIL}; }
    VarString *vs = (VarString *)str_id;
    if (vs->hash_ == 0) { vs->hash_ = FlHashString(vs->data_, vs->size_); }
    uint32_t h = vs->hash_;
    if (LIKELY(tbl->bucket_count_ == 0)) {
        for (uint32_t i = 0; i < tbl->count_; ++i) {
            if (tbl->quick_data_[i].hash == h) {
                CVar ek = tbl->quick_data_[i].key;
                if (LIKELY(ek.type_ == VAR_STRINGID)) {
                    if (ek.data_.i == str_id) { return tbl->quick_data_[i].val; }
                    VarString *evs = (VarString *)ek.data_.i;
                    if (evs->size_ == vs->size_ && memcmp(evs->data_, vs->data_, vs->size_) == 0) { return tbl->quick_data_[i].val; }
                } else if (UNLIKELY(ek.type_ == VAR_STRING)) {
                    if (ek.data_.s->size_ == vs->size_ && memcmp(ek.data_.s->data_, vs->data_, vs->size_) == 0) { return tbl->quick_data_[i].val; }
                }
            }
        }
    } else {
        uint32_t mask = tbl->bucket_count_ - 1;
        uint32_t idx = h & mask;
        TableNode *curr = &tbl->nodes_[idx];
        if (UNLIKELY(curr->entry.key.type_ == VAR_NIL)) { return (CVar){VAR_NIL}; }
        while (1) {
            if (curr->entry.hash == h) {
                CVar ek = curr->entry.key;
                if (LIKELY(ek.type_ == VAR_STRINGID)) {
                    if (ek.data_.i == str_id) { return curr->entry.val; }
                    VarString *evs = (VarString *)ek.data_.i;
                    if (evs->size_ == vs->size_ && memcmp(evs->data_, vs->data_, vs->size_) == 0) { return curr->entry.val; }
                } else if (UNLIKELY(ek.type_ == VAR_STRING)) {
                    if (ek.data_.s->size_ == vs->size_ && memcmp(ek.data_.s->data_, vs->data_, vs->size_) == 0) { return curr->entry.val; }
                }
            }
            uint32_t next = curr->next;
            if (next == 0xFFFFFFFF) { break; }
            curr = &tbl->nodes_[next];
        }
    }
    return (CVar){VAR_NIL};
}

static inline void FlSetTableStrId(CVar t, int64_t str_id, CVar v) {
    if (UNLIKELY(t.type_ != VAR_TABLE)) { FakeluaThrowError(_S, "attempt to index a non-table value"); }
    VarTable *tbl = t.data_.t;
    if (tbl->spec_set) {
        CVar k; k.type_ = VAR_STRINGID; k.data_.i = str_id;
        bool __finish = false;
        tbl->spec_set(tbl, k, v, &__finish);
        if (__finish) return;
    }
    // hash fallback
    VarString *vs = (VarString *)str_id;
    if (vs->hash_ == 0) { vs->hash_ = FlHashString(vs->data_, vs->size_); }
    uint32_t h = vs->hash_;
    CVar key_cvar; key_cvar.type_ = VAR_STRINGID; key_cvar.data_.i = str_id;
    if (UNLIKELY(v.type_ == VAR_NIL)) {
        FlSetTable((CVar){.type_ = VAR_TABLE, .data_.t = tbl}, key_cvar, v);
        return;
    }
    if (LIKELY(tbl->bucket_count_ == 0)) {
        for (uint32_t i = 0; i < tbl->count_; ++i) {
            if (tbl->quick_data_[i].hash == h) {
                CVar ek = tbl->quick_data_[i].key;
                if (LIKELY(ek.type_ == VAR_STRINGID)) {
                    if (ek.data_.i == str_id) { tbl->quick_data_[i].val = v; return; }
                    VarString *evs = (VarString *)ek.data_.i;
                    if (evs->size_ == vs->size_ && memcmp(evs->data_, vs->data_, vs->size_) == 0) { tbl->quick_data_[i].val = v; return; }
                } else if (UNLIKELY(ek.type_ == VAR_STRING)) {
                    if (ek.data_.s->size_ == vs->size_ && memcmp(ek.data_.s->data_, vs->data_, vs->size_) == 0) { tbl->quick_data_[i].val = v; return; }
                }
            }
        }
        if (tbl->count_ < 8) {
            tbl->quick_data_[tbl->count_].key = key_cvar;
            tbl->quick_data_[tbl->count_].val = v;
            tbl->quick_data_[tbl->count_].hash = h;
            tbl->count_++; return;
        }
        FlTableRehash(tbl);
    }
    if (UNLIKELY(tbl->count_ >= tbl->bucket_count_ || tbl->free_list_idx_ == 0xFFFFFFFF)) { FlTableRehash(tbl); }
    FlTableInsertRaw(tbl, key_cvar, v, h);
}

static inline void FlTableExpandMulti(CVar t, int64_t start_idx, CVar v) {
    if (LIKELY(v.type_ != VAR_MULTI)) {
        FlSetTableInt(t, start_idx, v);
    } else {
        VarMulti *m = v.data_.m;
        for (uint32_t i = 0; i < m->count; ++i) {
            FlSetTableInt(t, start_idx + i, m->vars[i]);
        }
    }
}


#define CheckNum(v) do { \
    if (UNLIKELY((v).type_ != VAR_INT && (v).type_ != VAR_FLOAT)) { \
        FakeluaThrowError(_S, "attempt to perform arithmetic on non-numeric value"); \
    } \
} while(0)

#define CheckInt(v, result) do { \
    if (LIKELY((v).type_ == VAR_INT)) { (result) = (v).data_.i; } \
    else if (UNLIKELY((v).type_ == VAR_FLOAT)) { \
        double __d = (v).data_.f; \
        if (!isfinite(__d)) { FakeluaThrowError(_S, "number has no integer representation"); } \
        double __ip; \
        if (modf(__d, &__ip) != 0.0) { FakeluaThrowError(_S, "number has no integer representation"); } \
        if (__ip < (double)INT64_MIN || __ip > (double)INT64_MAX) { FakeluaThrowError(_S, "number has no integer representation"); } \
        (result) = (int64_t)__ip; \
    } else { \
        FakeluaThrowError(_S, "attempt to perform bitwise operation on non-numeric value"); \
        (result) = 0; \
    } \
} while(0)

#define CVAR_TO_DOUBLE(v) ((v).type_ == VAR_INT ? (double)(v).data_.i : (v).data_.f)

#define OP_ARITH_IMPL(a, b, res, op) do { \
    CVar _ra = (a); CVar _rb = (b); CheckNum(_ra); CheckNum(_rb); \
    if (LIKELY(_ra.type_ == VAR_INT && _rb.type_ == VAR_INT)) { SET_INT(res, _ra.data_.i op _rb.data_.i); } \
    else { SET_FLOAT(res, CVAR_TO_DOUBLE(_ra) op CVAR_TO_DOUBLE(_rb)); } \
} while(0)

#define OpAdd(a, b, res) OP_ARITH_IMPL(a, b, res, +)
#define OpSub(a, b, res) OP_ARITH_IMPL(a, b, res, -)
#define OpMul(a, b, res) OP_ARITH_IMPL(a, b, res, *)

#define OpDiv(a, b, res) do { \
    CVar _ra = (a); CVar _rb = (b); CheckNum(_ra); CheckNum(_rb); \
    SET_FLOAT(res, CVAR_TO_DOUBLE(_ra) / CVAR_TO_DOUBLE(_rb)); \
} while(0)

#define OpFloorDiv(a, b, res) do { \
    CVar _ra = (a); CVar _rb = (b); CheckNum(_ra); CheckNum(_rb); \
    if (LIKELY(_ra.type_ == VAR_INT && _rb.type_ == VAR_INT)) { \
        if (UNLIKELY(_rb.data_.i == 0)) { FakeluaThrowError(_S, "floor division by zero"); } \
        int64_t _q = _ra.data_.i / _rb.data_.i; \
        if ((_ra.data_.i ^ _rb.data_.i) < 0 && _ra.data_.i % _rb.data_.i != 0) { _q -= 1; } \
        SET_INT(res, _q); \
    } else { \
        SET_FLOAT(res, floor(CVAR_TO_DOUBLE(_ra) / CVAR_TO_DOUBLE(_rb))); \
    } \
} while(0)

#define OpPow(a, b, res) do { \
    CVar _ra = (a); CVar _rb = (b); CheckNum(_ra); CheckNum(_rb); \
    SET_FLOAT(res, pow(CVAR_TO_DOUBLE(_ra), CVAR_TO_DOUBLE(_rb))); \
} while(0)

#define OpMod(a, b, res) do { \
    CVar _ra = (a); CVar _rb = (b); CheckNum(_ra); CheckNum(_rb); \
    if (LIKELY(_ra.type_ == VAR_INT && _rb.type_ == VAR_INT)) { \
        if (UNLIKELY(_rb.data_.i == 0)) { FakeluaThrowError(_S, "modulo by zero"); } \
        int64_t _q = _ra.data_.i / _rb.data_.i; \
        if ((_ra.data_.i ^ _rb.data_.i) < 0 && _ra.data_.i % _rb.data_.i != 0) { _q -= 1; } \
        SET_INT(res, _ra.data_.i - _rb.data_.i * _q); \
    } else { \
        double _fa = CVAR_TO_DOUBLE(_ra); \
        double _fb = CVAR_TO_DOUBLE(_rb); \
        SET_FLOAT(res, _fa - _fb * floor(_fa / _fb)); \
    } \
} while(0)

#define OP_BIT_IMPL(a, b, res, op) do { int64_t _ai; int64_t _bi; CheckInt(a, _ai); CheckInt(b, _bi); SET_INT(res, _ai op _bi); } while(0)

#define OpBitAnd(a, b, res) OP_BIT_IMPL(a, b, res, &)
#define OpBitXor(a, b, res) OP_BIT_IMPL(a, b, res, ^)
#define OpBitOr(a, b, res) OP_BIT_IMPL(a, b, res, |)

#define OP_SHIFT_IMPL(a, b, res, _right) do { \
    int64_t _ai; int64_t _bi; CheckInt(a, _ai); CheckInt(b, _bi); \
    if (_bi >= 64 || _bi <= -64) { SET_INT(res, 0); } \
    else if (_bi >= 0) { SET_INT(res, (int64_t)((_right) ? ((uint64_t)_ai >> _bi) : ((uint64_t)_ai << _bi))); } \
    else { SET_INT(res, (int64_t)((_right) ? ((uint64_t)_ai << (-_bi)) : ((uint64_t)_ai >> (-_bi)))); } \
} while(0)

#define OpRightShift(a, b, res) OP_SHIFT_IMPL(a, b, res, 1)
#define OpLeftShift(a, b, res) OP_SHIFT_IMPL(a, b, res, 0)

#define OP_CMP_IMPL(a, b, res, op) do { \
    CVar _ra = (a); CVar _rb = (b); CheckNum(_ra); CheckNum(_rb); \
    if (LIKELY(_ra.type_ == VAR_INT && _rb.type_ == VAR_INT)) { SET_BOOL(res, _ra.data_.i op _rb.data_.i); } \
    else { SET_BOOL(res, CVAR_TO_DOUBLE(_ra) op CVAR_TO_DOUBLE(_rb)); } \
} while(0)

#define OpLt(a, b, res) OP_CMP_IMPL(a, b, res, <)
#define OpGt(a, b, res) OP_CMP_IMPL(a, b, res, >)
#define OpLe(a, b, res) OP_CMP_IMPL(a, b, res, <=)
#define OpGe(a, b, res) OP_CMP_IMPL(a, b, res, >=)

#define OpEq(a, b, res) do { bool _eq; VarEqual(a, b, _eq); SET_BOOL(res, _eq); } while(0)

#define OpNe(a, b, res) do { bool _eq; VarEqual(a, b, _eq); SET_BOOL(res, !_eq); } while(0)

#define OpNot(a, res) do { bool _bt; IsTrue(a, _bt); SET_BOOL(res, !_bt); } while(0)

#define OpUnaryMinus(a, res) do { \
    CVar _ra = (a); CheckNum(_ra); \
    if (LIKELY(_ra.type_ == VAR_INT)) { SET_INT(res, -_ra.data_.i); } \
    else { SET_FLOAT(res, -_ra.data_.f); } \
} while(0)

#define OpBitNot(a, res) do { int64_t _ai; CheckInt(a, _ai); SET_INT(res, ~_ai); } while(0)

#define OpLen(a, res) do { \
    CVar _lv = (a); \
    if (LIKELY(_lv.type_ == VAR_STRING)) { SET_INT(res, STR_SIZE(_lv.data_.s)); } \
    else if (_lv.type_ == VAR_STRINGID) { SET_INT(res, STR_SIZE((VarString *)_lv.data_.i)); } \
    else if (_lv.type_ == VAR_TABLE) { SET_INT(res, TABLE_SIZE(_lv.data_.t)); } \
    else { FakeluaThrowError(_S, "attempt to get length of a non-string/table value"); } \
} while(0)

#define OpConcat(a, b, res) do { res = FlConcat(a, b); } while(0)

static inline int FlVarToStr(CVar v, char *buf, int buf_size) {
    switch (v.type_) {
        case VAR_NIL: memcpy(buf, "nil", 3); buf[3] = '\0'; return 3;
        case VAR_BOOL: if (v.data_.b) { memcpy(buf, "true", 4); buf[4] = '\0'; return 4; } else { memcpy(buf, "false", 5); buf[5] = '\0'; return 5; }
        case VAR_INT: return snprintf(buf, buf_size, "%lld", (long long)v.data_.i);
        case VAR_FLOAT: return snprintf(buf, buf_size, "%.17g", v.data_.f);
        case VAR_STRING:
        case VAR_STRINGID: FakeluaThrowError(_S, "FlVarToStr: string type should be handled by caller"); return 0;
        default: { int __n = snprintf(buf, buf_size - 1, "table(0x%llx", (unsigned long long)(uintptr_t)v.data_.t); if (__n > 0 && __n < buf_size - 1) { buf[__n] = ')'; buf[__n + 1] = '\0'; return __n + 1; } return __n; }
    }
}

static inline CVar FlConcat(CVar a, CVar b) {
    char buf_a[256], buf_b[256];
    const char *sa = buf_a; int la;
    const char *sb = buf_b; int lb;
    if (LIKELY(a.type_ == VAR_STRING)) { sa = STR_DATA(a.data_.s); la = STR_SIZE(a.data_.s); }
    else if (UNLIKELY(a.type_ == VAR_STRINGID)) { VarString *vs = (VarString *)a.data_.i; sa = STR_DATA(vs); la = STR_SIZE(vs); }
    else { la = FlVarToStr(a, buf_a, sizeof(buf_a)); }
    if (LIKELY(b.type_ == VAR_STRING)) { sb = STR_DATA(b.data_.s); lb = STR_SIZE(b.data_.s); }
    else if (UNLIKELY(b.type_ == VAR_STRINGID)) { VarString *vs = (VarString *)b.data_.i; sb = STR_DATA(vs); lb = STR_SIZE(vs); }
    else { lb = FlVarToStr(b, buf_b, sizeof(buf_b)); }
    int total = la + lb;
    if (UNLIKELY(total < la || total < lb)) { FakeluaThrowError(_S, "string concatenation result too long"); }
    VarString *vs = (VarString *)FakeluaAlloc(_S, sizeof(VarString) + total, !__fakelua_init_flag__);
    vs->size_ = total;
    vs->hash_ = 0;
    memcpy(vs->data_, sa, la);
    memcpy(vs->data_ + la, sb, lb);
    CVar result;
    result.type_ = VAR_STRING;
    result.flag_ = 0;
    result.data_.s = vs;
    return result;
}

#define GET_TABLE_ENTRY(tbl, idx, k, v) do { \
    uint32_t __spec_cnt = (tbl).data_.t->spec_count; \
    if (LIKELY((idx) < __spec_cnt)) { \
        (k) = (tbl).data_.t->spec_keys[(idx)]; \
        (v) = (tbl).data_.t->spec_vals[(idx)]; \
    } else if (LIKELY((tbl).data_.t->bucket_count_ == 0)) { \
        (k) = (tbl).data_.t->quick_data_[(idx) - __spec_cnt].key; \
        (v) = (tbl).data_.t->quick_data_[(idx) - __spec_cnt].val; \
    } else { \
        uint32_t __gti_node_idx = (tbl).data_.t->active_list_[(idx) - __spec_cnt]; \
        (k) = (tbl).data_.t->nodes_[__gti_node_idx].entry.key; \
        (v) = (tbl).data_.t->nodes_[__gti_node_idx].entry.val; \
    } \
} while(0)

#define FlFloorDivQuotient(a, b, q) do { \
    (q) = (a) / (b); \
    if (((a) ^ (b)) < 0 && (a) % (b) != 0) { (q) -= 1; } \
} while(0)

#define FlFloorDivInt(a, b, result) do { \
    int64_t __fl_a = (a); int64_t __fl_b = (b); \
    if (UNLIKELY(__fl_b == 0)) { FakeluaThrowError(_S, "floor division by zero"); } \
    int64_t __fl_q; \
    FlFloorDivQuotient(__fl_a, __fl_b, __fl_q); \
    (result) = __fl_q; \
} while(0)

#define FlModInt(a, b, result) do { \
    int64_t __fm_a = (a); int64_t __fm_b = (b); \
    if (UNLIKELY(__fm_b == 0)) { FakeluaThrowError(_S, "modulo by zero"); } \
    int64_t __fm_q; \
    FlFloorDivQuotient(__fm_a, __fm_b, __fm_q); \
    (result) = __fm_a - __fm_b * __fm_q; \
} while(0)

#define FlModFloat(a, b, result) do { \
    double __fmf_a = (a); double __fmf_b = (b); \
    (result) = __fmf_a - __fmf_b * floor(__fmf_a / __fmf_b); \
} while(0)

#define FlToIntChecked(v, result) do { \
    double __fi_d = (double)(v); \
    if (!isfinite(__fi_d)) { FakeluaThrowError(_S, "number has no integer representation"); } \
    double __fi_ip; \
    if (modf(__fi_d, &__fi_ip) != 0.0) { FakeluaThrowError(_S, "number has no integer representation"); } \
    if (__fi_ip < (double)INT64_MIN || __fi_ip >= 9223372036854775808.0) { FakeluaThrowError(_S, "number has no integer representation"); } \
    (result) = (int64_t)__fi_ip; \
} while(0)

#define FlShiftIntImpl(a, b, result, _right) do { \
    int64_t __sh_a = (a); int64_t __sh_b = (b); \
    if (UNLIKELY(__sh_b >= 64 || __sh_b <= -64)) { \
        (result) = (int64_t)0; \
    } else if (__sh_b >= 0) { \
        (result) = (int64_t)((_right) ? ((uint64_t)__sh_a >> __sh_b) : ((uint64_t)__sh_a << __sh_b)); \
    } else { \
        (result) = (int64_t)((_right) ? ((uint64_t)__sh_a << (-__sh_b)) : ((uint64_t)__sh_a >> (-__sh_b))); \
    } \
} while(0)

#define FlLShiftInt(a, b, result) FlShiftIntImpl(a, b, result, 0)

#define FlRShiftInt(a, b, result) FlShiftIntImpl(a, b, result, 1)

#define FlLenInt(v, result) do { \
    CVar __fl_v = (v); \
    if (LIKELY(__fl_v.type_ == VAR_STRING)) { \
        (result) = (int64_t)STR_SIZE(__fl_v.data_.s); \
    } else if (UNLIKELY(__fl_v.type_ == VAR_STRINGID)) { \
        (result) = (int64_t)STR_SIZE((VarString *)__fl_v.data_.i); \
    } else if (UNLIKELY(__fl_v.type_ == VAR_TABLE)) { \
        (result) = (int64_t)TABLE_SIZE(__fl_v.data_.t); \
    } else { \
        FakeluaThrowError(_S, "attempt to get length of a non-string/table value"); \
        (result) = 0; \
    } \
} while(0)

)";

}// namespace fakelua
