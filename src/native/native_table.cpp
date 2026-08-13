#include "native/native_table.h"
#include "native/native_common.h"
#include "compile/c_runtime_header.h"
#include "jit/jit_error_boundary.h"
#include "native/native_object.h"
#include "native/native_string.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_closure.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────────
// 宿主侧 VarTable 操作核心。
//
// 这些辅助函数必须与 c_runtime_header.h 中对应的 Fl* 运行时函数保持一致的哈希
// 计算、桶布局与 rehash 策略。JIT 生成的代码和宿主 C++ 会读写同一批 VarTable，
// 只要两侧对「键落在哪个桶」的判断出现分歧，一侧写入的键在另一侧就会查不到。
// ─────────────────────────────────────────────────────────────────────────────
namespace {

constexpr int kNilType = static_cast<int>(VarType::Nil);
constexpr int kIntType = static_cast<int>(VarType::Int);
constexpr int kFloatType = static_cast<int>(VarType::Float);
constexpr int kStringType = static_cast<int>(VarType::String);
constexpr int kStringIdType = static_cast<int>(VarType::StringId);

uint32_t IntKeyHash(int64_t k) {
    return static_cast<uint32_t>(k ^ (k >> 32));
}

// 与 c_runtime_header.h 的 VarHash 宏对应。
uint32_t VarKeyHash(CVar k) {
    switch (k.type_) {
        case kNilType:
            return 0;
        case static_cast<int>(VarType::Bool):
            return k.data_.b ? 1 : 0;
        case kIntType:
            return IntKeyHash(k.data_.i);
        case kFloatType: {
            uint64_t bits = 0;
            std::memcpy(&bits, &k.data_.f, sizeof(bits));
            return static_cast<uint32_t>(bits ^ (bits >> 32));
        }
        case kStringType:
            return k.data_.s->Hash();
        case kStringIdType:
            return reinterpret_cast<const VarString *>(k.data_.i)->Hash();
        case static_cast<int>(VarType::Table):
            return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(k.data_.t) ^ (reinterpret_cast<uintptr_t>(k.data_.t) >> 32));
        case static_cast<int>(VarType::Closure):
            return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(k.data_.cl) ^ (reinterpret_cast<uintptr_t>(k.data_.cl) >> 32));
        default:
            return 0;
    }
}

// 与 c_runtime_header.h 的 VarEqual 宏对应。
bool VarKeyEqual(CVar a, CVar b) {
    const bool a_str = (a.type_ == kStringType || a.type_ == kStringIdType);
    const bool b_str = (b.type_ == kStringType || b.type_ == kStringIdType);
    if (a_str && b_str) {
        return KeyToStringView(a) == KeyToStringView(b);
    }
    if (a.type_ == kIntType && b.type_ == kFloatType) {
        return static_cast<double>(a.data_.i) == b.data_.f;
    }
    if (a.type_ == kFloatType && b.type_ == kIntType) {
        return a.data_.f == static_cast<double>(b.data_.i);
    }
    if (a.type_ != b.type_) return false;
    switch (a.type_) {
        case kNilType:
            return true;
        case static_cast<int>(VarType::Bool):
            return a.data_.b == b.data_.b;
        case kIntType:
            return a.data_.i == b.data_.i;
        case kFloatType:
            return a.data_.f == b.data_.f;
        case static_cast<int>(VarType::Table):
            return a.data_.t == b.data_.t;
        case static_cast<int>(VarType::Closure):
            return a.data_.cl == b.data_.cl;
        default:
            return false;
    }
}

// 与 c_runtime_header.h 的 FlTableHasIntKey 对应。
bool TableHasIntKey(const VarTable *t, int64_t k) {
    if (t->bucket_count_ == 0) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            const auto &qd = t->quick_data_[i];
            if (qd.key.type_ == kIntType && qd.key.data_.i == k && qd.val.type_ != kNilType) return true;
        }
        return false;
    }
    const uint32_t mask = t->bucket_count_ - 1;
    const auto *curr = &t->nodes_[IntKeyHash(k) & mask];
    while (true) {
        if (curr->entry.key.type_ == kIntType && curr->entry.key.data_.i == k && curr->entry.val.type_ != kNilType) return true;
        if (curr->next == VarTable::INVALID_INDEX) return false;
        curr = &t->nodes_[curr->next];
    }
}

int64_t SeqScanFrom(const VarTable *t, int64_t base) {
    while (TableHasIntKey(t, base + 1)) { base++; }
    return base;
}

bool SeqCacheable(const VarTable *t) {
    return t->spec == nullptr && t->spec_count == 0;
}

