#include "compile/hm_type.h"
#include <cstdlib>
#include <cstring>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────
// TypeArena
// ─────────────────────────────────────────────────────────────────────────

void TypeArena::Reset() {
    for (void *blk: blocks_) std::free(blk);
    blocks_.clear();
    offset_ = 0;
}

void *TypeArena::Alloc(size_t n) {
    // 把 n 圆整到 8 字节对齐——Type 里都是指针，对齐后天然满足 max_align 要求
    n = (n + 7) & ~size_t(7);
    // 当前 block 已经没有足夠空间时，追加一个新 block
    if (blocks_.empty() || offset_ + n > kBlockSize) {
        void *blk = std::malloc(kBlockSize);
        blocks_.push_back(blk);
        offset_ = 0;
    }
    // 从当前 block 的 offset_ 位置切出 n 字节，推进游标
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
// 基础类型单例（每种 kind 全局只构造一次）
// ─────────────────────────────────────────────────────────────────────────
//
// 为什么需要单例：
//   - int/float/string/bool/nil/dynamic 这些类型没有内部状态，
//     所有"整数类型"在语义上完全等价
//   - 单例化后，Unify 里可以直接用 `a == b` 指针比较作为 fast path，
//     避免每次比较都走进 switch 分支
//   - 也节省内存：一个 int 字面量不再需要单独分配 Type 节点
namespace {
Type *MakePrimStatic(TypeArena &arena, TypeKind k) {
    Type *t = arena.Construct<Type>();
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
}// namespace

// 单例缓存：thread_local 保证多线程互不干扰
// 第一次被请求时由 EnsurePrims 一次性创建所有基础类型
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
        g_prims.int_ty = MakePrimStatic(arena, TypeKind::TY_INT);
        g_prims.float_ty = MakePrimStatic(arena, TypeKind::TY_FLOAT);
        g_prims.string_ty = MakePrimStatic(arena, TypeKind::TY_STRING);
        g_prims.bool_ty = MakePrimStatic(arena, TypeKind::TY_BOOL);
        g_prims.nil_ty = MakePrimStatic(arena, TypeKind::TY_NIL);
        g_prims.dynamic_ty = MakePrimStatic(arena, TypeKind::TY_DYNAMIC);
    }
}
}// namespace

// ─────────────────────────────────────────────────────────────────────────
// HmType constructors
// ─────────────────────────────────────────────────────────────────────────
namespace HmType {

Type *MakeVar(TypeArena &arena, int id) {
    Type *t = arena.Construct<Type>();
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
        case TypeKind::TY_INT:
            return g_prims.int_ty;
        case TypeKind::TY_FLOAT:
            return g_prims.float_ty;
        case TypeKind::TY_STRING:
            return g_prims.string_ty;
        case TypeKind::TY_BOOL:
            return g_prims.bool_ty;
        case TypeKind::TY_NIL:
            return g_prims.nil_ty;
        case TypeKind::TY_DYNAMIC:
            return g_prims.dynamic_ty;
        default:
            return g_prims.dynamic_ty;
    }
}

Type *MakeFun(TypeArena &arena, std::vector<Type *> params, Type *ret) {
    Type *t = arena.Construct<Type>();
    t->kind = TypeKind::TY_FUN;
    t->nparams = 0;
    t->ret = ret;
    t->params = std::move(params);
    return t;
}

Type *MakeFunN(TypeArena &arena, int nparams, Type *ret) {
    Type *t = arena.Construct<Type>();
    t->kind = TypeKind::TY_FUN;
    t->nparams = nparams;
    t->ret = ret;
    return t;
}

Type *MakeRecord(TypeArena &arena, bool open) {
    Type *t = arena.Construct<Type>();
    t->kind = open ? TypeKind::TY_RECORD_OPEN : TypeKind::TY_RECORD;
    t->is_open = open;
    t->layout_id = -1;
    return t;
}

Type *MakeArray(TypeArena &arena, Type *elem) {
    Type *t = arena.Construct<Type>();
    t->kind = TypeKind::TY_ARRAY;
    t->elem = elem;
    return t;
}

Type *MakeUnion(TypeArena &arena, std::vector<Type *> members) {
    Type *t = arena.Construct<Type>();
    t->kind = TypeKind::TY_UNION;
    t->members = std::move(members);
    return t;
}

}// namespace HmType

