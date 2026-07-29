#include "native/native_string.h"
#include "native/native_object.h"
#include "state/state.h"
#include "var/var.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace fakelua {

static int64_t NormalizePos(int64_t pos, int64_t len) {
    if (pos >= 0) {
        return pos;
    }
    return len + pos + 1;
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

    RegisterNativeFunction(s, "string.dump", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar fn_var = inter::GetNativeArg(state, args, n, 0);
        if (fn_var.type_ != static_cast<int>(VarType::Closure) || !fn_var.data_.cl) {
            return inter::NativeToFakeluaNil(state);
        }
        VarClosure *cl = fn_var.data_.cl;
        std::string code = cl->code_str ? std::string(cl->code_str) : "";
        std::string payload = "\x1bLua" + code;
        return inter::NativeToFakeluaStringView(state, payload);
    });

    auto load_impl = [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar code_var = inter::GetNativeArg(state, args, n, 0);
        std::string_view sv = KeyToStringView(code_var);
        if (sv.empty()) return inter::NativeToFakeluaNil(state);

        std::string code;
        if (sv.size() >= 4 && sv.substr(0, 4) == "\x1bLua") {
            code = std::string(sv.substr(4));
        } else {
            code = std::string(sv);
        }

        try {
            CompileConfig config;
            CompileString(state, code, config);

            // 编译成功后构造 Closure 保存源码
            auto &alloc = state->GetHeap().GetAllocator(false);
            char *saved_code = static_cast<char *>(alloc.Alloc(code.size() + 1));
            std::memcpy(saved_code, code.c_str(), code.size() + 1);

            VarClosure *cl = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure)));
            cl->func_ptr = nullptr;
            cl->upvalue_count = 0;
            cl->expected_arg_count = 0;
            cl->is_vararg = true;
            cl->code_str = saved_code;

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

}// namespace fakelua