// 与 c_runtime_header.h 的 FlSeqNoteIntSet 对应；对同一次写入重复调用是幂等的。
void SeqNoteIntSet(VarTable *t, int64_t k, bool is_nil) {
    if (!t || t->seq_len_valid_ == 0) return;
    if (!SeqCacheable(t)) {
        t->seq_len_valid_ = 0;
        return;
    }
    if (is_nil) {
        if (k >= 1 && k <= t->seq_len_) t->seq_len_ = k - 1;
        return;
    }
    if (k == t->seq_len_ + 1) t->seq_len_ = SeqScanFrom(t, k);
}

// 与 c_runtime_header.h 的 NORMALIZE_TABLE_KEY 对应：整数值的浮点键归一化为整数键，
// 使 t[2.0] 与 t[2] 命中同一个槽。
CVar NormalizeTableKey(CVar k) {
    if (k.type_ == kFloatType && std::isfinite(k.data_.f)) {
        double integral = 0.0;
        if (std::modf(k.data_.f, &integral) == 0.0 && integral >= static_cast<double>(INT64_MIN) && integral <= static_cast<double>(INT64_MAX)) {
            k.type_ = kIntType;
            k.data_.i = static_cast<int64_t>(integral);
        }
    }
    return k;
}

uint32_t NextPowerOfTwo(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}

void TableRehashTo(State *s, VarTable *tbl, uint32_t min_buckets);

// 与 c_runtime_header.h 的 FlTableInsertRaw 对应。溢出节点耗尽时返回 false，
// 由调用方扩容后重试。
bool TableInsertRaw(VarTable *tbl, CVar key, CVar val, uint32_t hash) {
    const uint32_t mask = tbl->bucket_count_ - 1;
    const uint32_t idx = hash & mask;
    auto *main_node = &tbl->nodes_[idx];
    if (main_node->entry.key.type_ == kNilType) {
        static_cast<CVar &>(main_node->entry.key) = key;
        static_cast<CVar &>(main_node->entry.val) = val;
        main_node->entry.hash = hash;
        main_node->next = VarTable::INVALID_INDEX;
        main_node->active_pos = tbl->count_;
        tbl->active_list_[tbl->count_] = idx;
        tbl->count_++;
        return true;
    }
    uint32_t curr_idx = idx;
    while (true) {
        auto *curr = &tbl->nodes_[curr_idx];
        if (curr->entry.hash == hash && VarKeyEqual(curr->entry.key, key)) {
            static_cast<CVar &>(curr->entry.val) = val;
            return true;
        }
        if (curr->next == VarTable::INVALID_INDEX) break;
        curr_idx = curr->next;
    }
    if (tbl->free_list_idx_ == VarTable::INVALID_INDEX) return false;
    const uint32_t new_node_idx = tbl->free_list_idx_;
    auto *new_node = &tbl->nodes_[new_node_idx];
    tbl->free_list_idx_ = new_node->next;
    static_cast<CVar &>(new_node->entry.key) = key;
    static_cast<CVar &>(new_node->entry.val) = val;
    new_node->entry.hash = hash;
    new_node->next = main_node->next;
    main_node->next = new_node_idx;
    new_node->active_pos = tbl->count_;
    tbl->active_list_[tbl->count_] = new_node_idx;
    tbl->count_++;
    return true;
}

// 与 c_runtime_header.h 的 FlTableRehash 对应。内容不变，故序列长度缓存无需失效。
void TableRehash(State *s, VarTable *tbl) {
    TableRehashTo(s, tbl, 0);
}

