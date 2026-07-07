#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace fakelua {

// ── 类型节点种类枚举 ──────────────────────────────────────────────────────
//
// 这是 HM 类型系统中所有"类型表达式"的 kind 标签。
// 与 inferred_type.h 的简单枚举不同，这里每个 kind 都对应一个 Type 结构体实例，
// 可以嵌套（例如 TY_FUN 的 params 里可以放 TY_VAR，TY_RECORD 的 fields 里可以放 TY_ARRAY）。
//
// 设计要点：
//   - TY_VAR 是 HM 算法的核心：表示一个"待求解"的类型未知量
//   - TY_RECORD 与 TY_RECORD_OPEN 的区别：后者允许"存在未知字段"，
//     在 unify 时对字段缺失更宽容（见 UnifyRecordFields）
//   - TY_DYNAMIC 是"放弃推断"的兜底——与 inferred_type.h 的 T_DYNAMIC 对应
enum class TypeKind : uint8_t {
    TY_INT,          // 整数类型（单例，所有 TY_INT 共享同一个 Type*）
    TY_FLOAT,        // 浮点类型（单例）
    TY_STRING,       // 字符串类型（单例）
    TY_BOOL,         // 布尔类型（单例）
    TY_NIL,          // nil 类型（单例）
    TY_DYNAMIC,      // 完全未知类型（单例），运行时用 CVar 表示
    TY_VAR,          // HM 类型变量——推断过程中产生的"未知量"，可被绑定到具体类型
    TY_FUN,          // 函数类型：params → ret
    TY_RECORD,       // 封闭 record：字段集合固定，不允许未知字段
    TY_RECORD_OPEN,  // 开放 record：字段集合固定，但允许存在未知字段（走 hash）
    TY_ARRAY,        // 数组类型：所有元素共享一个 elem 类型
    TY_UNION,        // 联合类型：members 中任一类型都可能（按位置逐一 unify）
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

// ── Type 节点的 Arena 分配器 ───────────────────────────────────────────────
//
// 设计目标：避免在 HM 推断过程中对每个 Type 节点单独 new/delete。
//
// Bump 分配策略：
//   - 内部维护一个 64KiB 的 block 链表，从当前 block 的 offset_ 起追加分配
//   - 当当前 block 不够时，新 malloc 一个 block 并把 offset_ 归零
//   - 整个 Arena 的生命周期 = 一次 Analyze 调用；分析结束一次性 Reset
//     （不需要逐节点释放，也没有析构——所以 Type 含 vector 等非平凡成员的
//      特殊节点需要谨慎使用，或仅在 arena reset 前完全不再访问）
//
// 和 std::allocator / pmr 的区别：
//   - 更快（纯粹 bump）
//   - 不支持单独释放（也不需要）
//   - 不够时追加 block 而不是 realloc（避免指针失效）
//
// Construct<T>(args...) 为什么需要 placement new：
//   - Type 结构体含 std::vector 等非平凡成员（unexecutable ctor 时默认构造也会初始化）
//   - 普通的 Alloc<T>() 只拿到一片未初始化的内存，必须用 placement new 调起构造函数
//   - 这样构造的对象直接"诞生"在 arena 内存里，不需要额外的拷贝
class TypeArena {
public:
    TypeArena() = default;
    TypeArena(const TypeArena &) = delete;
    TypeArena &operator=(const TypeArena &) = delete;

    // 释放所有 block（会调用 std::free，但不调用 Type 的析构函数——因为 bump 设计不需要）
    void Reset();
    [[nodiscard]] size_t BytesUsed() const { return offset_; }

    // 从当前 block 分配 n 字节（内部会把 n 圆整到 8 字节，保证指针对齐）
    void *Alloc(size_t n);

    // 类型化的 Alloc（返回 T*）
    // 注意：只分配内存，**不**调用 T 的构造函数！
    // 仅用于 T 是 POD / 你打算手动初始化全部字段时。
    // 对于含 std::vector、std::string 等非平凡成员的 T，请改用 Construct<T>(args...)。
    template <typename T>
    T *Alloc() {
        return static_cast<T *>(Alloc(sizeof(T)));
    }

    // 同上，用于数组
    template <typename T>
    T *AllocArray(size_t count) {
        return static_cast<T *>(Alloc(sizeof(T) * count));
    }

