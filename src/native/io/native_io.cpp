#include "native/io/native_io.h"
#include "native/native_common.h"
#include "native/object/native_object.h"
#include "native/string/native_string.h"
#include "var/var.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fakelua::io {

using string::GetStringArgView;

// ─────────────────────────────────────────────────────────────────────────────
// IoFile: wraps a FILE* inside a NativeObject.
// The FILE* is stored as an int64_t field named "__fp__".
// Lua side sees it as a userdata-like object with methods (via colon call).
// ─────────────────────────────────────────────────────────────────────────────

static constexpr const char *kFpKey = "__fp__";
static constexpr const char *kPopenKey = "__popen__";
static constexpr const char *kIoStateKey = "__io_state__";
static constexpr const char *kLinesCloseKey = "__io_lines_close__";

static std::unordered_map<State *, std::vector<NativeObject *>> g_io_wrappers;

struct IoStdFiles {
    NativeObject *in = nullptr;
    NativeObject *out = nullptr;
    NativeObject *err = nullptr;
};
static std::unordered_map<State *, IoStdFiles> g_io_std;

static bool is_std_handle(FILE *fp) {
    return fp == stdin || fp == stdout || fp == stderr;
}

static void register_io_wrapper(State *s, NativeObject *nat) {
    if (!s || !nat) return;
    nat->SetInt(kIoStateKey, reinterpret_cast<int64_t>(s));
    g_io_wrappers[s].push_back(nat);
}

static void unregister_io_wrapper(NativeObject *nat) {
    if (!nat) return;
    auto *st = reinterpret_cast<State *>(nat->GetInt(kIoStateKey, 0));
    nat->SetInt(kIoStateKey, 0);
    if (!st) return;
    auto it = g_io_wrappers.find(st);
    if (it == g_io_wrappers.end()) return;
    auto &v = it->second;
    v.erase(std::remove(v.begin(), v.end(), nat), v.end());
    if (v.empty()) g_io_wrappers.erase(it);
}

static void CloseIoFileHandle(NativeObject *self) {
    auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
    if (!fp) return;
    if (is_std_handle(fp)) return;
    bool is_popen = self->GetBool(kPopenKey, false);
    if (is_popen) {
        ::pclose(fp);
    } else {
        std::fclose(fp);
    }
    self->SetInt(kFpKey, 0);
}

void OnStateDeleted(State *s) {
    if (!s) return;
    auto it = g_io_wrappers.find(s);
    if (it != g_io_wrappers.end()) {
        auto wrappers = std::move(it->second);
        g_io_wrappers.erase(it);
        for (auto *nat : wrappers) {
            if (!nat) continue;
            nat->SetInt(kIoStateKey, 0);
            NativeObjectManager::Instance().DestroyGroup(nat->GetGroupId());
        }
    }
    g_io_std.erase(s);
}

// ─── 行读取辅助函数 ───
// 从 fp 读取一行（去掉换行），返回是否读到内容。与 file:read("*l") 逻辑一致。
static bool ReadLine(FILE *fp, std::string &result) {
    result.clear();
    char buf[4096];
    bool got_any = false;
    while (std::fgets(buf, sizeof(buf), fp)) {
        result += buf;
        got_any = true;
        if (!result.empty() && result.back() == '\n') break;
    }
    if (!got_any) return false;
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();
    return true;
}

// ─── file:lines() 迭代器原生函数 ───
// 闭包签名：CVar (*)(VarClosure *cl, CVar s, CVar var)
// upvalues[0] = State* (as int)
// upvalues[1] = FILE* (as int，从 iofile 对象获取)
extern "C" CVar FileLinesIterator(VarClosure *cl, CVar /*s*/, CVar /*var*/) {
    if (!cl || cl->upvalue_count < 2) {
        return CVar{static_cast<int>(VarType::Nil)};
    }
    State *iter_state = reinterpret_cast<State *>(cl->upvalues[0]->data_.i);
    auto *obj = reinterpret_cast<NativeObject *>(cl->upvalues[1]->data_.i);
    if (!iter_state || !obj || !obj->Alive()) {
        return inter::NativeToFakeluaNil(iter_state);
    }
    auto *fp = reinterpret_cast<FILE *>(obj->GetInt(kFpKey, 0));
    if (!fp) {
        return inter::NativeToFakeluaNil(iter_state);
    }

    std::string result;
    if (!ReadLine(fp, result)) {
        if (obj->GetBool(kLinesCloseKey, false)) {
            CloseIoFileHandle(obj);
        }
        return inter::NativeToFakeluaNil(iter_state);
    }
    return inter::NativeToFakeluaString(iter_state, result);
}

