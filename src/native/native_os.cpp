#include "native/native_os.h"
#include "native/native_object.h"
#include "native/native_table.h"
#include "var/var.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string_view>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/wait.h>
#endif

namespace fakelua {

// ─── Helper: extract int64 from CVar arg, return default if not numeric ───
static int64_t get_int_arg(State *state, CVar *args, int n, int index, int64_t default_val) {
    if (index >= n) return default_val;
    CVar a = inter::GetNativeArg(state, args, n, index);
    if (a.type_ == static_cast<int>(VarType::Int)) return a.data_.i;
    if (a.type_ == static_cast<int>(VarType::Float)) return static_cast<int64_t>(a.data_.f);
    return default_val;
}

void RegisterOsLibraryApi(State *s) {
    if (!s) return;

    // ─── os.clock() ───
    RegisterNativeFunction(s, "os.clock", 0, false, [](State *state, CVar *args, int n) -> CVar {
        double elapsed = static_cast<double>(std::clock()) / static_cast<double>(CLOCKS_PER_SEC);
        return inter::NativeToFakeluaFloat(state, elapsed);
    });

    // ─── os.date([format[, time]]]) ───
    RegisterNativeFunction(s, "os.date", 0, true, [](State *state, CVar *args, int n) -> CVar {
        // Determine time value
        std::time_t t_val = 0;
        bool has_time = false;
        if (n >= 2) {
            int64_t t = get_int_arg(state, args, n, 1, 0);
            t_val = static_cast<std::time_t>(t);
            has_time = true;
        }

        // Determine format string
        std::string_view fmt;
        bool use_default_format = false;

        if (n < 1) {
            use_default_format = true;
        } else {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            if (a0.type_ == static_cast<int>(VarType::Nil)) {
                use_default_format = true;
            } else {
                fmt = KeyToStringView(a0);
                if (fmt.empty()) {
                    use_default_format = true;
                }
            }
        }

        // Check for ! prefix (UTC)
        bool use_utc = false;
        if (!use_default_format && fmt.size() >= 1 && fmt[0] == '!') {
            use_utc = true;
            fmt.remove_prefix(1);
        }

        // Get the time
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

        // Default format if needed
        if (use_default_format) {
            fmt = "%c";
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
        int64_t t2 = get_int_arg(state, args, n, 0, 0);
        int64_t t1 = get_int_arg(state, args, n, 1, 0);
        double diff = static_cast<double>(t2) - static_cast<double>(t1);
        return inter::NativeToFakeluaFloat(state, diff);
    });

    // ─── os.execute([command]) ───
    RegisterNativeFunction(s, "os.execute", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) {
            // No command: check if shell is available
            CVar multi = inter::AllocMultiCVar(state, 3);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "exit"));
            inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, 0));
            return multi;
        }
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Nil)) {
            CVar multi = inter::AllocMultiCVar(state, 3);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "exit"));
            inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, 0));
            return multi;
        }
        std::string_view cmd_sv = KeyToStringView(a0);
        if (cmd_sv.empty()) {
            CVar multi = inter::AllocMultiCVar(state, 3);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "exit"));
            inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, 0));
            return multi;
        }
        int ret = std::system(std::string(cmd_sv).c_str());
#if defined(_WIN32)
        if (ret == 0) {
            CVar multi = inter::AllocMultiCVar(state, 3);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "exit"));
            inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, 0));
            return multi;
        }
        // Return (nil, "exit", code)
        CVar multi = inter::AllocMultiCVar(state, 3);
        inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
        inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "exit"));
        inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, ret));
        return multi;
#else
        if (ret == -1) {
            // Failed to spawn shell
            CVar multi = inter::AllocMultiCVar(state, 3);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "error"));
            inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, errno));
            return multi;
        }
        if (WIFEXITED(ret)) {
            int code = WEXITSTATUS(ret);
            if (code == 0) {
                CVar multi = inter::AllocMultiCVar(state, 3);
                inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaBool(state, true));
                inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "exit"));
                inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, 0));
                return multi;
            }
            CVar multi = inter::AllocMultiCVar(state, 3);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "exit"));
            inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, code));
            return multi;
        }
        if (WIFSIGNALED(ret)) {
            int sig = WTERMSIG(ret);
            CVar multi = inter::AllocMultiCVar(state, 3);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "signal"));
            inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, sig));
            return multi;
        }
        // Fallback
        CVar multi = inter::AllocMultiCVar(state, 3);
        inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
        inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "exit"));
        inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, ret));
        return multi;