// min_buckets>0 时至少扩到该容量（用于 table.move 预扩空目标表，避免逐次翻倍 rehash）。
void TableRehashTo(State *s, VarTable *tbl, uint32_t min_buckets) {
    const uint32_t old_count = tbl->count_;
    const uint32_t old_bucket_count = tbl->bucket_count_;
    auto *old_nodes = tbl->nodes_;

    uint32_t new_bucket_count = NextPowerOfTwo(old_count + 1);
    if (new_bucket_count <= old_bucket_count) new_bucket_count = old_bucket_count * 2;
    if (min_buckets > new_bucket_count) new_bucket_count = NextPowerOfTwo(min_buckets);
    if (new_bucket_count < 1) new_bucket_count = 1;

    auto &alloc = s->GetHeap().GetAllocator(false /* temp */);
    while (true) {
        const uint32_t overflow_count = new_bucket_count / 2;
        const uint32_t total_nodes = new_bucket_count + overflow_count;
        const size_t nodes_size = total_nodes * sizeof(VarTable::TableNode);
        auto *buffer = static_cast<char *>(alloc.Alloc(nodes_size + total_nodes * sizeof(uint32_t)));
        auto *new_nodes = reinterpret_cast<VarTable::TableNode *>(buffer);
        auto *new_active_list = reinterpret_cast<uint32_t *>(buffer + nodes_size);
        for (uint32_t i = 0; i < total_nodes; ++i) {
            new_nodes[i].entry.key.type_ = kNilType;
            new_nodes[i].next = VarTable::INVALID_INDEX;
            new_nodes[i].active_pos = VarTable::INVALID_INDEX;
        }

        auto *prev_nodes = tbl->nodes_;
        auto *prev_active_list = tbl->active_list_;
        const uint32_t prev_bucket_count = tbl->bucket_count_;
        const uint32_t prev_count = tbl->count_;
        const uint32_t prev_free_list_idx = tbl->free_list_idx_;

        tbl->nodes_ = new_nodes;
        tbl->active_list_ = new_active_list;
        tbl->bucket_count_ = new_bucket_count;
        tbl->count_ = 0;
        if (overflow_count > 0) {
            for (uint32_t i = 0; i + 1 < overflow_count; ++i) {
                tbl->nodes_[new_bucket_count + i].next = new_bucket_count + i + 1;
            }
            tbl->nodes_[new_bucket_count + overflow_count - 1].next = VarTable::INVALID_INDEX;
            tbl->free_list_idx_ = new_bucket_count;
        } else {
            tbl->free_list_idx_ = VarTable::INVALID_INDEX;
        }

        bool success = true;
        if (old_bucket_count == 0) {
            for (uint32_t i = 0; i < old_count; ++i) {
                const auto &qd = tbl->quick_data_[i];
                if (!TableInsertRaw(tbl, qd.key, qd.val, qd.hash)) {
                    success = false;
                    break;
                }
            }
        } else {
            for (uint32_t i = 0; i < old_bucket_count && success; ++i) {
                uint32_t curr_idx = i;
                while (curr_idx != VarTable::INVALID_INDEX) {
                    const auto *old_node = &old_nodes[curr_idx];
                    if (old_node->entry.key.type_ != kNilType) {
                        if (!TableInsertRaw(tbl, old_node->entry.key, old_node->entry.val, old_node->entry.hash)) {
                            success = false;
                            break;
                        }
                    }
                    curr_idx = old_node->next;
                }
            }
        }
        if (success) return;

        tbl->nodes_ = prev_nodes;
        tbl->active_list_ = prev_active_list;
        tbl->bucket_count_ = prev_bucket_count;
        tbl->count_ = prev_count;
        tbl->free_list_idx_ = prev_free_list_idx;
        new_bucket_count *= 2;
    }
}

// 通用的键写入，与 FlSetTableImpl 的插入路径对应（不含 nil 删除）。
void TableSetNonNil(State *s, VarTable *tbl, CVar key, CVar val, uint32_t hash) {
    if (tbl->bucket_count_ == 0) {
        for (uint32_t i = 0; i < tbl->count_; ++i) {
            auto &qd = tbl->quick_data_[i];
            if (qd.hash == hash && VarKeyEqual(qd.key, key)) {
                static_cast<CVar &>(qd.val) = val;
                return;
            }
        }
        if (tbl->count_ < VarTable::QUICK_DATA_SIZE) {
            auto &qd = tbl->quick_data_[tbl->count_];
            static_cast<CVar &>(qd.key) = key;
            static_cast<CVar &>(qd.val) = val;
            qd.hash = hash;
            tbl->count_++;
            return;
        }
        TableRehash(s, tbl);
    }
    if (tbl->count_ >= tbl->bucket_count_ || tbl->free_list_idx_ == VarTable::INVALID_INDEX) {
        TableRehash(s, tbl);
    }
    TableInsertRaw(tbl, key, val, hash);
}

