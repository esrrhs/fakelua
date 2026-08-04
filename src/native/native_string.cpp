#include "native/native_string.h"
#include "compile/c_runtime_header.h"
#include "native/native_object.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_multi.h"
#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fakelua {

static int64_t NormalizePos(int64_t pos, int64_t len) {
    if (pos >= 0) {
        return pos;
    }
    return len + pos + 1;
}

// ─── gmatch 迭代器状态（存储在闭包 upvalue 中） ───
struct GMatchState {
    std::string text;
    std::string pattern;
    size_t pos = 0;
};

// ─── string.pack / packsize / unpack 二进制序列化辅助 ───
struct PackMachine {
    bool big_endian = false;
    int align = 0;// 0 = no alignment

    static void WriteVal(std::string &out, const void *data, size_t n, bool big) {
        const auto *bytes = static_cast<const unsigned char *>(data);
        if (big) {
            for (size_t i = 0; i < n; ++i) out.push_back(static_cast<char>(bytes[n - 1 - i]));
        } else {
            for (size_t i = 0; i < n; ++i) out.push_back(static_cast<char>(bytes[i]));
        }
    }

    static void ReadVal(const unsigned char *src, void *dst, size_t n, bool big) {
        auto *d = static_cast<unsigned char *>(dst);
        if (big) {
            for (size_t i = 0; i < n; ++i) d[i] = src[n - 1 - i];
        } else {
            std::memcpy(d, src, n);
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
            align = 0;
            while (fmt < end && *fmt >= '0' && *fmt <= '9') {
                align = align * 10 + (*fmt - '0');
                ++fmt;
            }
            if (align <= 0) return -1;
            continue;
        }

        // Handle endianness
        if (c == '<') {
            big_endian = false;
            ++fmt;
            continue;
        }
        if (c == '>' || c == '=') {
            big_endian = true;
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
            int count = 0;
            while (fmt < end && *fmt >= '0' && *fmt <= '9') {
                count = count * 10 + (*fmt - '0');
                ++fmt;
            }
            if (count <= 0) return -1;
            if (arg_idx >= total_args) return -1;
            CVar val = inter::GetNativeArg(state, args, total_args, arg_idx);
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
            int sz = 0;
            while (fmt < end && *fmt >= '0' && *fmt <= '9') {
                sz = sz * 10 + (*fmt - '0');
                ++fmt;
            }
            if (sz <= 0) return -1;
            if (arg_idx >= total_args) return -1;
            CVar val = inter::GetNativeArg(state, args, total_args, arg_idx);
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
            align = 0;
            while (fmt < end && *fmt >= '0' && *fmt <= '9') {
                align = align * 10 + (*fmt - '0');
                ++fmt;
            }
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
            int count = 0;
            while (fmt < end && *fmt >= '0' && *fmt <= '9') {
                count = count * 10 + (*fmt - '0');
                ++fmt;
            }
            if (count <= 0) return -1;
            if (arg_idx >= total_args) return -1;
            ++arg_idx;
            total += static_cast<size_t>(count);
            continue;
        }
        if (c == 'i' || c == 'I') {
            ++fmt;
            int sz = 0;
            while (fmt < end && *fmt >= '0' && *fmt <= '9') {
                sz = sz * 10 + (*fmt - '0');
                ++fmt;
            }
            if (sz <= 0) return -1;
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

// ─── gmatch 迭代器原生函数 ───
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

    if (gs->pos >= gs->text.size()) {
        return inter::NativeToFakeluaNil(iter_state);
    }

    try {
        std::regex re(gs->pattern, std::regex::ECMAScript);
        std::smatch match;
        std::string sub = gs->text.substr(gs->pos);
        if (!std::regex_search(sub, match, re)) {
            gs->pos = gs->text.size();
            return inter::NativeToFakeluaNil(iter_state);
        }

        gs->pos += match.position() + match.length();
        if (match.length() == 0) {
            // 零宽匹配：前进一位避免死循环
            gs->pos += 1;
        }

        if (match.size() > 1) {
            int groups = static_cast<int>(match.size()) - 1;
            CVar multi = inter::AllocMultiCVar(iter_state, groups);
            for (int i = 0; i < groups; ++i) {
                inter::SetMultiCVarElement(multi, i, inter::NativeToFakeluaStringView(iter_state, match[i + 1].str()));
            }
            return multi;
        }
        return inter::NativeToFakeluaStringView(iter_state, match[0].str());
    } catch (const std::regex_error &) {
        return inter::NativeToFakeluaNil(iter_state);
    }
}

void RegisterStringLibraryApi(State *s) {
    if (!s) return;

    auto get_str_arg = [](CVar a) -> std::string {
        if (a.type_ == static_cast<int>(VarType::String) || a.type_ == static_cast<int>(VarType::StringId)) {
            return std::string(KeyToStringView(a));
        } else if (a.type_ != static_cast<int>(VarType::Nil)) {
            return AsVar(a).ToString(/*has_quote=*/false, /*has_postfix=*/false);
        }
        return "";
    };

    RegisterNativeFunction(s, "string.len", 1, false, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaInt(state, 0);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string s_str = get_str_arg(a0);
        return inter::NativeToFakeluaInt(state, static_cast<int64_t>(s_str.size()));
    });

    RegisterNativeFunction(s, "string.sub", 2, true, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string s_str = get_str_arg(a0);
        std::string_view sv = s_str;
        int64_t len = static_cast<int64_t>(sv.size());

        int64_t start_pos = inter::CVarToInteger(inter::GetNativeArg(state, args, n, 1), 1);
        int64_t end_pos = len;
        if (n >= 3) {
            end_pos = inter::CVarToInteger(inter::GetNativeArg(state, args, n, 2), len);
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

    RegisterNativeFunction(s, "string.rep", 2, true, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        int64_t rep_cnt = inter::CVarToInteger(inter::GetNativeArg(state, args, n, 1), 0);
        if (rep_cnt <= 0) return inter::NativeToFakeluaStringView(state, "");

        std::string sep = "";
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            sep = get_str_arg(a2);
        }

        std::string s_str = get_str_arg(a0);
        std::string_view sv = s_str;
        std::string res;
        res.reserve((sv.size() + sep.size()) * static_cast<size_t>(rep_cnt));
        for (int64_t i = 0; i < rep_cnt; ++i) {
            if (i > 0 && !sep.empty()) res += sep;
            res += sv;
        }
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.reverse", 1, false, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string res = get_str_arg(a0);
        std::reverse(res.begin(), res.end());
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.lower", 1, false, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string res = get_str_arg(a0);
        std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.upper", 1, false, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string res = get_str_arg(a0);
        std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.byte", 1, true, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::AllocMultiCVar(state, 0);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string s_str = get_str_arg(a0);
        std::string_view sv = s_str;
        int64_t len = static_cast<int64_t>(sv.size());
        if (len == 0) return inter::AllocMultiCVar(state, 0);

        int64_t start_pos = 1;
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            start_pos = inter::CVarToInteger(a1, 1);
        }

        int64_t end_pos = start_pos;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            end_pos = inter::CVarToInteger(a2, start_pos);
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
        std::string res;
        res.reserve(static_cast<size_t>(n));
        for (int i = 0; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            int64_t c = inter::CVarToInteger(arg_i, -1);
            if (c < 0 || c > 255) {
                return inter::NativeToFakeluaNil(state);
            }
            res.push_back(static_cast<char>(c));
        }
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.format", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar fmt_var = inter::GetNativeArg(state, args, n, 0);
        std::string_view fmt = KeyToStringView(fmt_var);

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

            CVar curr_arg = (arg_idx < n) ? inter::GetNativeArg(state, args, n, arg_idx++) : CVar{static_cast<int>(VarType::Nil)};

            if (spec == 'q') {
                std::string_view sval = KeyToStringView(curr_arg);
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
                std::string sval;
                if (curr_arg.type_ == static_cast<int>(VarType::String) || curr_arg.type_ == static_cast<int>(VarType::StringId)) {
                    sval = std::string(KeyToStringView(curr_arg));
                } else if (curr_arg.type_ == static_cast<int>(VarType::Int)) {
                    sval = std::to_string(curr_arg.data_.i);
                } else if (curr_arg.type_ == static_cast<int>(VarType::Float)) {
                    sval = std::to_string(curr_arg.data_.f);
                } else if (curr_arg.type_ == static_cast<int>(VarType::Bool)) {
                    sval = curr_arg.data_.b ? "true" : "false";
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
                int64_t ival = inter::CVarToInteger(curr_arg, 0);
                std::string llspec = spec_str;
                llspec.insert(llspec.size() - 1, "ll");
                char buf[128];
                snprintf(buf, sizeof(buf), llspec.c_str(), ival);
                res.append(buf);
            } else if (spec == 'u' || spec == 'x' || spec == 'X' || spec == 'o') {
                uint64_t uval = static_cast<uint64_t>(inter::CVarToInteger(curr_arg, 0));
                std::string llspec = spec_str;
                llspec.insert(llspec.size() - 1, "ll");
                char buf[128];
                snprintf(buf, sizeof(buf), llspec.c_str(), uval);
                res.append(buf);
            } else if (spec == 'f' || spec == 'e' || spec == 'E' || spec == 'g' || spec == 'G') {
                double fval = inter::CVarToNumber(curr_arg, 0.0);
                char buf[128];
                snprintf(buf, sizeof(buf), spec_str.c_str(), fval);
                res.append(buf);
            } else if (spec == 'c') {
                int64_t cval = inter::CVarToInteger(curr_arg, 0);
                res.push_back(static_cast<char>(cval));
            } else if (spec == 'p') {
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
                    auto val = static_cast<uintptr_t>(curr_arg.data_.f);
                    if (val == 0) {
                        res.append("0x0");
                    } else {
                        std::snprintf(buf, sizeof(buf), "0x%" PRIxPTR, val);
                        res.append(buf);
                    }
                } else {
                    // 非数值类型：输出 CVar 自身地址
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

    // ─── string.find(s, pattern [, init [, plain]]) ───
    // 在 s 中查找 pattern（ECMAScript 正则），返回起始位置与结束位置（1-based）。
    // 若 pattern 含捕获组，则后续返回值依次为各捕获。
    // 若 plain 为 true，则退化为纯子串查找（忽略正则元字符）。
    // 找不到时返回 nil。
    RegisterNativeFunction(s, "string.find", 2, true, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        std::string s_str = get_str_arg(a0);
        std::string pat_str = get_str_arg(a1);
        std::string_view sv = s_str;
        std::string_view pat_view = pat_str;
        int64_t len = static_cast<int64_t>(sv.size());

        int64_t init_pos = 1;
        if (n >= 3) {
            init_pos = inter::CVarToInteger(inter::GetNativeArg(state, args, n, 2), 1);
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

        try {
            std::regex re(std::string(pat_view), std::regex::ECMAScript);
            std::smatch match;
            if (!std::regex_search(sub, match, re)) return inter::NativeToFakeluaNil(state);

            int64_t start = init_pos + static_cast<int64_t>(match.position());
            int64_t end = start + static_cast<int64_t>(match.length()) - 1;
            int captures = static_cast<int>(match.size()) - 1;// 捕获组数
            int total = 2 + captures;                         // start, end, + 捕获
            CVar multi = inter::AllocMultiCVar(state, total);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaInt(state, start));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaInt(state, end));
            for (int i = 0; i < captures; ++i) {
                inter::SetMultiCVarElement(multi, i + 2, inter::NativeToFakeluaStringView(state, match[i + 1].str()));
            }
            return multi;
        } catch (const std::regex_error &) {
            return inter::NativeToFakeluaNil(state);
        }
    });

    // ─── string.match(s, pattern [, init]) ───
    // 与 string.find 相似，但不返回位置；仅返回捕获（或整个匹配，若无捕获组）。
    RegisterNativeFunction(s, "string.match", 2, true, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        std::string s_str = get_str_arg(a0);
        std::string pat_str = get_str_arg(a1);
        std::string_view sv = s_str;
        std::string_view pat_view = pat_str;
        int64_t len = static_cast<int64_t>(sv.size());

        int64_t init_pos = 1;
        if (n >= 3) {
            init_pos = inter::CVarToInteger(inter::GetNativeArg(state, args, n, 2), 1);
        }
        init_pos = NormalizePos(init_pos, len);
        if (init_pos < 1) init_pos = 1;
        if (init_pos > len + 1) {
            return inter::NativeToFakeluaNil(state);
        }
        std::string sub = std::string(sv.substr(static_cast<size_t>(init_pos - 1)));

        try {
            std::regex re(std::string(pat_view), std::regex::ECMAScript);
            std::smatch match;
            if (!std::regex_search(sub, match, re)) return inter::NativeToFakeluaNil(state);

            if (match.size() > 1) {
                // 有捕获组：返回所有捕获
                int groups = static_cast<int>(match.size()) - 1;
                CVar multi = inter::AllocMultiCVar(state, groups);
                for (int i = 0; i < groups; ++i) {
                    inter::SetMultiCVarElement(multi, i, inter::NativeToFakeluaStringView(state, match[i + 1].str()));
                }
                return multi;
            }
            // 无捕获组：返回整个匹配
            return inter::NativeToFakeluaStringView(state, match[0].str());
        } catch (const std::regex_error &) {
            return inter::NativeToFakeluaNil(state);
        }
    });

    // ─── string.gmatch(s, pattern) ───
    // 返回一个迭代器闭包；每次调用返回下一个匹配（或捕获）。
    RegisterNativeFunction(s, "string.gmatch", 2, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        std::string text(KeyToStringView(a0));
        std::string pattern(KeyToStringView(a1));

        // 使用 arena 分配器分配迭代器状态
        auto &alloc = state->GetHeap().GetAllocator(false);
        GMatchState *gs = new (alloc.Alloc(sizeof(GMatchState))) GMatchState{std::move(text), std::move(pattern), 0};

        // upvalue 0: State*
        CVar *uv0 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv0->type_ = static_cast<int>(VarType::Int);
        uv0->flag_ = 0;
        uv0->data_.i = reinterpret_cast<int64_t>(state);

        // upvalue 1: GMatchState*
        CVar *uv1 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv1->type_ = static_cast<int>(VarType::Int);
        uv1->flag_ = 0;
        uv1->data_.i = reinterpret_cast<int64_t>(gs);

        // 分配闭包
        VarClosure *cl = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure) + 2 * sizeof(CVar *)));
        cl->func_ptr = reinterpret_cast<void *>(GMatchIterator);
        cl->upvalue_count = 2;
        cl->expected_arg_count = 2;
        cl->is_vararg = false;
        cl->code_str = nullptr;
        cl->upvalues[0] = uv0;
        cl->upvalues[1] = uv1;

        CVar res{};
        res.type_ = static_cast<int>(VarType::Closure);
        res.flag_ = 0;
        res.data_.cl = cl;
        return res;
    });

    // ─── string.gsub(s, pattern, repl [, n]) ───
    RegisterNativeFunction(s, "string.gsub", 3, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 3) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CVar repl_var = inter::GetNativeArg(state, args, n, 2);
        std::string_view sv = KeyToStringView(a0);
        std::string_view pat_view = KeyToStringView(a1);

        int64_t max_replace = -1;
        if (n >= 4) {
            CVar a3 = inter::GetNativeArg(state, args, n, 3);
            max_replace = inter::CVarToInteger(a3, -1);
        }

        bool repl_is_table = (repl_var.type_ == static_cast<int>(VarType::Table) && repl_var.data_.t);
        bool repl_is_closure = (repl_var.type_ == static_cast<int>(VarType::Closure) && repl_var.data_.cl);

        try {
            std::regex re(std::string(pat_view), std::regex::ECMAScript);
            std::string input(sv);
            std::string result;
            result.reserve(input.size());
            int64_t count = 0;

            auto it = std::sregex_iterator(input.begin(), input.end(), re);
            auto end = std::sregex_iterator();
            size_t last_pos = 0;

            for (; it != end; ++it) {
                if (max_replace >= 0 && count >= max_replace) break;
                const std::smatch &match = *it;
                result.append(input, last_pos, match.position() - last_pos);

                std::string replacement;
                if (repl_is_closure) {
                    VarClosure *cl = repl_var.data_.cl;
                    void *addr = cl->func_ptr;
                    if (match.size() > 1) {
                        int call_arg_count = std::min(static_cast<int>(match.size()) - 1, 16);
                        CVar call_args[16];
                        for (int i = 0; i < call_arg_count; ++i) {
                            call_args[i] = inter::NativeToFakeluaStringView(state, match[i + 1].str());
                        }
                        CVar fn_res = (addr != nullptr) ? inter::DispatchCall(addr, call_args, call_arg_count) : FlEvalLoadClosure(state, cl, call_arg_count, call_args);
                        replacement = std::string(KeyToStringView(fn_res));
                    } else {
                        CVar call_arg = inter::NativeToFakeluaStringView(state, match[0].str());
                        CVar fn_res = (addr != nullptr) ? inter::DispatchCall(addr, &call_arg, 1) : FlEvalLoadClosure(state, cl, 1, &call_arg);
                        replacement = std::string(KeyToStringView(fn_res));
                    }
                } else if (repl_is_table) {
                    std::string key = (match.size() > 1) ? match[1].str() : match[0].str();
                    CVar val{static_cast<int>(VarType::Nil)};

                    // 尝试 spec 快速路径（仅当表是 NativeObject 包装时）
                    VarTable *tbl = repl_var.data_.t;
                    if (tbl->spec_get) {
                        using SpecGetFn = CVar (*)(VarTable *, CVar, bool *);
                        auto get_fn = reinterpret_cast<SpecGetFn>(tbl->spec_get);
                        bool finish = false;
                        val = get_fn(tbl, inter::NativeToFakeluaStringView(state, key), &finish);
                        if (!finish) val = CVar{static_cast<int>(VarType::Nil)};
                    }

                    // 回退到 quick_data 线性查找
                    if (val.type_ == static_cast<int>(VarType::Nil)) {
                        for (const auto &qd: tbl->quick_data_) {
                            auto sv = KeyToStringView(qd.key);
                            if (sv == key) {
                                val = qd.val;
                                break;
                            }
                        }
                    }

                    if (val.type_ == static_cast<int>(VarType::Nil)) {
                        replacement = match[0].str();
                    } else {
                        replacement = std::string(KeyToStringView(val));
                    }
                } else {
                    // 字符串替换：支持 $1 $2 ... $& $` $' $$
                    std::string repl_str(KeyToStringView(repl_var));
                    replacement.clear();
                    for (size_t i = 0; i < repl_str.size(); ++i) {
                        if (repl_str[i] == '$' && i + 1 < repl_str.size()) {
                            char next = repl_str[i + 1];
                            if (next == '$') {
                                replacement.push_back('$');
                                i++;
                            } else if (next == '&') {
                                replacement += match[0].str();
                                i++;
                            } else if (next == '`') {
                                replacement += match.prefix().str();
                                i++;
                            } else if (next == '\'') {
                                replacement += match.suffix().str();
                                i++;
                            } else if (next >= '1' && next <= '9') {
                                int idx = next - '1' + 1;
                                if (idx < static_cast<int>(match.size())) {
                                    replacement += match[idx].str();
                                }
                                i++;
                            } else {
                                replacement.push_back(repl_str[i]);
                            }
                        } else {
                            replacement.push_back(repl_str[i]);
                        }
                    }
                }

                result += replacement;
                last_pos = match.position() + match.length();
                count++;
            }
            result.append(input, last_pos, std::string::npos);

            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaStringView(state, result));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaInt(state, count));
            return multi;
        } catch (const std::regex_error &) {
            return inter::NativeToFakeluaNil(state);
        }
    });

    RegisterNativeFunction(s, "string.dump", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar fn_var = inter::GetNativeArg(state, args, n, 0);
        if (fn_var.type_ != static_cast<int>(VarType::Closure) || !fn_var.data_.cl) {
            return inter::NativeToFakeluaNil(state);
        }
        VarClosure *cl = fn_var.data_.cl;
        std::string code = cl->code_str ? std::string(cl->code_str) : "";
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
                }
            } else {
                payload.push_back(static_cast<char>(VarType::Nil));
            }
        }

        payload += code;
        return inter::NativeToFakeluaStringView(state, payload);
    });

    auto load_impl = [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar code_var = inter::GetNativeArg(state, args, n, 0);
        std::string code_arg = get_str_arg(code_var);
        std::string_view sv = code_arg;
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

            auto &alloc = state->GetHeap().GetAllocator(false);
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

    // ─── loadfile([filename [, mode [, env]]]) ───
    // 从文件加载 Lua 源码并编译。mode/env 参数被忽略（fakelua 无环境概念）。
    // 编译后文件中定义的顶层函数直接注册为全局函数，编译器的 __fakelua_init
    // 会自动执行文件级常量/变量初始化。成功返回 nil，失败返回 nil。
    RegisterNativeFunction(s, "loadfile", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar filename_var = inter::GetNativeArg(state, args, n, 0);
        std::string_view filename_sv = KeyToStringView(filename_var);
        if (filename_sv.empty()) return inter::NativeToFakeluaNil(state);

        // 读取文件内容
        std::ifstream ifs(std::string(filename_sv), std::ios::in | std::ios::binary);
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

    // ─── dofile([filename]) ───
    // fakelua 中 dofile 等价于 loadfile：加载文件、编译、顶层函数注册为全局，
    // 编译器 __fakelua_init 自动执行文件级初始化。成功返回 nil，失败返回 nil。
    RegisterNativeFunction(s, "dofile", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar filename_var = inter::GetNativeArg(state, args, n, 0);
        std::string_view filename_sv = KeyToStringView(filename_var);
        if (filename_sv.empty()) return inter::NativeToFakeluaNil(state);

        std::ifstream ifs(std::string(filename_sv), std::ios::in | std::ios::binary);
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

    // ─── string.pack (Lua 5.3 binary serialization) ───
    // 注册的签名是 (fmt, ...) 即 arg_count=1, is_vararg=true
    // 调用时：args[0]=fmt, args[1]=Multi(剩余参数)
    RegisterNativeFunction(s, "string.pack", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar fmt_var = inter::GetNativeArg(state, args, n, 0);
        std::string_view fmt = KeyToStringView(fmt_var);
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
                pm.align = 0;
                while (fmt_p < fmt_end && *fmt_p >= '0' && *fmt_p <= '9') {
                    pm.align = pm.align * 10 + (*fmt_p - '0');
                    ++fmt_p;
                }
                if (pm.align <= 0) return inter::NativeToFakeluaNil(state);
                continue;
            }
            if (c == '<') {
                pm.big_endian = false;
                ++fmt_p;
                continue;
            }
            if (c == '>' || c == '=') {
                pm.big_endian = true;
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
                int count = 0;
                while (fmt_p < fmt_end && *fmt_p >= '0' && *fmt_p <= '9') {
                    count = count * 10 + (*fmt_p - '0');
                    ++fmt_p;
                }
                if (count <= 0) return inter::NativeToFakeluaNil(state);
                if (val_idx >= values.size()) return inter::NativeToFakeluaNil(state);
                CVar val = values[val_idx++];
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
                int sz = 0;
                while (fmt_p < fmt_end && *fmt_p >= '0' && *fmt_p <= '9') {
                    sz = sz * 10 + (*fmt_p - '0');
                    ++fmt_p;
                }
                if (sz <= 0) return inter::NativeToFakeluaNil(state);
                if (val_idx >= values.size()) return inter::NativeToFakeluaNil(state);
                CVar val = values[val_idx++];
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
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(1);
                    int8_t sv = static_cast<int8_t>(v);
                    PackMachine::WriteVal(result, &sv, 1, pm.big_endian);
                    break;
                }
                case 'B': {
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(1);
                    uint8_t sv = static_cast<uint8_t>(v);
                    PackMachine::WriteVal(result, &sv, 1, pm.big_endian);
                    break;
                }
                case 'h': {
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(2);
                    int16_t sv = static_cast<int16_t>(v);
                    PackMachine::WriteVal(result, &sv, 2, pm.big_endian);
                    break;
                }
                case 'H': {
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(2);
                    uint16_t sv = static_cast<uint16_t>(v);
                    PackMachine::WriteVal(result, &sv, 2, pm.big_endian);
                    break;
                }
                case 'l': {
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(4);
                    int32_t sv = static_cast<int32_t>(v);
                    PackMachine::WriteVal(result, &sv, 4, pm.big_endian);
                    break;
                }
                case 'L': {
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(4);
                    uint32_t sv = static_cast<uint32_t>(v);
                    PackMachine::WriteVal(result, &sv, 4, pm.big_endian);
                    break;
                }
                case 'j': {
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(8);
                    PackMachine::WriteVal(result, &v, 8, pm.big_endian);
                    break;
                }
                case 'J': {
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(8);
                    uint64_t uv = static_cast<uint64_t>(v);
                    PackMachine::WriteVal(result, &uv, 8, pm.big_endian);
                    break;
                }
                case 'T': {
                    int64_t v = inter::CVarToInteger(val, 0);
                    align_up(8);
                    uint64_t uv = static_cast<uint64_t>(v);
                    PackMachine::WriteVal(result, &uv, 8, pm.big_endian);
                    break;
                }
                case 'f': {
                    double dv = inter::CVarToNumber(val, 0.0);
                    align_up(4);
                    float fv = static_cast<float>(dv);
                    PackMachine::WriteVal(result, &fv, 4, pm.big_endian);
                    break;
                }
                case 'd': {
                    double dv = inter::CVarToNumber(val, 0.0);
                    align_up(8);
                    PackMachine::WriteVal(result, &dv, 8, pm.big_endian);
                    break;
                }
                case 'z': {
                    std::string_view sv = KeyToStringView(val);
                    result.append(sv.data(), sv.size());
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

    // ─── string.packsize ───
    // 签名: (fmt, ...) packsize 主要需要 fmt，但 z 格式需要字符串长度
    RegisterNativeFunction(s, "string.packsize", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaInt(state, 0);
        CVar fmt_var = inter::GetNativeArg(state, args, n, 0);
        std::string_view fmt = KeyToStringView(fmt_var);
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
                pm.align = 0;
                while (fmt_p < fmt_end && *fmt_p >= '0' && *fmt_p <= '9') {
                    pm.align = pm.align * 10 + (*fmt_p - '0');
                    ++fmt_p;
                }
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
                int count = 0;
                while (fmt_p < fmt_end && *fmt_p >= '0' && *fmt_p <= '9') {
                    count = count * 10 + (*fmt_p - '0');
                    ++fmt_p;
                }
                if (count <= 0) return inter::NativeToFakeluaNil(state);
                total += static_cast<size_t>(count);
                continue;
            }
            if (c == 'i' || c == 'I') {
                ++fmt_p;
                int sz = 0;
                while (fmt_p < fmt_end && *fmt_p >= '0' && *fmt_p <= '9') {
                    sz = sz * 10 + (*fmt_p - '0');
                    ++fmt_p;
                }
                if (sz <= 0) return inter::NativeToFakeluaNil(state);
                if (pm.align > 0) {
                    size_t mod = total % static_cast<size_t>(pm.align);
                    if (mod != 0) total += static_cast<size_t>(pm.align) - mod;
                }
                total += static_cast<size_t>(sz);
                continue;
            }

            // z 格式需要字符串长度
            if (c == 'z') {
                ++fmt_p;
                if (str_arg_idx < values.size()) {
                    std::string_view sv = KeyToStringView(values[str_arg_idx]);
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

    // ─── string.unpack ───
    RegisterNativeFunction(s, "string.unpack", 2, true, [get_str_arg](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar fmt_var = inter::GetNativeArg(state, args, n, 0);
        CVar str_var = inter::GetNativeArg(state, args, n, 1);
        std::string fmt_str = get_str_arg(fmt_var);
        std::string data_str = get_str_arg(str_var);
        std::string_view fmt = fmt_str;
        std::string_view data = data_str;
        if (fmt.empty() || data.empty()) return inter::NativeToFakeluaNil(state);

        int start_pos = 1;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            start_pos = static_cast<int>(inter::CVarToInteger(a2, 1));
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
                pm.align = 0;
                while (fmt_p < fmt_end && *fmt_p >= '0' && *fmt_p <= '9') {
                    pm.align = pm.align * 10 + (*fmt_p - '0');
                    ++fmt_p;
                }
                if (pm.align <= 0) return inter::NativeToFakeluaNil(state);
                continue;
            }
            if (c == '<') {
                pm.big_endian = false;
                ++fmt_p;
                continue;
            }
            if (c == '>' || c == '=') {
                pm.big_endian = true;
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
                    int count = 0;
                    while (fmt_p < fmt_end && *fmt_p >= '0' && *fmt_p <= '9') {
                        count = count * 10 + (*fmt_p - '0');
                        ++fmt_p;
                    }
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
                    int sz = 0;
                    while (fmt_p < fmt_end && *fmt_p >= '0' && *fmt_p <= '9') {
                        sz = sz * 10 + (*fmt_p - '0');
                        ++fmt_p;
                    }
                    if (sz <= 0) return inter::NativeToFakeluaNil(state);
                    align_up();
                    if (!check_available(static_cast<size_t>(sz))) return inter::NativeToFakeluaNil(state);
                    uint64_t uv;
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

extern "C" CVar FlEvalLoadClosure(State *state, VarClosure *cl, int arg_num, const CVar *args) {
    if (!state || !cl || !cl->code_str) {
        return inter::NativeToFakeluaNil(state);
    }
    std::string code = cl->code_str;

    if (code.size() >= 4 && code.substr(0, 4) == "\x1bLua") {
        code = (code.size() >= 5) ? code.substr(5) : code.substr(4);
    }

    static uint64_t eval_counter = 0;
    std::string eval_fn_name = "__flua_eval_ld_" + std::to_string(++eval_counter);

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
        CVar res = FakeluaCallByName(state, JIT_TCC, eval_fn_name.c_str(), 0);
        return res;
    } catch (...) {
        return inter::NativeToFakeluaNil(state);
    }
}

}// namespace fakelua