// ─── 单格式读取辅助函数 ───
// 从 fp 读取一个格式（* *l *L *a *n 或数字字节数），返回 CVar（可能为 nil）
// 标准 Lua：file:read 接受 string（格式）或 number（字节数），Bool/Table 不合法
static CVar ReadOneFormat(FILE *fp, State *state, CVar fmt_var, int argno, const char *fname) {
    // 标准 Lua：file:read 接受 string 或 number，Bool/Table/Nil 不合法
    if (fmt_var.type_ != static_cast<int>(VarType::String) && fmt_var.type_ != static_cast<int>(VarType::StringId) &&
        fmt_var.type_ != static_cast<int>(VarType::Int) && fmt_var.type_ != static_cast<int>(VarType::Float)) {
        ThrowFakeluaException(std::string("bad argument #") + std::to_string(argno) + " to '" + fname + "' (string or number expected)");
    }
    std::string s_fmt;
    if (fmt_var.type_ == static_cast<int>(VarType::Int) || fmt_var.type_ == static_cast<int>(VarType::Float)) {
        s_fmt = std::to_string(inter::CVarToInteger(fmt_var, 0));
    } else {
        std::string temp_fmt;
        s_fmt = std::string(GetStringArgView(fmt_var, temp_fmt));
    }

    if (!s_fmt.empty()) {
        std::string_view fmt = s_fmt;
        if (fmt == "*l" || fmt == "l") {
            std::string result;
            if (!ReadLine(fp, result)) return inter::NativeToFakeluaNil(state);
            return inter::NativeToFakeluaString(state, result);
        } else if (fmt == "*L" || fmt == "L") {
            std::string result;
            char buf[4096];
            bool got_any = false;
            while (std::fgets(buf, sizeof(buf), fp)) {
                result += buf;
                got_any = true;
                if (!result.empty() && result.back() == '\n') break;
            }
            if (!got_any) return inter::NativeToFakeluaNil(state);
            return inter::NativeToFakeluaString(state, result);
        } else if (fmt == "*a" || fmt == "a") {
            std::string result;
            char chunk[4096];
            size_t nread;
            while ((nread = std::fread(chunk, 1, sizeof(chunk), fp)) > 0) {
                result.append(chunk, nread);
            }
            return inter::NativeToFakeluaString(state, result);
        } else if (fmt == "*n" || fmt == "n") {
            double val;
            if (std::fscanf(fp, "%lf", &val) == 1) {
                int64_t iv = 0;
                if (DoubleFitsInt64(val, &iv)) {
                    return inter::NativeToFakeluaLonglong(state, static_cast<long long>(iv));
                }
                return inter::NativeToFakeluaDouble(state, val);
            }
            return inter::NativeToFakeluaNil(state);
        }
        // 数字字符串表示读取指定字节数（例如 "10"）
        int64_t num_count = 0;
        try {
            num_count = std::stoll(s_fmt);
            if (num_count == 0) {
                int c = std::fgetc(fp);
                if (c == EOF) return inter::NativeToFakeluaNil(state);
                std::ungetc(c, fp);
                return inter::NativeToFakeluaStringView(state, std::string_view(""));
            }
            if (num_count > 0) {
                if (num_count > 64 * 1024 * 1024) {
                    ThrowFakeluaException("file:read: count too large");
                }
                std::string result(static_cast<size_t>(num_count), '\0');
                size_t nread = std::fread(result.data(), 1, static_cast<size_t>(num_count), fp);
                result.resize(nread);
                if (nread == 0) return inter::NativeToFakeluaNil(state);
                return inter::NativeToFakeluaString(state, result);
            }
        } catch (...) {
        }
        return inter::NativeToFakeluaNil(state);
    } else if (fmt_var.type_ == static_cast<int>(VarType::Int) || fmt_var.type_ == static_cast<int>(VarType::Float)) {
        int64_t count = 0;
        if (fmt_var.type_ == static_cast<int>(VarType::Int)) {
            count = fmt_var.data_.i;
        } else if (!DoubleFitsInt64(fmt_var.data_.f, &count)) {
            return inter::NativeToFakeluaNil(state);
        }
        if (count == 0) {
            int c = std::fgetc(fp);
            if (c == EOF) return inter::NativeToFakeluaNil(state);
            std::ungetc(c, fp);
            return inter::NativeToFakeluaStringView(state, std::string_view(""));
        }
        if (count < 0) return inter::NativeToFakeluaNil(state);
        if (count > 64 * 1024 * 1024) {
            ThrowFakeluaException("file:read: count too large");
        }
        std::string result(static_cast<size_t>(count), '\0');
        size_t nread = std::fread(result.data(), 1, static_cast<size_t>(count), fp);
        result.resize(nread);
        if (nread == 0) return inter::NativeToFakeluaNil(state);
        return inter::NativeToFakeluaString(state, result);
    }
    return inter::NativeToFakeluaNil(state);
}