// nil 删除：从 quick_data_ 或桶链中摘除键，与 FlSetTableImpl 的删除路径对应。
void TableDelete(VarTable *tbl, CVar key, uint32_t hash) {
    if (tbl->count_ == 0) return;
    if (tbl->bucket_count_ == 0) {
        for (uint32_t i = 0; i < tbl->count_; ++i) {
            auto &qd = tbl->quick_data_[i];
            if (qd.hash == hash && VarKeyEqual(qd.key, key)) {
                if (i + 1 < tbl->count_) tbl->quick_data_[i] = tbl->quick_data_[tbl->count_ - 1];
                tbl->count_--;
                return;
            }
        }
        return;
    }
    const uint32_t mask = tbl->bucket_count_ - 1;
    const uint32_t idx = hash & mask;
    auto *curr = &tbl->nodes_[idx];
    if (curr->entry.key.type_ == kNilType) return;

    const auto unlink_from_active = [&](uint32_t node_idx, uint32_t pos) {
        const uint32_t last_node_idx = tbl->active_list_[tbl->count_ - 1];
        if (node_idx != last_node_idx) {
            tbl->active_list_[pos] = last_node_idx;
            tbl->nodes_[last_node_idx].active_pos = pos;
        }
    };

    if (curr->entry.hash == hash && VarKeyEqual(curr->entry.key, key)) {
        if (curr->next != VarTable::INVALID_INDEX) {
            // 主位置被删除且存在冲突链：把链上下一个节点提升到主位置，
            // 这样「主位置为 nil 即整条链为空」的不变式得以保持。
            const uint32_t next_idx = curr->next;
            auto *next_node = &tbl->nodes_[next_idx];
            unlink_from_active(next_idx, next_node->active_pos);
            next_node->active_pos = VarTable::INVALID_INDEX;
            curr->entry = next_node->entry;
            curr->next = next_node->next;
            next_node->next = tbl->free_list_idx_;
            tbl->free_list_idx_ = next_idx;
        } else {
            unlink_from_active(idx, curr->active_pos);
            curr->active_pos = VarTable::INVALID_INDEX;
            curr->entry.key.type_ = kNilType;
        }
        tbl->count_--;
        return;
    }

    uint32_t prev_idx = idx;
    uint32_t curr_idx = curr->next;
    while (curr_idx != VarTable::INVALID_INDEX) {
        auto *node = &tbl->nodes_[curr_idx];
        if (node->entry.hash == hash && VarKeyEqual(node->entry.key, key)) {
            unlink_from_active(curr_idx, node->active_pos);
            node->active_pos = VarTable::INVALID_INDEX;
            tbl->nodes_[prev_idx].next = node->next;
            node->next = tbl->free_list_idx_;
            tbl->free_list_idx_ = curr_idx;
            tbl->count_--;
            return;
        }
        prev_idx = curr_idx;
        curr_idx = node->next;
    }
}

}// namespace

bool TableHelper::VarKeyEqualInt(CVar k, int64_t idx) {
    if (k.type_ == static_cast<int>(VarType::Int)) {
        return k.data_.i == idx;
    }
    if (k.type_ == static_cast<int>(VarType::Float)) {
        return k.data_.f == static_cast<double>(idx);
    }
    return false;
}

// 返回连续整数键前缀长度，与 # 运算符（FlGetTableSeqLen）语义一致。此前这里返回的是
// 「最大整数键」，与 JIT 内联的 table.insert 用的 # 语义不同，同一段脚本会因为调用点
// 是否被内联而得到不同结果。
int64_t TableHelper::GetTableLen(CVar tbl) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return 0;
    VarTable *t = tbl.data_.t;
    if (!SeqCacheable(t)) {
        return SeqScanFrom(t, static_cast<int64_t>(t->spec_count));
    }
    if (t->seq_len_valid_ != 0) {
        return t->seq_len_;
    }
    t->seq_len_ = SeqScanFrom(t, 0);
    t->seq_len_valid_ = 1;
    return t->seq_len_;
}

CVar TableHelper::GetTableInt(State *s, CVar tbl, int64_t idx) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return CVar{static_cast<int>(VarType::Nil)};
    VarTable *t = tbl.data_.t;

    if (t->spec_get) {
        using SpecGetFn = CVar (*)(VarTable *, CVar, bool *);
        auto get_fn = reinterpret_cast<SpecGetFn>(t->spec_get);
        CVar key_cvar{static_cast<int>(VarType::Int)};
        key_cvar.data_.i = idx;
        bool finish = false;
        CVar r = get_fn(t, key_cvar, &finish);
        if (finish) return r;
    }

    if (t->spec_count > 0 && t->spec_vals && t->spec_keys) {
        for (uint32_t i = 0; i < t->spec_count; ++i) {
            if (VarKeyEqualInt(t->spec_keys[i], idx)) {
                return t->spec_vals[i];
            }
        }
    }

    // 按哈希定位，而非遍历整张表。此前这里是 O(表大小) 的全量扫描，使得
    // table.concat/move/sort/unpack 这些逐元素读取的标准库函数整体退化为 O(n²)。
    const uint32_t hash = IntKeyHash(idx);
    if (t->bucket_count_ == 0) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            const auto &qd = t->quick_data_[i];
            if (qd.hash == hash && VarKeyEqualInt(qd.key, idx)) return qd.val;
        }
        return CVar{static_cast<int>(VarType::Nil)};
    }
    if (!t->nodes_) return CVar{static_cast<int>(VarType::Nil)};
    const uint32_t mask = t->bucket_count_ - 1;
    const auto *curr = &t->nodes_[hash & mask];
    while (true) {
        if (curr->entry.hash == hash && VarKeyEqualInt(curr->entry.key, idx)) return curr->entry.val;
        if (curr->next == VarTable::INVALID_INDEX) break;
        curr = &t->nodes_[curr->next];
    }
    return CVar{static_cast<int>(VarType::Nil)};
}

