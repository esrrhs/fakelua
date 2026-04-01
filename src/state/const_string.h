#pragma once

#include "fakelua.h"

namespace fakelua {

class State;
class VarString;

// 为每个常量字符串分配唯一自增ID
class ConstString {
public:
    explicit ConstString(State *s);

    // 根据字符串返回ID，没有则分配ID
    [[nodiscard]] int64_t Alloc(const std::string_view &str);

    // 根据ID返回字符串，没有则返回空字符串
    [[nodiscard]] static std::string_view GetString(int64_t id);

    // 根据ID返回字符串，没有则返回空字符串
    [[nodiscard]] static VarString *GetVarString(int64_t id);

    // 返回大写
    [[nodiscard]] size_t Size() const {
        return str_to_id_map_.size();
    }

private:
    State *s_;
    std::unordered_map<std::string_view, int64_t> str_to_id_map_;
};

}// namespace fakelua
