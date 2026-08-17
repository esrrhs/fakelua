#include "native/os/native_os.h"
#include "native/object/native_object.h"
#include "native/string/native_string.h"
#include "native/table/native_table.h"
#include "var/var.h"
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <string_view>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/wait.h>
#endif

namespace fakelua::os {

using table::TableHelper;
using string::GetStringArgView;

// ─── Helper: extract int64 from CVar arg, return default if not numeric ───
static int64_t get_int_arg(State *state, CVar *args, int n, int index, int64_t default_val) {
    if (index >= n) return default_val;
    CVar a = inter::GetNativeArg(state, args, n, index);
    return inter::CVarToInteger(a, default_val);
}

// ─── Helper: build a 3-value shell result (status, how, code) ───
static CVar MakeShellResult(State *state, CVar status, const char *how, int code) {
    CVar multi = inter::AllocMultiCVar(state, 3);
    inter::SetMultiCVarElement(multi, 0, status);
    inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, how));
    inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, code));
    return multi;
}

void RegisterOsLibraryApi(State *s) {
    if (!s) return;

    // ─── os.clock() ───
    RegisterNativeFunction(s, "os.clock", 0, false, [](State *state, CVar *args, int n) -> CVar {
        double elapsed = static_cast<double>(std::clock()) / static_cast<double>(CLOCKS_PER_SEC);
        return inter::NativeToFakeluaFloat(state, elapsed);
    });

    // ─── os.date([format[, time]]]) ───
    // Standard Lua: format = luaL_optstring(L, 1, "%c"); time = luaL_opt(L, checktime, 2, time(NULL));
    // format must be string (or number, coerced) or nil/absent; anything else (bool/table) errors.
    // time must be number (or numeric string, coerced) or nil/absent; anything else errors.
    RegisterNativeFunction(s, "os.date", 0, true, [](State *state, CVar *args, int n) -> CVar {
        std::time_t t_val = 0;
        bool has_time = false;
        std::string_view fmt = "%c";
        std::string temp_fmt;

        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            if (a0.type_ != static_cast<int>(VarType::Nil)) {
                // fakelua 扩展：os.date 的第一个参数可以是 string（格式）或 number/string（时间戳）
                // 标准 Lua 5.3 只接受 string，但 fakelua 支持 number 和 numeric string
                if (a0.type_ == static_cast<int>(VarType::Int) || a0.type_ == static_cast<int>(VarType::Float)) {
                    // 数字时间戳：os.date(1700000000)
                    t_val = static_cast<std::time_t>(inter::CVarToInteger(a0, 0));
                    has_time = true;
                } else if (a0.type_ == static_cast<int>(VarType::String) || a0.type_ == static_cast<int>(VarType::StringId)) {
                    // 字符串：可能是格式或时间戳
                    std::string temp_fmt_str;
                    std::string_view sv = GetStringArgView(a0, temp_fmt_str);
                    // 尝试解析为数字时间戳
                    try {
                        size_t pos = 0;
                        int64_t ts = std::stoll(std::string(sv), &pos, 10);
                        if (pos == sv.size()) {
                            // 纯数字字符串，视为时间戳
                            t_val = static_cast<std::time_t>(ts);
                            has_time = true;
                        } else {
                            // 非纯数字，视为格式
                            fmt = sv;
                        }
                    } catch (...) {
                        fmt = sv;
                    }
                } else {
                    ThrowFakeluaException("bad argument #1 to 'os.date' (string expected)");
                }
            }
        }

        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ != static_cast<int>(VarType::Nil)) {
                // 标准 Lua：os.date 的 time 参数必须是 number，String/Bool/Table 不合法
                CheckNumberArg(a1, 2, "os.date");
                t_val = static_cast<std::time_t>(inter::CVarToInteger(a1, 0));
                has_time = true;
            }
        }

        // Check for ! prefix (UTC)
        bool use_utc = false;
        if (fmt.size() >= 1 && fmt[0] == '!') {
            use_utc = true;
            fmt.remove_prefix(1);
        }

        // Get current time if not provided
        if (!has_time) {
            t_val = std::time(nullptr);
        }

        // Convert to tm
        std::tm tm_buf{};
#if defined(_WIN32)
        if (use_utc) {
            gmtime_s(&tm_buf, &t_val);
        } else {
            localtime_s(&tm_buf, &t_val);
        }
#else
        if (use_utc) {
            gmtime_r(&t_val, &tm_buf);
        } else {
            localtime_r(&t_val, &tm_buf);
        }
