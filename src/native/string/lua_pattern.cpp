#include "native/string/lua_pattern.h"
#include "util/exception.h"

#include <cctype>
#include <cstring>
#include <string>

namespace fakelua::string::lua_pattern {

namespace {

constexpr int kMaxCallDepth = 200;// 与 lstrlib.c 的 MAXCCALLS 一致，防止病态模式打爆 C 栈

static inline unsigned char U(char c) {
    return static_cast<unsigned char>(c);
}

// Lua 5.4 lstrlib.c 模式匹配器的 C++ 移植。所有边界都用显式尾指针表示，
// subject / pattern 中允许出现内嵌 '\0'。
//
// 关键边界（均已用 PUC-Rio Lua 5.4/5.5 实跑核对）：
//   - '*' item 匹配失败时允许零次重复（"b*" 在非 b 位置 / 串尾都匹配空串）；
//   - '?' item 失败（含串尾）时回退零次（"a?" 在串尾也产生空匹配）；
//   - item 零宽匹配成功时，贪婪重复整体判定失败（"()*" 不产生匹配）；
//   - '-' 惰性量词零宽合法，从「零次 + suffix」开始试。
class PatternMatcher {
public:
    PatternMatcher(const char *src, size_t slen, const char *pat, size_t plen)
        : src_(src), se_(src + slen), pstart_(pat), pe_(pat + plen) {
    }

    bool RunAt(size_t start, MatchResult &out) {
        if (start > static_cast<size_t>(se_ - src_)) return false;
        level_ = 0;
        depth_ = 0;
        const char *s = src_ + start;
        const char *ep = Match(s, pstart_);
        if (ep == nullptr) return false;
        out.begin = s;
        out.end = ep;
        out.level = level_;
        for (int i = 0; i < level_; ++i) {
            out.caps[i] = caps_[i];
        }
        return true;
    }

private:
    const char *src_;
    const char *se_;
    const char *pstart_;
    const char *pe_;
    int depth_ = 0;
    int level_ = 0;
    Capture caps_[kMaxCaptures]{};

    [[noreturn]] static void Fail(const std::string &msg) {
        ThrowFakeluaException(msg);
    }

    // 量词回溯时，一次 suffix 尝试可能开闭/回填捕获；失败后把全部捕获槽与
    // level 恢复到尝试前（end_capture 回填的是 caps_[level-1]，必须一并回滚，
    // 才能在新位置重新回填——与 Lua start/end_capture + max/min_expand 一致）。
    void RestoreCaps(const Capture *snap, int snap_level) {
        std::memcpy(caps_, snap, sizeof(caps_));
        level_ = snap_level;
    }

    // 递归匹配主入口：p 指向剩余模式（绝对位置）。
    const char *Match(const char *s, const char *p) {
        if (++depth_ > kMaxCallDepth) {
            ThrowFakeluaException("stack overflow");
        }
    init:
        if (p >= pe_) {
            --depth_;
            return s;
        }
        switch (*p) {
            case '(': {
                const char *r = (p + 1 < pe_ && p[1] == ')') ? StartCapture(s, p + 2, kCapPosition)
                                                              : StartCapture(s, p + 1, kCapUnfinished);
                --depth_;
                return r;
            }
            case ')': {
                const char *r = EndCapture(s, p + 1);
                --depth_;
                return r;
            }
            case '$': {
                if (p + 1 == pe_) {
                    --depth_;
                    return (s == se_) ? s : nullptr;
                }
                goto dflt;
            }
            case '%': {
                if (p + 1 < pe_) {
                    const char c1 = p[1];
                    if (c1 == 'b') {
                        if (p + 3 >= pe_) Fail("malformed pattern (missing arguments to '%b')");
                        s = MatchBalance(s, U(p[2]), U(p[3]));
                        if (s != nullptr) {
                            p += 4;
                            goto init;
                        }
                        --depth_;
                        return nullptr;
                    }
                    if (c1 == 'f') {
                        if (p + 2 >= pe_ || p[2] != '[') Fail("missing '[' after '%f' in pattern");
                        const char *ep = ClassEnd(p + 2);
                        const char prev = (s == src_) ? '\0' : s[-1];
                        const char cur = (s == se_) ? '\0' : *s;
                        if (!SingleMatchByte(U(prev), p + 2) && SingleMatchByte(U(cur), p + 2)) {
                            p = ep;
                            goto init;
                        }
                        --depth_;
                        return nullptr;
                    }
                    if (std::isdigit(U(c1))) {
                        const int idx = U(c1) - '1';
                        if (idx < 0) Fail("invalid capture index %0");
                        s = MatchCapture(s, idx);
                        if (s != nullptr) {
                            p += 2;
                            goto init;
                        }
                        --depth_;
                        return nullptr;
                    }
                }
                goto dflt;
            }
            default: dflt: {
                const char *ep = ClassEnd(p);
                const bool m = (s < se_) && SingleMatch(s, p);
                if (!m) {
                    if (ep < pe_ && (*ep == '*' || *ep == '?' || *ep == '-')) {
                        p = ep + 1;
                        goto init;
                    }
                    --depth_;
                    return nullptr;
                }
                // matched once
                if (ep >= pe_) {
                    ++s;
                    p = ep;
                    goto init;
                }
                switch (*ep) {
                    case '?': {
                        Capture snap[kMaxCaptures];
                        const int snap_level = level_;
                        std::memcpy(snap, caps_, sizeof(caps_));
                        const char *res = Match(s + 1, ep + 1);
                        if (res != nullptr) {
                            --depth_;
                            return res;
                        }
                        RestoreCaps(snap, snap_level);
                        p = ep + 1;
                        goto init;
                    }
                    case '+':
                        s++;
                        /* FALLTHROUGH */
                    case '*': {
                        const char *res = MaxExpand(s, p, ep);
                        --depth_;
                        return res;
                    }
                    case '-': {
                        const char *res = MinExpand(s, p, ep);
                        --depth_;
                        return res;
                    }
                    default:
                        s++;
                        p = ep;
                        goto init;
                }
            }
        }
    }

