#pragma once

// lua_pattern.h — PUC-Rio Lua 5.4 风格的模式匹配（pattern matching）。
//
// string.find/match/gmatch/gsub 的模式不是正则：Lua 模式不支持分组交替、量词不
// 可嵌套、转义用 '%' 而非 '\'，但额外支持平衡匹配 %bxy、前沿模式 %f[set]、惰性
// 量词 '-' 等正则无法表达的结构。这里提供自包含的字节模式匹配器，语义以
// Lua 5.4 lstrlib.c 为准（显式长度，subject/pattern 均可包含内嵌 '\0'）。
//
// 非法模式（悬空 '%'、缺失 ']'、捕获越界等）抛 FakeluaException，与 Lua 的
// "malformed pattern ..." 报错一致，而不是静默返回不匹配。

#include <cstddef>
#include <cstdint>

namespace fakelua::string::lua_pattern {

inline constexpr int kMaxCaptures = 32;
// 捕获长度哨兵（与 lstrlib.c 的 CAP_UNFINISHED / CAP_POSITION 对应）。
inline constexpr ptrdiff_t kCapUnfinished = -1;
inline constexpr ptrdiff_t kCapPosition = -2;

struct Capture {
    const char *init = nullptr;// 捕获起点（绝对指针；位置捕获也用它定位）
    ptrdiff_t len = 0;         // >=0：字节长度；否则为 kCap* 哨兵
};

struct MatchResult {
    const char *begin = nullptr;
    const char *end = nullptr; // 尾后指针（零宽匹配时 end == begin）
    int level = 0;             // 显式捕获个数
    Capture caps[kMaxCaptures]{};
};

// 在 src[start..slen) 的起点处做锚定匹配（Lua 的 string.match 语义）。
// start 可以等于 slen（允许在尾后位置匹配空串）。成功返回 true 并填 out。
// leading_caret_is_anchor=false 时不剥除前导 '^'（gmatch 把 '^' 当普通字符）。
bool MatchAt(const char *src, size_t slen, const char *pat, size_t plen, size_t start, MatchResult &out, bool leading_caret_is_anchor = true);

// 从 src[start] 起逐位扫描（string.find 语义），含尾后位置；成功返回 true。
bool Search(const char *src, size_t slen, const char *pat, size_t plen, size_t start, MatchResult &out);

}// namespace fakelua::string::lua_pattern
