#include "native/native_io.h"
#include "native/native_object.h"
#include "native/native_string.h"
#include "var/var.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────────
// IoFile: wraps a FILE* inside a NativeObject.
// The FILE* is stored as an int64_t field named "__fp__".
// Lua side sees it as a userdata-like object with methods (via colon call).
// ─────────────────────────────────────────────────────────────────────────────

static constexpr const char *kFpKey = "__fp__";
static constexpr const char *kPopenKey = "__popen__";
static constexpr int64_t kIoFileGroup = 999999;// 专用 group，0 不允许

// ─── file:lines() 迭代器原生函数 ───
// 闭包签名：CVar (*)(VarClosure *cl, CVar s, CVar var)
// upvalues[0] = State* (as int)
// upvalues[1] = FILE* (as int，从 iofile 对象获取)
extern "C" CVar FileLinesIterator(VarClosure *cl, CVar /*s*/, CVar /*var*/) {
    if (!cl || cl->upvalue_count < 2) {
        return CVar{static_cast<int>(VarType::Nil)};
    }
    State *iter_state = reinterpret_cast<State *>(cl->upvalues[0]->data_.i);
    FILE *fp = reinterpret_cast<FILE *>(cl->upvalues[1]->data_.i);
    if (!iter_state || !fp) {
        return inter::NativeToFakeluaNil(iter_state);
    }

    // 读一行（去掉换行），与 file:read("*l") 逻辑一致
    std::string result;
    char buf[4096];
    bool got_any = false;
    while (std::fgets(buf, sizeof(buf), fp)) {
        result += buf;
        got_any = true;
        if (!result.empty() && result.back() == '\n') break;
    }
    if (!got_any) return inter::NativeToFakeluaNil(iter_state);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();
    return inter::NativeToFakeluaString(iter_state, result);
}

// ─── 单格式读取辅助函数 ───
// 从 fp 读取一个格式（* *l *L *a *n 或数字字节数），返回 CVar（可能为 nil）
static CVar ReadOneFormat(FILE *fp, State *state, CVar fmt_var) {
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
            char buf[4096];
            bool got_any = false;
            while (std::fgets(buf, sizeof(buf), fp)) {
                result += buf;
                got_any = true;
                if (!result.empty() && result.back() == '\n') break;
            }
            if (!got_any) return inter::NativeToFakeluaNil(state);
            while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();
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
                if (val == static_cast<int64_t>(val) && std::isfinite(val)) {
                    return inter::NativeToFakeluaLonglong(state, static_cast<long long>(val));
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
        int64_t count = (fmt_var.type_ == static_cast<int>(VarType::Int)) ? fmt_var.data_.i : static_cast<int64_t>(fmt_var.data_.f);
        if (count == 0) {
            int c = std::fgetc(fp);
            if (c == EOF) return inter::NativeToFakeluaNil(state);
            std::ungetc(c, fp);
            return inter::NativeToFakeluaStringView(state, std::string_view(""));
        }
        if (count < 0) return inter::NativeToFakeluaNil(state);
        std::string result(static_cast<size_t>(count), '\0');
        size_t nread = std::fread(result.data(), 1, static_cast<size_t>(count), fp);
        result.resize(nread);
        if (nread == 0) return inter::NativeToFakeluaNil(state);
        return inter::NativeToFakeluaString(state, result);
    }
    return inter::NativeToFakeluaNil(state);
}

// Helper: create a file:lines() iterator closure
static CVar MakeFileLinesClosure(State *state, FILE *fp) {
    if (!state || !fp) return inter::NativeToFakeluaNil(state);
    auto &alloc = state->GetHeap().GetAllocator(false /* temp */);

    // upvalue 0: State* (用于分配返回值等)
    CVar *uv0 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
    uv0->type_ = static_cast<int>(VarType::Int);
    uv0->flag_ = 0;
    uv0->data_.i = reinterpret_cast<int64_t>(state);

    // upvalue 1: FILE* (迭代器要读取的文件句柄)
    CVar *uv1 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
    uv1->type_ = static_cast<int>(VarType::Int);
    uv1->flag_ = 0;
    uv1->data_.i = reinterpret_cast<int64_t>(fp);

    // 分配闭包：sizeof(VarClosure) + 2 * sizeof(CVar *)
    VarClosure *cl = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure) + 2 * sizeof(CVar *)));
    cl->func_ptr = reinterpret_cast<void *>(FileLinesIterator);
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
}