    // 贪婪重复：迭代扫描最大重复次数，然后自底向上回溯
    const char *MaxExpand(const char *s, const char *p, const char *ep) {
        ptrdiff_t i = 0;
        while (s + i < se_ && SingleMatch(s + i, p)) {
            i++;
        }
        // 每次失败后都会恢复到进入时的状态，所以快照只需在循环外取一次。
        Capture snap[kMaxCaptures];
        const int snap_level = level_;
        std::memcpy(snap, caps_, sizeof(caps_));
        while (i >= 0) {
            const char *res = Match(s + i, ep + 1);
            if (res != nullptr) return res;
            RestoreCaps(snap, snap_level);
            i--;
        }
        return nullptr;
    }

    // 惰性重复
    const char *MinExpand(const char *s, const char *p, const char *ep) {
        Capture snap[kMaxCaptures];
        const int snap_level = level_;
        std::memcpy(snap, caps_, sizeof(caps_));
        for (;;) {
            const char *res = Match(s, ep + 1);
            if (res != nullptr) return res;
            RestoreCaps(snap, snap_level);
            if (s < se_ && SingleMatch(s, p)) {
                s++;
            } else {
                return nullptr;
            }
        }
    }

    const char *StartCapture(const char *s, const char *after, ptrdiff_t what) {
        if (level_ >= kMaxCaptures) Fail("too many captures");
        caps_[level_].init = s;
        caps_[level_].len = what;
        ++level_;
        return Match(s, after);
    }

    const char *EndCapture(const char *s, const char *after) {
        int i = level_ - 1;
        while (i >= 0 && caps_[i].len != kCapUnfinished) {
            --i;
        }
        if (i < 0) Fail("invalid pattern capture");
        caps_[i].len = s - caps_[i].init;
        return Match(s, after);
    }

    const char *MatchCapture(const char *s, int i) {
        if (i >= level_) {
            Fail(std::string("invalid capture index %") + std::to_string(i + 1));
        }
        const ptrdiff_t l = caps_[i].len;
        if (l == kCapUnfinished) {
            Fail(std::string("invalid capture index %") + std::to_string(i + 1));
        }
        if (l == kCapPosition) {
            return (s == caps_[i].init) ? s : nullptr;
        }
        if (se_ - s < l) return nullptr;
        return (std::memcmp(caps_[i].init, s, static_cast<size_t>(l)) == 0) ? s + l : nullptr;
    }

    // %bxy 平衡匹配。Lua 5.4 语义：当 b == e 时，首字符匹配 b，之后遇首个 e 即终止；且无固定长度上限。
    const char *MatchBalance(const char *s, int b, int e) {
        if (s >= se_ || U(*s) != b) return nullptr;
        if (b == e) {
            // 成对相同分隔符（如 %b"" 或 %baa）：
            // s 匹配开头的 b，随后向前扫描直到遇到第一个闭合的 e
            const char *q = s + 1;
            while (q < se_) {
                if (U(*q) == e) return q + 1;
                ++q;
            }
            return nullptr;
        }
        const char *q = s + 1;
        int cont = 1;
        while (q < se_) {
            if (U(*q) == e) {
                if (--cont == 0) return q + 1;
            } else if (U(*q) == b) {
                ++cont;
            }
            ++q;
        }
        return nullptr;
    }