#endif
    });

    // ─── os.exit([code[, close]]) ───
    RegisterNativeFunction(s, "os.exit", 0, true, [](State *state, CVar *args, int n) -> CVar {
        int code = 0;
        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            if (a0.type_ == static_cast<int>(VarType::Bool)) {
                code = a0.data_.b ? 0 : 1;
            } else if (a0.type_ == static_cast<int>(VarType::Int)) {
                code = static_cast<int>(a0.data_.i);
            } else if (a0.type_ == static_cast<int>(VarType::Float)) {
                code = static_cast<int>(a0.data_.f);
            }
        }
        std::exit(code);
        // unreachable
    });

    // ─── os.getenv(varname) ───
    RegisterNativeFunction(s, "os.getenv", 1, false, [](State *state, CVar *args, int n) -> CVar {
        std::string_view varname = KeyToStringView(inter::GetNativeArg(state, args, n, 0));
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
        std::string_view filename = KeyToStringView(inter::GetNativeArg(state, args, n, 0));
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
        std::string_view oldname = KeyToStringView(inter::GetNativeArg(state, args, n, 0));
        std::string_view newname = KeyToStringView(inter::GetNativeArg(state, args, n, 1));
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
    RegisterNativeFunction(s, "os.setlocale", 1, true, [](State *state, CVar *args, int n) -> CVar {
        std::string_view locale = KeyToStringView(inter::GetNativeArg(state, args, n, 0));
        if (locale.empty()) {
            // query current locale
            const char *cur = std::setlocale(LC_ALL, nullptr);
            if (cur) {
                return inter::NativeToFakeluaStringView(state, std::string_view(cur));
            }
            return inter::NativeToFakeluaNil(state);
        }
        const char *prev = std::setlocale(LC_ALL, std::string(locale).c_str());
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
        if (tbl.type_ != static_cast<int>(VarType::Table)) {
            return inter::NativeToFakeluaNil(state);
        }

        // Helper: read a string-keyed field from a table
        auto get_field = [&](const char *key_name) -> int64_t {
            VarTable *t = tbl.data_.t;
            if (!t) return 0;
            int64_t id = state->GetConstString().Alloc(key_name);
            CVar key{static_cast<int>(VarType::StringId)};
            key.data_.i = id;
            // Try spec_get first
            if (t->spec_get) {
                using SpecGetFn = CVar (*)(VarTable *, CVar, bool *);
                auto get_fn = reinterpret_cast<SpecGetFn>(t->spec_get);
                bool finish = false;
                CVar r = get_fn(t, key, &finish);
                if (finish) {
                    if (r.type_ == static_cast<int>(VarType::Int)) return r.data_.i;
                    if (r.type_ == static_cast<int>(VarType::Float)) return static_cast<int64_t>(r.data_.f);
                    return 0;
                }
            }
            // Search quick_data_
            for (const auto &qd: t->quick_data_) {
                if (qd.key.type_ != static_cast<int>(VarType::Nil) && KeyToStringView(qd.key) == key_name) {
                    if (qd.val.type_ == static_cast<int>(VarType::Int)) return qd.val.data_.i;
                    if (qd.val.type_ == static_cast<int>(VarType::Float)) return static_cast<int64_t>(qd.val.data_.f);
                    return 0;
                }
            }
            // Search nodes_
            if (t->nodes_ && t->bucket_count_ > 0) {
                for (uint32_t i = 0; i < t->count_; ++i) {
                    uint32_t node_idx = t->active_list_[i];
                    const auto &entry = t->nodes_[node_idx].entry;
                    if (entry.key.type_ != static_cast<int>(VarType::Nil) && KeyToStringView(entry.key) == key_name) {
                        if (entry.val.type_ == static_cast<int>(VarType::Int)) return entry.val.data_.i;
                        if (entry.val.type_ == static_cast<int>(VarType::Float)) return static_cast<int64_t>(entry.val.data_.f);
                        return 0;
                    }
                }
            }
            return 0;
        };

        std::tm tm_buf{};
        tm_buf.tm_year = static_cast<int>(get_field("year") - 1900);
        tm_buf.tm_mon = static_cast<int>(get_field("month") - 1);
        tm_buf.tm_mday = static_cast<int>(get_field("day"));
        tm_buf.tm_hour = static_cast<int>(get_field("hour"));
        tm_buf.tm_min = static_cast<int>(get_field("min"));
        tm_buf.tm_sec = static_cast<int>(get_field("sec"));
        tm_buf.tm_isdst = -1;

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

}// namespace fakelua