// 创建一个 IoFile NativeObject 壳，内部 FILE* 存为 Int 字段
// is_popen=true 时用 pclose 而非 fclose 关闭
static NativeObject *MakeIoFile(State *s, FILE *fp, bool is_popen = false) {
    auto *obj = NativeObjectManager::Instance().Create(kIoFileGroup, "iofile");
    obj->SetInt(kFpKey, reinterpret_cast<int64_t>(fp));
    obj->SetBool(kPopenKey, is_popen);

    // ── file:read([format]) ──
    obj->RegisterMethod("read", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);

        // 无参数：默认 "*l"
        if (n < 1) {
            CVar fake_fmt{static_cast<int>(VarType::StringId)};
            fake_fmt.data_.i = state->GetConstString().Alloc("*l");
            return ReadOneFormat(fp, state, fake_fmt);
        }

        // 多格式参数：逐个读取，返回 multi-value
        if (n >= 2) {
            auto multi = inter::AllocMultiCVar(state, n);
            for (int i = 0; i < n; ++i) {
                CVar fmt_var = inter::GetNativeArg(state, args, n, i);
                CVar res = ReadOneFormat(fp, state, fmt_var);
                inter::SetMultiCVarElement(multi, i, res);
                if (res.type_ == static_cast<int>(VarType::Nil)) break;
            }
            return multi;
        }

        // 单格式参数
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        return ReadOneFormat(fp, state, a0);
    });

    // ── file:write(...) ──
    obj->RegisterMethod("write", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);

        for (int i = 0; i < n; i++) {
            CVar a = inter::GetNativeArg(state, args, n, i);
            std::string temp;
            std::string_view sv;
            if (a.type_ == static_cast<int>(VarType::String)) {
                sv = a.data_.s->Str();
            } else if (a.type_ == static_cast<int>(VarType::StringId)) {
                if (a.data_.i) {
                    const char *ptr = reinterpret_cast<const char *>(a.data_.i);
                    int sz = *reinterpret_cast<const int *>(ptr);
                    sv = {ptr + 8, static_cast<size_t>(sz)};
                }
            } else if (a.type_ == static_cast<int>(VarType::Int)) {
                temp = std::to_string(a.data_.i);
                sv = temp;
            } else if (a.type_ == static_cast<int>(VarType::Float)) {
                temp = std::to_string(a.data_.f);
                // 去掉末尾 0，保持最短表示
                while (temp.size() > 1 && temp.back() == '0') temp.pop_back();
                if (temp.size() > 1 && temp.back() == '.') temp.push_back('0');
                sv = temp;
            } else if (a.type_ == static_cast<int>(VarType::Bool)) {
                sv = a.data_.b ? "true" : "false";
            } else {
                continue;
            }
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
        // 不关闭 stdin/stdout/stderr
        if (fp == stdin || fp == stdout || fp == stderr) {
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
            whence_str = GetStringArgView(a0, temp_whence);
            if (whence_str.empty()) whence_str = "cur";
        }
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            offset = inter::CVarToInteger(a1, 0);
        }

        int whence;
        if (whence_str == "set") whence = SEEK_SET;
        else if (whence_str == "end")
            whence = SEEK_END;
        else
            whence = SEEK_CUR;

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
            mode = GetStringArgView(a0, temp_mode);
            if (mode.empty()) mode = "full";
        }
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            size = static_cast<size_t>(inter::CVarToInteger(a1, BUFSIZ));
        }

        int bufmode;
        if (mode == "no") bufmode = _IONBF;
        else if (mode == "line")
            bufmode = _IOLBF;
        else
            bufmode = _IOFBF;

        // 分配缓冲区（setvbuf 需要一块稳定的内存直到下次 setvbuf/fclose）
        // 对于无缓冲模式不需要缓冲区；全缓冲/行缓冲使用临时分配器
        char *buf = nullptr;
        if (bufmode != _IONBF) {
            auto &alloc = state->GetHeap().GetAllocator(false);
            buf = static_cast<char *>(alloc.Alloc(size > 0 ? size : BUFSIZ));
        }

        if (std::setvbuf(fp, buf, bufmode, size > 0 ? size : BUFSIZ) != 0) {
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
        return MakeFileLinesClosure(state, fp);
    });

    return obj;
}

// 将 CVar 参数转为字符串视图，用于 fwrite 等
static std::string_view ArgToStringView(CVar a, State *state, std::string &temp) {
    if (a.type_ == static_cast<int>(VarType::String)) {
        return a.data_.s->Str();
    } else if (a.type_ == static_cast<int>(VarType::StringId)) {
        if (a.data_.i) {
            const char *ptr = reinterpret_cast<const char *>(a.data_.i);
            int sz = *reinterpret_cast<const int *>(ptr);
            return {ptr + 8, static_cast<size_t>(sz)};
        }
        return {};
    } else if (a.type_ == static_cast<int>(VarType::Int)) {
        temp = std::to_string(a.data_.i);
        return temp;
    } else if (a.type_ == static_cast<int>(VarType::Float)) {
        temp = std::to_string(a.data_.f);
        while (temp.size() > 1 && temp.back() == '0') temp.pop_back();
        if (temp.size() > 1 && temp.back() == '.') temp.push_back('0');
        return temp;
    } else if (a.type_ == static_cast<int>(VarType::Bool)) {
        return a.data_.b ? "true" : "false";
    } else if (a.type_ == static_cast<int>(VarType::Nil)) {
        return "nil";
    }
    return {};
}

