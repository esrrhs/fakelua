#pragma once

typedef std::shared_ptr<std::string> StrContainerPtr;

namespace std {
template<>
struct hash<StrContainerPtr> {
    size_t operator()(const StrContainerPtr &k) const noexcept {
        return std::hash<std::string>()(*k);
    }
};

template<>
struct equal_to<StrContainerPtr> {
    bool operator()(const StrContainerPtr &k1, const StrContainerPtr &k2) const {
        return *k1 == *k2;
    }
};

}// namespace std

namespace fakelua {

template<typename K1, typename K2>
struct MyEqualTo {
    bool operator()(const K1 &__x, const K2 &__y) const {
        return __x == __y;
    }
};

template<>
struct MyEqualTo<std::string, StrContainerPtr> {
    bool operator()(const std::string &k1, const StrContainerPtr &k2) const {
        return k1 == *k2;
    }
};

template<>
struct MyEqualTo<StrContainerPtr, std::string> {
    bool operator()(const StrContainerPtr &k1, const std::string &k2) const {
        return *k1 == k2;
    }
};

template<>
struct MyEqualTo<std::string_view, StrContainerPtr> {
    bool operator()(const std::string_view &k1, const StrContainerPtr &k2) const {
        return k1 == *k2;
    }
};

template<>
struct MyEqualTo<StrContainerPtr, std::string_view> {
    bool operator()(const StrContainerPtr &k1, const std::string_view &k2) const {
        return *k1 == k2;
    }
};

}// namespace fakelua
