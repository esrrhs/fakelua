#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

// 为每个常量字符串分配唯一自增ID
class ConstString {
public:
    // 根据字符串返回ID，没有则分配ID
    [[nodiscard]] int64_t Alloc(const std::string_view &str);

    // 根据ID返回字符串，没有则返回空字符串
    [[nodiscard]] std::string_view Get(int64_t id) const;

    // 返回大写
    [[nodiscard]] size_t Size() const {
        return str_to_id_map_.size();
    }

private:
    std::unordered_map<std::string_view, int64_t> str_to_id_map_;
    std::unordered_map<int64_t, std::string> id_to_str_map_;
};

}// namespace fakelua