#endif

        // "*t" format: return a table with date fields
        if (fmt == "*t") {
            // 分配带 bucket 的表（9 个字段 > quick_data_ 的 8 槽）
            auto &alloc = state->GetHeap().GetAllocator(false);
            auto *vtbl = static_cast<VarTable *>(alloc.Alloc(sizeof(VarTable)));
            *vtbl = VarTable{};
            for (auto &qd: vtbl->quick_data_) {
                qd.key.type_ = static_cast<int>(VarType::Nil);
                qd.val.type_ = static_cast<int>(VarType::Nil);
            }
            vtbl->free_list_idx_ = VarTable::INVALID_INDEX;
            // 预分配 bucket（16 = 最小 2 的幂 ≥ 9）
            const uint32_t bucket_count = 16;
            vtbl->bucket_count_ = bucket_count;
            vtbl->nodes_ = static_cast<VarTable::TableNode *>(alloc.Alloc(sizeof(VarTable::TableNode) * bucket_count));
            vtbl->active_list_ = static_cast<uint32_t *>(alloc.Alloc(sizeof(uint32_t) * bucket_count));
            for (uint32_t i = 0; i < bucket_count; ++i) {
                vtbl->nodes_[i].entry.key.type_ = static_cast<int>(VarType::Nil);
                vtbl->nodes_[i].entry.val.type_ = static_cast<int>(VarType::Nil);
                vtbl->nodes_[i].next = VarTable::INVALID_INDEX;
            }
            CVar tbl_cvar{};
            tbl_cvar.type_ = static_cast<int>(VarType::Table);
            tbl_cvar.data_.t = vtbl;
            TableHelper::SetTableStrId(state, tbl_cvar, "year", inter::NativeToFakeluaInt(state, tm_buf.tm_year + 1900));
            TableHelper::SetTableStrId(state, tbl_cvar, "month", inter::NativeToFakeluaInt(state, tm_buf.tm_mon + 1));
            TableHelper::SetTableStrId(state, tbl_cvar, "day", inter::NativeToFakeluaInt(state, tm_buf.tm_mday));
            TableHelper::SetTableStrId(state, tbl_cvar, "hour", inter::NativeToFakeluaInt(state, tm_buf.tm_hour));
            TableHelper::SetTableStrId(state, tbl_cvar, "min", inter::NativeToFakeluaInt(state, tm_buf.tm_min));
            TableHelper::SetTableStrId(state, tbl_cvar, "sec", inter::NativeToFakeluaInt(state, tm_buf.tm_sec));
            TableHelper::SetTableStrId(state, tbl_cvar, "wday", inter::NativeToFakeluaInt(state, tm_buf.tm_wday + 1));
            TableHelper::SetTableStrId(state, tbl_cvar, "yday", inter::NativeToFakeluaInt(state, tm_buf.tm_yday + 1));
            TableHelper::SetTableStrId(state, tbl_cvar, "isdst", inter::NativeToFakeluaBool(state, tm_buf.tm_isdst > 0));
            return tbl_cvar;
        }

        // Format
        char buf[256];
        size_t len = std::strftime(buf, sizeof(buf), std::string(fmt).c_str(), &tm_buf);
        if (len == 0) {
            buf[0] = '\0';
        }
        return inter::NativeToFakeluaStringView(state, std::string_view(buf, len));
    });

    // ─── os.difftime(t2, t1) ───
    RegisterNativeFunction(s, "os.difftime", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        // fakelua 扩展：os.difftime 接受 number 或 numeric string
        CheckNumberArg(a0, 1, "os.difftime");
        CheckNumberArg(a1, 2, "os.difftime");
        int64_t t2 = inter::CVarToInteger(a0, 0);
        int64_t t1 = inter::CVarToInteger(a1, 0);
        double diff = static_cast<double>(t2) - static_cast<double>(t1);
        return inter::NativeToFakeluaFloat(state, diff);
    });

    // ─── os.execute([command]) ───
    RegisterNativeFunction(s, "os.execute", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) {
            // No command: check if shell is available
            return MakeShellResult(state, inter::NativeToFakeluaBool(state, true), "exit", 0);
        }
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Nil)) {
            return MakeShellResult(state, inter::NativeToFakeluaBool(state, true), "exit", 0);
        }
        CheckStringArg(a0, 1, "os.execute");
        std::string s_cmd;
        std::string_view cmd_sv = GetStringArgView(a0, s_cmd);
        if (cmd_sv.empty()) {
            return MakeShellResult(state, inter::NativeToFakeluaBool(state, true), "exit", 0);
        }
        int ret = std::system(std::string(cmd_sv).c_str());
#if defined(_WIN32)
        if (ret == 0) {
            return MakeShellResult(state, inter::NativeToFakeluaBool(state, true), "exit", 0);
        }
        // Return (nil, "exit", code)
        return MakeShellResult(state, inter::NativeToFakeluaNil(state), "exit", ret);
