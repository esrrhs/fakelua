#pragma once

#include "fakelua.h"
#include "util/common.h"

typedef std::shared_ptr<std::string> str_container_ptr;

namespace std {
template<>
struct hash<str_container_ptr> {
    size_t operator()(const str_container_ptr &k) const {
        return std::hash<std::string>()(*k);
    }
};

template<>
struct equal_to<str_container_ptr> {
    bool operator()(const str_container_ptr &k1, const str_container_ptr &k2) const {
        return *k1 == *k2;
    }
};

}// namespace std

namespace fakelua {

template<typename K1, typename K2>
struct my_equal_to {
    bool operator()(const K1 &__x, const K2 &__y) const {
        return __x == __y;
    }
};

template<>
struct my_equal_to<std::string, str_container_ptr> {
    bool operator()(const std::string &k1, const str_container_ptr &k2) const {
        return k1 == *k2;
    }
};

template<>
struct my_equal_to<str_container_ptr, std::string> {
    bool operator()(const str_container_ptr &k1, const std::string &k2) const {
        return *k1 == k2;
    }
};

template<>
struct my_equal_to<std::string_view, str_container_ptr> {
    bool operator()(const std::string_view &k1, const str_container_ptr &k2) const {
        return k1 == *k2;
    }
};

template<>
struct my_equal_to<str_container_ptr, std::string_view> {
    bool operator()(const str_container_ptr &k1, const std::string_view &k2) const {
        return *k1 == k2;
    }
};

// a simple concurrent hashmap.
// just put every bucket a read write lock.
// and if there is a rehash, it must be called in one thread, and that's the caller's responsibility,
// the caller should make sure that there is no other thread accessing the hashmap.
// so the hashmap will be very simple and efficient.
template<typename K, typename V>
class concurrent_hashmap {
public:
    concurrent_hashmap(size_t init_bucket_size) : size_(0) {
        buckets_.resize(init_bucket_size);
        buckets_mutex_ = new std::shared_mutex[init_bucket_size];
    }

    ~concurrent_hashmap() {
        delete[] buckets_mutex_;
    }

    // set. thread safe.
    void set(const K &key, const V &value) {
        auto hash = std::hash<K>()(key);
        auto bucket_index = hash % buckets_.size();
        auto &bucket = buckets_[bucket_index];
        auto &entries = bucket.entries;
        auto &mutex = buckets_mutex_[bucket_index];
        std::unique_lock<std::shared_mutex> write_lock(mutex);
        for (auto &entry: entries) {
            if (std::equal_to<K>()(entry.key, key)) {
                entry.value = value;
                return;
            }
        }
        entries.emplace_back(key, value);
        size_++;
    }

    // get. thread safe.
    bool get(const K &key, V &value) const {
        auto hash = std::hash<K>()(key);
        auto bucket_index = hash % buckets_.size();
        auto &bucket = buckets_[bucket_index];
        auto &entries = bucket.entries;
        auto &mutex = buckets_mutex_[bucket_index];
        std::shared_lock<std::shared_mutex> read_lock(mutex);
        for (auto &entry: entries) {
            if (std::equal_to<K>()(entry.key, key)) {
                value = entry.value;
                return true;
            }
        }
        return false;
    }

    template<class Key>
    bool get_by_other_type(const Key &key, V &value) const {
        auto hash = std::hash<Key>()(key);
        auto bucket_index = hash % buckets_.size();
        auto &bucket = buckets_[bucket_index];
        auto &entries = bucket.entries;
        auto &mutex = buckets_mutex_[bucket_index];
        std::shared_lock<std::shared_mutex> read_lock(mutex);
        for (auto &entry: entries) {
            if (my_equal_to<K, Key>()(entry.key, key)) {
                value = entry.value;
                return true;
            }
        }
        return false;
    }

    // remove. thread safe.
    void remove(const K &key) {
        auto hash = std::hash<K>()(key);
        auto bucket_index = hash % buckets_.size();
        auto &bucket = buckets_[bucket_index];
        auto &entries = bucket.entries;
        auto &mutex = buckets_mutex_[bucket_index];
        std::unique_lock<std::shared_mutex> write_lock(mutex);
        for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
            if (std::equal_to<K>()(iter->key, key)) {
                entries.erase(iter);
                size_--;
                return;
            }
        }
    }

    // get size. thread safe.
    size_t size() const {
        return size_;
    }

    // check rehash. not thread safe, should be called in one thread. and make sure that there is no other thread accessing the hashmap.
    bool check_rehash() {
        if (size_ / buckets_.size() < 1) {
            return false;
        }
        std::vector<bucket> new_buckets(buckets_.size() * 2);
        for (auto &bucket: buckets_) {
            for (auto &entry: bucket.entries) {
                auto hash = std::hash<K>()(entry.key);
                auto bucket_index = hash % new_buckets.size();
                auto &new_bucket = new_buckets[bucket_index];
                auto &new_entries = new_bucket.entries;
                new_entries.emplace_back(std::move(entry));
            }
        }
        buckets_ = std::move(new_buckets);
        delete[] buckets_mutex_;
        buckets_mutex_ = new std::shared_mutex[buckets_.size()];
        return true;
    }

    // clear the hashmap. not thread safe, just like check_rehash.
    void clear() {
        for (auto &bucket: buckets_) {
            bucket.entries.clear();
        }
    }

private:
    struct entry {
        K key;
        V value;
    };
    struct bucket {
        std::vector<entry> entries;
    };
    std::vector<bucket> buckets_;
    std::shared_mutex *buckets_mutex_;
    size_t size_;
};

}// namespace fakelua
