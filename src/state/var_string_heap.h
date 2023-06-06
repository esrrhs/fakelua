#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "util/concurrent_hashmap.h"
#include "util/concurrent_vector.h"
#include "var/var_string.h"
#include "var/var_type.h"
#include "util/no_copy.h"

namespace fakelua {

// every state has a string heap.
// the string heap contains all the strings in the state.
// it converts string to an uint32 index, to speed up the string compare, and the table key compare.
// the index top 1 bit is the type, 0 for short string, 1 for long string. low 31 bits is the index in the vector.
class var_string_heap : public no_copy<var_string_heap> {
public:
    var_string_heap() : short_str_to_index_map_(STRING_HEAP_INIT_BUCKET_SIZE),
                        short_str_vec_(STRING_HEAP_INIT_BUCKET_SIZE),
                        long_str_vec_(0) {
    }

    ~var_string_heap() = default;

    // alloc a string, and return the var_string which contains the index. thread safe.
    // for short string, same string has same index.
    // for long string, every call alloc will return a new index.
    var_string alloc(const std::string_view &str);

    // get the string view of a string by var_string which alloced by this heap. thread safe.
    std::string_view get(const var_string &str) const;

    // clear the string heap, not thread safe. should be called only in one thread.
    void clear() {
        short_str_to_index_map_.clear();
        short_str_vec_.clear();
        short_str_index_ = 0;
        long_str_vec_.clear();
    }

private:
    // since the string heap is shared by all running thread in states, we need a concurrent hashmap to store the strings.
    // the key is the string, the value is the index.
    concurrent_hashmap<str_container_ptr, uint32_t> short_str_to_index_map_;
    // the key is the index, the value is the string.
    concurrent_vector<str_container_ptr> short_str_vec_;
    // the index of the next short string.
    std::atomic_uint32_t short_str_index_;

    // the vector stores the long strings.
    concurrent_vector<str_container_ptr> long_str_vec_;
};

}// namespace fakelua
