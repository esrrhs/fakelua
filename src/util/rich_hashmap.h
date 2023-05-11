#pragma once

#include "fakelua.h"
#include "hash_func.h"
#include "util/common.h"

namespace fakelua {

// a simple hashmap. waste memory, but fast. so we are rich.
// use fixed continuous memory to store the entries instead of linked list or allocated array.
// and we spilt the key and value to two arrays, so we can use the cache more efficiently. even use the SIMD.
template<typename K, typename V, size_t BucketHeight>
class rich_hashmap {
public:
    static_assert(std::is_integral<K>::value, "K must be integral type.");

    rich_hashmap(size_t init_bucket_size) : size_(0) {
        buckets_.resize(init_bucket_size);
        for (auto &bucket: buckets_) {
            bucket.size = 0;
        }
    }

    ~rich_hashmap() = default;

    // set
    void set(const K &key, const V &value) {
        // try to set
        if (inner_set(key, value)) {
            return;
        }

        // no space, rehash.
        auto cur_buckets_size = buckets_.size();
        // run infinite loop until set ok. it may cause big memory usage. but we are rich.
        while (true) {
            size_t new_size = size_ * 2 > cur_buckets_size ? size_ * 2 : cur_buckets_size + 1;
            // try rehash
            if (!rehash(new_size)) {
                // rehash fail, we pretend rehash ok, and set the cur_buckets_size to new_size. try again.
                cur_buckets_size = new_size;
                continue;
            }

            // rehash ok. try to set again.
            // if failed, we go next loop.
            if (inner_set(key, value)) {
                // set ok.
                return;
            }
        }
    }

    // get
    bool get(const K &key, V &value) const {
        auto hash = std::hash<K>()(key);
        auto bucket_index = hash % buckets_.size();
        auto &bucket = buckets_[bucket_index];
        for (size_t i = 0; i < bucket.size; i++) {
            if (std::equal_to<K>()(bucket.key[i], key)) {
                value = bucket.value[i];
                return true;
            }
        }
        return false;
    }

    // remove
    void remove(const K &key) {
        auto hash = std::hash<K>()(key);
        auto bucket_index = hash % buckets_.size();
        auto &bucket = buckets_[bucket_index];
        for (size_t i = 0; i < bucket.size; i++) {
            if (std::equal_to<K>()(bucket.key[i], key)) {
                bucket.key[i] = bucket.key[bucket.size - 1];
                bucket.value[i] = bucket.value[bucket.size - 1];
                bucket.size--;
                size_--;
                return;
            }
        }
    }

    // get size
    size_t size() const {
        return size_;
    }

    // get bucket size
    size_t bucket_size() const {
        return buckets_.size();
    }

    // clear the hashmap
    void clear() {
        for (auto &bucket: buckets_) {
            bucket.size = 0;
        }
        size_ = 0;
    }

private:
    // inner set.
    bool inner_set(const K &key, const V &value) {
        auto hash = std::hash<K>()(key);
        auto bucket_index = hash % buckets_.size();
        auto &bucket = buckets_[bucket_index];
        for (size_t i = 0; i < bucket.size; i++) {
            if (std::equal_to<K>()(bucket.key[i], key)) {
                bucket.value[i] = value;
                return true;
            }
        }

        // has space.
        if (bucket.size < BucketHeight) {
            bucket.key[bucket.size] = key;
            bucket.value[bucket.size] = value;
            bucket.size++;
            size_++;
            return true;
        }

        // no space.
        return false;
    }

    // rehash
    bool rehash(size_t new_size) {
        std::vector<bucket> new_buckets;
        new_buckets.resize(new_size);
        for (auto &bucket: new_buckets) {
            bucket.size = 0;
        }

        for (auto &bucket: buckets_) {
            for (size_t i = 0; i < bucket.size; i++) {
                auto hash = std::hash<K>()(bucket.key[i]);
                auto bucket_index = hash % new_buckets.size();
                auto &new_bucket = new_buckets[bucket_index];
                if (new_bucket.size < BucketHeight) {
                    new_bucket.key[new_bucket.size] = bucket.key[i];
                    new_bucket.value[new_bucket.size] = bucket.value[i];
                    new_bucket.size++;
                } else {
                    // no space, rehash fail. try again later.
                    return false;
                }
            }
        }

        buckets_ = std::move(new_buckets);
        return true;
    }

private:
    struct bucket {
        size_t size;
        K key[BucketHeight];
        V value[BucketHeight];
    };
    std::vector<bucket> buckets_;
    size_t size_;
};

}// namespace fakelua
