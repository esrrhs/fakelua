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

    // 匹配单个「可量化」模式 item（不含其后的量词）：字面量、'.'、字符类、
    // 集合、%b/%f/%d 转义、'$'。捕获括号 '(' ')' 由 Match 直接处理——Lua 语法里
    // 捕获组后面不跟量词（其后的 * + - ? 是普通字面量字符）。
    const char *MatchOneItem(const char *s, const char *p, const char *&item_end) {
        if (p >= pe_) {
            item_end = p;
            return s;
        }
        switch (*p) {
            case '$':
                item_end = p + 1;
                // '$' 只在模式末尾是锚点，其他位置是普通字符。
                if (p + 1 == pe_) return (s == se_) ? s : nullptr;
                return SingleChar(s, p);
            case '%': {
                if (p + 1 >= pe_) Fail("malformed pattern (ends with '%')");
                const char c1 = p[1];
                if (c1 == 'b') {
                    if (p + 3 >= pe_) Fail("malformed pattern (missing arguments to '%b')");// 需要 p[2], p[3]
                    item_end = p + 4;
                    // 只返回平衡段尾后位置，剩余模式由调用方续接（量词亦可跟在 %b 后）。
                    return MatchBalance(s, U(p[2]), U(p[3]));
                }
                if (c1 == 'f') {
                    if (p + 2 >= pe_ || p[2] != '[') Fail("missing '[' after '%f' in pattern");
                    const char *set_after = ClassEnd(p + 2);// ']' 之后
                    item_end = set_after;
                    const char prev = (s == src_) ? '\0' : s[-1];
                    const bool cur_in = (s < se_) && SingleMatch(s, p + 2);
                    const bool prev_in = SingleMatchByte(U(prev), p + 2);
                    // 前沿模式：从前一字符不在集合中过渡到当前字符在集合中
                    // （串首视作 '\0'）。零宽 item：ep=s 或 nullptr，续接交给调用方。
                    return (cur_in && !prev_in) ? s : nullptr;
                }
                if (std::isdigit(U(c1))) {
                    const int idx = U(c1) - '1';// %1..%9 → 0..8（%0 非法）
                    if (idx < 0) Fail("invalid capture index %0");
                    item_end = p + 2;
                    return MatchCapture(s, idx);
                }
                item_end = p + 2;
                return SingleChar(s, p);
            }
            default:
                // '[' 集合或普通字面量；item 尾后位置统一用 ClassEnd（集合跳过 ']'）。
                item_end = ClassEnd(p);
                return SingleChar(s, p);
        }
    }

    // 递归匹配主入口：p 指向剩余模式（绝对位置）。
    const char *Match(const char *s, const char *p) {
        if (++depth_ > kMaxCallDepth) {
            ThrowFakeluaException("stack overflow");
        }
        // 捕获括号：开闭捕获后直接续接剩余模式，绕过量词分发（Lua 里捕获组
        // 不可量化；括号后面的量词字符按普通字面量匹配）。
        if (p < pe_ && *p == '(') {
            const char *r = (p + 1 < pe_ && p[1] == ')') ? StartCapture(s, p + 2, kCapPosition)
                                                          : StartCapture(s, p + 1, kCapUnfinished);
            --depth_;
            return r;
        }
        if (p < pe_ && *p == ')') {
            const char *r = EndCapture(s, p + 1);
            --depth_;
            return r;
        }
        const char *after;
        const char *ep = MatchOneItem(s, p, after);
        if (after >= pe_) {
            // 模式结束：成功/失败直接返回（goto init 式尾调用在这里就是返回值）。
            --depth_;
            return ep;
        }
        const char q = *after;
        const char *suffix = after + 1;
        const char *ret = nullptr;
        switch (q) {
            case '?': {
                // 先试一次，再回退零次；失败尝试整体回滚捕获状态。
                if (ep != nullptr) {
                    Capture snap[kMaxCaptures];
                    const int snap_level = level_;
                    std::memcpy(snap, caps_, sizeof(caps_));
                    ret = Match(ep, suffix);
                    if (ret == nullptr) RestoreCaps(snap, snap_level);
                }
                if (ret == nullptr) {
                    Capture snap[kMaxCaptures];
                    const int snap_level = level_;
                    std::memcpy(snap, caps_, sizeof(caps_));
                    ret = Match(s, suffix);
                    if (ret == nullptr) RestoreCaps(snap, snap_level);
                }
                break;
            }
            case '*':
                // 零次或多次；item 失败也算零次匹配成功。
                ret = Greedy(s, ep, p, suffix);
                break;
            case '+': {
                // 一次或多次：item 必须先成功；随后等价于从 ep 起的 '*'。
                if (ep == nullptr) {
                    ret = nullptr;
                } else {
                    const char *next_after;
                    const char *more = MatchOneItem(ep, p, next_after);
                    ret = Greedy(ep, more, p, suffix);
                }
                break;
            }
            case '-':
                ret = Lazy(s, p, suffix);
                break;
            default:
                // 不是量词：after 本身就是下一个 item 的起点（不是 after+1）。
                ret = ep ? Match(ep, after) : nullptr;
                break;
        }
        --depth_;
        return ret;
    }

    // 贪婪重复。cur 为下一次 item 匹配的起点，ep 为该起点上 item 的结果
    // （nullptr 表示 item 失败）。所有失败的 suffix 回溯尝试都必须把捕获栈恢复
    // 到进入本次重复时的状态（Lua end_capture/max_expand 语义）。
    const char *Greedy(const char *cur, const char *ep, const char *ip, const char *suffix) {
        const int entry_level = level_;
        if (ep == nullptr) {
            // 无法再多消费一个 item：在当前位置接 suffix（零次进一步重复合法）。
            Capture snap[kMaxCaptures];
            std::memcpy(snap, caps_, sizeof(caps_));
            const char *r = Match(cur, suffix);
            if (r == nullptr) RestoreCaps(snap, entry_level);
            return r;
        }
        if (ep == cur) {
            // item 零宽匹配成功：Lua 判定整条贪婪路径失败，不回退到零次 suffix。
            return nullptr;
        }
        const char *next_after;
        const char *nxt = MatchOneItem(ep, ip, next_after);
        if (const char *deeper = Greedy(ep, nxt, ip, suffix); deeper != nullptr) {
            return deeper;
        }
        // 后续重复失败：整体回滚捕获后在当前 item 尾后试 suffix。
        Capture snap[kMaxCaptures];
        std::memcpy(snap, caps_, sizeof(caps_));
        const char *r = Match(ep, suffix);
        if (r == nullptr) RestoreCaps(snap, entry_level);
        return r;
    }

    // 惰性重复：始终先在当前位置试 suffix，失败才再消费一个 item（必须非零宽）。
    // 每次 suffix 尝试前后做捕获整体快照/恢复，使 end_capture 能在新位置重新回填。
    const char *Lazy(const char *cur, const char *ip, const char *suffix) {
        const int entry_level = level_;
        const char *s = cur;
        for (;;) {
            Capture snap[kMaxCaptures];
            const int snap_level = level_;
            std::memcpy(snap, caps_, sizeof(caps_));
            if (const char *r = Match(s, suffix); r != nullptr) {
                return r;
            }
            RestoreCaps(snap, snap_level);
            const char *after;
            const char *nxt = MatchOneItem(s, ip, after);
            if (nxt == nullptr || nxt == s) {
                level_ = entry_level;
                return nullptr;
            }
            s = nxt;// 已消费 item 的捕获保留给下一轮 suffix
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

    // %bxy 平衡匹配。
    const char *MatchBalance(const char *s, int b, int e) {
        if (b == e || s >= se_ || U(*s) != b) return nullptr;
        const char *q = s + 1;
        int cont = 1;
        int steps = 0;
        while (q < se_) {
            if (U(*q) == b) {
                ++cont;
            } else if (U(*q) == e) {
                --cont;
                if (cont == 0) return q + 1;
            }
            ++q;
            if (++steps > kMaxCallDepth) Fail("stack overflow");
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