    // 分配 + 构造一步到位（placement new）
    //
    // 为什么用 placement new 而不是先 Alloc 再手动赋值：
    //   - Type 里的 std::vector、std::string 等成员需要被构造函数正确初始化
    //   - 直接 memcpy 会跳过 ctor，导致这些成员的 internal state 异常
    //   - `new (p) T(args...)` 在已分配的内存 p 上调用 T 的构造函数，一步到位
    //
    // 典型用法：
    //   Type *t = arena.Construct<Type>();              // 默认构造
    //   auto *rec = arena.Construct<MyRec>(1, "name"); // 任意自定义构造函数
    template <typename T, typename... Args>
    T *Construct(Args &&...args) {
        void *p = Alloc(sizeof(T));
        return new (p) T(std::forward<Args>(args)...);
    }

    // 把 std::string 以 '\0' 结尾的 C 字符串形式拷进 arena——
    // 供 RecordField.name 等字段使用，避免外部 string 被析构后变成悬空指针
    const char *AllocString(const std::string &s);

    // 一份通用的内存拷贝设施（内部也是 bump，只是把 src 拷进去）
    void *AllocCopy(const void *src, size_t n);

private:
    static constexpr size_t kBlockSize = 64 * 1024;  // 每个 block 64 KiB

    std::vector<void *> blocks_;  // 已分配的所有 block（Reset 时逐个 free）
    size_t offset_ = 0;           // 当前 block 内的写入游标（下一笔分配的起点）
};

// ── Record 字段描述 ────────────────────────────────────────────────────────
// 用于 Type 中 TY_RECORD / TY_RECORD_OPEN 的 fields 向量元素
//
// name / c_field_name 使用 const char* 而不是 std::string：
//   - 它们指向由 TypeArena::AllocString 分配的内存
//   - 避免在 bump 分配的 Type 里嵌入非平凡的 std::string（防止析构问题）
//   - 整个 arena reset 时这些字符串随之失效，不需要单独释放
//
// 关于 optional / is_int_key 的语义，与 shape_type.h::FieldDef 一致：
//   - optional = true → 该字段在控制流的某条路径上可能不存在（φ 合并产物）
//   - is_int_key = true → 该字段来源于数组式 table 字面量（如 {1, 2, 3}）
struct RecordField {
    const char *name = nullptr;
    const char *c_field_name = nullptr;
    Type *type = nullptr;
    bool optional = false;
    bool is_int_key = false;
};

// ── HM 类型表示 ───────────────────────────────────────────────────────────
//
// 这是一个"union-like"（类共用体）结构体：kind 字段决定下面哪一组字段是活跃的。
// 没有用真实 union 的原因：std::vector 等非平凡成员在 union 里需要特殊处理（手动 ctor/dtor），
// 为了可读性和安全性退化为一个 kind-tagged struct——代价是内存略大（约 max(sizeof(各组字段))）。
//
// 各组活跃字段：
//
//   TY_VAR:    var_id, bound
//   TY_FUN:    params / nparams, ret
//   TY_RECORD/_OPEN: fields, is_open, layout_id
//   TY_ARRAY:  elem
//   TY_UNION:  members
//   单例类型（TY_INT 等）：只需要 kind 与全局单例指针匹配
//
// 关于 bound 字段（HM 算法的核心）：
//   - 类型变量（TY_VAR）被求解/绑定时，另一侧的类型会写入 bound
//   - 用链表形式串成"绑定链"：a → b → int，
//     查找实际类型就是顺着 bound 递归或迭代，这就是 Prune 操作的目的
//   - 绑定不可撤销（monovar 语义），所以推断失败时只能整体降级为 TY_DYNAMIC
//
// 关于 layout_id：
//   - 与 shape_type.h 的 shape_id 语义同源，记录此 record 在代码生成侧对应的 C 结构体布局
//   - 这里只是预留接口，实际赋值由 unified_type_analyzer 统一完成
struct Type {
    TypeKind kind = TypeKind::TY_DYNAMIC;

    // ── TY_VAR ──────────── var_id 唯一标识一个变量；bound 指向链中的下一个节点 ──
    int var_id = -1;          // 变量 ID（在 TypeVarTable 的 vars_ 数组里的下标）
    Type *bound = nullptr;    // "绑定"指针；未绑时为 nullptr，已绑时指向下一个 Type 节点
                              // 这是 HM 链式绑定的基础：Prune 就是沿 bound 一路走到尽头

