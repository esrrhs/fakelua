#include "native/string/native_string.h"
#include "compile/c_runtime_header.h"
#include "jit/jit_error_boundary.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "native/string/lua_pattern.h"
#include "native/table/native_table.h"
#include "state/state.h"
#include "util/utf8_io.h"
#include "var/var.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/endian/conversion.hpp>
#include <cctype>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <locale>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fakelua::string {

static inline int64_t NormalizePos(int64_t pos, int64_t len) {
    if (pos >= 0) {
        return pos;
    }
    return len + pos + 1;
}

// Lua 模式（pattern matching，见 lua_pattern.h）辅助。
// find/match/gmatch/gsub 使用的是 Lua 5.4 语义的模式，不是正则。
namespace {

namespace lp = lua_pattern;

// 把一个捕获值转成 Lua 值：位置捕获返回整数，其余返回（可能为空的）字符串。
CVar CaptureToCVar(State *state, const char *src_base, const lp::MatchResult &m, int i) {
    const auto &cap = m.caps[i];
    if (cap.len == lp::kCapPosition) {
        return inter::NativeToFakeluaInt(state, static_cast<int64_t>(cap.init - src_base) + 1);
    }
    return inter::NativeToFakeluaStringView(state, std::string_view(cap.init, static_cast<size_t>(cap.len)));
}

// gsub 字符串替换中 %1-%9 引用捕获：位置捕获转成十进制数字串，其余为捕获切片。
static std::string CaptureToLuaString(const char *src_base, const lp::MatchResult &m, int i) {
    const auto &cap = m.caps[i];
    if (cap.len == lp::kCapPosition) {
        return std::to_string(static_cast<int64_t>(cap.init - src_base) + 1);
    }
    if (cap.len == lp::kCapUnfinished) {
        ThrowFakeluaException("unfinished capture");
    }
    return std::string(cap.init, static_cast<size_t>(cap.len));
}

// 组装 find 的返回：start, end（1-based，零宽匹配允许 end == start-1），后接捕获。
// 匹配成功但存在未闭合捕获时与 Lua 一样报 "unfinished capture"。
static void CheckCapturesFinished(const lp::MatchResult &m) {
    for (int i = 0; i < m.level; ++i) {
        if (m.caps[i].len == lp::kCapUnfinished) {
            ThrowFakeluaException("unfinished capture");
        }
    }
}

CVar BuildFindResult(State *state, const char *src_base, const lp::MatchResult &m) {
    CheckCapturesFinished(m);
    const int64_t start = m.begin - src_base + 1;
    const int64_t finish = m.end - src_base;// 1-based 闭区间尾（零宽时为 start-1）
    const int total = 2 + m.level;
    CVar multi = inter::AllocMultiCVar(state, total);
    inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaInt(state, start));
    inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaInt(state, finish));
    for (int i = 0; i < m.level; ++i) {
        inter::SetMultiCVarElement(multi, i + 2, CaptureToCVar(state, src_base, m, i));
    }
    return multi;
}

// Lua integer widths are 1..16; we pack through uint64 so cap at 8.
// Missing size (bare "i"/"I") defaults to sizeof(lua_Integer) == 8.
static int ParsePackIntegralSize(const char *&p, const char *end) {
    if (p >= end || *p < '0' || *p > '9') return 8;
    int sz = 0;
    while (p < end && *p >= '0' && *p <= '9') {
        int d = *p - '0';
        if (sz > (INT_MAX - d) / 10) {
            ThrowFakeluaException("integral size out of limits");
        }
        sz = sz * 10 + d;
        ++p;
    }
    if (sz < 1 || sz > 8) {
        ThrowFakeluaException("integral size out of limits");
    }
    return sz;
}

static constexpr int kMaxPackBlock = 64 * 1024 * 1024;
static constexpr int kMaxPackAlign = 32;

// Parse a decimal size that is present in the format. No digits → default_v.
// Overflow or value outside [minv, maxv] throws (except default_v when absent).
static int ParsePackDecSize(const char *&p, const char *end, int minv, int maxv, int default_v) {
    if (p >= end || *p < '0' || *p > '9') return default_v;
    int64_t sz = 0;
    while (p < end && *p >= '0' && *p <= '9') {
        int d = *p - '0';
        if (sz > (INT64_MAX - d) / 10) {
            ThrowFakeluaException("size out of limits");
        }
        sz = sz * 10 + d;
        ++p;
    }
    if (sz < minv || sz > maxv) {
        ThrowFakeluaException("size out of limits");
    }
    return static_cast<int>(sz);
}

static void CheckFormatItemSize(std::string_view spec) {
    int64_t n = 0;
    for (char ch: spec) {
        if (ch >= '0' && ch <= '9') {
            int d = ch - '0';
            if (n > (1024 * 1024 - d) / 10) {
                ThrowFakeluaException("invalid format (width or precision too large)");
            }
            n = n * 10 + d;
        } else {
            n = 0;
        }
    }
}

}// namespace

// gmatch 迭代器状态（存储在闭包 upvalue 中，arena 分配，无需手动释放）。
// text/pattern 自有副本，跨 tick / arena reset 都安全。
// prev 是上一次产出匹配的【尾后位置】；首轮标记为 SIZE_MAX，使位置 0 的零宽
// 匹配也能产出（与 PUC-Rio gmatch_iter 一致）。
struct GMatchState {
    std::string text;
    std::string pattern;
    size_t prev = std::numeric_limits<size_t>::max();
};

// string.pack / packsize / unpack 二进制序列化辅助
struct PackMachine {
    bool big_endian = false;
    int align = 0;// 0 = no alignment

    static bool NativeIsBig() {
        return boost::endian::order::native == boost::endian::order::big;
    }

    template<class T, std::size_t N>
    static void StoreN(unsigned char *p, T v, bool big) {
        if (big) {
            boost::endian::endian_store<T, N, boost::endian::order::big>(p, v);
        } else {
            boost::endian::endian_store<T, N, boost::endian::order::little>(p, v);
        }
    }

    template<class T, std::size_t N>
    static T LoadN(const unsigned char *p, bool big) {
        if (big) {
            return boost::endian::endian_load<T, N, boost::endian::order::big>(p);
        }
        return boost::endian::endian_load<T, N, boost::endian::order::little>(p);
    }

    // data 指向主机字节序下的整数/IEEE 位型。n=3,5,6,7 时调用方传入的是 uint64_t*。
    static void WriteVal(std::string &out, const void *data, size_t n, bool big) {
        unsigned char buf[8];
        switch (n) {
            case 1: {
                uint8_t x = 0;
                std::memcpy(&x, data, 1);
                StoreN<uint8_t, 1>(buf, x, big);
                break;
            }
            case 2: {
                uint16_t x = 0;
                std::memcpy(&x, data, 2);
                StoreN<uint16_t, 2>(buf, x, big);
                break;
            }
            case 3: {
                uint64_t x = 0;
                std::memcpy(&x, data, 8);
                StoreN<uint64_t, 3>(buf, x, big);
                break;
            }
            case 4: {
                uint32_t x = 0;
                std::memcpy(&x, data, 4);
                StoreN<uint32_t, 4>(buf, x, big);
                break;
            }
            case 5: {
                uint64_t x = 0;
                std::memcpy(&x, data, 8);
                StoreN<uint64_t, 5>(buf, x, big);
                break;
            }
            case 6: {
                uint64_t x = 0;
                std::memcpy(&x, data, 8);
                StoreN<uint64_t, 6>(buf, x, big);
                break;
            }
            case 7: {
                uint64_t x = 0;
                std::memcpy(&x, data, 8);
                StoreN<uint64_t, 7>(buf, x, big);
                break;
            }
            case 8: {
                uint64_t x = 0;
                std::memcpy(&x, data, 8);
                StoreN<uint64_t, 8>(buf, x, big);
                break;
            }
            default:
                return;
        }
        out.append(reinterpret_cast<char *>(buf), n);
    }

    static void ReadVal(const unsigned char *src, void *dst, size_t n, bool big) {
        switch (n) {
            case 1: {
                uint8_t x = LoadN<uint8_t, 1>(src, big);
                std::memcpy(dst, &x, 1);
                break;
            }
            case 2: {
                uint16_t x = LoadN<uint16_t, 2>(src, big);
                std::memcpy(dst, &x, 2);
                break;
            }
            case 3: {
                uint64_t x = LoadN<uint64_t, 3>(src, big);
                std::memcpy(dst, &x, 8);
                break;
            }
            case 4: {
                uint32_t x = LoadN<uint32_t, 4>(src, big);
                std::memcpy(dst, &x, 4);
                break;
            }
            case 5: {
                uint64_t x = LoadN<uint64_t, 5>(src, big);
                std::memcpy(dst, &x, 8);
                break;
            }
            case 6: {
                uint64_t x = LoadN<uint64_t, 6>(src, big);
                std::memcpy(dst, &x, 8);
                break;
            }
            case 7: {
                uint64_t x = LoadN<uint64_t, 7>(src, big);
                std::memcpy(dst, &x, 8);
                break;
            }
            case 8: {
                uint64_t x = LoadN<uint64_t, 8>(src, big);
                std::memcpy(dst, &x, 8);
                break;
            }
            default:
                break;
        }
    }

    void PadToMultiple(std::string &out, size_t item_size) {
        if (align <= 0) return;
        size_t current = out.size();
        size_t mod = current % static_cast<size_t>(align);
        if (mod != 0) {
            out.append(static_cast<size_t>(align) - mod, '\0');
        }
    }

    int PackSpec(std::string &out, const char *fmt, const char *end, State *state, CVar *args, int &arg_idx, int total_args);
    int SizeSpec(const char *fmt, const char *end, State *state, CVar *args, int &arg_idx, int total_args);
};

