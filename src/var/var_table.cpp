#include "var_table.h"
#include "util/common.h"
#include "var.h"

namespace fakelua {

Var VarTable::Get(const Var &key) const {
    if (count_ == 0) {
        return const_null_var;
    }

    const auto h = static_cast<uint32_t>(key.Hash());

    if (bucket_count_ == 0) {// 快速模式：手动展开线性扫描
        if (quick_data_[0].hash == h && quick_data_[0].key.Equal(key)) {
            return quick_data_[0].val;
        }
        if (count_ > 1 && quick_data_[1].hash == h && quick_data_[1].key.Equal(key)) {
            return quick_data_[1].val;
        }
        if (count_ > 2 && quick_data_[2].hash == h && quick_data_[2].key.Equal(key)) {
            return quick_data_[2].val;
        }
        if (count_ > 3 && quick_data_[3].hash == h && quick_data_[3].key.Equal(key)) {
            return quick_data_[3].val;
        }
        return const_null_var;
    } else {// 慢速模式：拉链法（首节点嵌入）
        const uint32_t mask = bucket_count_ - 1;
        const uint32_t idx = h & mask;
        const TableNode *curr = &nodes_[idx];
        if (curr->entry.key.Type() == VarType::Nil) {
            return const_null_var;
        }
        while (curr) {
            if (curr->entry.hash == h && curr->entry.key.Equal(key)) {
                return curr->entry.val;
            }
            curr = curr->next;
        }
    }
    return const_null_var;
}

void VarTable::Set(const Var &key, const Var &val, bool can_be_nil) {
    const auto h = static_cast<uint32_t>(key.Hash());

    if (val.Type() == VarType::Nil && !can_be_nil) {
        // 执行删除操作
        if (count_ == 0) {
            return;
        }

        if (bucket_count_ == 0) {// 快速模式删除：手动展开
            if (quick_data_[0].hash == h && quick_data_[0].key.Equal(key)) {
                if (count_ > 1) {
                    quick_data_[0] = quick_data_[count_ - 1];
                }
                count_--;
                return;
            }
            if (count_ > 1 && quick_data_[1].hash == h && quick_data_[1].key.Equal(key)) {
                if (count_ > 2) {
                    quick_data_[1] = quick_data_[count_ - 1];
                }
                count_--;
                return;
            }
            if (count_ > 2 && quick_data_[2].hash == h && quick_data_[2].key.Equal(key)) {
                if (count_ > 3) {
                    quick_data_[2] = quick_data_[count_ - 1];
                }
                count_--;
                return;
            }
            if (count_ > 3 && quick_data_[3].hash == h && quick_data_[3].key.Equal(key)) {
                count_--;
                return;
            }
        } else {// 慢速模式删除
            const uint32_t mask = bucket_count_ - 1;
            const uint32_t idx = h & mask;
            TableNode *curr = &nodes_[idx];

            if (curr->entry.key.Type() == VarType::Nil) {
                return;
            }

            // 如果要删除的是主桶位置的节点
            if (curr->entry.hash == h && curr->entry.key.Equal(key)) {
                if (curr->next) {
                    // 将溢出池中的后续节点内容拷贝到主桶
                    TableNode *next_node = curr->next;
                    curr->entry = next_node->entry;
                    curr->next = next_node->next;
                    // 释放后续节点到自由链表
                    next_node->next = free_list_;
                    free_list_ = next_node;
                } else {
                    curr->entry.key.SetNil();
                }
                count_--;
                return;
            }

            // 沿着溢出链查找
            TableNode *prev = curr;
            curr = curr->next;
            while (curr) {
                if (curr->entry.hash == h && curr->entry.key.Equal(key)) {
                    prev->next = curr->next;
                    // 释放该节点到自由链表
                    curr->next = free_list_;
                    free_list_ = curr;
                    count_--;
                    return;
                }
                prev = curr;
                curr = curr->next;
            }
        }
        return;
    }

    // 执行插入或更新操作
    if (bucket_count_ == 0) {// 快速模式：尝试匹配现有 Key
        if (count_ > 0 && quick_data_[0].hash == h && quick_data_[0].key.Equal(key)) {
            quick_data_[0].val = val;
            return;
        }
        if (count_ > 1 && quick_data_[1].hash == h && quick_data_[1].key.Equal(key)) {
            quick_data_[1].val = val;
            return;
        }
        if (count_ > 2 && quick_data_[2].hash == h && quick_data_[2].key.Equal(key)) {
            quick_data_[2].val = val;
            return;
        }
        if (count_ > 3 && quick_data_[3].hash == h && quick_data_[3].key.Equal(key)) {
            quick_data_[3].val = val;
            return;
        }

        if (count_ < QUICK_DATA_SIZE) {
            quick_data_[count_].key = key;
            quick_data_[count_].val = val;
            quick_data_[count_].hash = h;
            count_++;
            return;
        }
        // 升级为拉链法哈希表
        Rehash(QUICK_DATA_SIZE * 2);
    }

    // 慢速模式插入
    if (count_ >= bucket_count_ * 1.5) {
        Rehash(bucket_count_ * 2);
    }

    const uint32_t mask = bucket_count_ - 1;
    const uint32_t idx = h & mask;
    TableNode *main_node = &nodes_[idx];

    // 如果主桶位置为空
    if (main_node->entry.key.Type() == VarType::Nil) {
        main_node->entry.key = key;
        main_node->entry.val = val;
        main_node->entry.hash = h;
        main_node->next = nullptr;
        count_++;
        return;
    }

    // 检查并更新现有节点
    TableNode *curr = main_node;
    while (curr) {
        if (curr->entry.hash == h && curr->entry.key.Equal(key)) {
            curr->entry.val = val;
            return;
        }
        curr = curr->next;
    }

    // 从自由链表取出一个溢出节点，链入主桶的后续
    DEBUG_ASSERT(free_list_ != nullptr);
    TableNode *new_node = free_list_;
    free_list_ = free_list_->next;

    new_node->entry.key = key;
    new_node->entry.val = val;
    new_node->entry.hash = h;
    new_node->next = main_node->next;
    main_node->next = new_node;
    count_++;
}

void VarTable::Rehash(uint32_t new_bucket_count) {
    TableNode *old_nodes = nodes_;
    const uint32_t old_bucket_count = bucket_count_;
    const uint32_t old_count = count_;

    // 池大小设定为桶数量的 2 倍（1 倍主桶，1 倍溢出池）
    const uint32_t total_nodes = new_bucket_count * 2;
    const size_t total_size = total_nodes * sizeof(TableNode);
    nodes_ = static_cast<TableNode *>(malloc(total_size));
    memset(nodes_, 0, total_size);

    bucket_count_ = new_bucket_count;
    count_ = 0;

    // 初始化自由链表（从溢出池区域开始）
    TableNode *pool = nodes_ + new_bucket_count;
    for (uint32_t i = 0; i < new_bucket_count - 1; ++i) {
        pool[i].next = &pool[i + 1];
    }
    pool[new_bucket_count - 1].next = nullptr;
    free_list_ = pool;

    if (old_bucket_count == 0) {
        // 从 quick_data_ 迁移：手动展开
        if (old_count > 0) Set(quick_data_[0].key, quick_data_[0].val, false);
        if (old_count > 1) Set(quick_data_[1].key, quick_data_[1].val, false);
        if (old_count > 2) Set(quick_data_[2].key, quick_data_[2].val, false);
        if (old_count > 3) Set(quick_data_[3].key, quick_data_[3].val, false);
    } else {
        // 从旧主桶节点和其链表迁移
        for (uint32_t i = 0; i < old_bucket_count; ++i) {
            if (const TableNode *curr = &old_nodes[i]; curr->entry.key.Type() != VarType::Nil) {
                Set(curr->entry.key, curr->entry.val, false);
                curr = curr->next;
                while (curr) {
                    Set(curr->entry.key, curr->entry.val, false);
                    curr = curr->next;
                }
            }
        }
        free(old_nodes);
    }

    DEBUG_ASSERT(count_ == old_count);
}

Var VarTable::KeyAt(size_t pos) const {
    if (pos >= count_) {
        return const_null_var;
    }

    if (bucket_count_ == 0) {
        return quick_data_[pos].key;
    } else {
        size_t current = 0;
        for (uint32_t i = 0; i < bucket_count_; ++i) {
            if (const TableNode *curr = &nodes_[i]; curr->entry.key.Type() != VarType::Nil) {
                if (current == pos) return curr->entry.key;
                current++;
                curr = curr->next;
                while (curr) {
                    if (current == pos) return curr->entry.key;
                    current++;
                    curr = curr->next;
                }
            }
        }
    }
    return const_null_var;
}

Var VarTable::ValueAt(size_t pos) const {
    if (pos >= count_) {
        return const_null_var;
    }

    if (bucket_count_ == 0) {
        return quick_data_[pos].val;
    } else {
        size_t current = 0;
        for (uint32_t i = 0; i < bucket_count_; ++i) {
            if (const TableNode *curr = &nodes_[i]; curr->entry.key.Type() != VarType::Nil) {
                if (current == pos) return curr->entry.val;
                current++;
                curr = curr->next;
                while (curr) {
                    if (current == pos) return curr->entry.val;
                    current++;
                    curr = curr->next;
                }
            }
        }
    }
    return const_null_var;
}

}// namespace fakelua