    // ── TY_FUN ──────────── params 显式参数类型，ret 返回类型 ──────────────
    // 当 params 为空时：表示参数类型尚未具体化，用 nparams 表达"已知参数个数但未知类型"
    // 当 params 非空时：params.size() 就是参数个数，用于支持参数级的精确 unify
    std::vector<Type *> params;  // 参数类型列表（允许为空，表示仅知道 nparams）
    int nparams = 0;             // 参数个数（params 字段为空时的 fallback）
    Type *ret = nullptr;         // 函数返回类型

    // ── TY_RECORD / TY_RECORD_OPEN ─────────────────────────────────────────
    std::vector<RecordField> fields;  // record 的字段集合
    bool is_open = false;             // 是否开放（true → 允许存在未知字段）
    int layout_id = -1;               // 与 shape registry 对应的布局 id（见注释上方）

    // ── TY_ARRAY ───────────────────────────────────────────────────────────
    Type *elem = nullptr;  // 数组元素类型（所有位置共享一个类型——不是元组）

    // ── TY_UNION ───────────────────────────────────────────────────────────
    // 即 sum type：实际值是 members 中的某一项。
    // 当前实现采用"位置逐一 unify"策略：a.members[i] 必须与 b.members[i] unify。
    // 因此 union 的顺序有意义——这不是理想的"无序 union"，
    // 但作为推断的副产物已足够：推断产生同构的 union 才会被 unify。
    std::vector<Type *> members;
};

// ── 便捷构造函数（从指定 arena 分配） ─────────────────────────────────────
//
// 所有返回的指针归属 arena 所有——arena.reset() 之前一直有效，
// 不需要也不能单独 delete/free。
//
// 设计要点：
//   - 单例类型（int/float/bool/nil/dynamic/string）全局只构造一次，
//     后续每次 MakePrim 返回相同指针（见 hm_type.cpp 的 g_prims 缓存）
//     → Unify 里可以直接用 `a == b` 做 fast path
//   - 复合类型（fun/record/array/union/var）每次都是新的
namespace HmType {

// 创建一个指定 var_id 的 TY_VAR（未绑定状态）
// 注：日常使用应该通过 TypeVarTable::NewVar 构造——它负责 var_id 自增和 vars_ 存储
// 这个函数适合"我知道我要哪个 id"的跨上下文的场景
Type *MakeVar(TypeArena &arena, int id);

// 返回基础类型的单例指针（arena 第一次被请求时创建，之后复用）
// 如果传入的不是基础类型，会退回到 dynamic（与 HY 的 safe-ish 保持一致）
Type *MakePrim(TypeArena &arena, TypeKind k);

// 构造函数类型
//  - MakeFun：带完整参数类型列表
//  - MakeFunN：只关心参数个数、不关心参数类型（对应 varargs / 未知参数场景）
Type *MakeFun(TypeArena &arena, std::vector<Type *> params, Type *ret);
Type *MakeFunN(TypeArena &arena, int nparams, Type *ret);

// 构造 record 类型；open = true 则为开放 record（TY_RECORD_OPEN）
Type *MakeRecord(TypeArena &arena, bool open = false);
inline Type *MakeOpenRecord(TypeArena &arena) { return MakeRecord(arena, true); }

// 构造数组类型
Type *MakeArray(TypeArena &arena, Type *elem);

// 构造 union 类型（要求 members 已经按"可同构 unify"的顺序排好）
Type *MakeUnion(TypeArena &arena, std::vector<Type *> members);

}// namespace HmType

// ── HM 类型变量表 ─────────────────────────────────────────────────────────
//
// HM 算法的状态机。一份 TypeVarTable = 一套"未知量集合"。
//
// 核心流程（简化版）：
//   1. NewVar()   → 产生一个新的未知量（TY_VAR，var_id 自增）
//   2. Unify(a,b) → 把表达式之间的等式约束注入系统
//   3. Prune(t)   → 求解 t 当前最简形式（沿 bound 链走到尽头）
//
// 数据结构：
//   - vars_[MAX_VARS] 是变量本体，以数组形式静态存放在 TypeVarTable 里（不是 arena 上）。
//     目的是让 var_id 可以直接作下标 O(1) 找到对应 Type*。
//   - arena_ 是用来存储"变量绑定的目标类型"（fun/record/array 等复合类型）。
//   - next_var_id_ 是下一个可分配的变量 id；Reset() 归零即可开始新一轮分析。
//
// 为什么需要 Prune：
//   绑定是不可逆的单向链 a → b → int，多次 unify 会形成长链。
//   每次读变量前必须先 Prune 才能看到它的"真正"类型，否则可能只看到中间指针。
//   用 while 迭代实现是为了避开递归深度问题。
//
// 为什么需要 OccursCheck：
//   要绑 α = T 之前，必须确认 α 没有出现在 T 里。否则会产生无限/递归类型
//   （例如 α = α → int），导致后续 unify 无法终止。
//   OccursCheck 在绑定前做一次深度遍历，发现冲突则拒绝绑定。
//
// Unify 算法（核心，详见 hm_type.cpp UnifyImpl）：
//   1. 若 a == b，立即成功（包括单例类型的 fast path）
//   2. Prune 两个操作数，把任何已经绑定的变量跳到实际类型
//   3. 若 a 是未绑变量：OccursCheck(a, b) 通过则绑 a.bound = b
//   4. 若 b 是未绑变量：与上对称
//   5. 若都不是变量：要求 kind 相同，然后按 kind 递归 unify 子结构
//      int/float/... 单例：直接成功
//      fun：参数列表 + 返回值递归
//      record：按名字匹配字段，缺失字段看 is_open
//      array：elem 递归
//      union：按位置递归
//   6. 任何失败返回 false，让上层决定是否降级
class TypeVarTable {
public:
    static constexpr int MAX_VARS = 4096;  // 单次分析允许的变量最大数