int PackMachine::PackSpec(std::string &out, const char *fmt, const char *end, State *state, CVar *args, int &arg_idx, int total_args) {
    while (fmt < end) {
        char c = *fmt;

        // Spaces are ignored
        if (c == ' ') {
            ++fmt;
            continue;
        }

        // Handle alignment directive !n
        if (c == '!') {
            ++fmt;
            align = ParsePackDecSize(fmt, end, 1, kMaxPackAlign, 0);
            if (align <= 0) return -1;
            continue;
        }

        // Handle endianness
        if (c == '<') {
            big_endian = false;
            ++fmt;
            continue;
        }
        if (c == '>') {
            big_endian = true;
            ++fmt;
            continue;
        }
        // '=' : native endianness, no alignment (x86 = little-endian)
        if (c == '=') {
            big_endian = PackMachine::NativeIsBig();
            align = 0;
            ++fmt;
            continue;
        }

        // Padding byte
        if (c == 'X') {
            ++fmt;
            out.push_back('\0');
            continue;
        }

        // Specifiers that take a size argument: i[n], I[n], c[n]
        if (c == 'c') {
            ++fmt;
            int count = ParsePackDecSize(fmt, end, 1, kMaxPackBlock, 0);
            if (count <= 0) return -1;
            if (arg_idx >= total_args) return -1;
            CVar val = inter::GetNativeArg(state, args, total_args, arg_idx);
            // 标准 Lua：c[n] 的参数必须是 string，Bool/Table 不合法
            if (val.type_ == static_cast<int>(VarType::Bool) || val.type_ == static_cast<int>(VarType::Table)) {
                ThrowFakeluaException("bad argument to 'pack' (string expected)");
            }
            ++arg_idx;
            std::string_view sv = KeyToStringView(val);
            size_t copy_len = sv.size() < static_cast<size_t>(count) ? sv.size() : static_cast<size_t>(count);
            out.append(sv.data(), copy_len);
            if (copy_len < static_cast<size_t>(count)) {
                out.append(static_cast<size_t>(count) - copy_len, '\0');
            }
            continue;
        }

        if (c == 'i' || c == 'I') {
            bool is_unsigned = (c == 'I');
            ++fmt;
            int sz = ParsePackIntegralSize(fmt, end);
            if (arg_idx >= total_args) return -1;
            CVar val = inter::GetNativeArg(state, args, total_args, arg_idx);
            if (val.type_ == static_cast<int>(VarType::Bool) || val.type_ == static_cast<int>(VarType::Table)) {
                ThrowFakeluaException("bad argument to 'string.pack' (number expected)");
            }
            ++arg_idx;
            PadToMultiple(out, static_cast<size_t>(sz));

            if (is_unsigned) {
                uint64_t v = static_cast<uint64_t>(inter::CVarToInteger(val, 0));
                if (sz < 8) {
                    uint64_t mask = (uint64_t{1} << (sz * 8)) - 1;
                    v &= mask;
                }
                WriteVal(out, &v, static_cast<size_t>(sz), big_endian);
            } else {
                int64_t v = inter::CVarToInteger(val, 0);
                uint64_t uv = static_cast<uint64_t>(v);
                if (sz < 8) {
                    uint64_t mask = (uint64_t{1} << (sz * 8)) - 1;
                    uv &= mask;
                }
                WriteVal(out, &uv, static_cast<size_t>(sz), big_endian);
            }
            continue;
        }

        // Fixed-size specifiers
        if (arg_idx >= total_args) return -1;
        CVar val = inter::GetNativeArg(state, args, total_args, arg_idx);
        if (val.type_ == static_cast<int>(VarType::Bool) || val.type_ == static_cast<int>(VarType::Table)) {
            ThrowFakeluaException("bad argument to 'string.pack' (number expected)");
        }
        ++arg_idx;

        switch (c) {
            case 'b': {// signed char
                int64_t v = inter::CVarToInteger(val, 0);
                char b = static_cast<char>(v);
                out.push_back(b);
                break;
            }
            case 'B': {// unsigned char
                int64_t v = inter::CVarToInteger(val, 0);
                unsigned char b = static_cast<unsigned char>(v);
                out.push_back(static_cast<char>(b));
                break;
            }
            case 'h': {// signed short
                int64_t v = inter::CVarToInteger(val, 0);
                PadToMultiple(out, 2);
                int16_t sv = static_cast<int16_t>(v);
                WriteVal(out, &sv, 2, big_endian);
                break;
            }
            case 'H': {// unsigned short
                int64_t v = inter::CVarToInteger(val, 0);
                PadToMultiple(out, 2);
                uint16_t sv = static_cast<uint16_t>(v);
                WriteVal(out, &sv, 2, big_endian);
                break;
            }
            case 'l': {// signed long (4 bytes in Lua)
                int64_t v = inter::CVarToInteger(val, 0);
                PadToMultiple(out, 4);
                int32_t sv = static_cast<int32_t>(v);
                WriteVal(out, &sv, 4, big_endian);
                break;
            }
            case 'L': {// unsigned long (4 bytes in Lua)
                int64_t v = inter::CVarToInteger(val, 0);
                PadToMultiple(out, 4);
                uint32_t sv = static_cast<uint32_t>(v);
                WriteVal(out, &sv, 4, big_endian);
                break;
            }
            case 'j': {// lua_integer (int64)
                int64_t v = inter::CVarToInteger(val, 0);
                PadToMultiple(out, 8);
                WriteVal(out, &v, 8, big_endian);
                break;
            }
            case 'J': {// lua_unsigned (uint64)
                int64_t v = inter::CVarToInteger(val, 0);
                PadToMultiple(out, 8);
                uint64_t uv = static_cast<uint64_t>(v);
                WriteVal(out, &uv, 8, big_endian);
                break;
            }
            case 'T': {// size_t (8 bytes)
                int64_t v = inter::CVarToInteger(val, 0);
                PadToMultiple(out, 8);
                uint64_t uv = static_cast<uint64_t>(v);
                WriteVal(out, &uv, 8, big_endian);
                break;
            }
            case 'f': {// float (4 bytes)
                double dv = inter::CVarToNumber(val, 0.0);
                PadToMultiple(out, 4);
                float fv = static_cast<float>(dv);
                WriteVal(out, &fv, 4, big_endian);
                break;
            }
            case 'd': {// double (8 bytes)
                double dv = inter::CVarToNumber(val, 0.0);
                PadToMultiple(out, 8);
                WriteVal(out, &dv, 8, big_endian);
                break;
            }
            case 'z': {// zero-terminated string
                std::string_view sv = KeyToStringView(val);
                out.append(sv.data(), sv.size());
                out.push_back('\0');
                break;
            }
            default:
                return -1;// unknown specifier
        }
        ++fmt;
    }
    return 0;
}

int PackMachine::SizeSpec(const char *fmt, const char *end, State *state, CVar *args, int &arg_idx, int total_args) {
    size_t total = 0;
    while (fmt < end) {
        char c = *fmt;
        if (c == ' ') {
            ++fmt;
            continue;
        }
        if (c == '!') {
            ++fmt;
            align = ParsePackDecSize(fmt, end, 1, kMaxPackAlign, 0);
            if (align <= 0) return -1;
            continue;
        }
        if (c == '<' || c == '>' || c == '=') {
            ++fmt;
            continue;
        }
        if (c == 'X') {
            ++fmt;
            total += 1;
            continue;
        }
        if (c == 'c') {
            ++fmt;
            int count = ParsePackDecSize(fmt, end, 1, kMaxPackBlock, 0);
            if (count <= 0) return -1;
            if (arg_idx >= total_args) return -1;
            ++arg_idx;
            total += static_cast<size_t>(count);
            continue;
        }
        if (c == 'i' || c == 'I') {
            ++fmt;
            int sz = ParsePackIntegralSize(fmt, end);
            if (arg_idx >= total_args) return -1;
            ++arg_idx;
            if (align > 0) {
                size_t mod = total % static_cast<size_t>(align);
                if (mod != 0) total += static_cast<size_t>(align) - mod;
            }
            total += static_cast<size_t>(sz);
            continue;
        }

        if (arg_idx >= total_args) return -1;
        ++arg_idx;

        size_t item_size = 0;
        switch (c) {
            case 'b':
            case 'B':
                item_size = 1;
                break;
            case 'h':
            case 'H':
                item_size = 2;
                break;
            case 'l':
            case 'L':
                item_size = 4;
                break;
            case 'j':
            case 'J':
            case 'T':
                item_size = 8;
                break;
            case 'f':
                item_size = 4;
                break;
            case 'd':
                item_size = 8;
                break;
            case 'z': {
                CVar val = inter::GetNativeArg(state, args, total_args, arg_idx - 1);
                std::string_view sv = KeyToStringView(val);
                total += sv.size() + 1;
                ++fmt;
                continue;
            }
            default:
                return -1;
        }
        if (align > 0) {
            size_t mod = total % static_cast<size_t>(align);
            if (mod != 0) total += static_cast<size_t>(align) - mod;
        }
        total += item_size;
        ++fmt;
    }
    return static_cast<int>(total);
}

// gmatch 迭代器原生函数
// 闭包签名：CVar (*)(VarClosure *cl, CVar s, CVar var)
// upvalues[0] = State* (as int)
// upvalues[1] = GMatchState* (as int，由 arena 分配，无需手动释放)
extern "C" CVar GMatchIterator(VarClosure *cl, CVar /*s*/, CVar /*var*/) {
    if (!cl || cl->upvalue_count < 2) {
        return CVar{static_cast<int>(VarType::Nil)};
    }
    State *iter_state = reinterpret_cast<State *>(cl->upvalues[0]->data_.i);
    GMatchState *gs = reinterpret_cast<GMatchState *>(cl->upvalues[1]->data_.i);
    if (!iter_state || !gs) {
        return inter::NativeToFakeluaNil(iter_state);
    }

    const size_t slen = gs->text.size();
    size_t pos = (gs->prev == std::numeric_limits<size_t>::max()) ? 0 : gs->prev;
    // gmatch 不像 gsub/find 有锚定概念：模式里的 '^' 是普通字符。
    const char *src = gs->text.data();
    const char *pat = gs->pattern.c_str();
    while (pos <= slen) {
        lua_pattern::MatchResult m;
        if (lua_pattern::MatchAt(src, slen, pat, gs->pattern.size(), pos, m, /*leading_caret_is_anchor=*/false)) {
            const size_t mend = static_cast<size_t>(m.end - src);
            // 零宽匹配只在「不是紧跟上一个产出位置」时产出，否则跳过一位，
            // 避免重复空匹配（PUC-Rio gmatch_iter 语义）。
            if (mend != pos || pos != gs->prev) {
                gs->prev = mend;
                CheckCapturesFinished(m);
                if (m.level > 0) {
                    CVar multi = inter::AllocMultiCVar(iter_state, m.level);
                    for (int i = 0; i < m.level; ++i) {
                        inter::SetMultiCVarElement(multi, i, CaptureToCVar(iter_state, src, m, i));
                    }
                    return multi;
                }
                return inter::NativeToFakeluaStringView(iter_state, std::string_view(m.begin, static_cast<size_t>(m.end - m.begin)));
            }
        }
        ++pos;
    }
    gs->prev = std::numeric_limits<size_t>::max();// 标记迭代结束
    return inter::NativeToFakeluaNil(iter_state);
}

std::string_view GetStringArgView(CVar a, std::string &temp) {
    if (a.type_ == static_cast<int>(VarType::String) || a.type_ == static_cast<int>(VarType::StringId)) {
        return KeyToStringView(a);
    } else if (a.type_ == static_cast<int>(VarType::Int) || a.type_ == static_cast<int>(VarType::Float)) {
        temp = AsVar(a).ToString(/*has_quote=*/false, /*has_postfix=*/false);
        return temp;
    }
    return {};
}

void RegisterStringLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "string.len", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaInt(state, 0);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.len");
        std::string temp;
        std::string_view sv = GetStringArgView(a0, temp);
        return inter::NativeToFakeluaInt(state, static_cast<int64_t>(sv.size()));
    });

    RegisterNativeFunction(s, "string.sub", 2, true, [](State *state, CVar *args, int n) -> CVar {
        // 与 Lua 一致：缺参直接报错，不再悄悄返回空串（差分 fuzz 已抓到过这种宽松）。
        if (n < 1) ThrowBadArgument(1, "string.sub", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.sub", "number expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.sub");
        std::string temp;
        std::string_view sv = GetStringArgView(a0, temp);
        int64_t len = static_cast<int64_t>(sv.size());

        int64_t start_pos = CheckIntegerArg(inter::GetNativeArg(state, args, n, 1), 2, "string.sub");
        int64_t end_pos = len;
        if (n >= 3) {
            end_pos = CheckIntegerArg(inter::GetNativeArg(state, args, n, 2), 3, "string.sub");
        }

        start_pos = NormalizePos(start_pos, len);
        end_pos = NormalizePos(end_pos, len);

        if (start_pos < 1) start_pos = 1;
        if (end_pos > len) end_pos = len;

        if (start_pos > end_pos || start_pos > len || end_pos < 1) {
            return inter::NativeToFakeluaStringView(state, "");
        }

        size_t sub_len = static_cast<size_t>(end_pos - start_pos + 1);
        return inter::NativeToFakeluaStringView(state, sv.substr(static_cast<size_t>(start_pos - 1), sub_len));
    });

    RegisterNativeFunction(s, "string.rep", 2, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.rep");
        std::string temp;
        std::string_view sv = GetStringArgView(a0, temp);

        CVar rep_var = inter::GetNativeArg(state, args, n, 1);
        // luaL_checkinteger：2^63 / NaN / 1.5 必须报错，不能 CVarToInteger 回落到 0（变成空串）。
        int64_t rep_cnt = CheckIntegerArg(rep_var, 2, "string.rep");
        if (rep_cnt <= 0) return inter::NativeToFakeluaStringView(state, "");

        std::string sep = "";
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            CheckStringArg(a2, 3, "string.rep");
            std::string temp_sep;
            sep = std::string(GetStringArgView(a2, temp_sep));
        }

        if (sv.empty() && sep.empty()) {
            return inter::NativeToFakeluaStringView(state, "");
        }

        size_t unit_len = sv.size() + sep.size();
        if (unit_len > 0 && static_cast<uint64_t>(rep_cnt) > (1073741824ULL / unit_len)) {
            ThrowFakeluaException("string.rep: resulting string too large");
        }

        std::string res;
        res.reserve(unit_len * static_cast<size_t>(rep_cnt));
        for (int64_t i = 0; i < rep_cnt; ++i) {
            if (i > 0 && !sep.empty()) res += sep;
            res += sv;
        }
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.reverse", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.reverse");
        std::string temp;
        std::string str_val(GetStringArgView(a0, temp));
        std::reverse(str_val.begin(), str_val.end());
        return inter::NativeToFakeluaStringView(state, str_val);
    });

    RegisterNativeFunction(s, "string.lower", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.lower");
        std::string temp;
        std::string_view sv = GetStringArgView(a0, temp);
        VarString *vs = VarString::AllocTempRaw(state, sv.size());
        char *out = vs->MutableData();
        for (size_t i = 0; i < sv.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(sv[i]);
            // ASCII 快路径（与 C++ bench / Lua 常见用法一致）；非 A-Z 原样拷贝
            out[i] = (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : static_cast<char>(c);
        }
        CVar ret{static_cast<int>(VarType::String)};
        ret.data_.s = vs;
        return ret;
    });

    RegisterNativeFunction(s, "string.upper", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.upper");
        std::string temp;
        std::string_view sv = GetStringArgView(a0, temp);
        VarString *vs = VarString::AllocTempRaw(state, sv.size());
        char *out = vs->MutableData();
        for (size_t i = 0; i < sv.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(sv[i]);
            out[i] = (c >= 'a' && c <= 'z') ? static_cast<char>(c - 32) : static_cast<char>(c);
        }
        CVar ret{static_cast<int>(VarType::String)};
        ret.data_.s = vs;
        return ret;
    });

    // string.trim(s) — 两端空白（C locale isspace），Boost.Algorithm
    RegisterNativeFunction(s, "string.trim", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.trim", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.trim");
        std::string temp;
        std::string out(GetStringArgView(a0, temp));
        boost::algorithm::trim(out, std::locale::classic());
        return inter::NativeToFakeluaStringView(state, out);
    });

    // string.trim_left(s) / string.trim_right(s) — 只去左/右空白
    RegisterNativeFunction(s, "string.trim_left", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.trim_left", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.trim_left");
        std::string temp;
        std::string out(GetStringArgView(a0, temp));
        boost::algorithm::trim_left(out, std::locale::classic());
        return inter::NativeToFakeluaStringView(state, out);
    });

    RegisterNativeFunction(s, "string.trim_right", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.trim_right", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.trim_right");
        std::string temp;
        std::string out(GetStringArgView(a0, temp));
        boost::algorithm::trim_right(out, std::locale::classic());
        return inter::NativeToFakeluaStringView(state, out);
    });

    // string.split(s, sep) — 按分隔串切开（可多字符），空段保留；sep 不能为空
    RegisterNativeFunction(s, "string.split", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.split", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.split", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.split");
        CheckStringArg(a1, 2, "string.split");
        std::string temp0, temp1;
        std::string input(GetStringArgView(a0, temp0));
        std::string sep(GetStringArgView(a1, temp1));
        if (sep.empty()) {
            ThrowFakeluaException("bad argument #2 to 'string.split' (separator must be non-empty)");
        }
        std::vector<std::string> parts;
        boost::algorithm::iter_split(parts, input, boost::algorithm::first_finder(sep));
        CVar tbl = table::TableHelper::CreateTable(state);
        for (size_t i = 0; i < parts.size(); ++i) {
            table::TableHelper::SetTableInt(state, tbl, static_cast<int64_t>(i + 1), inter::NativeToFakeluaStringView(state, parts[i]));
        }
        return tbl;
    });

    // string.join(tbl, sep) — split 的逆操作（Boost.Algorithm join）
    RegisterNativeFunction(s, "string.join", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.join", "table expected");
        if (n < 2) ThrowBadArgument(2, "string.join", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        if (a0.type_ != static_cast<int>(VarType::Table) || !a0.data_.t) {
            ThrowBadArgument(1, "string.join", "table expected");
        }
        CheckStringArg(a1, 2, "string.join");
        std::string temp_sep;
        std::string sep(GetStringArgView(a1, temp_sep));
        int64_t len = table::TableHelper::GetTableLen(a0);
        if (len < 0 || static_cast<uint64_t>(len) > 10000000ULL) {
            ThrowFakeluaException("string.join: too many items");
        }
        std::vector<std::string> parts;
        parts.reserve(len > 0 ? static_cast<size_t>(len) : 0);
        for (int64_t i = 1; i <= len; ++i) {
            CVar item = table::TableHelper::GetTableInt(state, a0, i);
            CheckStringArg(item, 1, "string.join");
            std::string temp;
            parts.emplace_back(GetStringArgView(item, temp));
        }
        return inter::NativeToFakeluaStringView(state, boost::algorithm::join(parts, sep));
    });

    // string.starts_with(s, prefix)
    RegisterNativeFunction(s, "string.starts_with", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.starts_with", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.starts_with", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.starts_with");
        CheckStringArg(a1, 2, "string.starts_with");
        std::string temp0, temp1;
        std::string_view sv = GetStringArgView(a0, temp0);
        std::string_view prefix = GetStringArgView(a1, temp1);
        return inter::NativeToFakeluaBool(state, boost::algorithm::starts_with(sv, prefix));
    });

    // string.ends_with(s, suffix)
    RegisterNativeFunction(s, "string.ends_with", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.ends_with", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.ends_with", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.ends_with");
        CheckStringArg(a1, 2, "string.ends_with");
        std::string temp0, temp1;
        std::string_view sv = GetStringArgView(a0, temp0);
        std::string_view suffix = GetStringArgView(a1, temp1);
        return inter::NativeToFakeluaBool(state, boost::algorithm::ends_with(sv, suffix));
    });

    // string.contains(s, needle)
    RegisterNativeFunction(s, "string.contains", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.contains", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.contains", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.contains");
        CheckStringArg(a1, 2, "string.contains");
        std::string temp0, temp1;
        std::string_view sv = GetStringArgView(a0, temp0);
        std::string_view needle = GetStringArgView(a1, temp1);
        return inter::NativeToFakeluaBool(state, boost::algorithm::contains(sv, needle));
    });

    // string.replace(s, from, to) — 字面量全局替换（Boost.Algorithm）
    RegisterNativeFunction(s, "string.replace", 3, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.replace", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.replace", "string expected");
        if (n < 3) ThrowBadArgument(3, "string.replace", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CVar a2 = inter::GetNativeArg(state, args, n, 2);
        CheckStringArg(a0, 1, "string.replace");
        CheckStringArg(a1, 2, "string.replace");
        CheckStringArg(a2, 3, "string.replace");
        std::string temp0, temp1, temp2;
        std::string out(GetStringArgView(a0, temp0));
        std::string from(GetStringArgView(a1, temp1));
        std::string to(GetStringArgView(a2, temp2));
        if (from.empty()) {
            ThrowFakeluaException("bad argument #2 to 'string.replace' (search string must be non-empty)");
        }
        boost::algorithm::replace_all(out, from, to);
        return inter::NativeToFakeluaStringView(state, out);
    });

    // string.iequals(a, b) — ASCII 大小写不敏感相等
    RegisterNativeFunction(s, "string.iequals", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.iequals", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.iequals", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.iequals");
        CheckStringArg(a1, 2, "string.iequals");
        std::string temp0, temp1;
        std::string_view a = GetStringArgView(a0, temp0);
        std::string_view b = GetStringArgView(a1, temp1);
        return inter::NativeToFakeluaBool(state, boost::algorithm::iequals(a, b, std::locale::classic()));
    });

    // string.icontains(s, needle) — ASCII 大小写不敏感包含
    RegisterNativeFunction(s, "string.icontains", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.icontains", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.icontains", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.icontains");
        CheckStringArg(a1, 2, "string.icontains");
        std::string temp0, temp1;
        std::string_view sv = GetStringArgView(a0, temp0);
        std::string_view needle = GetStringArgView(a1, temp1);
        return inter::NativeToFakeluaBool(state, boost::algorithm::icontains(sv, needle, std::locale::classic()));
    });

    // string.istarts_with(s, prefix) / string.iends_with(s, suffix) — ASCII 大小写不敏感
    RegisterNativeFunction(s, "string.istarts_with", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.istarts_with", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.istarts_with", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.istarts_with");
        CheckStringArg(a1, 2, "string.istarts_with");
        std::string temp0, temp1;
        std::string_view sv = GetStringArgView(a0, temp0);
        std::string_view prefix = GetStringArgView(a1, temp1);
        return inter::NativeToFakeluaBool(state, boost::algorithm::istarts_with(sv, prefix, std::locale::classic()));
    });

    RegisterNativeFunction(s, "string.iends_with", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "string.iends_with", "string expected");
        if (n < 2) ThrowBadArgument(2, "string.iends_with", "string expected");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.iends_with");
        CheckStringArg(a1, 2, "string.iends_with");
        std::string temp0, temp1;
        std::string_view sv = GetStringArgView(a0, temp0);
        std::string_view suffix = GetStringArgView(a1, temp1);
        return inter::NativeToFakeluaBool(state, boost::algorithm::iends_with(sv, suffix, std::locale::classic()));
    });

    RegisterNativeFunction(s, "string.byte", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "string.byte");
        std::string temp;
        std::string_view sv = GetStringArgView(a0, temp);
        int64_t len = static_cast<int64_t>(sv.size());
        if (len == 0) return inter::NativeToFakeluaNil(state);

        int64_t start_pos = 1;
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            // 与 Lua 5.4 对齐：超大 float 报 "number has no integer representation"
            start_pos = CheckIntegerArg(a1, 2, "string.byte");
        }

        int64_t end_pos = start_pos;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            end_pos = CheckIntegerArg(a2, 3, "string.byte");
        }

        start_pos = NormalizePos(start_pos, len);
        end_pos = NormalizePos(end_pos, len);

        if (start_pos < 1 || start_pos > len || end_pos < start_pos) {
            return inter::AllocMultiCVar(state, 0);
        }

        if (end_pos > len) end_pos = len;
        int count = static_cast<int>(end_pos - start_pos + 1);

        if (count == 1) {
            return inter::NativeToFakeluaInt(state, static_cast<unsigned char>(sv[static_cast<size_t>(start_pos - 1)]));
        }

        CVar multi = inter::AllocMultiCVar(state, count);
        for (int i = 0; i < count; ++i) {
            CVar item = inter::NativeToFakeluaInt(state, static_cast<unsigned char>(sv[static_cast<size_t>(start_pos - 1 + i)]));
            inter::SetMultiCVarElement(multi, i, item);
        }
        return multi;
    });

    RegisterNativeFunction(s, "string.char", 0, true, [](State *state, CVar *args, int n) -> CVar {
        // 单参数热路径：直接写 1 字节 VarString，跳过 std::string 中转
        if (n == 1) {
            CVar arg0 = inter::GetNativeArg(state, args, n, 0);
            if (arg0.type_ == static_cast<int>(VarType::Int) || arg0.type_ == static_cast<int>(VarType::Float)) {
                if (arg0.type_ == static_cast<int>(VarType::Float)) {
                    int64_t iv = 0;
                    if (!DoubleFitsInt64(arg0.data_.f, &iv)) {
                        ThrowFakeluaException("bad argument to 'string.char' (number has no integer representation)");
                    }
                }
                int64_t c = inter::CVarToInteger(arg0, -1);
                if (c < 0 || c > 255) {
                    ThrowFakeluaException("bad argument to 'string.char' (value out of range)");
                }
                VarString *vs = VarString::AllocTempRaw(state, 1);
                vs->MutableData()[0] = static_cast<char>(c);
                CVar ret{static_cast<int>(VarType::String)};
                ret.data_.s = vs;
                return ret;
            }
            ThrowFakeluaException("bad argument to 'string.char' (number expected)");
        }
        std::string res;
        res.reserve(static_cast<size_t>(n));
        for (int i = 0; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            // 标准 Lua：string.char 要求 number 参数，Bool/Table/String/Nil 不合法
            if (arg_i.type_ != static_cast<int>(VarType::Int) && arg_i.type_ != static_cast<int>(VarType::Float)) {
                ThrowFakeluaException("bad argument to 'string.char' (number expected)");
            }
            if (arg_i.type_ == static_cast<int>(VarType::Float)) {
                int64_t iv = 0;
                if (!DoubleFitsInt64(arg_i.data_.f, &iv)) {
                    ThrowFakeluaException("bad argument to 'string.char' (number has no integer representation)");
                }
            }
            int64_t c = inter::CVarToInteger(arg_i, -1);
            if (c < 0 || c > 255) {
                ThrowFakeluaException("bad argument to 'string.char' (value out of range)");
            }
            res.push_back(static_cast<char>(c));
        }
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.format", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar fmt_var = inter::GetNativeArg(state, args, n, 0);
        std::string temp_fmt;
        std::string_view fmt = GetStringArgView(fmt_var, temp_fmt);

        // 热路径：string.format("%d", int) —— bench 与常见用法，跳过通用解析器
        if (fmt == "%d" && n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ == static_cast<int>(VarType::Int) || a1.type_ == static_cast<int>(VarType::Float) || a1.type_ == static_cast<int>(VarType::String) ||
                a1.type_ == static_cast<int>(VarType::StringId)) {
                int64_t ival = inter::CVarToInteger(a1, 0);
                char buf[32];
                int len = snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(ival));
                if (len < 0) len = 0;
                return inter::NativeToFakeluaStringView(state, std::string_view(buf, static_cast<size_t>(len)));
            }
            if (a1.type_ == static_cast<int>(VarType::Bool) || a1.type_ == static_cast<int>(VarType::Table) || a1.type_ == static_cast<int>(VarType::Nil)) {
                ThrowFakeluaException("bad argument to 'format' (number expected)");
            }
        }

        std::string res;
        res.reserve(fmt.size() + 32);

        int arg_idx = 1;
        size_t i = 0;
        size_t len = fmt.size();

        while (i < len) {
            if (fmt[i] != '%') {
                res.push_back(fmt[i++]);
                continue;
            }

            i++;
            if (i >= len) {
                res.push_back('%');
                break;
            }

            if (fmt[i] == '%') {
                res.push_back('%');
                i++;
                continue;
            }

            size_t spec_start = i - 1;
            while (i < len && (std::isdigit(static_cast<unsigned char>(fmt[i])) || fmt[i] == '-' || fmt[i] == '+' || fmt[i] == ' ' || fmt[i] == '#' || fmt[i] == '.' || fmt[i] == '0')) {
                i++;
            }

            if (i >= len) {
                res.append(fmt.substr(spec_start));
                break;
            }

            char spec = fmt[i++];
            std::string spec_str(fmt.substr(spec_start, i - spec_start));
            if (spec == 'n') {
                ThrowFakeluaException("invalid option '%n' to 'format'");
            }
            CheckFormatItemSize(spec_str);

            CVar curr_arg = (arg_idx < n) ? inter::GetNativeArg(state, args, n, arg_idx++) : CVar{static_cast<int>(VarType::Nil)};

            if (spec == 'q') {
                if (curr_arg.type_ == static_cast<int>(VarType::Bool) || curr_arg.type_ == static_cast<int>(VarType::Table)) {
                    ThrowFakeluaException("bad argument to 'format' (string expected)");
                }
                std::string temp_q;
                std::string_view sval = GetStringArgView(curr_arg, temp_q);
                res.push_back('"');
                for (char c: sval) {
                    if (c == '"') res.append("\\\"");
                    else if (c == '\\')
                        res.append("\\\\");
                    else if (c == '\n')
                        res.append("\\n");
                    else if (c == '\r')
                        res.append("\\r");
                    else
                        res.push_back(c);
                }
                res.push_back('"');
            } else if (spec == 's') {
                // 标准 Lua：%s 的参数必须是 string/number，Bool/Table 不合法
                if (curr_arg.type_ == static_cast<int>(VarType::Bool) || curr_arg.type_ == static_cast<int>(VarType::Table)) {
                    ThrowFakeluaException("bad argument to 'format' (string expected)");
                }
                std::string sval;
                if (curr_arg.type_ == static_cast<int>(VarType::String) || curr_arg.type_ == static_cast<int>(VarType::StringId)) {
                    sval = std::string(KeyToStringView(curr_arg));
                } else if (curr_arg.type_ == static_cast<int>(VarType::Int)) {
                    sval = std::to_string(curr_arg.data_.i);
                } else if (curr_arg.type_ == static_cast<int>(VarType::Float)) {
                    sval = std::to_string(curr_arg.data_.f);
                } else {
                    sval = AsVar(curr_arg).ToString(/*has_quote=*/false, /*has_postfix=*/false);
                }
                if (spec_str == "%s") {
                    res.append(sval);
                } else {
                    int needed = snprintf(nullptr, 0, spec_str.c_str(), sval.c_str());
                    if (needed > 0) {
                        std::vector<char> buf(static_cast<size_t>(needed) + 1);
                        snprintf(buf.data(), buf.size(), spec_str.c_str(), sval.c_str());
                        res.append(buf.data());
                    }
                }
            } else if (spec == 'd' || spec == 'i') {
                // 标准 Lua 5.3：整数格式接受 number 或 numeric string
                if (curr_arg.type_ == static_cast<int>(VarType::Bool) || curr_arg.type_ == static_cast<int>(VarType::Table) || curr_arg.type_ == static_cast<int>(VarType::Nil)) {
                    ThrowFakeluaException("bad argument to 'format' (number expected)");
                }
                int64_t ival = inter::CVarToInteger(curr_arg, 0);
                std::string llspec = spec_str;
                llspec.insert(llspec.size() - 1, "ll");
                int needed = snprintf(nullptr, 0, llspec.c_str(), static_cast<long long>(ival));
                if (needed > 0) {
                    std::vector<char> buf(static_cast<size_t>(needed) + 1);
                    snprintf(buf.data(), buf.size(), llspec.c_str(), static_cast<long long>(ival));
                    res.append(buf.data());
                }
            } else if (spec == 'u' || spec == 'x' || spec == 'X' || spec == 'o') {
                // 标准 Lua 5.3：无符号整数格式接受 number 或 numeric string
                if (curr_arg.type_ == static_cast<int>(VarType::Bool) || curr_arg.type_ == static_cast<int>(VarType::Table) || curr_arg.type_ == static_cast<int>(VarType::Nil)) {
                    ThrowFakeluaException("bad argument to 'format' (number expected)");
                }
                uint64_t uval = static_cast<uint64_t>(inter::CVarToInteger(curr_arg, 0));
                std::string llspec = spec_str;
                llspec.insert(llspec.size() - 1, "ll");
                int needed = snprintf(nullptr, 0, llspec.c_str(), static_cast<unsigned long long>(uval));
                if (needed > 0) {
                    std::vector<char> buf(static_cast<size_t>(needed) + 1);
                    snprintf(buf.data(), buf.size(), llspec.c_str(), static_cast<unsigned long long>(uval));
                    res.append(buf.data());
                }
            } else if (spec == 'f' || spec == 'e' || spec == 'E' || spec == 'g' || spec == 'G') {
                // 标准 Lua 5.3：浮点格式接受 number 或 numeric string
                if (curr_arg.type_ == static_cast<int>(VarType::Bool) || curr_arg.type_ == static_cast<int>(VarType::Table) || curr_arg.type_ == static_cast<int>(VarType::Nil)) {
                    ThrowFakeluaException("bad argument to 'format' (number expected)");
                }
                double fval = inter::CVarToNumber(curr_arg, 0.0);
                int needed = snprintf(nullptr, 0, spec_str.c_str(), fval);
                if (needed > 0) {
                    std::vector<char> buf(static_cast<size_t>(needed) + 1);
                    snprintf(buf.data(), buf.size(), spec_str.c_str(), fval);
                    res.append(buf.data());
                }
            } else if (spec == 'c') {
                // 标准 Lua 5.3：%c 接受 number 或 numeric string
                if (curr_arg.type_ == static_cast<int>(VarType::Bool) || curr_arg.type_ == static_cast<int>(VarType::Table) || curr_arg.type_ == static_cast<int>(VarType::Nil)) {
                    ThrowFakeluaException("bad argument to 'format' (number expected)");
                }
                int64_t cval = inter::CVarToInteger(curr_arg, 0);
                res.push_back(static_cast<char>(cval));
            } else if (spec == 'p') {
                // fakelua 扩展：%p 接受 number 或 nil（输出指针地址），Bool/Table/String 不合法
                if (curr_arg.type_ == static_cast<int>(VarType::Bool) || curr_arg.type_ == static_cast<int>(VarType::Table)) {
                    ThrowFakeluaException("bad argument to 'format' (number expected)");
                }
                // 指针地址：格式化为 0x 前缀的十六进制（始终输出 0x...，即使值为 0）
                // 使用 uintptr_t 保证 64 位指针不截断（Windows 上 unsigned long 仅 32 位）
                if (curr_arg.type_ == static_cast<int>(VarType::Int)) {
                    char buf[64];
                    auto val = static_cast<uintptr_t>(curr_arg.data_.i);
                    if (val == 0) {
                        res.append("0x0");
                    } else {
                        std::snprintf(buf, sizeof(buf), "0x%" PRIxPTR, val);
                        res.append(buf);
                    }
                } else if (curr_arg.type_ == static_cast<int>(VarType::Float)) {
                    char buf[64];
                    int64_t iv = 0;
                    if (DoubleFitsInt64(curr_arg.data_.f, &iv)) {
                        // 整数值的浮点（如 2.0）允许转为指针地址输出
                        uintptr_t val = static_cast<uintptr_t>(iv);
                        if (val == 0) {
                            res.append("0x0");
                        } else {
                            std::snprintf(buf, sizeof(buf), "0x%" PRIxPTR, val);
                            res.append(buf);
                        }
                    } else {
                        // 非整数浮点（如 1.5、2^63、NaN、Inf）不能作为指针地址，报错
                        ThrowFakeluaException("bad argument to 'format' (number has no integer representation)");
                    }
                } else {
                    // 非数值类型（nil 等）：输出 CVar 自身地址
                    char buf[64];
                    std::snprintf(buf, sizeof(buf), "0x%" PRIxPTR, reinterpret_cast<uintptr_t>(&curr_arg));
                    res.append(buf);
                }
            } else {
                res.append(spec_str);
            }
        }
        return inter::NativeToFakeluaStringView(state, res);
    });

    // string.find(s, pattern [, init [, plain]])
    // 在 s 中查找 Lua 模式 pattern，返回起始位置与结束位置（1-based）。
    // 若 pattern 含捕获，则后续返回值依次为各捕获。
    // 若 plain 为 true，则退化为纯子串查找（忽略模式元字符）。
    // 找不到时返回 nil。
    RegisterNativeFunction(s, "string.find", 2, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.find");
        CheckStringArg(a1, 2, "string.find");
        std::string temp0, temp1;
        std::string_view sv = GetStringArgView(a0, temp0);
        std::string_view pat_view = GetStringArgView(a1, temp1);
        int64_t len = static_cast<int64_t>(sv.size());

        int64_t init_pos = 1;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            CheckNumberArg(a2, 3, "string.find");
            init_pos = inter::CVarToInteger(a2, 1);
        }
        init_pos = NormalizePos(init_pos, len);
        if (init_pos < 1) init_pos = 1;
        if (init_pos > len + 1) {
            return inter::NativeToFakeluaNil(state);
        }

        bool plain = false;
        if (n >= 4) {
            CVar a3 = inter::GetNativeArg(state, args, n, 3);
            plain = (a3.type_ == static_cast<int>(VarType::Bool) && a3.data_.b);
        }

        std::string sub = std::string(sv.substr(static_cast<size_t>(init_pos - 1)));

        if (plain) {
            // 纯子串查找
            size_t pos = sub.find(std::string(pat_view));
            if (pos == std::string::npos) return inter::NativeToFakeluaNil(state);
            int64_t start = init_pos + static_cast<int64_t>(pos);
            int64_t end = start + static_cast<int64_t>(pat_view.size()) - 1;
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaInt(state, start));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaInt(state, end));
            return multi;
        }

        // Lua 模式匹配：在原串上从 init_pos-1 起扫描（位置 1-based 由 BuildFindResult 处理）。
        // 非法模式直接抛错（与 Lua 的 malformed pattern 一致）。
        lua_pattern::MatchResult m;
        if (!lua_pattern::Search(sv.data(), static_cast<size_t>(len), pat_view.data(), pat_view.size(),
                                 static_cast<size_t>(init_pos - 1), m)) {
            return inter::NativeToFakeluaNil(state);
        }
        return BuildFindResult(state, sv.data(), m);
    });

    // string.match(s, pattern [, init])
    // 与 find 一样从 init 起搜索，只是不返回位置；前导 '^' 锚定到 init 点。
    // 有捕获返回所有捕获（位置捕获为整数），无捕获返回整个匹配。
    RegisterNativeFunction(s, "string.match", 2, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.match");
        CheckStringArg(a1, 2, "string.match");

        std::string temp0, temp1;
        std::string_view sv = GetStringArgView(a0, temp0);
        std::string_view pat_view = GetStringArgView(a1, temp1);
        int64_t len = static_cast<int64_t>(sv.size());

        int64_t init_pos = 1;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            CheckNumberArg(a2, 3, "string.match");
            init_pos = inter::CVarToInteger(a2, 1);
        }
        init_pos = NormalizePos(init_pos, len);
        if (init_pos < 1) init_pos = 1;
        if (init_pos > len + 1) {
            return inter::NativeToFakeluaNil(state);
        }

        lua_pattern::MatchResult m;
        // Search 内部会处理前导 '^' 的锚定；无 '^' 时从 init 起逐位搜索。
        if (!lua_pattern::Search(sv.data(), static_cast<size_t>(len), pat_view.data(), pat_view.size(),
                                 static_cast<size_t>(init_pos - 1), m)) {
            return inter::NativeToFakeluaNil(state);
        }
        if (m.level > 0) {
            CheckCapturesFinished(m);
            CVar multi = inter::AllocMultiCVar(state, m.level);
            for (int i = 0; i < m.level; ++i) {
                inter::SetMultiCVarElement(multi, i, CaptureToCVar(state, sv.data(), m, i));
            }
            return multi;
        }
        return inter::NativeToFakeluaStringView(state, std::string_view(m.begin, static_cast<size_t>(m.end - m.begin)));
    });

    // string.gmatch(s, pattern)
    // 返回一个迭代器闭包；每次调用返回下一个匹配（有捕获时返回捕获值）。
    // 模式惰性解析，非法模式在首次迭代时抛错（与 Lua 一致）。
    RegisterNativeFunction(s, "string.gmatch", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.gmatch");
        CheckStringArg(a1, 2, "string.gmatch");
        std::string temp0, temp1;
        std::string text(GetStringArgView(a0, temp0));
        std::string pattern(GetStringArgView(a1, temp1));

        // 模式语法惰性校验（与 Lua 一致：迭代时才解析），这里只持有副本。
        // arena 分配迭代器状态（text/pattern 自有副本）。
        auto &alloc = state->GetValueAllocator();
        GMatchState *gs = alloc.New<GMatchState>();
        gs->text = std::move(text);
        gs->pattern = std::move(pattern);

        // 使用共享辅助函数创建迭代器闭包
        return MakeIteratorClosure(state, reinterpret_cast<void *>(GMatchIterator), gs);
    });

    // string.gsub(s, pattern, repl [, n])
    // Lua 模式替换。repl 为 string（%0-%9/%% 引用捕获）/ function（收到捕获或整个
    // 匹配；返回 nil/false 保留原文）/ table（以首个捕获或整个匹配为键查询，nil/false
    // 保留原文）。前导 '^' 表示只在起点尝试一次。
    RegisterNativeFunction(s, "string.gsub", 3, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 3) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "string.gsub");
        CheckStringArg(a1, 2, "string.gsub");

        CVar repl_var = inter::GetNativeArg(state, args, n, 2);
        std::string temp0, temp1;
        std::string_view sv = GetStringArgView(a0, temp0);
        std::string_view pat_view = GetStringArgView(a1, temp1);
        const size_t slen = sv.size();

        int64_t max_replace = -1;
        if (n >= 4) {
            CVar a3 = inter::GetNativeArg(state, args, n, 3);
            if (a3.type_ == static_cast<int>(VarType::Bool) || a3.type_ == static_cast<int>(VarType::Table)) {
                ThrowFakeluaException("bad argument #4 to 'string.gsub' (number expected)");
            }
            max_replace = inter::CVarToInteger(a3, -1);
        }

        // 标准 Lua：gsub 的替换参数必须是 string/function/table，Bool 不合法
        if (repl_var.type_ == static_cast<int>(VarType::Bool)) {
            ThrowFakeluaException("bad argument #3 to 'string.gsub' (string/function/table expected, got boolean)");
        }
        const bool repl_is_table = (repl_var.type_ == static_cast<int>(VarType::Table) && repl_var.data_.t);
        const bool repl_is_closure = (repl_var.type_ == static_cast<int>(VarType::Closure) && repl_var.data_.cl);
        const std::string repl_str = (repl_is_table || repl_is_closure) ? std::string() : std::string(KeyToStringView(repl_var));

        const bool anchored = !pat_view.empty() && pat_view[0] == '^';
        const char *eff_pat = pat_view.data() + (anchored ? 1 : 0);
        const size_t eff_len = pat_view.size() - (anchored ? 1 : 0);

        std::string result;
        result.reserve(slen);
        size_t copied = 0;    // 已拷进 result 的原文位置
        size_t src = 0;       // 下一次搜索起点
        size_t prev_end = std::numeric_limits<size_t>::max();// 上一次产出匹配的尾后位置
        int64_t count = 0;

        while ((max_replace < 0 || count < max_replace) && src <= slen) {
            // 从 src 起逐位尝试；与 gmatch 相同的零宽规则：起点处的零宽匹配若紧跟上
            // 一次产出匹配的尾后位置，则跳过该位继续找（避免相邻空匹配）。
            lua_pattern::MatchResult m;
            bool found = false;
            size_t trial = src;
            const size_t trial_end = anchored ? src : slen;
            for (; trial <= trial_end; ++trial) {
                if (!lua_pattern::MatchAt(sv.data(), slen, eff_pat, eff_len, trial, m)) {
                    if (anchored) break;
                    continue;
                }
                const size_t mst = static_cast<size_t>(m.begin - sv.data());
                const size_t men = static_cast<size_t>(m.end - sv.data());
                if (men == mst && mst == prev_end) {
                    if (anchored) break;
                    continue;
                }
                found = true;
                break;
            }
            if (!found) break;
            CheckCapturesFinished(m);
            const size_t st = static_cast<size_t>(m.begin - sv.data());
            const size_t en = static_cast<size_t>(m.end - sv.data());
            src = st;

            result.append(sv.data() + copied, st - copied);

            const std::string_view whole(m.begin, en - st);
            std::string replacement;
            bool keep_original = false;

            if (repl_is_closure) {
                VarClosure *cl = repl_var.data_.cl;
                const int call_arg_count = (m.level > 0) ? m.level : 1;
                if (call_arg_count > static_cast<int>(kMaxFunctionInputParams)) {
                    ThrowFakeluaException(std::format("string.gsub: too many capture arguments ({}), max is {}",
                                                      call_arg_count, kMaxFunctionInputParams));
                }
                std::vector<CVar> call_args(static_cast<size_t>(call_arg_count));
                if (m.level > 0) {
                    for (int i = 0; i < call_arg_count; ++i) {
                        call_args[static_cast<size_t>(i)] = CaptureToCVar(state, sv.data(), m, i);
                    }
                } else {
                    call_args[0] = inter::NativeToFakeluaStringView(state, whole);
                }
                // 在发起 gsub 的引擎里同步调用替换函数（与 pool:with 同理）。
                CVar fn_res = (cl->func_ptr != nullptr)
                                      ? inter::DispatchCallClosure(state, cl, call_args.data(), call_arg_count, state->CurrentJit())
                                      : FlEvalLoadClosure(state, cl, call_arg_count, call_args.data());
                if (fn_res.type_ == static_cast<int>(VarType::Nil) ||
                    (fn_res.type_ == static_cast<int>(VarType::Bool) && !fn_res.data_.b)) {
                    keep_original = true;
                } else if (fn_res.type_ == static_cast<int>(VarType::Bool)) {
                    ThrowFakeluaException("invalid replacement value (a boolean)");
                } else if (fn_res.type_ == static_cast<int>(VarType::Table)) {
                    ThrowFakeluaException("invalid replacement value (a table)");
                } else {
                    replacement = std::string(KeyToStringView(fn_res));
                }
            } else if (repl_is_table) {
                CVar val;
                if (m.level > 0) {
                    const auto &cap0 = m.caps[0];
                    if (cap0.len == lua_pattern::kCapPosition) {
                        val = table::TableHelper::GetTableInt(state, repl_var, static_cast<int64_t>(cap0.init - sv.data()) + 1);
                    } else {
                        std::string key(cap0.init, static_cast<size_t>(cap0.len));
                        val = table::TableHelper::GetTableStrId(state, repl_var, key.c_str());
                    }
                } else {
                    std::string key(whole);
                    val = table::TableHelper::GetTableStrId(state, repl_var, key.c_str());
                }
                if (val.type_ == static_cast<int>(VarType::Nil) ||
                    (val.type_ == static_cast<int>(VarType::Bool) && !val.data_.b)) {
                    keep_original = true;
                } else if (val.type_ == static_cast<int>(VarType::Bool) || val.type_ == static_cast<int>(VarType::Table)) {
                    ThrowFakeluaException("invalid replacement value (a boolean)");
                } else {
                    replacement = std::string(KeyToStringView(val));
                }
            } else {
                // 字符串替换：%0=整个匹配，%1-%9=捕获，%%=百分号，其余 %x 报错。
                for (size_t i = 0; i < repl_str.size(); ++i) {
                    if (repl_str[i] != '%') {
                        replacement.push_back(repl_str[i]);
                        continue;
                    }
                    if (i + 1 >= repl_str.size()) {
                        ThrowFakeluaException("invalid use of '%' in replacement string");
                    }
                    const char next = repl_str[++i];
                    if (next == '%') {
                        replacement.push_back('%');
                    } else if (next == '0') {
                        replacement.append(whole.data(), whole.size());
                    } else if (next >= '1' && next <= '9') {
                        const int idx = next - '1';
                        if (idx >= m.level) {
                            ThrowFakeluaException(std::format("invalid capture index %{}", idx + 1));
                        }
                        replacement.append(CaptureToLuaString(sv.data(), m, idx));
                    } else {
                        ThrowFakeluaException(std::format("invalid use of '%{}' in a replacement string", next));
                    }
                }
            }

            if (keep_original) {
                result.append(whole.data(), whole.size());
            } else {
                result += replacement;
            }
            ++count;
            prev_end = en;

            if (en == st) {
                // 零宽匹配：保留当前字节（若有），下一轮从后一位置继续，避免死循环。
                if (st < slen) {
                    result.push_back(sv[st]);
                    copied = st + 1;
                } else {
                    copied = en;
                }
                src = en + 1;
            } else {
                copied = en;
                src = en;
            }
            if (anchored) break;
        }
        result.append(sv.data() + copied, slen - copied);

        CVar multi = inter::AllocMultiCVar(state, 2);
        inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaStringView(state, result));
        inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaInt(state, count));
        return multi;
    });

    RegisterNativeFunction(s, "string.dump", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar fn_var = inter::GetNativeArg(state, args, n, 0);
        if (fn_var.type_ != static_cast<int>(VarType::Closure) || !fn_var.data_.cl) {
            ThrowFakeluaException("bad argument #1 to 'string.dump' (function expected)");
        }
        VarClosure *cl = fn_var.data_.cl;
        std::string code;
        if (cl->code_str && cl->code_str != reinterpret_cast<const char *>(static_cast<uintptr_t>(1))) {
            code = cl->code_str;
        }
        std::string payload = "\x1bLua";
        payload.push_back(static_cast<char>(cl->upvalue_count));
        payload.push_back(static_cast<char>(cl->expected_arg_count));
        payload.push_back(cl->is_vararg ? 1 : 0);

        for (int i = 0; i < cl->upvalue_count; ++i) {
            if (cl->upvalues[i]) {
                CVar uv = *cl->upvalues[i];
                payload.push_back(static_cast<char>(uv.type_));
                if (uv.type_ == static_cast<int>(VarType::Int)) {
                    int64_t v = uv.data_.i;
                    payload.append(reinterpret_cast<const char *>(&v), sizeof(v));
                } else if (uv.type_ == static_cast<int>(VarType::Float)) {
                    double v = uv.data_.f;
                    payload.append(reinterpret_cast<const char *>(&v), sizeof(v));
                } else if (uv.type_ == static_cast<int>(VarType::Bool)) {
                    payload.push_back(uv.data_.b ? 1 : 0);
                } else if (uv.type_ == static_cast<int>(VarType::String) || uv.type_ == static_cast<int>(VarType::StringId)) {
                    std::string temp;
                    std::string_view sv = GetStringArgView(uv, temp);
                    uint32_t len = static_cast<uint32_t>(sv.size());
                    payload.append(reinterpret_cast<const char *>(&len), sizeof(len));
                    payload.append(sv.data(), sv.size());
                } else if (uv.type_ != static_cast<int>(VarType::Nil)) {
                    ThrowFakeluaException("string.dump: cannot dump upvalue of type " + VarTypeToString(static_cast<VarType>(uv.type_)));
                }
            } else {
                payload.push_back(static_cast<char>(VarType::Nil));
            }
        }

        payload += code;
        return inter::NativeToFakeluaStringView(state, payload);
    });

    auto load_impl = [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar code_var = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(code_var, 1, "load");
        if (code_var.type_ != static_cast<int>(VarType::String) && code_var.type_ != static_cast<int>(VarType::StringId) && code_var.type_ != static_cast<int>(VarType::Int) &&
            code_var.type_ != static_cast<int>(VarType::Float)) {
            return inter::NativeToFakeluaNil(state);
        }
        std::string temp;
        std::string_view sv = GetStringArgView(code_var, temp);
        if (sv.empty()) return inter::NativeToFakeluaNil(state);

        int upval_cnt = 0;
        int exp_arg_cnt = 0;
        bool is_varg = true;
        std::string code;
        std::vector<CVar> saved_upvalues;

        if (sv.size() >= 4 && sv.substr(0, 4) == "\x1bLua") {
            size_t idx = 4;
            if (idx + 3 <= sv.size()) {
                upval_cnt = static_cast<unsigned char>(sv[idx++]);
                exp_arg_cnt = static_cast<unsigned char>(sv[idx++]);
                is_varg = (sv[idx++] != 0);

                for (int i = 0; i < upval_cnt && idx < sv.size(); ++i) {
                    int type = static_cast<unsigned char>(sv[idx++]);
                    CVar uv{};
                    uv.type_ = type;
                    if (type == static_cast<int>(VarType::Int) && idx + sizeof(int64_t) <= sv.size()) {
                        std::memcpy(&uv.data_.i, sv.data() + idx, sizeof(int64_t));
                        idx += sizeof(int64_t);
                    } else if (type == static_cast<int>(VarType::Float) && idx + sizeof(double) <= sv.size()) {
                        std::memcpy(&uv.data_.f, sv.data() + idx, sizeof(double));
                        idx += sizeof(double);
                    } else if (type == static_cast<int>(VarType::Bool) && idx < sv.size()) {
                        uv.data_.b = (sv[idx++] != 0);
                    } else if ((type == static_cast<int>(VarType::String) || type == static_cast<int>(VarType::StringId)) && idx + sizeof(uint32_t) <= sv.size()) {
                        uint32_t len = 0;
                        std::memcpy(&len, sv.data() + idx, sizeof(len));
                        idx += sizeof(uint32_t);
                        if (len > sv.size() - idx) {
                            uv.type_ = static_cast<int>(VarType::Nil);
                        } else {
                            uv = inter::NativeToFakeluaStringView(state, std::string_view(sv.data() + idx, len));
                            idx += len;
                        }
                    } else if (type != static_cast<int>(VarType::Nil)) {
                        uv.type_ = static_cast<int>(VarType::Nil);
                    }
                    saved_upvalues.push_back(uv);
                }
            }
            code = std::string(sv.substr(idx));
        } else {
            code = std::string(sv);
        }

        try {
            CompileConfig config;
            std::string wrapper_code;
            if (!code.empty()) {
                if (code.find("function") == std::string::npos && code.find("return") == std::string::npos) {
                    wrapper_code = "return " + code;
                } else {
                    wrapper_code = code;
                }
            }

            auto &alloc = state->GetValueAllocator();
            char *saved_code = nullptr;
            if (!wrapper_code.empty()) {
                saved_code = static_cast<char *>(alloc.Alloc(wrapper_code.size() + 1));
                std::memcpy(saved_code, wrapper_code.c_str(), wrapper_code.size() + 1);
            }

            VarClosure *cl = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure) + static_cast<size_t>(upval_cnt) * sizeof(CVar *)));
            cl->func_ptr = nullptr;
            cl->upvalue_count = upval_cnt;
            cl->expected_arg_count = exp_arg_cnt;
            cl->is_vararg = is_varg;
            cl->code_str = saved_code;

            for (int i = 0; i < upval_cnt; ++i) {
                CVar *u = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
                *u = (i < static_cast<int>(saved_upvalues.size())) ? saved_upvalues[i] : CVar{static_cast<int>(VarType::Nil)};
                cl->upvalues[i] = u;
            }

            CVar res{};
            res.type_ = static_cast<int>(VarType::Closure);
            res.data_.cl = cl;
            return res;
        } catch (...) {
            return inter::NativeToFakeluaNil(state);
        }
    };

    RegisterNativeFunction(s, "load", 1, true, load_impl);
    RegisterNativeFunction(s, "loadstring", 1, true, load_impl);

    // loadfile([filename [, mode [, env]]])
    // 从文件加载 Lua 源码并编译。mode/env 参数被忽略（fakelua 无环境概念）。
    // 编译后文件中定义的顶层函数直接注册为全局函数，编译器的 __fakelua_init
    // 会自动执行文件级常量/变量初始化。成功返回 nil，失败返回 nil。
    RegisterNativeFunction(s, "loadfile", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar filename_var = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(filename_var, 1, "loadfile");
        std::string temp;
        std::string_view filename_sv = GetStringArgView(filename_var, temp);
        if (filename_sv.empty()) return inter::NativeToFakeluaNil(state);

        utf8_io::ifstream ifs(std::string(filename_sv), std::ios::in | std::ios::binary);
        if (!ifs.is_open()) return inter::NativeToFakeluaNil(state);
        std::string source((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        // 编译文件内容，顶层函数注册为全局，编译器自动执行 __fakelua_init
        try {
            CompileConfig config;
            CompileString(state, source, config);
        } catch (...) {
            return inter::NativeToFakeluaNil(state);
        }
        return inter::NativeToFakeluaNil(state);
    });

    // dofile([filename])
    // fakelua 中 dofile 等价于 loadfile：加载文件、编译、顶层函数注册为全局，
    // 编译器 __fakelua_init 自动执行文件级初始化。成功返回 nil，失败返回 nil。
    RegisterNativeFunction(s, "dofile", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar filename_var = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(filename_var, 1, "dofile");
        std::string temp;
        std::string_view filename_sv = GetStringArgView(filename_var, temp);
        if (filename_sv.empty()) return inter::NativeToFakeluaNil(state);

        utf8_io::ifstream ifs(std::string(filename_sv), std::ios::in | std::ios::binary);
        if (!ifs.is_open()) return inter::NativeToFakeluaNil(state);
        std::string source((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        try {
            CompileConfig config;
            CompileString(state, source, config);
        } catch (...) {
            return inter::NativeToFakeluaNil(state);
        }
        return inter::NativeToFakeluaNil(state);
    });

    // string.pack (Lua 5.3 binary serialization)
    // 注册的签名是 (fmt, ...) 即 arg_count=1, is_vararg=true
    // 调用时：args[0]=fmt, args[1]=Multi(剩余参数)
    RegisterNativeFunction(s, "string.pack", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar fmt_var = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(fmt_var, 1, "string.pack");
        std::string temp_fmt;
        std::string_view fmt = GetStringArgView(fmt_var, temp_fmt);
        if (fmt.empty()) return inter::NativeToFakeluaStringView(state, "");

        // 收集所有值参数：
        // - vararg 通过 args[1] 的 Multi 传入（FakeluaCallByName 的 vararg 处理）
        // - 或者当 n > 2 时，值直接作为 args[1..n-1] 传入
        std::vector<CVar> values;
        if (n >= 2) {
            CVar vararg = inter::GetNativeArg(state, args, n, 1);
            if (vararg.type_ == static_cast<int>(VarType::Multi)) {
                VarMulti *m = vararg.data_.m;
                if (m) {
                    for (uint32_t j = 0; j < m->GetCount(); ++j) {
                        values.push_back(m->GetVars()[j]);
                    }
                }
            } else {
                // 值直接作为 args[1..n-1] 传入
                for (int i = 1; i < n; ++i) {
                    values.push_back(inter::GetNativeArg(state, args, n, i));
                }
            }
        }

        PackMachine pm;
        std::string result;
        const char *fmt_p = fmt.data();
        const char *fmt_end = fmt.data() + fmt.size();
        size_t val_idx = 0;

        while (fmt_p < fmt_end) {
            char c = *fmt_p;
            if (c == ' ') {
                ++fmt_p;
                continue;
            }
            if (c == '!') {
                ++fmt_p;
                pm.align = ParsePackDecSize(fmt_p, fmt_end, 1, kMaxPackAlign, 0);
                if (pm.align <= 0) return inter::NativeToFakeluaNil(state);
                continue;
            }
            if (c == '<') {
                pm.big_endian = false;
                ++fmt_p;
                continue;
            }
            if (c == '>') {
                pm.big_endian = true;
                ++fmt_p;
                continue;
            }
            // '=' : native endianness, no alignment (x86 = little-endian)
            if (c == '=') {
                pm.big_endian = PackMachine::NativeIsBig();
                pm.align = 0;
                ++fmt_p;
                continue;
            }
            if (c == 'X') {
                ++fmt_p;
                result.push_back('\0');
                continue;
            }
            if (c == 'c') {
                ++fmt_p;
                int count = ParsePackDecSize(fmt_p, fmt_end, 1, kMaxPackBlock, 0);
                if (count <= 0) return inter::NativeToFakeluaNil(state);
                if (val_idx >= values.size()) return inter::NativeToFakeluaNil(state);
                CVar val = values[val_idx++];
                CheckStringArg(val, val_idx, "string.pack");
                std::string_view sv = KeyToStringView(val);
                size_t copy_len = sv.size() < static_cast<size_t>(count) ? sv.size() : static_cast<size_t>(count);
                result.append(sv.data(), copy_len);
                if (copy_len < static_cast<size_t>(count)) {
                    result.append(static_cast<size_t>(count) - copy_len, '\0');
                }
                continue;
            }

            // 对齐处理
            auto align_up = [&](size_t item_size) {
                if (pm.align > 0) {
                    size_t mod = result.size() % static_cast<size_t>(pm.align);
                    if (mod != 0) result.append(static_cast<size_t>(pm.align) - mod, '\0');
                }
            };

            if (c == 'i' || c == 'I') {
                bool is_unsigned = (c == 'I');
                ++fmt_p;
                int sz = ParsePackIntegralSize(fmt_p, fmt_end);
                if (val_idx >= values.size()) return inter::NativeToFakeluaNil(state);
                CVar val = values[val_idx++];
                CheckNumberArg(val, val_idx, "string.pack");
                align_up(static_cast<size_t>(sz));

                if (is_unsigned) {
                    uint64_t v = static_cast<uint64_t>(inter::CVarToInteger(val, 0));
                    if (sz < 8) v &= ((uint64_t{1} << (sz * 8)) - 1);
                    PackMachine::WriteVal(result, &v, static_cast<size_t>(sz), pm.big_endian);
                } else {
                    int64_t v = inter::CVarToInteger(val, 0);
                    uint64_t uv = static_cast<uint64_t>(v);
                    if (sz < 8) uv &= ((uint64_t{1} << (sz * 8)) - 1);
                    PackMachine::WriteVal(result, &uv, static_cast<size_t>(sz), pm.big_endian);
                }
                continue;
            }

            if (val_idx >= values.size()) return inter::NativeToFakeluaNil(state);
            CVar val = values[val_idx++];

            switch (c) {
                case 'b': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    int64_t v = inter::CVarToInteger(val, 0);
                    result.push_back(static_cast<char>(v));
                    break;
                }
                case 'B': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    int64_t v = inter::CVarToInteger(val, 0);
                    result.push_back(static_cast<char>(static_cast<unsigned char>(v)));
                    break;
                }
                case 'h': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(2);
                    int16_t sv = static_cast<int16_t>(v);
                    PackMachine::WriteVal(result, &sv, 2, pm.big_endian);
                    break;
                }
                case 'H': {
                    if (val.type_ == static_cast<int>(VarType::Bool) || val.type_ == static_cast<int>(VarType::Table)) {
                        ThrowFakeluaException("bad argument to 'pack' (number expected)");
                    }
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(2);
                    uint16_t sv = static_cast<uint16_t>(v);
                    PackMachine::WriteVal(result, &sv, 2, pm.big_endian);
                    break;
                }
                case 'l': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(4);
                    int32_t sv = static_cast<int32_t>(v);
                    PackMachine::WriteVal(result, &sv, 4, pm.big_endian);
                    break;
                }
                case 'L': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(4);
                    uint32_t sv = static_cast<uint32_t>(v);
                    PackMachine::WriteVal(result, &sv, 4, pm.big_endian);
                    break;
                }
                case 'j': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(8);
                    PackMachine::WriteVal(result, &v, 8, pm.big_endian);
                    break;
                }
                case 'J': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(8);
                    uint64_t uv = static_cast<uint64_t>(v);
                    PackMachine::WriteVal(result, &uv, 8, pm.big_endian);
                    break;
                }
                case 'T': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(8);
                    uint64_t uv = static_cast<uint64_t>(v);
                    PackMachine::WriteVal(result, &uv, 8, pm.big_endian);
                    break;
                }
                case 'f': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    double dv = inter::CVarToNumber(val, 0.0);
                    align_up(4);
                    float fv = static_cast<float>(dv);
                    PackMachine::WriteVal(result, &fv, 4, pm.big_endian);
                    break;
                }
                case 'd': {
                    CheckNumberArg(val, val_idx, "string.pack");
                    double dv = inter::CVarToNumber(val, 0.0);
                    align_up(8);
                    PackMachine::WriteVal(result, &dv, 8, pm.big_endian);
                    break;
                }
                case 'z': {
                    if (val.type_ == static_cast<int>(VarType::Nil) || val.type_ == static_cast<int>(VarType::Bool) || val.type_ == static_cast<int>(VarType::Table)) {
                        ThrowFakeluaException("bad argument to 'pack' (string expected)");
                    }
                    std::string s_val;
                    std::string_view sv = GetStringArgView(val, s_val);
                    if (!sv.empty()) {
                        result.append(sv.data(), sv.size());
                    }
                    result.push_back('\0');
                    break;
                }
                default:
                    return inter::NativeToFakeluaNil(state);
            }
            ++fmt_p;
        }
        return inter::NativeToFakeluaStringView(state, result);
    });

    // string.packsize
    // 签名: (fmt, ...) packsize 主要需要 fmt，但 z 格式需要字符串长度
    RegisterNativeFunction(s, "string.packsize", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaInt(state, 0);
        CVar fmt_var = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(fmt_var, 1, "string.packsize");
        std::string temp_fmt;
        std::string_view fmt = GetStringArgView(fmt_var, temp_fmt);
        if (fmt.empty()) return inter::NativeToFakeluaInt(state, 0);

        PackMachine pm;
        const char *fmt_p = fmt.data();
        const char *fmt_end = fmt.data() + fmt.size();
        size_t total = 0;
        size_t str_arg_idx = 0;

        // 收集所有值参数（与 pack 相同的方式）
        std::vector<CVar> values;
        if (n >= 2) {
            CVar vararg = inter::GetNativeArg(state, args, n, 1);
            if (vararg.type_ == static_cast<int>(VarType::Multi)) {
                VarMulti *m = vararg.data_.m;
                if (m) {
                    for (uint32_t j = 0; j < m->GetCount(); ++j) {
                        values.push_back(m->GetVars()[j]);
                    }
                }
            } else {
                for (int i = 1; i < n; ++i) {
                    values.push_back(inter::GetNativeArg(state, args, n, i));
                }
            }
        }

        while (fmt_p < fmt_end) {
            char c = *fmt_p;
            if (c == ' ') {
                ++fmt_p;
                continue;
            }
            if (c == '!') {
                ++fmt_p;
                pm.align = ParsePackDecSize(fmt_p, fmt_end, 1, kMaxPackAlign, 0);
                if (pm.align <= 0) return inter::NativeToFakeluaNil(state);
                continue;
            }
            if (c == '<' || c == '>' || c == '=') {
                ++fmt_p;
                continue;
            }
            if (c == 'X') {
                ++fmt_p;
                total += 1;
                continue;
            }
            if (c == 'c') {
                ++fmt_p;
                int count = ParsePackDecSize(fmt_p, fmt_end, 1, kMaxPackBlock, 0);
                if (count <= 0) return inter::NativeToFakeluaNil(state);
                total += static_cast<size_t>(count);
                ++str_arg_idx;// c[n] 消耗一个参数
                continue;
            }
            if (c == 'i' || c == 'I') {
                ++fmt_p;
                int sz = ParsePackIntegralSize(fmt_p, fmt_end);
                if (pm.align > 0) {
                    size_t mod = total % static_cast<size_t>(pm.align);
                    if (mod != 0) total += static_cast<size_t>(pm.align) - mod;
                }
                total += static_cast<size_t>(sz);
                ++str_arg_idx;// i[n]/I[n] 消耗一个参数
                continue;
            }

            // z 格式需要字符串长度
            if (c == 'z') {
                ++fmt_p;
                if (str_arg_idx < values.size()) {
                    CVar val = values[str_arg_idx];
                    if (val.type_ == static_cast<int>(VarType::Bool) || val.type_ == static_cast<int>(VarType::Table)) {
                        ThrowFakeluaException("bad argument to 'packsize' (string expected)");
                    }
                    std::string temp;
                    std::string_view sv = GetStringArgView(val, temp);
                    total += sv.size() + 1;// string + null
                    ++str_arg_idx;
                } else {
                    total += 1;// just null terminator if no string provided
                }
                continue;
            }

            size_t item_size = 0;
            switch (c) {
                case 'b':
                case 'B':
                    item_size = 1;
                    break;
                case 'h':
                case 'H':
                    item_size = 2;
                    break;
                case 'l':
                case 'L':
                    item_size = 4;
                    break;
                case 'j':
                case 'J':
                case 'T':
                    item_size = 8;
                    break;
                case 'f':
                    item_size = 4;
                    break;
                case 'd':
                    item_size = 8;
                    break;
                default:
                    return inter::NativeToFakeluaNil(state);
            }
            if (pm.align > 0) {
                size_t mod = total % static_cast<size_t>(pm.align);
                if (mod != 0) total += static_cast<size_t>(pm.align) - mod;
            }
            total += item_size;
            ++fmt_p;
        }
        return inter::NativeToFakeluaInt(state, static_cast<int64_t>(total));
    });

    // string.unpack
    RegisterNativeFunction(s, "string.unpack", 2, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar fmt_var = inter::GetNativeArg(state, args, n, 0);
        CVar str_var = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(fmt_var, 1, "string.unpack");
        CheckStringArg(str_var, 2, "string.unpack");
        std::string temp_fmt, temp_data;
        std::string_view fmt = GetStringArgView(fmt_var, temp_fmt);
        std::string_view data = GetStringArgView(str_var, temp_data);
        if (fmt.empty() || data.empty()) return inter::NativeToFakeluaNil(state);

        int64_t start_pos = 1;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            if (a2.type_ == static_cast<int>(VarType::Bool) || a2.type_ == static_cast<int>(VarType::Table)) {
                ThrowFakeluaException("bad argument #3 to 'string.unpack' (number expected)");
            }
            start_pos = inter::CVarToInteger(a2, 1);
        }
        start_pos = NormalizePos(start_pos, static_cast<int64_t>(data.size()));
        if (start_pos < 1 || start_pos > static_cast<int64_t>(data.size())) {
            return inter::NativeToFakeluaNil(state);
        }

        const unsigned char *buf = reinterpret_cast<const unsigned char *>(data.data());
        size_t init_pos = static_cast<size_t>(start_pos - 1);
        size_t pos = init_pos;// 0-based index into data
        size_t data_len = data.size();

        PackMachine pm;
        std::vector<CVar> results;

        const char *fmt_p = fmt.data();
        const char *fmt_end = fmt.data() + fmt.size();

        while (fmt_p < fmt_end) {
            char c = *fmt_p;
            if (c == ' ') {
                ++fmt_p;
                continue;
            }
            if (c == '!') {
                ++fmt_p;
                pm.align = ParsePackDecSize(fmt_p, fmt_end, 1, kMaxPackAlign, 0);
                if (pm.align <= 0) return inter::NativeToFakeluaNil(state);
                continue;
            }
            if (c == '<') {
                pm.big_endian = false;
                ++fmt_p;
                continue;
            }
            if (c == '>') {
                pm.big_endian = true;
                ++fmt_p;
                continue;
            }
            if (c == '=') {
                pm.big_endian = PackMachine::NativeIsBig();
                ++fmt_p;
                continue;
            }
            if (c == 'X') {
                ++fmt_p;
                pos += 1;
                continue;
            }

            auto align_up = [&]() {
                if (pm.align > 0) {
                    size_t rel = pos - init_pos;
                    size_t mod = rel % static_cast<size_t>(pm.align);
                    if (mod != 0) pos += static_cast<size_t>(pm.align) - mod;
                }
            };

            auto check_available = [&](size_t need) -> bool { return pos + need <= data_len; };

            switch (c) {
                case 'b': {// signed char
                    if (!check_available(1)) return inter::NativeToFakeluaNil(state);
                    int8_t v;
                    PackMachine::ReadVal(buf + pos, &v, 1, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaInt(state, static_cast<int64_t>(v)));
                    pos += 1;
                    break;
                }
                case 'B': {// unsigned char
                    if (!check_available(1)) return inter::NativeToFakeluaNil(state);
                    uint8_t v;
                    PackMachine::ReadVal(buf + pos, &v, 1, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaInt(state, static_cast<int64_t>(v)));
                    pos += 1;
                    break;
                }
                case 'h': {// signed short
                    align_up();
                    if (!check_available(2)) return inter::NativeToFakeluaNil(state);
                    int16_t v;
                    PackMachine::ReadVal(buf + pos, &v, 2, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaInt(state, static_cast<int64_t>(v)));
                    pos += 2;
                    break;
                }
                case 'H': {// unsigned short
                    align_up();
                    if (!check_available(2)) return inter::NativeToFakeluaNil(state);
                    uint16_t v;
                    PackMachine::ReadVal(buf + pos, &v, 2, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaInt(state, static_cast<int64_t>(v)));
                    pos += 2;
                    break;
                }
                case 'l': {// signed long (4 bytes)
                    align_up();
                    if (!check_available(4)) return inter::NativeToFakeluaNil(state);
                    int32_t v;
                    PackMachine::ReadVal(buf + pos, &v, 4, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaInt(state, static_cast<int64_t>(v)));
                    pos += 4;
                    break;
                }
                case 'L': {// unsigned long (4 bytes)
                    align_up();
                    if (!check_available(4)) return inter::NativeToFakeluaNil(state);
                    uint32_t v;
                    PackMachine::ReadVal(buf + pos, &v, 4, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaInt(state, static_cast<int64_t>(v)));
                    pos += 4;
                    break;
                }
                case 'j': {// lua_integer (int64)
                    align_up();
                    if (!check_available(8)) return inter::NativeToFakeluaNil(state);
                    int64_t v;
                    PackMachine::ReadVal(buf + pos, &v, 8, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaInt(state, v));
                    pos += 8;
                    break;
                }
                case 'J': {// lua_unsigned (uint64)
                    align_up();
                    if (!check_available(8)) return inter::NativeToFakeluaNil(state);
                    uint64_t v;
                    PackMachine::ReadVal(buf + pos, &v, 8, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaInt(state, static_cast<int64_t>(v)));
                    pos += 8;
                    break;
                }
                case 'T': {// size_t (8 bytes)
                    align_up();
                    if (!check_available(8)) return inter::NativeToFakeluaNil(state);
                    uint64_t v;
                    PackMachine::ReadVal(buf + pos, &v, 8, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaInt(state, static_cast<int64_t>(v)));
                    pos += 8;
                    break;
                }
                case 'f': {// float (4 bytes)
                    align_up();
                    if (!check_available(4)) return inter::NativeToFakeluaNil(state);
                    float v;
                    PackMachine::ReadVal(buf + pos, &v, 4, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaFloat(state, static_cast<double>(v)));
                    pos += 4;
                    break;
                }
                case 'd': {// double (8 bytes)
                    align_up();
                    if (!check_available(8)) return inter::NativeToFakeluaNil(state);
                    double v;
                    PackMachine::ReadVal(buf + pos, &v, 8, pm.big_endian);
                    results.push_back(inter::NativeToFakeluaFloat(state, v));
                    pos += 8;
                    break;
                }
                case 'z': {// zero-terminated string
                    if (pos >= data_len) return inter::NativeToFakeluaNil(state);
                    size_t end = pos;
                    while (end < data_len && buf[end] != '\0') ++end;
                    size_t str_len = end - pos;
                    results.push_back(inter::NativeToFakeluaStringView(state, std::string_view(data.data() + pos, str_len)));
                    pos = end + 1;// skip the null terminator
                    break;
                }
                case 'c': {// fixed-length string (no value consumed from args in unpack)
                    ++fmt_p;
                    int count = ParsePackDecSize(fmt_p, fmt_end, 1, kMaxPackBlock, 0);
                    if (count <= 0) return inter::NativeToFakeluaNil(state);
                    if (!check_available(static_cast<size_t>(count))) return inter::NativeToFakeluaNil(state);
                    results.push_back(inter::NativeToFakeluaStringView(state, std::string_view(data.data() + pos, static_cast<size_t>(count))));
                    pos += static_cast<size_t>(count);
                    continue;// already advanced fmt_p
                }
                case 'i':
                case 'I': {// sized integer
                    bool is_unsigned = (c == 'I');
                    ++fmt_p;
                    int sz = ParsePackIntegralSize(fmt_p, fmt_end);
                    align_up();
                    if (!check_available(static_cast<size_t>(sz))) return inter::NativeToFakeluaNil(state);
                    uint64_t uv = 0;
                    PackMachine::ReadVal(buf + pos, &uv, static_cast<size_t>(sz), pm.big_endian);
                    if (is_unsigned) {
                        results.push_back(inter::NativeToFakeluaInt(state, static_cast<int64_t>(uv)));
                    } else {
                        // Sign-extend
                        int64_t sv;
                        if (sz >= 8) {
                            sv = static_cast<int64_t>(uv);
                        } else {
                            uint64_t sign_bit = uint64_t{1} << (sz * 8 - 1);
                            if (uv & sign_bit) {
                                sv = static_cast<int64_t>(uv | (~uint64_t{0} << (sz * 8)));
                            } else {
                                sv = static_cast<int64_t>(uv);
                            }
                        }
                        results.push_back(inter::NativeToFakeluaInt(state, sv));
                    }
                    pos += static_cast<size_t>(sz);
                    continue;// already advanced fmt_p
                }
                default:
                    return inter::NativeToFakeluaNil(state);
            }
            ++fmt_p;
        }

        // Return all unpacked values plus the position after the last read (1-based)
        int count = static_cast<int>(results.size());
        CVar multi = inter::AllocMultiCVar(state, count + 1);
        for (int i = 0; i < count; ++i) {
            inter::SetMultiCVarElement(multi, i, results[static_cast<size_t>(i)]);
        }
        inter::SetMultiCVarElement(multi, count, inter::NativeToFakeluaInt(state, static_cast<int64_t>(pos + 1)));
        return multi;
    });
}

