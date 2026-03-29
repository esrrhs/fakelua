#pragma once

#include "util/common.h"
#include "var/var.h"

namespace fakelua {

// 混合模式哈希表：
// 1. 元素较少时（QUICK_DATA_SIZE），使用 quick_data_ 进行线性扫描。
// 2. 元素增多时，自动升级为 buckets_，使用拉链法 (Chaining) 哈希表。
// 3. 内存布局设计为 C 兼容，方便未来在 C 代码中直接访问。
class VarTable {
public:
    // 基础条目：键、值、哈希
    struct VarEntry {
        Var key;
        Var val;
        uint32_t hash;
    };

    // 链表节点：基础条目 + 下一个节点指针
    struct TableNode {
        VarEntry entry;
        TableNode *next = nullptr;
    };

public:
    VarTable() = default;
    ~VarTable() = default;

    // 禁止拷贝
    VarTable(const VarTable &) = delete;
    VarTable &operator=(const VarTable &) = delete;

    // 根据 Key 获取 Value。如果不存在则返回 const_null_var。
    [[nodiscard]] Var Get(const Var &key) const;

    // 设置键值对。如果 val 为 Nil 且 can_be_nil 为 false，则执行删除操作。
    void Set(State *s, const Var &key, const Var &val, bool can_be_nil);

    // 获取当前元素数量
    [[nodiscard]] size_t Size() const {
        return count_;
    }

    // 获取指定位置的 Key
    [[nodiscard]] Var KeyAt(size_t pos) const;

    // 获取指定位置的 Value
    [[nodiscard]] Var ValueAt(size_t pos) const;

private:
    // 重新哈希并扩容
    void Rehash(State *s, uint32_t new_capacity);

private:
    static constexpr uint32_t QUICK_DATA_SIZE = 4;// 快速路径的最大容量
    uint32_t count_ = 0;                          // 当前元素数量
    uint32_t bucket_count_ = 0;                   // 桶的数量（必须是 2 的幂）
    TableNode *nodes_ = nullptr;                  // 指向内存块开头（包含主桶节点和溢出池节点）
    VarEntry quick_data_[QUICK_DATA_SIZE] = {};   // 嵌入式数组（不包含 next 指针）
    TableNode *free_list_ = nullptr;              // 溢出池中的自由节点链表
};


}// namespace fakelua