    // 字符类（%a/%d/... 大写取反）；非字母类名按字面量匹配（%e 匹配 'e'）。
    static bool MatchClass(int c, int cl) {
        bool res;
        switch (std::tolower(cl)) {
            case 'a': res = std::isalpha(c) != 0; break;
            case 'c': res = std::iscntrl(c) != 0; break;
            case 'd': res = std::isdigit(c) != 0; break;
            case 'g': res = std::isgraph(c) != 0; break;
            case 'l': res = std::islower(c) != 0; break;
            case 'p': res = std::ispunct(c) != 0; break;
            case 's': res = std::isspace(c) != 0; break;
            case 'u': res = std::isupper(c) != 0; break;
            case 'w': res = std::isalnum(c) != 0; break;
            case 'x': res = std::isxdigit(c) != 0; break;
            case 'z': res = (c == 0); break;// Lua 5.1/5.5 保留类：零字节
            default: return cl == c;
        }
        return std::islower(cl) ? res : !res;
    }

    bool SingleMatchByte(int c, const char *p) const {
        switch (*p) {
            case '%':
                return MatchClass(c, U(p[1]));
            case '.':
                return true;
            case '[': {
                bool sig = true;
                const char *q = p + 1;
                if (q < pe_ && *q == '^') {
                    sig = false;
                    ++q;
                }
                // do-while 语义：先测成员再看是否落在结束 ']' 上——因此紧跟 '[' 或
                // '[^' 的 ']' 会被当成普通成员测一次（[]x] 里的首 ']'）。
                do {
                    bool in;
                    if (*q == '%') {
                        // '%' 转义成员（含集合内嵌字符类如 [%d_]）。
                        in = (q + 1 < pe_) && MatchClass(c, U(q[1]));
                        q += 2;
                    } else {
                        const unsigned char lo = U(*q);
                        // 区间：普通字符 '-' 普通字符；紧贴 ']' 的 '-' 是字面量。
                        if (q + 2 < pe_ && q[1] == '-' && q[2] != ']') {
                            in = (lo <= c && c <= U(q[2]));
                            q += 3;
                        } else {
                            in = (c == lo);
                            q += 1;
                        }
                    }
                    if (in) return sig;
                } while (q < pe_ && *q != ']');
                return !sig;
            }
            default:
                return c == U(*p);
        }
    }

    bool SingleMatch(const char *s, const char *p) const {
        return s < se_ && SingleMatchByte(U(*s), p);
    }

    const char *SingleChar(const char *s, const char *p) const {
        if (SingleMatch(s, p)) return s + 1;
        return nullptr;
    }

    // 返回模式 item 的尾后位置；校验悬空 '%' 与缺失 ']'。
    const char *ClassEnd(const char *p) const {
        if (*p == '%') {
            if (p + 1 < pe_) return p + 2;
            Fail("malformed pattern (ends with '%')");
        }
        if (*p == '[') {
            const char *q = p + 1;
            if (q < pe_ && *q == '^') ++q;
            if (q < pe_ && *q == ']') ++q;// 集合开头的 ']' 是字面量
            while (q < pe_ && *q != ']') {
                if (*q == '%') {
                    ++q;
                    if (q >= pe_) Fail("malformed pattern (ends with '%')");
                }
                ++q;
                if (q > pe_) Fail("malformed pattern (missing ']')");
            }
            if (q >= pe_) Fail("malformed pattern (missing ']')");
            return q + 1;
        }
        if (p < pe_) return p + 1;
        Fail("malformed pattern (missing ']')");
    }
};

}// namespace

bool MatchAt(const char *src, size_t slen, const char *pat, size_t plen, size_t start, MatchResult &out, bool leading_caret_is_anchor) {
    // 前导 '^' 只表示锚定（MatchAt 本身就是锚定），剥除；其余位置是普通字符。
    const char *p = pat;
    size_t use_len = plen;
    if (leading_caret_is_anchor && plen > 0 && pat[0] == '^') {
        ++p;
        --use_len;
    }
    PatternMatcher ms(src, slen, p, use_len);
    return ms.RunAt(start, out);
}

bool Search(const char *src, size_t slen, const char *pat, size_t plen, size_t start, MatchResult &out) {
    const char *p = pat;
    size_t use_len = plen;
    bool anchored = false;
    if (plen > 0 && pat[0] == '^') {
        anchored = true;
        ++p;
        --use_len;
    }
    if (start > slen) return false;
    PatternMatcher ms(src, slen, p, use_len);
    if (anchored) {
        return ms.RunAt(start, out);
    }
    for (size_t pos = start; pos <= slen; ++pos) {
        if (ms.RunAt(pos, out)) return true;
    }
    return false;
}

}// namespace fakelua::string::lua_pattern