// load() 生成函数名用的发号器，每 State 一份
struct EvalCounter {
    uint64_t next = 0;
};

extern "C" CVar FlEvalLoadClosure(State *state, VarClosure *cl, int arg_num, const CVar *args) {
    if (!state || !cl || !cl->code_str) {
        return inter::NativeToFakeluaNil(state);
    }
    std::string code = cl->code_str;

    if (code.size() >= 4 && code.substr(0, 4) == "\x1bLua") {
        code = (code.size() >= 5) ? code.substr(5) : code.substr(4);
    }

    // 每 State 一份：生成的函数名只需在本 State 的函数表里唯一（下面的 CompileString 和
    // FakeluaCallByName 都是按 State 查的），所以不需要一个全进程共享的发号器。
    auto &eval_counter = state->GetModuleState<EvalCounter>();
    std::string eval_fn_name = "__flua_eval_ld_" + std::to_string(++eval_counter.next);

    std::string upval_decls;
    for (int i = 0; i < cl->upvalue_count; ++i) {
        if (cl->upvalues[i]) {
            CVar uv = *cl->upvalues[i];
            if (uv.type_ == static_cast<int>(VarType::Int)) {
                upval_decls += "local x = " + std::to_string(uv.data_.i) + "\n";
            } else if (uv.type_ == static_cast<int>(VarType::Float)) {
                upval_decls += "local x = " + std::to_string(uv.data_.f) + "\n";
            } else if (uv.type_ == static_cast<int>(VarType::Bool)) {
                upval_decls += "local x = " + std::string(uv.data_.b ? "true\n" : "false\n");
            }
        }
    }

    std::string full_code;
    if (code.find("function") == std::string::npos && code.find("return") == std::string::npos) {
        full_code = upval_decls + "function " + eval_fn_name + "()\nreturn " + code + "\nend";
    } else {
        full_code = upval_decls + "function " + eval_fn_name + "()\n" + code + "\nend";
    }

    try {
        CompileConfig config;
        CompileString(state, full_code, config);
        // 边界不能省：本函数要靠下面的 catch 把错误吞成 nil，若让错误直接跳到更外层的
        // 边界，这里的 std::string 就不会析构，语义也从"返回 nil"变成了向上抛。
        CVar res = RunWithJitErrorBoundary(state, [&] { return FakeluaCallByName(state, JIT_TCC, eval_fn_name.c_str(), 0); });
        return res;
    } catch (...) {
        return inter::NativeToFakeluaNil(state);
    }
}

}// namespace fakelua::string