// ─────────────────────────────────────────────────────────────────────────
// TypeVarTable
// ─────────────────────────────────────────────────────────────────────────

Type *TypeVarTable::NewVar() {
    if (next_var_id_ >= MAX_VARS) {
        // 变量槽用完——退化返回 dynamic，保证分析继续而不是崩溃
        // （极端情况下出现，通常意味着表达式非常复杂或存在无限展开）
        return HmType::MakePrim(arena_, TypeKind::TY_DYNAMIC);
    }
    // 直接在 vars_ 数组槽位上"重新初始化"——因为 Type 不是 arena 分配的，
    // 不能 placement new（已经有对象了），只能逐字段覆盖
    Type *v = &vars_[next_var_id_];
    v->kind = TypeKind::TY_VAR;
    v->var_id = next_var_id_;// var_id 等于下标，支持 O(1) 按 id 反查
    v->bound = nullptr;      // 初始未绑定
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
    // 迭代方式沿 bound 走到尽头——用 while 而不是递归，避免类型特别深时爆栈
    // 注意：这里不做路径压缩（path compression），
    // 因为 bump arena 绑定的 bound 指针不允许被轻易改写（可能影响其他引用）
    while (t && t->kind == TypeKind::TY_VAR && t->bound) {
        t = t->bound;
    }
    return t;
}

// ── Occurs check ──────────────────────────────────────────────────────────
//
// 目的：防止把变量绑到包含它自己的类型。
//   例如 α = α → int：
//      Prune(α) = α；如果不 OccursCheck 就直接绑上去，后续 Unify 会沿着 bound 链无限展开。
//
// 实现要点：
//   - 跟踪已访问的 var_id 集合，处理类型 DAG 里"一个变量在多个子结构里出现"的情况
//     避免遍历无限循环（图上有环）
//   - 使用 visited 去重而非假设输入是树，是对"类型图"（而非类型树）的稳健处理
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
        for (int v: st.visited)
            if (v == t->var_id) return false;
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
            for (Type *p: t->params)
                if (OccursWalk(p, st)) return true;
            return OccursWalk(t->ret, st);
        case TypeKind::TY_RECORD:
        case TypeKind::TY_RECORD_OPEN: {
            for (const auto &f: t->fields)
                if (OccursWalk(f.type, st)) return true;
            return false;
        }
        case TypeKind::TY_ARRAY:
            return OccursWalk(t->elem, st);
        case TypeKind::TY_UNION:
            for (Type *m: t->members)
                if (OccursWalk(m, st)) return true;
            return false;
        case TypeKind::TY_VAR:
            return false;// handled by Prune above
    }
    return false;
}
}// namespace

bool TypeVarTable::OccursCheck(Type *var, Type *t) {
    var = Prune(var);
    if (!var || var->kind != TypeKind::TY_VAR) return false;
    OccursState st;
    st.var_id = var->var_id;
    return OccursWalk(t, st);
}

