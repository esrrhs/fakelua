#pragma once

#include "jit/vm_function.h"
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua {

// NativeVarScope —— 编译期原生变量作用域管理器。
//
// 追踪在当前函数体中以原生 C 类型（int64_t / double）声明的变量。
// 每进入一个 C 作用域调用 Enter()，退出时调用 Exit()；
// DeclareNativeVar 在当前最内层作用域注册变量的原生类型。
// IsTyped / GetType 从内到外逐层查找，返回精确类型或 T_DYNAMIC。
class NativeVarScope {
public:
    // 进入一个新的 C 作用域。
    void Enter() {
        scopes_.emplace_back();
    }

    // 退出当前 C 作用域，丢弃其中所有局部声明。
    void Exit() {
        if (!scopes_.empty()) {
            scopes_.pop_back();
        }
    }

    // 清空所有作用域（在每次函数体编译开始时调用）。
    void Clear() {
        scopes_.clear();
    }

    // 在当前最内层作用域声明变量的原生类型。
    // native_type 为 T_DYNAMIC 时表示 CVar。
    void Declare(const std::string &name, InferredType native_type) {
        if (scopes_.empty()) {
            Enter();
        }
        scopes_.back()[name] = native_type;
    }

    // 判断变量是否已声明为非 CVar 的原生类型（int64_t / double）。
    [[nodiscard]] bool IsTyped(const std::string &name) const {
        for (const auto &scope: std::views::reverse(scopes_)) {
            if (const auto it = scope.find(name); it != scope.end()) {
                return it->second != T_DYNAMIC;
            }
        }
        return false;
    }

    // 返回变量声明时的原生类型，未找到时返回 T_DYNAMIC。
    [[nodiscard]] InferredType GetType(const std::string &name) const {
        for (const auto &scope: std::views::reverse(scopes_)) {
            if (const auto it = scope.find(name); it != scope.end()) {
                return it->second;
            }
        }
        return T_DYNAMIC;
    }

private:
    // 作用域栈：每个元素是一个 name -> InferredType 映射。
    std::vector<std::unordered_map<std::string, InferredType>> scopes_;
};

}// namespace fakelua
