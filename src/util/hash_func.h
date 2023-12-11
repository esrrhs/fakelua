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

}// namespace fakelua