// ── Unify 核心算法（HM 统一化） ───────────────────────────────────────────
//
// Unify 是整个 HM 推断的灵魂——它把"两个表达式应该类型相同"的等式约束
// 注进类型变量表里，并在冲突时返回 false。
//
// 算法大纲（按优先级从高到低）：
//   1. 同指针 fast path：a == b 直接成功
//   2. Prune 两个操作数——把任何已绑的变量跳到它们指向的实际类型
//      （这是关键：不 prune 可能看到"旧的中间指针"而错过已绑的答案）
//   3. 再次检查 a == b（两个变量可能 prune 到同一个目标）
//   4. 若 a 是变量：OccursCheck(a, b)；通过则绑 a.bound = b
//   5. 若 b 是变量：对称处理
//   6. 都不是变量：
//      - kind 不同 → 失败
//      - 按 kind 递归匹配子结构（单例直接成功；复合类型逐个字段/成员递归）
//
// 对算法正确性的直觉：
//   - 绑定是不可逆的单向链；查询靠 Prune 沿着链走
//   - 不允许重绑；一旦绑定就是永久结论
//   - OccursCheck 保证不会引入无限类型——这是"简单 HM 不原生支持 recursive types"的体现
//
// 失败处理策略：
//   - Unify 失败表示"类型不相容"，上层可决定：
//     (a) 直接放弃当前推断分支
//     (b) 调用 UnifySoft 尝试降级（见本文件后半段）
//     (c) 将涉及到的变量/字段统一绑定到 TY_DYNAMIC
namespace {
// 前置声明——因为 UnifyImpl / BindTo / UnifyRecordFields 互相递归调用
bool UnifyImpl(TypeVarTable *table, Type *a, Type *b);

bool BindTo(Type *var, Type *target) {
    // 调用方必须先 Prune var，确保它是未绑定的 TY_VAR
    var->bound = target;
    return true;
}

// helper：对两个 record 类型按字段名对齐并递归 unify
//
// 规则（与 shape_type.h 的 ShapeRegistry::Meet 类似，但这里是"等式"不是"上界"）：
//   - 两侧都有的字段：类型必须 unify 成功
//   - 仅单侧有的字段：is_open 那一侧可以容忍缺失（未知字段走 hash）
//     两侧都不 open 则失败
//
// 注意：Unify 的后缀参数 `table` 在 UnifyRecordFields 中传 nullptr 也没影响
// （因为内部递归调用的 UnifyImpl 不在 BindVar 路径里用到 table）
bool UnifyRecordFields(Type *ra, Type *rb) {
    // 遍历 a 的每个字段
    for (auto &fa: ra->fields) {
        RecordField *fb = nullptr;
        for (auto &f: rb->fields) {
            if (f.name == fa.name) {
                fb = &f;
                break;
            }
        }
        if (fb) {
            // 两侧都有——类型必须严格相等（递归 unify）
            if (!UnifyImpl(nullptr, fa.type, fb->type)) return false;
        } else {
            // a 有、b 没有：
            //   - b 是开放 record → OK（b 可以拥有更多未知字段）
            //   - b 是封闭 record → 字段齐性不够，失败
            if (!rb->is_open) {
                return false;
            }
        }
    }
    // 再遍历 b 的每个字段（方向相反，检查 b 多出来的字段 a 是否能容忍）
    for (auto &fb: rb->fields) {
        RecordField *fa = nullptr;
        for (auto &f: ra->fields) {
            if (f.name == fb.name) {
                fa = &f;
                break;
            }
        }
        if (!fa) {
            if (!ra->is_open) return false;
        }
    }
    return true;
}

// Unify 真正的实现（不是 TypeVarTable::Unify 的公共接口）
bool UnifyImpl(TypeVarTable *table, Type *a, Type *b) {
    // (1) 同指针：天然的 fast path
    if (a == b) return true;

    // (2) 对两个操作数做 Prune，把一跳一跳的变量链"压平"到最终类型
    //     ★ 关键：Prune 必须在第一步做，否则会看到未更新的中间指针
    while (a && a->kind == TypeKind::TY_VAR && a->bound) a = a->bound;
    while (b && b->kind == TypeKind::TY_VAR && b->bound) b = b->bound;

    // (3) 例如：αᵦ 和ᵦ 都绑到 int，prune 后就 a == b，立刻成功
    if (a == b) return true;

    // (4) a 是未绑变量：绑 a = b
    //     注意：a == b 的情况早处理过，所以这里 a ≠ b
    if (a && a->kind == TypeKind::TY_VAR) {
        // 先 OccursCheck：不能让 a 出现在 b 里
        // 例：a=α, b=α → (3) 已处理
        //      a=α, b=int → OK，直接绑
        //      a=α, b=α→int → 失败（会形成无限类型）
        if (TypeVarTable::OccursCheck(a, b)) {
            // 严格 HM 的直接返回 false 让上层做降级处理，
            // 不允许这里静默绑 dynamic（那是 UnifySoft 的职责）
            return false;
        }
        // 无论 table 是否为 nullptr 目前实现相同；保留分岔是给将来 table 走不同绑定策略留口
        if (table) {
            a->bound = b;
        } else {
            a->bound = b;
        }
        return true;
    }
    // (5) b 是未绑变量：与上对称
    if (b && b->kind == TypeKind::TY_VAR) {
        if (TypeVarTable::OccursCheck(b, a)) return false;
        b->bound = a;
        return true;
    }

    // (6) 两侧都不是变量——要求 kind 和结构都相容
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
        // 单例类型：走到这里说明 kind 相同，直接成功（地址未必相同，因为变量 prune 后可能不走单例）
        case TypeKind::TY_INT:
        case TypeKind::TY_FLOAT:
        case TypeKind::TY_STRING:
        case TypeKind::TY_BOOL:
        case TypeKind::TY_NIL:
        case TypeKind::TY_DYNAMIC:
            return true;
        // 函数类型：参数数量对齐，参数列表（若双方都已具体化）和返回值分别递归
        case TypeKind::TY_FUN: {
            // 参数数量从"显式 params 向量"或"nparams 标记"两个出口获取
            int an = a->params.empty() ? a->nparams : (int) a->params.size();
            int bn = b->params.empty() ? b->nparams : (int) b->params.size();
            if (an != bn) return false;
            if (!UnifyImpl(table, a->ret, b->ret)) return false;
            // 只有双方都已具体化参数类型时才逐参数 unify（否则只知道参数个数）
            if (!a->params.empty() && !b->params.empty()) {
                for (size_t i = 0; i < a->params.size(); ++i) {
                    if (!UnifyImpl(table, a->params[i], b->params[i])) return false;
                }
            }
            return true;
        }
        // record：按名字匹配字段并递归
        case TypeKind::TY_RECORD:
        case TypeKind::TY_RECORD_OPEN:
            return UnifyRecordFields(a, b);
        // array：elem 类型必须相等
        case TypeKind::TY_ARRAY:
            return UnifyImpl(table, a->elem, b->elem);
        // union：逐成员递归——顺序敏感，见头文件 Type::members 注释
        case TypeKind::TY_UNION:
            if (a->members.size() != b->members.size()) return false;
            for (size_t i = 0; i < a->members.size(); ++i) {
                if (!UnifyImpl(table, a->members[i], b->members[i])) return false;
            }
            return true;
        // 两个未绑变量走到这里说明 var_id 不同（a == b 已处理过），不能再绑——失败
        case TypeKind::TY_VAR:
            return a == b;
    }
    return false;
}
}// namespace

