#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

// a simple concurrent vector.
template<typename K>
class concurrent_vector {
public:
    concurrent_vector(size_t init_size) : size_(0) {
        vec_.resize(init_size);
        used_.resize(init_size);
    }

    ~concurrent_vector() {
    }

    // push_back. thread safe.
    int push_back(const K &key) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        vec_.push_back(key);
        used_.push_back(true);
        int index = vec_.size() - 1;
        size_++;
        return index;
    }

    // set. thread safe.
    void set(int index, const K &key) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (index >= (int) vec_.size()) {
            vec_.resize(2 * index);
            used_.resize(2 * index);
        }
        vec_[index] = key;
        if (!used_[index]) {
            used_[index] = true;
            size_++;
        }
    }

    // get. thread safe.
    bool get(int index, K &key) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        if (index >= (int) vec_.size() || !used_[index]) {
            return false;
        }
        key = vec_[index];
        return true;
    }

    // remove. thread safe.
    bool remove(int index) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (index >= (int) vec_.size() || !used_[index]) {
            return false;
        }
        used_[index] = false;
        size_--;
        return true;
    }

    // get size. thread safe.
    size_t size() const {
        return size_;
    }

    // clear the vector. not thread safe, just like check_rehash.
    void clear() {
        vec_.clear();
        used_.clear();
        size_ = 0;
    }

private:
    std::vector<K> vec_;
    std::vector<bool> used_;
    mutable std::shared_mutex mutex_;
    size_t size_;
};

}// namespace fakelua