// ─────────────────────────────────────────────────────────────────────────────
// io 库注册
// ─────────────────────────────────────────────────────────────────────────────

void RegisterIoLibraryApi(State *s) {
    if (!s) return;

    // 确保 io 文件对象的 group 已创建
    NativeObjectManager::Instance().CreateGroup(kIoFileGroup);

    // ─── io.open(filename [, mode]) → file | nil, err ───
    RegisterNativeFunction(s, "io.open", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar fn_arg = inter::GetNativeArg(state, args, n, 0);
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
            if (a1.type_ == static_cast<int>(VarType::String) || a1.type_ == static_cast<int>(VarType::StringId)) {
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
            return inter::NativeToFakeluaNil(state);
        }
        auto *fp = reinterpret_cast<FILE *>(obj->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaBool(state, true);
        if (fp == stdin || fp == stdout || fp == stderr) {
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
            return ReadOneFormat(stdin, state, fake_fmt);
        }
        // 多格式参数：逐个读取，返回 multi-value
        if (n >= 2) {
            auto multi = inter::AllocMultiCVar(state, n);
            for (int i = 0; i < n; ++i) {
                CVar fmt_var = inter::GetNativeArg(state, args, n, i);
                CVar res = ReadOneFormat(stdin, state, fmt_var);
                inter::SetMultiCVarElement(multi, i, res);
            }
            return multi;
        }
        // 单格式参数
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        return ReadOneFormat(stdin, state, a0);
    });

    // ─── io.write(...) → true ───
    // 写入 stdout
    RegisterNativeFunction(s, "io.write", 0, true, [](State *state, CVar *args, int n) -> CVar {
        std::string temp;
        for (int i = 0; i < n; i++) {
            CVar a = inter::GetNativeArg(state, args, n, i);
            std::string_view sv = ArgToStringView(a, state, temp);
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
        std::string cmd_str;
        if (cmd_arg.type_ == static_cast<int>(VarType::String) || cmd_arg.type_ == static_cast<int>(VarType::StringId)) {
            cmd_str = std::string(KeyToStringView(cmd_arg));
        } else if (cmd_arg.type_ != static_cast<int>(VarType::Nil)) {
            cmd_str = AsVar(cmd_arg).ToString(/*has_quote=*/false, /*has_postfix=*/false);
        }
        std::string_view command = cmd_str;
        if (command.empty()) {
            auto multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, "missing command"));
            return multi;
        }
        std::string mode = "r";
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ == static_cast<int>(VarType::String) || a1.type_ == static_cast<int>(VarType::StringId)) {
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
            static NativeObject *stdin_obj = nullptr;
            if (!stdin_obj) stdin_obj = MakeIoFile(state, stdin);
            return inter::NativeToFakeluaNativeObject(state, stdin_obj);
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
            static NativeObject *stdout_obj = nullptr;
            if (!stdout_obj) stdout_obj = MakeIoFile(state, stdout);
            return inter::NativeToFakeluaNativeObject(state, stdout_obj);
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
        std::string fn_str;
        if (fn_arg.type_ == static_cast<int>(VarType::String) || fn_arg.type_ == static_cast<int>(VarType::StringId)) {
            fn_str = std::string(KeyToStringView(fn_arg));
        } else if (fn_arg.type_ != static_cast<int>(VarType::Nil)) {
            fn_str = AsVar(fn_arg).ToString(/*has_quote=*/false, /*has_postfix=*/false);
        }
        std::string_view filename = fn_str;
        if (filename.empty()) return inter::NativeToFakeluaNil(state);
        FILE *fp = std::fopen(std::string(filename).c_str(), "r");
        if (!fp) return inter::NativeToFakeluaNil(state);
        return MakeFileLinesClosure(state, fp);
    });

    // ─── io.stdin / io.stdout / io.stderr 文件对象 ───
    // 以全局变量形式注册（使用静态对象，跨调用保持有效）
    {
        static NativeObject *obj_in = MakeIoFile(s, stdin);
        static NativeObject *obj_out = MakeIoFile(s, stdout);
        static NativeObject *obj_err = MakeIoFile(s, stderr);
        RegisterNativeFunction(s, "io.stdin", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar { return inter::NativeToFakeluaNativeObject(state, obj_in); });
        RegisterNativeFunction(s, "io.stdout", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar { return inter::NativeToFakeluaNativeObject(state, obj_out); });
        RegisterNativeFunction(s, "io.stderr", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar { return inter::NativeToFakeluaNativeObject(state, obj_err); });
    }
}

}// namespace fakelua