CVar TableHelper::GetTableStrId(State *s, CVar tbl, const char *str_key) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t || !str_key) return CVar{static_cast<int>(VarType::Nil)};
    VarTable *t = tbl.data_.t;
    std::string_view target_key(str_key);

    auto var_key_match_str = [](CVar k, std::string_view target) -> bool {
        if (k.type_ == static_cast<int>(VarType::StringId)) {
            if (!k.data_.i) return false;
            const char *ptr = reinterpret_cast<const char *>(k.data_.i);
            int sz = *reinterpret_cast<const int *>(ptr);
            return static_cast<size_t>(sz) == target.size() && std::memcmp(ptr + 8, target.data(), target.size()) == 0;
        }
        if (k.type_ == static_cast<int>(VarType::String)) {
            if (!k.data_.s) return false;
            return k.data_.s->Str() == target;
        }
        return false;
    };

    if (t->spec_get) {
        using SpecGetFn = CVar (*)(VarTable *, CVar, bool *);
        auto get_fn = reinterpret_cast<SpecGetFn>(t->spec_get);
        CVar key_cvar = inter::NativeToFakeluaString(s, std::string(target_key));
        bool finish = false;
        CVar r = get_fn(t, key_cvar, &finish);
        if (finish && r.type_ != static_cast<int>(VarType::Nil)) return r;
    }

    if (t->spec_count > 0 && t->spec_vals && t->spec_keys) {
        for (uint32_t i = 0; i < t->spec_count; ++i) {
            if (var_key_match_str(t->spec_keys[i], target_key)) {
                return t->spec_vals[i];
            }
        }
    }

    // 按哈希定位，而非遍历整张表。
    const uint32_t hash = VarString::HashOf(target_key);
    if (t->bucket_count_ == 0) {
        for (uint32_t i = 0; i < t->count_; ++i) {
            const auto &qd = t->quick_data_[i];
            if (qd.hash == hash && var_key_match_str(qd.key, target_key)) return qd.val;
        }
        return CVar{static_cast<int>(VarType::Nil)};
    }
    if (!t->nodes_) return CVar{static_cast<int>(VarType::Nil)};
    const uint32_t mask = t->bucket_count_ - 1;
    const auto *curr = &t->nodes_[hash & mask];
    while (true) {
        if (curr->entry.hash == hash && var_key_match_str(curr->entry.key, target_key)) return curr->entry.val;
        if (curr->next == VarTable::INVALID_INDEX) break;
        curr = &t->nodes_[curr->next];
    }
    return CVar{static_cast<int>(VarType::Nil)};
}

void TableHelper::SetTableInt(State *s, CVar tbl, int64_t idx, CVar val) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return;
    VarTable *t = tbl.data_.t;

    if (t->spec_set) {
        using SpecSetFn = void (*)(VarTable *, CVar, CVar, bool *);
        auto set_fn = reinterpret_cast<SpecSetFn>(t->spec_set);
        CVar key_cvar{static_cast<int>(VarType::Int)};
        key_cvar.data_.i = idx;
        bool finish = false;
        set_fn(t, key_cvar, val, &finish);
        if (finish) return;
    }

    if (t->spec_count > 0 && t->spec_vals && t->spec_keys) {
        for (uint32_t i = 0; i < t->spec_count; ++i) {
            if (VarKeyEqualInt(t->spec_keys[i], idx)) {
                t->spec_vals[i] = val;
                return;
            }
        }
    }

    // 走与 JIT 侧 FlSetTableInt 相同的哈希插入与扩容路径。此前 quick_data_ 的 8 个槽用满后
    // 会把整数键 std::to_string 成十进制字符串再按 StringId 存入：JIT 侧按 VAR_INT 键查找
    // 时永远匹配不上，于是 table.move 等原生函数写入的第 9 个及之后的元素全部丢失。
    CVar key{static_cast<int>(VarType::Int)};
    key.data_.i = idx;
    const uint32_t hash = IntKeyHash(idx);
    if (val.type_ == kNilType) {
        TableDelete(t, key, hash);
    } else {
        TableSetNonNil(s, t, key, val, hash);
    }
    SeqNoteIntSet(t, idx, val.type_ == kNilType);
}

void TableHelper::SetTable(State *s, CVar tbl, CVar key, CVar val) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return;
    VarTable *t = tbl.data_.t;

    key = NormalizeTableKey(key);
    if (key.type_ == kNilType) return;

    if (t->spec_set) {
        using SpecSetFn = void (*)(VarTable *, CVar, CVar, bool *);
        auto set_fn = reinterpret_cast<SpecSetFn>(t->spec_set);
        bool finish = false;
        set_fn(t, key, val, &finish);
        if (finish) return;
    }

    const uint32_t hash = VarKeyHash(key);
    if (val.type_ == kNilType) {
        TableDelete(t, key, hash);
    } else {
        TableSetNonNil(s, t, key, val, hash);
    }
    if (key.type_ == kIntType) SeqNoteIntSet(t, key.data_.i, val.type_ == kNilType);
}

