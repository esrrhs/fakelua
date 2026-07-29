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
}

}// namespace fakelua
