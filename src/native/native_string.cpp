#include "native/native_string.h"
#include "native/native_object.h"
#include "compile/c_runtime_header.h"
#include "state/state.h"
#include "var/var.h"
#include <algorithm>
#include <cctype>
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

    RegisterNativeFunction(s, "string.len", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaInt(state, 0);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string_view sv = KeyToStringView(a0);
        return inter::NativeToFakeluaInt(state, static_cast<int64_t>(sv.size()));
    });

    RegisterNativeFunction(s, "string.sub", 2, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        std::string_view sv = KeyToStringView(a0);
        int64_t len = static_cast<int64_t>(sv.size());

        int64_t start_pos = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : 1;
        int64_t end_pos = len;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            if (a2.type_ == static_cast<int>(VarType::Int)) end_pos = a2.data_.i;
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
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        std::string_view sv = KeyToStringView(a0);
        int64_t rep_cnt = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : 0;
        if (rep_cnt <= 0) return inter::NativeToFakeluaStringView(state, "");

        std::string sep = "";
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            std::string_view sep_sv = KeyToStringView(a2);
            sep = std::string(sep_sv);
        }

        std::string res;
        res.reserve((sv.size() + sep.size()) * static_cast<size_t>(rep_cnt));
        for (int64_t i = 0; i < rep_cnt; ++i) {
            if (i > 0 && !sep.empty()) res += sep;
            res += sv;
        }
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.reverse", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string res(KeyToStringView(a0));
        std::reverse(res.begin(), res.end());
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.lower", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string res(KeyToStringView(a0));
        std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.upper", 1, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaStringView(state, "");
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string res(KeyToStringView(a0));
        std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return inter::NativeToFakeluaStringView(state, res);
    });

    RegisterNativeFunction(s, "string.byte", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        std::string_view sv = KeyToStringView(a0);
        int64_t len = static_cast<int64_t>(sv.size());
        if (len == 0) return inter::NativeToFakeluaNil(state);

        int64_t start_pos = 1;
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ == static_cast<int>(VarType::Int)) start_pos = a1.data_.i;
        }

        int64_t end_pos = start_pos;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            if (a2.type_ == static_cast<int>(VarType::Int)) end_pos = a2.data_.i;
        }

        start_pos = NormalizePos(start_pos, len);
        end_pos = NormalizePos(end_pos, len);

        if (start_pos < 1 || start_pos > len || end_pos < start_pos) {
            return inter::NativeToFakeluaNil(state);
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
            if (arg_i.type_ == static_cast<int>(VarType::Int)) {
                res.push_back(static_cast<char>(arg_i.data_.i));
            }
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
                char buf[1024];
                snprintf(buf, sizeof(buf), spec_str.c_str(), sval.c_str());
                res.append(buf);
            } else if (spec == 'd' || spec == 'i') {
                int64_t ival =
                        (curr_arg.type_ == static_cast<int>(VarType::Int)) ? curr_arg.data_.i : (curr_arg.type_ == static_cast<int>(VarType::Float) ? static_cast<int64_t>(curr_arg.data_.f) : 0);
                std::string llspec = spec_str;
                llspec.insert(llspec.size() - 1, "ll");
                char buf[128];
                snprintf(buf, sizeof(buf), llspec.c_str(), ival);
                res.append(buf);
            } else if (spec == 'u' || spec == 'x' || spec == 'X' || spec == 'o') {
                uint64_t uval = (curr_arg.type_ == static_cast<int>(VarType::Int)) ? static_cast<uint64_t>(curr_arg.data_.i) : 0;
                std::string llspec = spec_str;
                llspec.insert(llspec.size() - 1, "ll");
                char buf[128];
                snprintf(buf, sizeof(buf), llspec.c_str(), uval);
                res.append(buf);
            } else if (spec == 'f' || spec == 'e' || spec == 'E' || spec == 'g' || spec == 'G') {
                double fval =
                        (curr_arg.type_ == static_cast<int>(VarType::Float)) ? curr_arg.data_.f : (curr_arg.type_ == static_cast<int>(VarType::Int) ? static_cast<double>(curr_arg.data_.i) : 0.0);
                char buf[128];
                snprintf(buf, sizeof(buf), spec_str.c_str(), fval);
                res.append(buf);
            } else if (spec == 'c') {
                int64_t cval = (curr_arg.type_ == static_cast<int>(VarType::Int)) ? curr_arg.data_.i : 0;
                res.push_back(static_cast<char>(cval));
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
    RegisterNativeFunction(s, "string.find", 2, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        std::string_view sv = KeyToStringView(a0);
        std::string_view pat_view = KeyToStringView(a1);
        int64_t len = static_cast<int64_t>(sv.size());

        int64_t init_pos = 1;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            if (a2.type_ == static_cast<int>(VarType::Int)) init_pos = a2.data_.i;
        }
        init_pos = NormalizePos(init_pos, len);
        if (init_pos < 1) init_pos = 1;

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
            int total = 2 + captures;                        // start, end, + 捕获
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
    RegisterNativeFunction(s, "string.match", 2, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 2) return inter::NativeToFakeluaNil(state);
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        std::string_view sv = KeyToStringView(a0);
        std::string_view pat_view = KeyToStringView(a1);
        int64_t len = static_cast<int64_t>(sv.size());

        int64_t init_pos = 1;
        if (n >= 3) {
            CVar a2 = inter::GetNativeArg(state, args, n, 2);
            if (a2.type_ == static_cast<int>(VarType::Int)) init_pos = a2.data_.i;
        }
        init_pos = NormalizePos(init_pos, len);
        if (init_pos < 1) init_pos = 1;
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

        // 使用 arena 分配器分配迭代器状态（生命周期由 arena 管理，无需手动释放）
        // 注意：alloc.Alloc 只分配原始内存，需要用 placement new 构造含 std::string 的成员
        auto &alloc = state->GetHeap().GetAllocator(false);
        GMatchState *gs = new (alloc.Alloc(sizeof(GMatchState))) GMatchState{std::move(text), std::move(pattern), 0};

        // upvalue 0: State* (用于分配返回值等)
        CVar *uv0 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv0->type_ = static_cast<int>(VarType::Int);
        uv0->flag_ = 0;
        uv0->data_.i = reinterpret_cast<int64_t>(state);

        // upvalue 1: GMatchState* (迭代器状态)
        CVar *uv1 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
        uv1->type_ = static_cast<int>(VarType::Int);
        uv1->flag_ = 0;
        uv1->data_.i = reinterpret_cast<int64_t>(gs);

        // 分配闭包：sizeof(VarClosure) + 2 * sizeof(CVar *)
        VarClosure *cl = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure) + 2 * sizeof(CVar *)));
        cl->func_ptr = reinterpret_cast<void *>(GMatchIterator);
        cl->upvalue_count = 2;
        // for-in 循环会以 (s, var) 两个参数调用迭代器，所以 expected_arg_count = 2
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
    // 全局替换。repl 可以是字符串、表或函数。
    // 返回：替换后的字符串 与 替换次数。
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
            if (a3.type_ == static_cast<int>(VarType::Int)) max_replace = a3.data_.i;
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
                    // 调用 repl 函数，参数为匹配（+ 捕获）
                    VarClosure *cl = repl_var.data_.cl;
                    void *addr = cl->func_ptr;
                    int arg_count = static_cast<int>(match.size());
                    if (arg_count == 0) {
                        replacement = match[0].str();
                    } else {
                        // 准备参数数组（最多 3 个）
                        CVar call_args[3];
                        int call_arg_count = std::min(arg_count, 3);
                        for (int i = 0; i < call_arg_count; ++i) {
                            call_args[i] = inter::NativeToFakeluaStringView(state, match[i].str());
                        }

                        CVar fn_res;
                        if (addr != nullptr) {
                            // JIT 已编译：直接通过 func_ptr 调用
                            fn_res = inter::DispatchCall(addr, call_args, call_arg_count);
                        } else {
                            // func_ptr 为空：通过 FlEvalLoadClosure 动态编译并执行
                            fn_res = FlEvalLoadClosure(state, cl, call_arg_count, call_args);
                        }
                        replacement = std::string(KeyToStringView(fn_res));
                    }
                } else if (repl_is_table) {
                    // 用表查找：以整个匹配为 key
                    std::string key = match[0].str();
                    CVar val{static_cast<int>(VarType::Nil)};

                    // 尝试 spec 快速路径（仅当表是 NativeObject 包装时）
                    VarTable *tbl = repl_var.data_.t;
                    if (tbl->spec_get) {
                        using SpecGetFn = CVar (*)(VarTable *, CVar, bool *);
                        auto get_fn = reinterpret_cast<SpecGetFn>(tbl->spec_get);
                        bool finish = false;
                        CVar key_cvar = inter::NativeToFakeluaStringView(state, key);
                        val = get_fn(tbl, key_cvar, &finish);
                        if (!finish) val = CVar{static_cast<int>(VarType::Nil)};
                    }

                    // 回退到 quick_data 线性查找
                    if (val.type_ == static_cast<int>(VarType::Nil)) {
                        CVar key_cvar = inter::NativeToFakeluaStringView(state, key);
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

    auto load_impl = [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar code_var = inter::GetNativeArg(state, args, n, 0);
        std::string_view sv = KeyToStringView(code_var);
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
