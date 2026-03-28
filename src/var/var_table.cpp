#include "var_table.h"
#include "var.h"
#include "fakelua.h"
#include "util/common.h"
#include <cstdlib>

namespace fakelua {

VarTable::~VarTable() {
    if (slow_data_) {
        free(slow_data_);
    }
}

Var VarTable::Get(const Var &key) const {
    if (count_ == 0) {
        return const_null_var;
    }

    uint32_t h = (uint32_t)key.Hash();

    if (capacity_ == 0) { // 快速模式：对紧凑数组进行线性扫描
        for (uint32_t i = 0; i < count_; ++i) {
            if (nodes_[i].hash == h && nodes_[i].key.Equal(key)) {
                return nodes_[i].val;
            }
        }
    } else { // 慢速模式：采用罗宾汉哈希查找
        uint32_t mask = capacity_ - 1;
        uint32_t idx = h & mask;
        uint32_t dist = 0; 

        while (nodes_[idx].key.Type() != VarType::Nil) {
            // 罗宾汉哈希提前退出判定
            if (dist > nodes_[idx].dist) {
                return const_null_var;
            }

            if (nodes_[idx].hash == h && nodes_[idx].key.Equal(key)) {
                return nodes_[idx].val;
            }

            idx = (idx + 1) & mask;
            dist++;
        }
    }
    return const_null_var;
}

void VarTable::Set(const Var &key, const Var &val, bool can_be_nil) {
    uint32_t h = (uint32_t)key.Hash();

    if (val.Type() == VarType::Nil && !can_be_nil) {
        // 执行删除操作
        if (count_ == 0) {
            return;
        }

        if (capacity_ == 0) { // 快速模式删除
            for (uint32_t i = 0; i < count_; ++i) {
                if (nodes_[i].hash == h && nodes_[i].key.Equal(key)) {
                    // 通过交换最后一个元素保持数组紧凑
                    if (i < count_ - 1) {
                        nodes_[i] = nodes_[count_ - 1];
                    }
                    // 直接减少计数，无需清理任何槽位数据
                    count_--;
                    return;
                }
            }
        } else { // 慢速模式删除
            uint32_t mask = capacity_ - 1;
            uint32_t i = h & mask;
            uint32_t dist = 0;

            while (nodes_[i].key.Type() != VarType::Nil) {
                if (dist > nodes_[i].dist) {
                    return; // Key 不存在
                }

                if (nodes_[i].hash == h && nodes_[i].key.Equal(key)) {
                    // 找到目标，开始执行后移填补删除。
                    // 逻辑上只需将目标 key 设为 Nil 即可形成“洞口”。
                    nodes_[i].key.SetNil();
                    count_--;

                    uint32_t j = i;
                    while (true) {
                        i = (i + 1) & mask;
                        if (nodes_[i].key.Type() == VarType::Nil) {
                            break;
                        }
                        
                        uint32_t k = nodes_[i].hash & mask;
                        // 判断 nodes[i] 的原始哈希位置是否在 (j, i] 之外（循环判断）
                        if ((i > j && (k <= j || k > i)) || (i < j && (k <= j && k > i))) {
                            // 将 i 位置的元素前移到洞口 j
                            nodes_[j] = nodes_[i];
                            nodes_[j].dist = (j - k) & mask; // 更新前移后的探测距离
                            
                            // 元素移走后，i 处形成新的洞口，只需标记 key 为 Nil
                            nodes_[i].key.SetNil();
                            j = i;
                        }
                    }
                    return;
                }
                i = (i + 1) & mask;
                dist++;
            }
        }
        return;
    }

    // 执行插入或更新操作
    if (capacity_ == 0) { // 快速模式
        for (uint32_t i = 0; i < count_; ++i) {
            if (nodes_[i].hash == h && nodes_[i].key.Equal(key)) {
                nodes_[i].val = val;
                return;
            }
        }
        if (count_ < QUICK_DATA_SIZE) {
            nodes_[count_].key = key;
            nodes_[count_].val = val;
            nodes_[count_].hash = h;
            count_++;
            return;
        }
        // 升级为哈希表
        Rehash(QUICK_DATA_SIZE * 2);
    }

    // 慢速模式插入逻辑
    if (count_ >= capacity_ * 0.75) {
        Rehash(capacity_ * 2);
    }

    uint32_t mask = capacity_ - 1;
    uint32_t idx = h & mask;
    uint32_t dist = 0;
    
    TableEntry entry = {key, val, h, dist};

    while (true) {
        if (nodes_[idx].key.Type() == VarType::Nil) {
            entry.dist = dist;
            nodes_[idx] = entry;
            count_++;
            return;
        }

        if (nodes_[idx].hash == entry.hash && nodes_[idx].key.Equal(entry.key)) {
            nodes_[idx].val = entry.val;
            return;
        }

        if (dist > nodes_[idx].dist) {
            // 罗宾汉交换
            entry.dist = dist;
            TableEntry temp = nodes_[idx];
            nodes_[idx] = entry;
            entry = temp;
            dist = entry.dist;
        }

        idx = (idx + 1) & mask;
        dist++;
    }
}

void VarTable::Rehash(uint32_t new_capacity) {
    auto old_nodes = nodes_;
    auto old_capacity = capacity_;
    uint32_t old_count = count_;
    
    // 申请并清零新空间，calloc 会将所有 key.type 初始化为 VAR_NIL
    TableEntry* new_nodes = (TableEntry*)calloc(new_capacity, sizeof(TableEntry));
    
    nodes_ = new_nodes;
    capacity_ = new_capacity;
    count_ = 0;
    slow_data_ = new_nodes;

    if (old_capacity == 0) {
        for (uint32_t i = 0; i < old_count; ++i) {
            Set(old_nodes[i].key, old_nodes[i].val, false);
        }
    } else {
        for (uint32_t i = 0; i < old_capacity; ++i) {
            if (old_nodes[i].key.Type() != VarType::Nil) {
                Set(old_nodes[i].key, old_nodes[i].val, false);
            }
        }
        free(old_nodes);
    }
    
    DEBUG_ASSERT(count_ == old_count);
}

void VarTable::GetKeys(std::vector<Var>& keys) const {
    if (capacity_ == 0) {
        for (uint32_t i = 0; i < count_; ++i) {
            keys.push_back(nodes_[i].key);
        }
    } else {
        for (uint32_t i = 0; i < capacity_; ++i) {
            if (nodes_[i].key.Type() != VarType::Nil) {
                keys.push_back(nodes_[i].key);
            }
        }
    }
}

Var VarTable::KeyAt(size_t pos) const {
    if (pos >= count_) {
        return const_null_var;
    }

    if (capacity_ == 0) {
        return nodes_[pos].key;
    } else {
        size_t current = 0;
        for (uint32_t i = 0; i < capacity_; ++i) {
            if (nodes_[i].key.Type() != VarType::Nil) {
                if (current == pos) {
                    return nodes_[i].key;
                }
                current++;
            }
        }
    }
    return const_null_var;
}

Var VarTable::ValueAt(size_t pos) const {
    if (pos >= count_) {
        return const_null_var;
    }

    if (capacity_ == 0) {
        return nodes_[pos].val;
    } else {
        size_t current = 0;
        for (uint32_t i = 0; i < capacity_; ++i) {
            if (nodes_[i].key.Type() != VarType::Nil) {
                if (current == pos) {
                    return nodes_[i].val;
                }
                current++;
            }
        }
    }
    return const_null_var;
}

}// namespace fakelua