// 公共接口：直接用 this 作为 table 调用 UnifyImpl
bool TypeVarTable::Unify(Type *a, Type *b) {
    return UnifyImpl(this, a, b);
}

// 软 unify：在 unify 失败后尝试"挽救"
//
// 策略分三步：
//   1. 先按严格模式试一次——成功就直接返回（严格模式优先）
//   2. 失败后把两侧都 prune;
//      若其中一侧是未绑变量，直接把它绑到 TY_DYNAMIC（降级）
//      ——这是最便宜的挽救：把"不知道“变成”动态"，推断继续走
//   3. 如果两侧都不是变量但 kind 相同，还能再试一次 record 的特殊宽容路径：
//      UnifyRecordFields 对开放 record 天生宽容（允许缺失字段）
//
// 为什么不做"绑 struct 失败时把失败字段降级为 dynamic"这么细粒度：
//   实现成本 vs 收益权衡——目前失败场景中只要不是 record/变量，直接返回 false 由上层处理
bool TypeVarTable::UnifySoft(Type *a, Type *b) {
    if (Unify(a, b)) return true;

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

    // kind 不同的两个具体类型无法挽救——结构差距太大
    if (a->kind != b->kind) {
        return false;
    }

    // 同 kind 但结构不兼容——只对 record 作最后宽容处理
    switch (a->kind) {
        case TypeKind::TY_RECORD:
        case TypeKind::TY_RECORD_OPEN:
            return UnifyRecordFields(a, b);
        default:
            return false;
    }
}

void TypeVarTable::BindVar(Type *var, Type *target) {
    // 外部可以安全传入任何 var 指针——它会先 Prune 找到真正代表当前变量身份的实体
    var = Prune(var);
    if (!var || var->kind != TypeKind::TY_VAR) return;// 非变量或空：什么都不做
    var->bound = target;
}

bool TypeVarTable::BindTo(Type *var, Type *target) {
    var = Prune(var);
    if (!var || var->kind != TypeKind::TY_VAR) return false;// 失败返回 false
    var->bound = target;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────
// 全局便利设施的实现
// ─────────────────────────────────────────────────────────────────────────
namespace {
// 把 arena 和 table 放在一个 struct 里，用 thread_local 存储：
//   - arena 和 table 必须一一对应（table 绑定变量时分配的复合类型存在 arena 里）
//   - 按线程隔离测试
//   - Init 首次调用时廉价创建，Shutdown 时一次性释放
struct GlobalHMState {
    TypeArena arena;
    TypeVarTable table;

    GlobalHMState() : table(arena) {
    }
};

thread_local GlobalHMState *g_state = nullptr;
}// namespace

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

}// namespace fakelua
