#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var/var.h"

namespace fakelua {

// 哈希表条目：包含键、值以及缓存的哈希值和探测距离
struct TableEntry {
    Var key;
    Var val;
    uint32_t hash; // 缓存 key 的哈希值，避免重复计算
    uint32_t dist; // 缓存探测距离 (PSL)，用于罗宾汉哈希的快速查找退出
};

// 混合模式哈希表：
// 1. 元素较少时（QUICK_DATA_SIZE），使用 quick_data_ 进行线性扫描，保持紧凑布局，零内存分配。
// 2. 元素增多时，自动升级为 slow_data_，采用罗宾汉哈希（Robin Hood Hashing）算法。
// 3. 使用线性探测（Linear Probing）解决冲突，并配合后移填补（Backward Shift）进行删除。
class VarTable {
public:
    VarTable() = default;
    ~VarTable();

    // 禁止拷贝，因为 slow_data_ 是手动管理的堆内存
    VarTable(const VarTable&) = delete;
    VarTable& operator=(const VarTable&) = delete;

    // 根据 Key 获取 Value。如果不存在则返回 const_null_var。
    [[nodiscard]] Var Get(const Var &key) const;

    // 设置键值对。如果 val 为 Nil 且 can_be_nil 为 false，则执行删除操作。
    void Set(const Var &key, const Var &val, bool can_be_nil);

    // 获取当前元素数量
    [[nodiscard]] size_t Size() const {
        return count_;
    }

    // 获取 Table 中所有的 Key，主要用于 pairs 遍历
    void GetKeys(std::vector<Var>& keys) const;

    // 获取指定位置的 Key（在 Quick 模式下为 O(1)，Slow 模式下需线性扫描有效位）
    [[nodiscard]] Var KeyAt(size_t pos) const;

    // 获取指定位置的 Value
    [[nodiscard]] Var ValueAt(size_t pos) const;

private:
    // 重新哈希并扩容
    void Rehash(uint32_t new_capacity);

private:
    static constexpr uint32_t QUICK_DATA_SIZE = 4; // 快速路径的最大容量
    uint32_t count_ = 0;                           // 当前元素数量
    uint32_t capacity_ = 0;                        // 哈希表容量（Slow 模式下必须是 2 的幂）
    TableEntry* nodes_ = quick_data_;              // 指向当前活跃的数据节点（quick 或 slow）
    TableEntry quick_data_[QUICK_DATA_SIZE] = {};  // 嵌入式数组，用于处理小表
    TableEntry* slow_data_ = nullptr;              // 堆分配的哈希表空间
};

}// namespace fakelua
