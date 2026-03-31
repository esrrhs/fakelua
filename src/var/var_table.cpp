#include "var_table.h"
#include "state/state.h"
#include "util/common.h"
#include "var.h"
#include <bit>

namespace fakelua {

Var VarTable::Get(const Var &key) const {
    if (key.Type() == VarType::Nil) {
        ThrowFakeluaException("table index is nil");
    }

    if (count_ == 0) {
        return const_null_var;
    }

    const auto h = static_cast<uint32_t>(key.Hash());

    if (bucket_count_ == 0) {
        // 快速模式：手动展开以适配 TCC 编译环境
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
    } else {
        const uint32_t mask = bucket_count_ - 1;
        uint32_t curr_idx = h & mask;
        const TableNode *curr = &nodes_[curr_idx];

        if (curr->entry.key.Type() == VarType::Nil) {
            return const_null_var;
        }

        for (;;) {
            if (curr->entry.hash == h && curr->entry.key.Equal(key)) {
                return curr->entry.val;
            }
            curr_idx = curr->next;
            if (curr_idx == INVALID_INDEX) {
                break;
            }
            curr = &nodes_[curr_idx];
        }
    }
    return const_null_var;
}

void VarTable::Set(State *s, const Var &key, const Var &val, bool can_be_nil) {
    if (key.Type() == VarType::Nil) {
        ThrowFakeluaException("table index is nil");
    }
    const auto h = static_cast<uint32_t>(key.Hash());

    if (val.Type() == VarType::Nil && !can_be_nil) {
        if (count_ == 0) {
            return;
        }

        if (bucket_count_ == 0) {
            // 快速模式删除：手动展开
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
        } else {
            const uint32_t mask = bucket_count_ - 1;
            const uint32_t idx = h & mask;
            TableNode *curr = &nodes_[idx];

            if (curr->entry.key.Type() == VarType::Nil) {
                return;
            }

            if (curr->entry.hash == h && curr->entry.key.Equal(key)) {
                if (curr->next != INVALID_INDEX) {
                    const uint32_t next_idx = curr->next;
                    TableNode *next_node = &nodes_[next_idx];
                    
                    // AIA 移除逻辑：next_node 物理消失
                    const uint32_t pos = next_node->active_pos;
                    if (const uint32_t last_node_idx = active_list_[count_ - 1]; next_idx != last_node_idx) {
                        active_list_[pos] = last_node_idx;
                        nodes_[last_node_idx].active_pos = pos;
                    }
                    next_node->active_pos = INVALID_INDEX;

                    curr->entry = next_node->entry;
                    curr->next = next_node->next;
                    next_node->next = free_list_idx_;
                    free_list_idx_ = next_idx;
                } else {
                    // AIA 移除逻辑：curr 物理标记为 Nil
                    const uint32_t pos = curr->active_pos;
                    if (const uint32_t last_node_idx = active_list_[count_ - 1]; idx != last_node_idx) {
                        active_list_[pos] = last_node_idx;
                        nodes_[last_node_idx].active_pos = pos;
                    }
                    curr->active_pos = INVALID_INDEX;
                    curr->entry.key.SetNil();
                }
                count_--;
                return;
            }

            uint32_t prev_idx = idx;
            uint32_t curr_idx = curr->next;
            while (curr_idx != INVALID_INDEX) {
                TableNode *node = &nodes_[curr_idx];
                if (node->entry.hash == h && node->entry.key.Equal(key)) {
                    // AIA 移除逻辑：node 物理移除
                    const uint32_t pos = node->active_pos;
                    if (const uint32_t last_node_idx = active_list_[count_ - 1]; curr_idx != last_node_idx) {
                        active_list_[pos] = last_node_idx;
                        nodes_[last_node_idx].active_pos = pos;
                    }
                    node->active_pos = INVALID_INDEX;

                    nodes_[prev_idx].next = node->next;
                    node->next = free_list_idx_;
                    free_list_idx_ = curr_idx;
                    count_--;
                    return;
                }
                prev_idx = curr_idx;
                curr_idx = node->next;
            }
        }
        return;
    }

    if (bucket_count_ == 0) {
        // 快速模式更新/插入：手动展开
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
        Rehash(s);
    }

    if (count_ >= bucket_count_ || free_list_idx_ == INVALID_INDEX) {
        Rehash(s);
    }

    [[maybe_unused]] bool success = InsertRaw(key, val, h);
    DEBUG_ASSERT(success);
}

bool VarTable::InsertRaw(const Var &key, const Var &val, uint32_t hash) {
    const uint32_t mask = bucket_count_ - 1;
    const uint32_t idx = hash & mask;
    TableNode *main_node = &nodes_[idx];

    if (main_node->entry.key.Type() == VarType::Nil) {
        main_node->entry.key = key;
        main_node->entry.val = val;
        main_node->entry.hash = hash;
        main_node->next = INVALID_INDEX;
        
        main_node->active_pos = count_;
        active_list_[count_] = idx;
        
        count_++;
        return true;
    }

    uint32_t curr_idx = idx;
    for (;;) {
        TableNode *curr = &nodes_[curr_idx];
        if (curr->entry.hash == hash && curr->entry.key.Equal(key)) {
            curr->entry.val = val;
            return true;
        }
        if (curr->next == INVALID_INDEX) {
            break;
        }
        curr_idx = curr->next;
    }

    if (free_list_idx_ == INVALID_INDEX) {
        return false;
    }

    const uint32_t new_node_idx = free_list_idx_;
    TableNode *new_node = &nodes_[new_node_idx];
    free_list_idx_ = new_node->next;

    new_node->entry.key = key;
    new_node->entry.val = val;
    new_node->entry.hash = hash;
    new_node->next = main_node->next;
    main_node->next = new_node_idx;

    new_node->active_pos = count_;
    active_list_[count_] = new_node_idx;

    count_++;
    return true;
}

void VarTable::Rehash(State *s) {
    const uint32_t old_count = count_;
    const uint32_t old_bucket_count = bucket_count_;
    const TableNode *old_nodes = nodes_;

    uint32_t new_bucket_count = std::bit_ceil(old_count + 1);
    if (new_bucket_count <= old_bucket_count) {
        new_bucket_count = old_bucket_count * 2;
    }

    while (true) {
        const uint32_t overflow_count = new_bucket_count / 2;
        const uint32_t total_nodes = new_bucket_count + overflow_count;
        
        const size_t nodes_size = total_nodes * sizeof(TableNode);
        const size_t active_list_size = total_nodes * sizeof(uint32_t);
        auto *buffer = static_cast<char *>(s->GetHeap().GetTempAllocator().Alloc(nodes_size + active_list_size));
        
        auto *new_nodes = reinterpret_cast<TableNode *>(buffer);
        auto *new_active_list = reinterpret_cast<uint32_t *>(buffer + nodes_size);

        for (uint32_t i = 0; i < total_nodes; ++i) {
            new_nodes[i].entry.key.SetNil();
            new_nodes[i].next = INVALID_INDEX;
            new_nodes[i].active_pos = INVALID_INDEX;
        }

        TableNode *prev_nodes = nodes_;
        uint32_t *prev_active_list = active_list_;
        const uint32_t prev_bucket_count = bucket_count_;
        const uint32_t prev_count = count_;
        const uint32_t prev_free_list_idx = free_list_idx_;

        nodes_ = new_nodes;
        active_list_ = new_active_list;
        bucket_count_ = new_bucket_count;
        count_ = 0;
        for (uint32_t i = 0; i < overflow_count - 1; ++i) {
            nodes_[new_bucket_count + i].next = new_bucket_count + i + 1;
        }
        nodes_[new_bucket_count + overflow_count - 1].next = INVALID_INDEX;
        free_list_idx_ = new_bucket_count;

        bool success = true;
        if (old_bucket_count == 0) {
            // 数据迁移时的源数据同样需要手动展开访问
            if (old_count > 0) {
                if (!InsertRaw(quick_data_[0].key, quick_data_[0].val, quick_data_[0].hash)) {
                    success = false;
                }
            }
            if (success && old_count > 1) {
                if (!InsertRaw(quick_data_[1].key, quick_data_[1].val, quick_data_[1].hash)) {
                    success = false;
                }
            }
            if (success && old_count > 2) {
                if (!InsertRaw(quick_data_[2].key, quick_data_[2].val, quick_data_[2].hash)) {
                    success = false;
                }
            }
            if (success && old_count > 3) {
                if (!InsertRaw(quick_data_[3].key, quick_data_[3].val, quick_data_[3].hash)) {
                    success = false;
                }
            }
        } else {
            for (uint32_t i = 0; i < old_bucket_count; ++i) {
                uint32_t curr_idx = i;
                while (curr_idx != INVALID_INDEX) {
                    const TableNode *old_node = &old_nodes[curr_idx];
                    if (old_node->entry.key.Type() != VarType::Nil) {
                        if (!InsertRaw(old_node->entry.key, old_node->entry.val, old_node->entry.hash)) {
                            success = false;
                            break;
                        }
                    }
                    curr_idx = old_node->next;
                }
                if (!success) {
                    break;
                }
            }
        }

        if (success) {
            break;
        } else {
            nodes_ = prev_nodes;
            active_list_ = prev_active_list;
            bucket_count_ = prev_bucket_count;
            count_ = prev_count;
            free_list_idx_ = prev_free_list_idx;
            new_bucket_count *= 2;
        }
    }
}

Var VarTable::KeyAt(size_t pos) const {
    if (pos >= count_) {
        return const_null_var;
    }

    if (bucket_count_ == 0) {
        return quick_data_[pos].key;
    } else {
        return nodes_[active_list_[pos]].entry.key;
    }
}

Var VarTable::ValueAt(size_t pos) const {
    if (pos >= count_) {
        return const_null_var;
    }

    if (bucket_count_ == 0) {
        return quick_data_[pos].val;
    } else {
        return nodes_[active_list_[pos]].entry.val;
    }
}

}// namespace fakelua