    explicit TypeVarTable(TypeArena &arena) : arena_(arena) {}

    // 创建一个新的类型变量（TY_VAR，初始未绑定）
    // 若变量数已超上限，退化为 TY_DYNAMIC——保证分析不会因资源枯竭崩溃
    Type *NewVar();

    // Prune：沿 bound 链把变量"解引用"到它最终绑到的类型
    //   - 若变量最终绑到具体类型（非 TY_VAR），返回该具体类型
    //   - 若变量未绑（bound == nullptr），返回变量本身
    //   - 非 TY_VAR 输入直接返回自身
    static Type *Prune(Type *t);

    // OccursCheck：var 是否出现在 t 里？
    //   用于绑定 var = t 之前防止产生无限类型。
    //   实现时使用 visited 集合去重，避免在共享 DAG 中无限遍历。
    static bool OccursCheck(Type *var, Type *t);

    // Unify 两个类型，必要时会绑定变量。
    //   返回 true 表示成功；false 表示发现类型不相容（例如 int vs record）。
    bool Unify(Type *a, Type *b);

    // "软" unify：用于 record 子类型降级场景——
    //   若常规 Unify 失败，尝试把不兼容的一侧降级为 TY_DYNAMIC 而不是整体失败。
    //   这个策略让开放 record（open record）和不完整类型能在不抛错的前提下继续推断。
    bool UnifySoft(Type *a, Type *b);

    // 清空所有变量状态（不释放内存，只重置 next_var_id_）
    // 因为 vars_ 在栈/对象内，只要重置 id 起点就能复用
    void Reset();

    // 已分配的变量个数
    [[nodiscard]] int VarCount() const { return next_var_id_; }

    // 暴露 arena 给 Make* 工厂函数用
    TypeArena &arena() { return arena_; }

private:
    Type vars_[MAX_VARS];         // 变量本体（按 var_id 直接下标访问）
    int next_var_id_ = 0;         // 下一个空闲 var_id
    TypeArena &arena_;            // 复合类型节点的分配器

    // 内部工具：绑定 var = target
    //   调用方必须保证 var 是 Prune 后的未绑 TY_VAR，否则行为未定义
    void BindVar(Type *var, Type *target);

    // 另一个内部工具：直接写 var.bound = target（带 prune + 类型校验）
    //   与 BindVar 的差别是它多做了 prune 前置条件检查
    static bool BindTo(Type *var, Type *target);
};

// ── 全局便利设施（可选） ─────────────────────────────────────────────────
//
// 为每线程保留一份全局状态——让"不想传递 table/arena"的地方也能用 HM 类型。
// 内部使用 thread_local，所以多线程测试互不干扰。
//
// 设计建议：product 代码应尽量显式传递 TypeVarTable& / TypeArena&，
// 只在测试入口或 REPL 这类"最外层"用它。
//
// 注意：类型单例（int/float/... 的全局 TY_XXX 指针）也存在这个全局 arena 里。
extern TypeVarTable *g_type_table;
extern TypeArena *g_type_arena;

// 懒初始化全局 table/arena（第一次调用时创建）
void InitTypeTable();

// 销毁全局状态——任何指向其中 Type 的指针在调用后都会失效
void ShutdownTypeTable();

}// namespace fakelua
