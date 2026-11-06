#pragma once

// 仅供单元测试使用的白盒访问器，不属于公开 API，也不会随安装头发布。
// 公开头 include/fakelua.h 里只保留该类型的前向声明与 friend 声明，
// 方法声明在本文件，定义在 src/native/object/native_object.cpp。

#include "fakelua.h"

#include <cstdint>
#include <string_view>

namespace fakelua {

struct NativeObjectVersionTestAccess {
    // 绕过版本号维护直接写字段，用于验证 SelfCheck 能发现漏标写入。
    static void RawSetInt(NativeObject *obj, std::string_view key, int64_t val);
    // 调整全局版本戳，用于测试饱和边界。
    static void SetStamp(uint64_t stamp);
    [[nodiscard]] static uint64_t GetStamp();
    // 转发到 NativeObject 的私有 CVar 写入/读取路径。
    static void SetFromCVar(NativeObject *obj, std::string_view key, CVar v);
    [[nodiscard]] static CVar GetAsCVar(NativeObject *obj, std::string_view key, State *s);
};

}// namespace fakelua