void TableHelper::SetTableStrId(State *s, CVar tbl, const char *str_key, CVar val) {
    if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) return;
    VarTable *t = tbl.data_.t;
    const int64_t id = s->GetConstString().Alloc(str_key);
    const auto *vs = reinterpret_cast<const VarString *>(id);
    const uint32_t hash = vs->Hash();

    CVar key{static_cast<int>(VarType::StringId)};
    key.data_.i = id;

    // 此前这里是一套独立的探测逻辑：只比较 hash 而不比较键内容（哈希冲突会串值），
    // 并且在 quick_data_ 满、桶也满时直接返回，静默丢弃写入。现在统一走带扩容的公共路径。
    if (val.type_ == kNilType) {
        TableDelete(t, key, hash);
    } else {
        TableSetNonNil(s, t, key, val, hash);
    }
}

// Use shared CheckNumberArg from native_common.h

CVar TableHelper::CreateTable(State *state) {
    VarTable *vtbl = static_cast<VarTable *>(FakeluaAlloc(state, sizeof(VarTable), false));
    *vtbl = VarTable{};
    for (auto &qd: vtbl->quick_data_) {
        qd.key.type_ = static_cast<int>(VarType::Nil);
        qd.val.type_ = static_cast<int>(VarType::Nil);
    }
    vtbl->free_list_idx_ = VarTable::INVALID_INDEX;
    // 空表的连续整数键前缀长度确定为 0，可直接标记缓存有效。
    vtbl->seq_len_ = 0;
    vtbl->seq_len_valid_ = 1;
    CVar tbl_cvar{};
    tbl_cvar.type_ = static_cast<int>(VarType::Table);
    tbl_cvar.data_.t = vtbl;
    return tbl_cvar;
}

static CVar CreateEmptyTable(State *state) {
    return TableHelper::CreateTable(state);
}

void RegisterTableLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "table.insert", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'table.insert' (table expected)");
        }
        int64_t len = TableHelper::GetTableLen(tbl);

        if (n == 1) return inter::NativeToFakeluaNil(state);
        if (n == 2) {
            CVar val = inter::GetNativeArg(state, args, n, 1);
            TableHelper::SetTableInt(state, tbl, len + 1, val);
        } else {
            CVar pos_var = inter::GetNativeArg(state, args, n, 1);
            CVar val = inter::GetNativeArg(state, args, n, 2);
            CheckNumberArg(pos_var, 2, "table.insert");
            int64_t pos = inter::CVarToInteger(pos_var, 1);
            if (pos < 1 || pos > len + 1) return inter::NativeToFakeluaNil(state);
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
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'table.remove' (table expected)");
        }
        int64_t len = TableHelper::GetTableLen(tbl);
        int64_t pos = len;
        if (n >= 2) {
            CVar pos_var = inter::GetNativeArg(state, args, n, 1);
            CheckNumberArg(pos_var, 2, "table.remove");
            pos = inter::CVarToInteger(pos_var, len);
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
        // 标准 Lua：table.concat 第一个参数必须是 table，否则抛出异常
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'table.concat' (table expected)");
        }
        std::string sep = "";
        if (n >= 2) {
            CVar sep_var = inter::GetNativeArg(state, args, n, 1);
            // 标准 Lua：table.concat 的 sep 必须是 string（数字会被转换为字符串，这是标准行为）
            CheckStringArg(sep_var, 2, "table.concat");
            std::string temp_sep;
            sep = std::string(GetStringArgView(sep_var, temp_sep));
        }
        int64_t start_i = 1;
        if (n >= 3) {
            CVar start_var = inter::GetNativeArg(state, args, n, 2);
            CheckNumberArg(start_var, 3, "table.concat");
            start_i = inter::CVarToInteger(start_var, 1);
        }
        int64_t end_j = TableHelper::GetTableLen(tbl);
        if (n >= 4) {
            CVar end_var = inter::GetNativeArg(state, args, n, 3);
            CheckNumberArg(end_var, 4, "table.concat");
            end_j = inter::CVarToInteger(end_var, end_j);
        }

        // 两遍扫描：先算总长再一次写入 arena，避免 std::string 增长 + NativeToFakelua 二次拷贝
        std::vector<std::string> owned;// Int/Float 转字符串的临时缓冲
        std::vector<std::string_view> parts;
        const int64_t nparts = (end_j >= start_i) ? (end_j - start_i + 1) : 0;
        parts.reserve(static_cast<size_t>(nparts) + (sep.empty() ? 0 : static_cast<size_t>(nparts)));
        // parts 保存指向 owned 元素的 view，必须预留足够容量：一旦扩容，
        // SSO 短字符串的数据随对象一起搬走，已保存的 view 会全部悬空。
        owned.reserve(static_cast<size_t>(nparts));
        size_t total = 0;
        for (int64_t idx = start_i; idx <= end_j; ++idx) {
            if (idx > start_i && !sep.empty()) {
                parts.push_back(sep);
                total += sep.size();
            }
            CVar item = TableHelper::GetTableInt(state, tbl, idx);
            if (item.type_ == static_cast<int>(VarType::Int)) {
                owned.push_back(std::to_string(item.data_.i));
                parts.push_back(owned.back());
                total += owned.back().size();
            } else if (item.type_ == static_cast<int>(VarType::Float)) {
                owned.push_back(std::format("{}", item.data_.f));
                parts.push_back(owned.back());
                total += owned.back().size();
            } else if (item.type_ == static_cast<int>(VarType::String) || item.type_ == static_cast<int>(VarType::StringId)) {
                auto sv = KeyToStringView(item);
                parts.push_back(sv);
                total += sv.size();
            } else {
                ThrowFakeluaException("invalid value in table for 'concat'");
            }
        }
        VarString *vs = VarString::AllocTempRaw(state, total);
        char *dst = vs->MutableData();
        for (const auto &p: parts) {
            if (!p.empty()) {
                std::memcpy(dst, p.data(), p.size());
                dst += p.size();
            }
        }
        CVar ret{static_cast<int>(VarType::String)};
        ret.data_.s = vs;
        return ret;
    });

    RegisterNativeFunction(s, "table.unpack", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'unpack' (table expected)");
        }
        int64_t start_i = 1;
        if (n >= 2) {
            CVar start_var = inter::GetNativeArg(state, args, n, 1);
            CheckNumberArg(start_var, 2, "table.unpack");
            start_i = inter::CVarToInteger(start_var, 1);
        }
        int64_t end_j = TableHelper::GetTableLen(tbl);
        if (n >= 3) {
            CVar end_var = inter::GetNativeArg(state, args, n, 2);
            CheckNumberArg(end_var, 3, "table.unpack");
            end_j = inter::CVarToInteger(end_var, end_j);
        }

        if (start_i > end_j) return inter::AllocMultiCVar(state, 0);
        int64_t diff = end_j - start_i + 1;
        if (diff <= 0 || diff > 1000000) return inter::AllocMultiCVar(state, 0);
        int count = static_cast<int>(diff);
        CVar multi = inter::AllocMultiCVar(state, count);
        for (int i = 0; i < count; ++i) {
            CVar item = TableHelper::GetTableInt(state, tbl, start_i + i);
            inter::SetMultiCVarElement(multi, i, item);
        }
        return multi;
    });

    RegisterNativeFunction(s, "table.pack", 0, true, [](State *state, CVar *args, int n) -> CVar {
        CVar tbl_cvar = CreateEmptyTable(state);

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
        if (a2.type_ == static_cast<int>(VarType::Nil)) {
            a2 = a1;
        }
        if (a1.type_ != static_cast<int>(VarType::Table) || !a1.data_.t || a2.type_ != static_cast<int>(VarType::Table) || !a2.data_.t) {
            ThrowFakeluaException("bad argument to 'table.move' (table expected)");
        }
        CheckNumberArg(f_var, 2, "table.move");
        CheckNumberArg(e_var, 3, "table.move");
        CheckNumberArg(t_var, 4, "table.move");

        int64_t f = inter::CVarToInteger(f_var, 1);
        int64_t e = inter::CVarToInteger(e_var, 0);
        int64_t t = inter::CVarToInteger(t_var, 1);

        if (e >= f) {
            uint64_t count = static_cast<uint64_t>(e) - static_cast<uint64_t>(f) + 1;
            if (count > 10000000ULL) return a2;
            int64_t icount = static_cast<int64_t>(count);
            bool same_table = (a1.type_ == static_cast<int>(VarType::Table) && a2.type_ == static_cast<int>(VarType::Table) && a1.data_.t == a2.data_.t);
            // 空目标表预扩容：避免 1→2→4→… 反复 rehash（大 n 时常数项明显）
            if (!same_table && a2.data_.t->count_ == 0 && icount > 8) {
                TableRehashTo(state, a2.data_.t, static_cast<uint32_t>(icount));
            }
            if (!same_table || t <= f || t > e) {
                for (int64_t i = 0; i < icount; ++i) {
                    CVar val = TableHelper::GetTableInt(state, a1, f + i);
                    TableHelper::SetTableInt(state, a2, t + i, val);
                }
            } else {
                for (int64_t i = icount - 1; i >= 0; --i) {
                    CVar val = TableHelper::GetTableInt(state, a1, f + i);
                    TableHelper::SetTableInt(state, a2, t + i, val);
                }
            }
        }
        return a2;
    });

    RegisterNativeFunction(s, "table.sort", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'table.sort' (table expected)");
        }
        int64_t len = TableHelper::GetTableLen(tbl);
        if (len <= 1) return inter::NativeToFakeluaNil(state);

        std::vector<CVar> vec(len);
        for (int64_t i = 0; i < len; ++i) {
            vec[i] = TableHelper::GetTableInt(state, tbl, i + 1);
        }

        CVar comp = (n >= 2) ? inter::GetNativeArg(state, args, n, 1) : CVar{static_cast<int>(VarType::Nil)};
        if (comp.type_ != static_cast<int>(VarType::Nil) && comp.type_ != static_cast<int>(VarType::Closure)) {
            ThrowBadArgument(2, "table.sort", "function expected");
        }

        if (comp.type_ == static_cast<int>(VarType::Closure) && comp.data_.cl) {
            VarClosure *cl = comp.data_.cl;
            auto comp_func = [&](const CVar &a, const CVar &b) -> bool {
                CVar res{static_cast<int>(VarType::Nil)};
                if (cl->func_ptr) {
                    void *addr = cl->func_ptr;
                    // 比较器是 JIT 代码：错误必须先在边界转成异常，才能穿过 stable_sort 回到这里
                    res = RunWithJitErrorBoundary([&] { return reinterpret_cast<CVar (*)(VarClosure *, CVar, CVar)>(addr)(cl, a, b); });
                } else if (cl->code_str) {
                    CVar args_arr[2] = {a, b};
                    res = FlEvalLoadClosure(state, cl, 2, args_arr);
                }
                bool is_true = !(res.type_ == static_cast<int>(VarType::Nil) || (res.type_ == static_cast<int>(VarType::Bool) && !res.data_.b));
                return is_true;
            };
            std::stable_sort(vec.begin(), vec.end(), comp_func);
        } else {
            auto default_comp = [](const CVar &a, const CVar &b) -> bool {
                if (a.type_ == static_cast<int>(VarType::Int) && b.type_ == static_cast<int>(VarType::Int)) {
                    return a.data_.i < b.data_.i;
                }
                if (a.type_ == static_cast<int>(VarType::Float) && b.type_ == static_cast<int>(VarType::Float)) {
                    return a.data_.f < b.data_.f;
                }
                if (a.type_ == static_cast<int>(VarType::Int) && b.type_ == static_cast<int>(VarType::Float)) {
                    return static_cast<double>(a.data_.i) < b.data_.f;
                }
                if (a.type_ == static_cast<int>(VarType::Float) && b.type_ == static_cast<int>(VarType::Int)) {
                    return a.data_.f < static_cast<double>(b.data_.i);
                }
                // fakelua 扩展：允许 number 与 string 混合比较（转换为 string）
                // 标准 Lua 5.3 只允许 string-string 或 number-number，但 fakelua 支持混合
                bool a_is_str = (a.type_ == static_cast<int>(VarType::String) || a.type_ == static_cast<int>(VarType::StringId));
                bool b_is_str = (b.type_ == static_cast<int>(VarType::String) || b.type_ == static_cast<int>(VarType::StringId));
                bool a_is_num = (a.type_ == static_cast<int>(VarType::Int) || a.type_ == static_cast<int>(VarType::Float));
                bool b_is_num = (b.type_ == static_cast<int>(VarType::Int) || b.type_ == static_cast<int>(VarType::Float));
                if (a_is_str && b_is_str) {
                    return std::string(KeyToStringView(a)) < std::string(KeyToStringView(b));
                }
                // number 与 string 混合：都转换为 string比较（fakelua 扩展）
                if ((a_is_num && b_is_str) || (a_is_str && b_is_num)) {
                    std::string sa = a_is_str ? std::string(KeyToStringView(a)) : AsVar(a).ToString(/*has_quote=*/false, /*has_postfix=*/false);
                    std::string sb = b_is_str ? std::string(KeyToStringView(b)) : AsVar(b).ToString(/*has_quote=*/false, /*has_postfix=*/false);
                    return sa < sb;
                }
                // 其他类型组合（bool 等）抛出异常
                ThrowFakeluaException("attempt to compare two values");
                return false; // unreachable
            };
            std::stable_sort(vec.begin(), vec.end(), default_comp);
        }

        for (int64_t i = 0; i < len; ++i) {
            TableHelper::SetTableInt(state, tbl, i + 1, vec[i]);
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "table.create", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return CreateEmptyTable(state);
        CVar seq_var = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(seq_var, 1, "table.create");
        int64_t count = inter::CVarToInteger(seq_var, 0);
        if (count < 0) count = 0;

        CVar val = (n >= 2) ? inter::GetNativeArg(state, args, n, 1) : CVar{static_cast<int>(VarType::Nil)};
        CVar tbl_cvar = CreateEmptyTable(state);
        if (val.type_ != static_cast<int>(VarType::Nil)) {
            for (int64_t i = 1; i <= count; ++i) {
                TableHelper::SetTableInt(state, tbl_cvar, i, val);
            }
        }
        return tbl_cvar;
    });
}

}// namespace fakelua
