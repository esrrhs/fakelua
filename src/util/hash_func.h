#pragma once

using StrContainerPtr = std::shared_ptr<std::string>;

namespace std {
template<>
struct hash<StrContainerPtr> {
    size_t operator()(const StrContainerPtr &key) const noexcept {
        return std::hash<std::string>()(*key);
    }
};

template<>
struct equal_to<StrContainerPtr> {
    bool operator()(const StrContainerPtr &key1, const StrContainerPtr &key2) const {
        return *key1 == *key2;
    }
};

}// namespace std

namespace fakelua {

template<typename K1, typename K2>
struct MyEqualTo {
    bool operator()(const K1 &lhs, const K2 &rhs) const {
        return lhs == rhs;
    }
};

template<>
struct MyEqualTo<std::string, StrContainerPtr> {
    bool operator()(const std::string &key1, const StrContainerPtr &key2) const {
        return key1 == *key2;
    }
};

template<>
struct MyEqualTo<StrContainerPtr, std::string> {
    bool operator()(const StrContainerPtr &key1, const std::string &key2) const {
        return *key1 == key2;
    }
};

template<>
struct MyEqualTo<std::string_view, StrContainerPtr> {
    bool operator()(const std::string_view &key1, const StrContainerPtr &key2) const {
        return key1 == *key2;
    }
};

template<>
struct MyEqualTo<StrContainerPtr, std::string_view> {
    bool operator()(const StrContainerPtr &key1, const std::string_view &key2) const {
        return *key1 == key2;
    }
};

}// namespace fakelua
