#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "util/concurrent_hashmap.h"
#include "var/var_string.h"
#include "var/var_type.h"

namespace fakelua {

// every state has a string heap
// the string heap contains all the strings in the state
class var_string_heap {
public:
    var_string_heap() : short_str_map_(STRING_HEAP_INIT_BUCKET_SIZE) {
    }

    ~var_string_heap() = default;

    // alloc a string, if the string is short, then generate a index. otherwise, copy the string.
    var_string alloc(const std::string &str);

    // clear the string heap
    void clear() {
        short_str_map_.clear();
        long_str_vec_.clear();
    }

private:
    // since the string heap is shared by all the states, we need a concurrent hashmap to store the strings.
    concurrent_hashmap<std::string, var_string> short_str_map_;
    // the index of the next short string.
    std::atomic_uint32_t short_str_index_;
    // the vector stores the long strings.
    typedef std::shared_ptr<std::string> str_container_ptr;
    std::vector<str_container_ptr> long_str_vec_;
};

}// namespace fakelua