// Helper: create a file:lines() iterator closure
// Uses shared MakeIteratorClosure from native_common.h to dedupe the standard
// 2-upvalue (State*, opaque state) iterator pattern.
static CVar MakeFileLinesClosure(State *state, NativeObject *file) {
    if (!state || !file) return inter::NativeToFakeluaNil(state);
    return MakeIteratorClosure(state, reinterpret_cast<void *>(FileLinesIterator), file);
}

// 将 CVar 参数转为字符串视图，用于 fwrite 等
// 标准 Lua 的 io.write/file:write 仅接受字符串或数字（内部走 luaL_checklstring，
// 数字会被自动转换为字符串），Bool/Table 等类型一律报错，而不是静默转换/跳过。
static std::string_view ArgToStringView(CVar a, State * /*state*/, std::string &temp, int argno, const char *fname) {
    // 先做类型检查：标准 Lua 仅接受 string 或 number
    if (a.type_ != static_cast<int>(VarType::String) && a.type_ != static_cast<int>(VarType::StringId) &&
        a.type_ != static_cast<int>(VarType::Int) && a.type_ != static_cast<int>(VarType::Float)) {
        CheckStringArg(a, argno, fname);
        return {};
    }
    // 复用 GetStringArgView 处理 String/StringId/Int/Float 的转换
    return GetStringArgView(a, temp);
}