#else
        if (ret == -1) {
            // Failed to spawn shell
            return MakeShellResult(state, inter::NativeToFakeluaNil(state), "error", errno);
        }
        if (WIFEXITED(ret)) {
            int code = WEXITSTATUS(ret);
            if (code == 0) {
                return MakeShellResult(state, inter::NativeToFakeluaBool(state, true), "exit", 0);
            }
            return MakeShellResult(state, inter::NativeToFakeluaNil(state), "exit", code);
        }
        if (WIFSIGNALED(ret)) {
            int sig = WTERMSIG(ret);
            return MakeShellResult(state, inter::NativeToFakeluaNil(state), "signal", sig);
        }
        // Fallback
        return MakeShellResult(state, inter::NativeToFakeluaNil(state), "exit", ret);
#endif
    });

    // ─── os.exit([code[, close]]) ───
    RegisterNativeFunction(s, "os.exit", 0, true, [](State *state, CVar *args, int n) -> CVar {
        int code = 0;
        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            if (a0.type_ == static_cast<int>(VarType::Bool)) {
                code = a0.data_.b ? 0 : 1;
            } else if (a0.type_ == static_cast<int>(VarType::Nil)) {
                code = 0;
            } else if (a0.type_ == static_cast<int>(VarType::Int)) {
                code = static_cast<int>(a0.data_.i);
            } else if (a0.type_ == static_cast<int>(VarType::Float)) {
                code = static_cast<int>(a0.data_.f);
            } else {
                // 标准 Lua：os.exit 的 code 参数必须是 number（或 nil/true/false），String 不合法
                ThrowFakeluaException("bad argument #1 to 'os.exit' (number expected)");
            }
        }
        std::exit(code);
        // unreachable
    });

    // ─── os.getenv(varname) ───
    RegisterNativeFunction(s, "os.getenv", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        // fakelua 扩展：os.getenv 接受 string 或 number（转换为 string）
        std::string s_var;
        std::string_view varname;
        if (a0.type_ == static_cast<int>(VarType::Int) || a0.type_ == static_cast<int>(VarType::Float)) {
            s_var = std::to_string(inter::CVarToInteger(a0, 0));
            varname = s_var;
        } else if (a0.type_ == static_cast<int>(VarType::String) || a0.type_ == static_cast<int>(VarType::StringId)) {
            varname = GetStringArgView(a0, s_var);
        } else {
            ThrowFakeluaException("bad argument #1 to 'os.getenv' (string expected)");
        }
        if (varname.empty()) {
            return inter::NativeToFakeluaNil(state);
        }
        const char *val = std::getenv(std::string(varname).c_str());
        if (val) {
            return inter::NativeToFakeluaStringView(state, std::string_view(val));
        }
        return inter::NativeToFakeluaNil(state);
    });

    // ─── os.remove(filename) ───
    RegisterNativeFunction(s, "os.remove", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "os.remove");
        std::string s_fn;
        std::string_view filename = GetStringArgView(a0, s_fn);
        if (filename.empty()) {
            return inter::NativeToFakeluaNil(state);
        }
        int ret = std::remove(std::string(filename).c_str());
        if (ret == 0) {
            return inter::NativeToFakeluaBool(state, true);
        }
        return inter::NativeToFakeluaNil(state);
    });

    // ─── os.rename(oldname, newname) ───
    RegisterNativeFunction(s, "os.rename", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckStringArg(a0, 1, "os.rename");
        CheckStringArg(a1, 2, "os.rename");
        std::string s_old, s_new;
        std::string_view oldname = GetStringArgView(a0, s_old);
        std::string_view newname = GetStringArgView(a1, s_new);
        if (oldname.empty() || newname.empty()) {
            return inter::NativeToFakeluaNil(state);
        }
        int ret = std::rename(std::string(oldname).c_str(), std::string(newname).c_str());
        if (ret == 0) {
            return inter::NativeToFakeluaBool(state, true);
        }
        return inter::NativeToFakeluaNil(state);
    });


    // ─── os.setlocale(locale[, category]) ───
    // Standard Lua: category = luaL_checkoption(L, 2, "all", catnames), locale = luaL_optstring(L, 1, NULL).
    RegisterNativeFunction(s, "os.setlocale", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(a0, 1, "os.setlocale");
        std::string s_locale;
        std::string_view locale = GetStringArgView(a0, s_locale);

        int category = LC_ALL;
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ != static_cast<int>(VarType::Nil)) {
                CheckStringArg(a1, 2, "os.setlocale");
                std::string s_cat;
                std::string_view cat = GetStringArgView(a1, s_cat);
                if (cat == "all") category = LC_ALL;
                else if (cat == "collate") category = LC_COLLATE;
                else if (cat == "ctype") category = LC_CTYPE;
                else if (cat == "monetary") category = LC_MONETARY;
                else if (cat == "numeric") category = LC_NUMERIC;
                else if (cat == "time") category = LC_TIME;
                else {
                    std::string msg = "bad argument #2 to 'os.setlocale' (invalid option '" + std::string(cat) + "')";
                    ThrowFakeluaException(msg.c_str());
                }
            }
        }

        if (locale.empty()) {
            // query current locale
            const char *cur = std::setlocale(category, nullptr);
            if (cur) {
                return inter::NativeToFakeluaStringView(state, std::string_view(cur));
            }
            return inter::NativeToFakeluaNil(state);
        }
        const char *prev = std::setlocale(category, std::string(locale).c_str());
        if (prev) {
            return inter::NativeToFakeluaStringView(state, std::string_view(prev));
        }
        return inter::NativeToFakeluaNil(state);
    });

    // ─── os.time([table]) ───
    RegisterNativeFunction(s, "os.time", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) {
            // No args: return current time
            std::time_t now = std::time(nullptr);
            return inter::NativeToFakeluaInt(state, static_cast<int64_t>(now));
        }
        CVar tbl = inter::GetNativeArg(state, args, n, 0);
        if (tbl.type_ == static_cast<int>(VarType::Nil)) {
            std::time_t now = std::time(nullptr);
            return inter::NativeToFakeluaInt(state, static_cast<int64_t>(now));
        }
        if (tbl.type_ != static_cast<int>(VarType::Table) || !tbl.data_.t) {
            ThrowFakeluaException("bad argument #1 to 'os.time' (table expected)");
        }

        // Helper: read a string-keyed field from a table
        // fakelua 扩展：os.time 的 table 字段可以是 number 或 numeric string（自动转换）
        auto get_field = [&](const char *key_name, int64_t default_val) -> int64_t {
            VarTable *t = tbl.data_.t;
            if (!t) return default_val;
            int64_t id = state->GetConstString().Alloc(key_name);
            CVar key{static_cast<int>(VarType::StringId)};
            key.data_.i = id;
            if (t->spec_get) {
                using SpecGetFn = CVar (*)(VarTable *, CVar, bool *);
                auto get_fn = reinterpret_cast<SpecGetFn>(t->spec_get);
                bool finish = false;
                CVar r = get_fn(t, key, &finish);
                if (finish) {
                    return inter::CVarToInteger(r, default_val);
                }
            }
            for (const auto &qd: t->quick_data_) {
                if (qd.key.type_ != static_cast<int>(VarType::Nil) && KeyToStringView(qd.key) == key_name) {
                    return inter::CVarToInteger(qd.val, default_val);
                }
            }
            if (t->nodes_ && t->bucket_count_ > 0) {
                for (uint32_t i = 0; i < t->count_; ++i) {
                    uint32_t node_idx = t->active_list_[i];
                    const auto &entry = t->nodes_[node_idx].entry;
                    if (entry.key.type_ != static_cast<int>(VarType::Nil) && KeyToStringView(entry.key) == key_name) {
                        return inter::CVarToInteger(entry.val, default_val);
                    }
                }
            }
            return default_val;
        };

        std::tm tm_buf{};
        tm_buf.tm_year = static_cast<int>(get_field("year", 1900) - 1900);
        tm_buf.tm_mon = static_cast<int>(get_field("month", 1) - 1);
        tm_buf.tm_mday = static_cast<int>(get_field("day", 1));
        tm_buf.tm_hour = static_cast<int>(get_field("hour", 12));
        tm_buf.tm_min = static_cast<int>(get_field("min", 0));
        tm_buf.tm_sec = static_cast<int>(get_field("sec", 0));
        tm_buf.tm_isdst = static_cast<int>(get_field("isdst", -1));

        std::time_t t = std::mktime(&tm_buf);
        if (t == -1) {
            return inter::NativeToFakeluaNil(state);
        }
        return inter::NativeToFakeluaInt(state, static_cast<int64_t>(t));
    });

    // ─── os.tmpname() ───
    RegisterNativeFunction(s, "os.tmpname", 0, false, [](State *state, CVar *args, int n) -> CVar {
#if defined(_WIN32)
        // Windows: use GetTempPath + GetTempFileName for a safe temp file name
        char temp_path[MAX_PATH];
        char temp_file[MAX_PATH];
        if (GetTempPathA(MAX_PATH, temp_path) == 0) {
            return inter::NativeToFakeluaNil(state);
        }
        if (GetTempFileNameA(temp_path, "lua", 0, temp_file) == 0) {
            return inter::NativeToFakeluaNil(state);
        }
        return inter::NativeToFakeluaStringView(state, std::string_view(temp_file));
#else
        // POSIX: use mkstemp for a safe temp file name
        char tmpname[] = "/tmp/lua_XXXXXX";
        int fd = mkstemp(tmpname);
        if (fd < 0) return inter::NativeToFakeluaNil(state);
        close(fd);
        return inter::NativeToFakeluaStringView(state, std::string_view(tmpname));
#endif
    });
}

}// namespace fakelua::os
