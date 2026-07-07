#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace fakelua {

// ── Type kind enumeration ──────────────────────────────────────────────────
enum class TypeKind : uint8_t {
    TY_INT,
    TY_FLOAT,
    TY_STRING,
    TY_BOOL,
    TY_NIL,
    TY_DYNAMIC,
    TY_VAR,         // Type variable (HM)
    TY_FUN,         // Function type
    TY_RECORD,      // Record (closed)
    TY_RECORD_OPEN, // Open record
    TY_ARRAY,       // Array part
    TY_UNION,       // Union of types
};

inline const char *TypeKindToString(TypeKind k) {
    switch (k) {
        case TypeKind::TY_INT:          return "int";
        case TypeKind::TY_FLOAT:        return "float";
        case TypeKind::TY_STRING:       return "string";
        case TypeKind::TY_BOOL:         return "bool";
        case TypeKind::TY_NIL:          return "nil";
        case TypeKind::TY_DYNAMIC:      return "dynamic";
        case TypeKind::TY_VAR:          return "var";
        case TypeKind::TY_FUN:          return "fun";
        case TypeKind::TY_RECORD:       return "record";
        case TypeKind::TY_RECORD_OPEN:  return "record_open";
        case TypeKind::TY_ARRAY:        return "array";
        case TypeKind::TY_UNION:        return "union";
    }
    return "?";
}

// ── Forward declaration ────────────────────────────────────────────────────
struct Type;

// ── Arena allocator for Type nodes ─────────────────────────────────────────
// A simple bump allocator that resets as a whole.  Used to avoid per-Type
// heap allocation during type inference; all Type nodes live for the lifetime
// of a single analysis (one Analyze call).
class TypeArena {
public:
    TypeArena() = default;
    TypeArena(const TypeArena &) = delete;
    TypeArena &operator=(const TypeArena &) = delete;

    void Reset();
    [[nodiscard]] size_t BytesUsed() const { return offset_; }

    // Allocate n bytes (no spec — bump allocator, implementation-aligned).
    void *Alloc(size_t n);

    // Convenience typed allocators.
    // NOTE: For types with non-trivial constructors (e.g. std::vector members),
    // use Construct<T>(args...) instead.
    template <typename T>
    T *Alloc() {
        return static_cast<T *>(Alloc(sizeof(T)));
    }

    template <typename T>
    T *AllocArray(size_t count) {
        return static_cast<T *>(Alloc(sizeof(T) * count));
    }

    // Allocate AND construct a T with the given arguments (placement new).
    template <typename T, typename... Args>
    T *Construct(Args &&...args) {
        void *p = Alloc(sizeof(T));
        return new (p) T(std::forward<Args>(args)...);
    }

    // Duplicate a std::string into the arena.
    const char *AllocString(const std::string &s);

    // Duplicate a buffer.
    void *AllocCopy(const void *src, size_t n);

private:
    static constexpr size_t kBlockSize = 64 * 1024;  // 64 KiB per block

    std::vector<void *> blocks_;
    size_t offset_ = 0;
};

// ── Record field ───────────────────────────────────────────────────────────
struct RecordField {
    const char *name = nullptr;
    const char *c_field_name = nullptr;
    Type *type = nullptr;
    bool optional = false;
    bool is_int_key = false;
};

// ── Type representation ────────────────────────────────────────────────────
struct Type {
    TypeKind kind = TypeKind::TY_DYNAMIC;

    // For TY_VAR
    int var_id = -1;          // Variable ID (valid when kind == TY_VAR)
    Type *bound = nullptr;    // Bound type (for prune chain)

    // For TY_FUN
    std::vector<Type *> params;  // Parameter types (empty → consult nparams)
    int nparams = 0;
    Type *ret = nullptr;

    // For TY_RECORD / TY_RECORD_OPEN
    std::vector<RecordField> fields;
    bool is_open = false;
    int layout_id = -1;

    // For TY_ARRAY
    Type *elem = nullptr;

    // For TY_UNION
    std::vector<Type *> members;
};

// ── Convenience constructors (allocate from an arena) ─────────────────────
// All returned pointers are owned by the arena and remain valid until the
// arena is reset.
namespace HmType {

Type *MakeVar(TypeArena &arena, int id);

// Primitive singleton types are shared (allocated once on the arena at init).
Type *MakePrim(TypeArena &arena, TypeKind k);

Type *MakeFun(TypeArena &arena, std::vector<Type *> params, Type *ret);
Type *MakeFunN(TypeArena &arena, int nparams, Type *ret);

Type *MakeRecord(TypeArena &arena, bool open = false);
inline Type *MakeOpenRecord(TypeArena &arena) { return MakeRecord(arena, true); }

Type *MakeArray(TypeArena &arena, Type *elem);

Type *MakeUnion(TypeArena &arena, std::vector<Type *> members);

}// namespace HmType

// ── Type variable table ────────────────────────────────────────────────────
class TypeVarTable {
public:
    static constexpr int MAX_VARS = 4096;

    explicit TypeVarTable(TypeArena &arena) : arena_(arena) {}

    // Create a new type variable (always TY_VAR, unbound).
    Type *NewVar();

    // Prune: follow binding chain to find the ultimately bound (non-TY_VAR)
    // type, or the unbound variable at the chain's end.
    static Type *Prune(Type *t);

    // Occurs check: does var (after pruning) appear anywhere in t (after
    // pruning)?  Returns true if var occurs in t, indicating an infinite type.
    static bool OccursCheck(Type *var, Type *t);

    // Unify two types.  Binds type variables as needed.  Returns true on
    // success, false on type mismatch.
    bool Unify(Type *a, Type *b);

    // Intended-partial unification for record subtyping.  Like Unify but when a
    // record field cannot be matched it is instead set to TY_DYNAMIC rather
    // than failing outright (used for open-record / recursive-type degrade).
    bool UnifySoft(Type *a, Type *b);

    // Reset for a fresh analysis.  Clears all variables back to unbound.
    void Reset();

    // Number of allocated variables.
    [[nodiscard]] int VarCount() const { return next_var_id_; }

    // The arena this table allocates Type nodes from.
    TypeArena &arena() { return arena_; }

private:
    Type vars_[MAX_VARS];
    int next_var_id_ = 0;
    TypeArena &arena_;

    // Bind var to target (var must be a TY_VAR and unbound after pruning).
    void BindVar(Type *var, Type *target);

    // Helper: meet/combine into the TY_VAR's bound without fully unifying.
    // Returns true on success.
    static bool BindTo(Type *var, Type *target);
};

// ── Global convenience (optional) ─────────────────────────────────────────
// A single process-wide table.  Initialised lazily; most code should pass an
// explicit TypeVarTable & arena instead.
extern TypeVarTable *g_type_table;
extern TypeArena *g_type_arena;
void InitTypeTable();
void ShutdownTypeTable();

}// namespace fakelua