// 创建一个 IoFile NativeObject 壳，内部 FILE* 存为 Int 字段
// is_popen=true 时用 pclose 而非 fclose 关闭
static NativeObject *MakeIoFile(State *s, FILE *fp, bool is_popen = false) {
    int64_t gid = NativeObjectManager::Instance().CreateGroup();
    auto *obj = NativeObjectManager::Instance().Create(gid, "iofile");
    obj->SetInt(kFpKey, reinterpret_cast<int64_t>(fp));
    obj->SetBool(kPopenKey, is_popen);
    register_io_wrapper(s, obj);
    obj->SetFinalizer([](NativeObject *self) {
        unregister_io_wrapper(self);
        CloseIoFileHandle(self);
    });

    // ── file:read([format]) ──
    obj->RegisterMethod("read", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);

        // 无参数：默认 "*l"
        if (n < 1) {
            CVar fake_fmt{static_cast<int>(VarType::StringId)};
            fake_fmt.data_.i = state->GetConstString().Alloc("*l");
            return ReadOneFormat(fp, state, fake_fmt, 1, "file:read");
        }

        // 多格式参数：逐个读取，返回 multi-value
        if (n >= 2) {
            auto multi = inter::AllocMultiCVar(state, n);
            for (int i = 0; i < n; ++i) {
                CVar fmt_var = inter::GetNativeArg(state, args, n, i);
                CVar res = ReadOneFormat(fp, state, fmt_var, i + 1, "file:read");
                inter::SetMultiCVarElement(multi, i, res);
                if (res.type_ == static_cast<int>(VarType::Nil)) break;
            }
            return multi;
        }

        // 单格式参数
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        return ReadOneFormat(fp, state, a0, 1, "file:read");
    });

    // ── file:write(...) ──
    obj->RegisterMethod("write", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);

        std::string temp;
        for (int i = 0; i < n; i++) {
            CVar a = inter::GetNativeArg(state, args, n, i);
            std::string_view sv = ArgToStringView(a, state, temp, i + 1, "file:write");
            if (!sv.empty()) {
                std::fwrite(sv.data(), 1, sv.size(), fp);
            }
        }
        // 返回 self 以支持链式调用
        return inter::NativeToFakeluaNativeObject(state, self);
    });

    // ── file:flush() ──
    obj->RegisterMethod("flush", [](NativeObject *self, State *state, CVar * /*args*/, int /*n*/) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);
        std::fflush(fp);
        return inter::NativeToFakeluaBool(state, true);
    });

    // ── file:close() ──
    obj->RegisterMethod("close", [](NativeObject *self, State *state, CVar * /*args*/, int /*n*/) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaBool(state, true);
        if (is_std_handle(fp)) {
            return inter::NativeToFakeluaBool(state, true);
        }
        bool is_popen = self->GetBool(kPopenKey, false);
        int ret = is_popen ? ::pclose(fp) : std::fclose(fp);
        self->SetInt(kFpKey, 0);
        if (ret == 0) {
            return inter::NativeToFakeluaBool(state, true);
        }
        return inter::NativeToFakeluaNil(state);
    });

    // ── file:seek([whence [, offset]]) ──
    obj->RegisterMethod("seek", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);

        std::string_view whence_str = "cur";
        std::string temp_whence;
        int64_t offset = 0;

        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            if (a0.type_ != static_cast<int>(VarType::Nil)) {
                // 标准 Lua：file:seek 的 whence 必须是 string（CheckStringArg 已拒绝非 string）
                CheckStringArg(a0, 1, "file:seek");
                whence_str = GetStringArgView(a0, temp_whence);
                if (whence_str.empty()) whence_str = "cur";
            }
        }
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            // 标准 Lua：file:seek 的 offset 必须是 number
            CheckNumberArg(a1, 2, "file:seek");
            offset = inter::CVarToInteger(a1, 0);
        }

        int whence;
        if (whence_str == "set") whence = SEEK_SET;
        else if (whence_str == "end")
            whence = SEEK_END;
        else if (whence_str == "cur")
            whence = SEEK_CUR;
        else
            ThrowFakeluaException("bad argument #1 to 'seek' (invalid option '" + std::string(whence_str) + "')");

        if (std::fseek(fp, static_cast<long>(offset), whence) != 0) {
            return inter::NativeToFakeluaNil(state);
        }
        long pos = std::ftell(fp);
        return inter::NativeToFakeluaLonglong(state, static_cast<long long>(pos));
    });

    // ── file:setvbuf(mode [, size]) ──
    // mode: "no" 无缓冲, "full" 全缓冲, "line" 行缓冲
    // 成功返回 file，失败返回 nil, errmsg
    obj->RegisterMethod("setvbuf", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);

        std::string_view mode = "full";
        std::string temp_mode;
        size_t size = BUFSIZ;
        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            CheckStringArg(a0, 1, "file:setvbuf");
            mode = GetStringArgView(a0, temp_mode);
            if (mode.empty()) mode = "full";
        }
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            // 标准 Lua：file:setvbuf 的 size 必须是 number
            CheckNumberArg(a1, 2, "file:setvbuf");
            int64_t sz = inter::CVarToInteger(a1, BUFSIZ);
            if (sz <= 0 || sz > 64 * 1024 * 1024) {
                ThrowFakeluaException("file:setvbuf: size out of limits");
            }
            size = static_cast<size_t>(sz);
        }

        int bufmode;
        if (mode == "no") bufmode = _IONBF;
        else if (mode == "line")
            bufmode = _IOLBF;
        else
            bufmode = _IOFBF;

        // Let libc own the buffer. A Lua temp-heap allocation is rewound by
        // State::Reset() and would UAF on the next fread/fwrite/fclose.
        if (std::setvbuf(fp, nullptr, bufmode, size > 0 ? size : BUFSIZ) != 0) {
            auto multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaCstr(state, "setvbuf failed"));
            return multi;
        }
        // 成功返回 file 对象自身
        return inter::NativeToFakeluaNativeObject(state, self);
    });

    // ── file:lines() → iterator closure ───
    // 返回一个闭包，用于 for line in file:lines() do ... end
    // 迭代器逐行读取，到文件末尾时返回 nil（for-in 循环自动结束）
    obj->RegisterMethod("lines", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);
        return MakeFileLinesClosure(state, self);
    });

    return obj;
}

static NativeObject *StdHandle(State *s, FILE *fp) {
    auto &stdf = g_io_std[s];
    NativeObject **slot = nullptr;
    if (fp == stdin) slot = &stdf.in;
    else if (fp == stdout)
        slot = &stdf.out;
    else
        slot = &stdf.err;
    if (!*slot) *slot = MakeIoFile(s, fp);
    return *slot;
}

// ─────────────────────────────────────────────────────────────────────────────
// io 库注册
// ─────────────────────────────────────────────────────────────────────────────

void RegisterIoLibraryApi(State *s) {
    if (!s) return;

    // ─── io.open(filename [, mode]) → file | nil, err ───
    RegisterNativeFunction(s, "io.open", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar fn_arg = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(fn_arg, 1, "io.open");
        std::string temp_fn;
        std::string_view filename = GetStringArgView(fn_arg, temp_fn);
        if (filename.empty()) {
            auto multi = inter::AllocMultiCVar(state, 3);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, "missing filename"));
            inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, EINVAL));
            return multi;
        }
        std::string mode = "r";
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ != static_cast<int>(VarType::Nil)) {
                CheckStringArg(a1, 2, "io.open");
                mode = std::string(KeyToStringView(a1));
            }
        }
        FILE *fp = std::fopen(std::string(filename).c_str(), mode.c_str());
        if (!fp) {
            auto multi = inter::AllocMultiCVar(state, 3);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, std::strerror(errno)));
            inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(state, errno));
            return multi;
        }
        auto *obj = MakeIoFile(state, fp);
        return inter::NativeToFakeluaNativeObject(state, obj);
    });

    // ─── io.close([file]) ───
    // 关闭指定 file（默认当前输出文件，简化为 flush stdout）
    RegisterNativeFunction(s, "io.close", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) {
            std::fflush(stdout);
            return inter::NativeToFakeluaBool(state, true);
        }
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        NativeObject *obj = NativeObject::Unwrap(a0);
        if (!obj || obj->GetTypeName() != "iofile") {
            ThrowFakeluaException("bad argument #1 to 'io.close' (FILE* expected)");
        }
        auto *fp = reinterpret_cast<FILE *>(obj->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaBool(state, true);
        if (is_std_handle(fp)) {
            return inter::NativeToFakeluaBool(state, true);
        }
        bool is_popen = obj->GetBool(kPopenKey, false);
        int ret = is_popen ? ::pclose(fp) : std::fclose(fp);
        obj->SetInt(kFpKey, 0);
        return ret == 0 ? inter::NativeToFakeluaBool(state, true) : inter::NativeToFakeluaNil(state);
    });

    // ─── io.read([format ...]) → string|number|nil ───
    // 从 stdin 读取，支持多格式参数（返回 multi-value）
    RegisterNativeFunction(s, "io.read", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) {
            CVar fake_fmt{static_cast<int>(VarType::StringId)};
            fake_fmt.data_.i = state->GetConstString().Alloc("*l");
            return ReadOneFormat(stdin, state, fake_fmt, 1, "io.read");
        }
        // 多格式参数：逐个读取，返回 multi-value
        if (n >= 2) {
            auto multi = inter::AllocMultiCVar(state, n);
            for (int i = 0; i < n; ++i) {
                CVar fmt_var = inter::GetNativeArg(state, args, n, i);
                CVar res = ReadOneFormat(stdin, state, fmt_var, i + 1, "io.read");
                inter::SetMultiCVarElement(multi, i, res);
            }
            return multi;
        }
        // 单格式参数
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        return ReadOneFormat(stdin, state, a0, 1, "io.read");
    });

    // ─── io.write(...) → true ───
    // 写入 stdout
    RegisterNativeFunction(s, "io.write", 0, true, [](State *state, CVar *args, int n) -> CVar {
        std::string temp;
        for (int i = 0; i < n; i++) {
            CVar a = inter::GetNativeArg(state, args, n, i);
            std::string_view sv = ArgToStringView(a, state, temp, i + 1, "io.write");
            if (!sv.empty()) {
                std::fwrite(sv.data(), 1, sv.size(), stdout);
            }
        }
        return inter::NativeToFakeluaBool(state, true);
    });

    // ─── io.flush() → true ───
    RegisterNativeFunction(s, "io.flush", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
        std::fflush(stdout);
        return inter::NativeToFakeluaBool(state, true);
    });

    // ─── io.type(v) → "file" | "closed file" | nil ───
    RegisterNativeFunction(s, "io.type", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        NativeObject *obj = NativeObject::Unwrap(a0);
        if (obj && obj->GetTypeName() == "iofile") {
            auto *fp = reinterpret_cast<FILE *>(obj->GetInt(kFpKey, 0));
            return fp ? inter::NativeToFakeluaStringView(state, std::string_view("file")) : inter::NativeToFakeluaStringView(state, std::string_view("closed file"));
        }
        return inter::NativeToFakeluaNil(state);
    });

    // ─── io.tmpfile() → file|nil ───
    RegisterNativeFunction(s, "io.tmpfile", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
        FILE *fp = std::tmpfile();
        if (!fp) return inter::NativeToFakeluaNil(state);
        auto *obj = MakeIoFile(state, fp);
        return inter::NativeToFakeluaNativeObject(state, obj);
    });

    // ─── io.popen(command [, mode]) → file | nil, err ───
    // 执行外部命令并打开管道。读模式 "r" 读取命令输出，写模式 "w" 向命令写入。
    // 关闭时使用 pclose（由 __popen__ 标志自动区分）。
    RegisterNativeFunction(s, "io.popen", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar cmd_arg = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(cmd_arg, 1, "io.popen");
        std::string cmd_str;
        std::string_view command = GetStringArgView(cmd_arg, cmd_str);
        if (command.empty()) {
            auto multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, "missing command"));
            return multi;
        }
        std::string mode = "r";
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ != static_cast<int>(VarType::Nil)) {
                CheckStringArg(a1, 2, "io.popen");
                mode = std::string(KeyToStringView(a1));
            }
        }
        FILE *fp = ::popen(std::string(command).c_str(), mode.c_str());
        if (!fp) {
            auto multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, std::strerror(errno)));
            return multi;
        }
        auto *obj = MakeIoFile(state, fp, true /* is_popen */);
        return inter::NativeToFakeluaNativeObject(state, obj);
    });

    // ─── io.input([file]) → file ───
    // 设置/获取当前默认输入文件（简化：仅返回参数或 stdin 包装）
    RegisterNativeFunction(s, "io.input", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) {
            return inter::NativeToFakeluaNativeObject(state, StdHandle(state, stdin));
        }
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        NativeObject *obj = NativeObject::Unwrap(a0);
        if (obj && obj->GetTypeName() == "iofile") {
            return inter::NativeToFakeluaNativeObject(state, obj);
        }
        return inter::NativeToFakeluaNil(state);
    });

    // ─── io.output([file]) → file ───
    RegisterNativeFunction(s, "io.output", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) {
            return inter::NativeToFakeluaNativeObject(state, StdHandle(state, stdout));
        }
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        NativeObject *obj = NativeObject::Unwrap(a0);
        if (obj && obj->GetTypeName() == "iofile") {
            return inter::NativeToFakeluaNativeObject(state, obj);
        }
        return inter::NativeToFakeluaNil(state);
    });

    // ─── io.lines([filename]) → iterator|nil ───
    // 简化：如果指定文件名则打开文件返回对象（用户手动循环 read），否则返回 nil
    RegisterNativeFunction(s, "io.lines", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) {
            // stdin 迭代不支持（需要维护跨调用的 FILE* 状态）
            return inter::NativeToFakeluaNil(state);
        }
        CVar fn_arg = inter::GetNativeArg(state, args, n, 0);
        CheckStringArg(fn_arg, 1, "io.lines");
        std::string fn_str;
        std::string_view filename = GetStringArgView(fn_arg, fn_str);
        if (filename.empty()) return inter::NativeToFakeluaNil(state);
        FILE *fp = std::fopen(std::string(filename).c_str(), "r");
        if (!fp) return inter::NativeToFakeluaNil(state);
        auto *obj = MakeIoFile(state, fp);
        obj->SetBool(kLinesCloseKey, true);
        return MakeFileLinesClosure(state, obj);
    });

    // ─── io.stdin / io.stdout / io.stderr 文件对象 ───
    // 每个 State 一份包装，随 FakeluaDeleteState 销毁，避免跨 VM 共用壳。
    {
        StdHandle(s, stdin);
        StdHandle(s, stdout);
        StdHandle(s, stderr);
        RegisterNativeFunction(s, "io.stdin", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
            return inter::NativeToFakeluaNativeObject(state, StdHandle(state, stdin));
        });
        RegisterNativeFunction(s, "io.stdout", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
            return inter::NativeToFakeluaNativeObject(state, StdHandle(state, stdout));
        });
        RegisterNativeFunction(s, "io.stderr", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
            return inter::NativeToFakeluaNativeObject(state, StdHandle(state, stderr));
        });
    }
}

}// namespace fakelua::io
