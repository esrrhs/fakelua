#include "native/native_io.h"
#include "native/native_object.h"
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
static constexpr int64_t kIoFileGroup = 999999;// 专用 group，0 不允许

// 创建一个 IoFile NativeObject 壳，内部 FILE* 存为 Int 字段
static NativeObject *MakeIoFile(State *s, FILE *fp) {
    auto *obj = NativeObjectManager::Instance().Create(kIoFileGroup, "iofile");
    obj->SetInt(kFpKey, reinterpret_cast<int64_t>(fp));

    // ── file:read([format]) ──
    obj->RegisterMethod("read", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);

        // 默认 "*l"：读一行（去掉换行）
        if (n < 1) {
            std::string result;
            char buf[4096];
            bool got_any = false;
            while (std::fgets(buf, sizeof(buf), fp)) {
                result += buf;
                got_any = true;
                // 如果读到换行说明一行结束
                if (!result.empty() && result.back() == '\n') break;
            }
            if (!got_any) return inter::NativeToFakeluaNil(state);
            while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();
            return inter::NativeToFakeluaString(state, result);
        }

        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::String) || a0.type_ == static_cast<int>(VarType::StringId)) {
            std::string_view fmt = KeyToStringView(a0);
            if (fmt == "*l") {
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
            } else if (fmt == "*L") {
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
            } else if (fmt == "*a") {
                std::string result;
                char chunk[4096];
                size_t nread;
                while ((nread = std::fread(chunk, 1, sizeof(chunk), fp)) > 0) {
                    result.append(chunk, nread);
                }
                if (result.empty()) return inter::NativeToFakeluaNil(state);
                return inter::NativeToFakeluaString(state, result);
            } else if (fmt == "*n") {
                double val;
                if (std::fscanf(fp, "%lf", &val) == 1) {
                    if (val == static_cast<int64_t>(val) && std::isfinite(val)) {
                        return inter::NativeToFakeluaLonglong(state, static_cast<long long>(val));
                    }
                    return inter::NativeToFakeluaDouble(state, val);
                }
                return inter::NativeToFakeluaNil(state);
            }
            // 未知格式返回 nil
            return inter::NativeToFakeluaNil(state);
        } else if (a0.type_ == static_cast<int>(VarType::Int) || a0.type_ == static_cast<int>(VarType::Float)) {
            // 数字参数：读取指定字节数
            int64_t count = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : static_cast<int64_t>(a0.data_.f);
            if (count <= 0) return inter::NativeToFakeluaStringView(state, std::string_view(""));
            std::string result(static_cast<size_t>(count), '\0');
            size_t nread = std::fread(result.data(), 1, static_cast<size_t>(count), fp);
            result.resize(nread);
            if (nread == 0) return inter::NativeToFakeluaNil(state);
            return inter::NativeToFakeluaString(state, result);
        }
        return inter::NativeToFakeluaNil(state);
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
        int ret = std::fclose(fp);
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
        int64_t offset = 0;

        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            if (a0.type_ == static_cast<int>(VarType::String) || a0.type_ == static_cast<int>(VarType::StringId)) {
                whence_str = KeyToStringView(a0);
            }
        }
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ == static_cast<int>(VarType::Int)) offset = a1.data_.i;
            else if (a1.type_ == static_cast<int>(VarType::Float)) offset = static_cast<int64_t>(a1.data_.f);
        }

        int whence;
        if (whence_str == "set") whence = SEEK_SET;
        else if (whence_str == "end") whence = SEEK_END;
        else whence = SEEK_CUR;

        if (std::fseek(fp, static_cast<long>(offset), whence) != 0) {
            return inter::NativeToFakeluaNil(state);
        }
        long pos = std::ftell(fp);
        return inter::NativeToFakeluaLonglong(state, static_cast<long long>(pos));
    });

    // ── file:setvbuf(mode [, size]) ──（简化实现，仅标记）──
    obj->RegisterMethod("setvbuf", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        auto *fp = reinterpret_cast<FILE *>(self->GetInt(kFpKey, 0));
        if (!fp) return inter::NativeToFakeluaNil(state);

        std::string_view mode = "full";
        size_t size = BUFSIZ;
        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            if (a0.type_ == static_cast<int>(VarType::String) || a0.type_ == static_cast<int>(VarType::StringId)) {
                mode = KeyToStringView(a0);
            }
        }
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            if (a1.type_ == static_cast<int>(VarType::Int)) size = static_cast<size_t>(a1.data_.i);
            else if (a1.type_ == static_cast<int>(VarType::Float)) size = static_cast<size_t>(a1.data_.f);
        }

        int bufmode;
        if (mode == "no") bufmode = _IONBF;
        else if (mode == "line") bufmode = _IOLBF;
        else bufmode = _IOFBF;

        if (std::setvbuf(fp, nullptr, bufmode, size) != 0) {
            return inter::NativeToFakeluaNil(state);
        }
        return inter::NativeToFakeluaBool(state, true);
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
        std::string_view filename = KeyToStringView(inter::GetNativeArg(state, args, n, 0));
        if (filename.empty()) {
            auto multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, "missing filename"));
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
            auto multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaNil(state));
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(state, std::strerror(errno)));
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
        int ret = std::fclose(fp);
        obj->SetInt(kFpKey, 0);
        return ret == 0 ? inter::NativeToFakeluaBool(state, true) : inter::NativeToFakeluaNil(state);
    });

    // ─── io.read([format]) → string|number|nil ───
    // 从 stdin 读取
    RegisterNativeFunction(s, "io.read", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) {
            // 默认 "*l"：循环读取直到换行或 EOF，支持超长行
            std::string result;
            char buf[4096];
            bool got_any = false;
            while (std::fgets(buf, sizeof(buf), stdin)) {
                result += buf;
                got_any = true;
                if (!result.empty() && result.back() == '\n') break;
            }
            if (!got_any) return inter::NativeToFakeluaNil(state);
            while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();
            return inter::NativeToFakeluaString(state, result);
        }
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::String) || a0.type_ == static_cast<int>(VarType::StringId)) {
            std::string_view fmt = KeyToStringView(a0);
            if (fmt == "*l") {
                std::string result;
                char buf[4096];
                bool got_any = false;
                while (std::fgets(buf, sizeof(buf), stdin)) {
                    result += buf;
                    got_any = true;
                    if (!result.empty() && result.back() == '\n') break;
                }
                if (!got_any) return inter::NativeToFakeluaNil(state);
                while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();
                return inter::NativeToFakeluaString(state, result);
            } else if (fmt == "*L") {
                std::string result;
                char buf[4096];
                bool got_any = false;
                while (std::fgets(buf, sizeof(buf), stdin)) {
                    result += buf;
                    got_any = true;
                    if (!result.empty() && result.back() == '\n') break;
                }
                if (!got_any) return inter::NativeToFakeluaNil(state);
                return inter::NativeToFakeluaString(state, result);
            } else if (fmt == "*a") {
                std::string result;
                char chunk[4096];
                size_t nread;
                while ((nread = std::fread(chunk, 1, sizeof(chunk), stdin)) > 0) {
                    result.append(chunk, nread);
                }
                return result.empty() ? inter::NativeToFakeluaNil(state) : inter::NativeToFakeluaString(state, result);
            } else if (fmt == "*n") {
                double val;
                if (std::fscanf(stdin, "%lf", &val) == 1) {
                    if (val == static_cast<int64_t>(val) && std::isfinite(val)) {
                        return inter::NativeToFakeluaLonglong(state, static_cast<long long>(val));
                    }
                    return inter::NativeToFakeluaDouble(state, val);
                }
                return inter::NativeToFakeluaNil(state);
            }
        } else if (a0.type_ == static_cast<int>(VarType::Int) || a0.type_ == static_cast<int>(VarType::Float)) {
            int64_t count = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : static_cast<int64_t>(a0.data_.f);
            if (count <= 0) return inter::NativeToFakeluaStringView(state, std::string_view(""));
            std::string result(static_cast<size_t>(count), '\0');
            size_t nread = std::fread(result.data(), 1, static_cast<size_t>(count), stdin);
            result.resize(nread);
            if (nread == 0) return inter::NativeToFakeluaNil(state);
            return inter::NativeToFakeluaString(state, result);
        }
        return inter::NativeToFakeluaNil(state);
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
            return fp ? inter::NativeToFakeluaStringView(state, std::string_view("file"))
                      : inter::NativeToFakeluaStringView(state, std::string_view("closed file"));
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
        std::string_view filename = KeyToStringView(inter::GetNativeArg(state, args, n, 0));
        if (filename.empty()) return inter::NativeToFakeluaNil(state);
        FILE *fp = std::fopen(std::string(filename).c_str(), "r");
        if (!fp) return inter::NativeToFakeluaNil(state);
        auto *obj = MakeIoFile(state, fp);
        return inter::NativeToFakeluaNativeObject(state, obj);
    });

    // ─── io.stdin / io.stdout / io.stderr 文件对象 ───
    // 以全局变量形式注册（使用静态对象，跨调用保持有效）
    {
        static NativeObject *obj_in = MakeIoFile(s, stdin);
        static NativeObject *obj_out = MakeIoFile(s, stdout);
        static NativeObject *obj_err = MakeIoFile(s, stderr);
        RegisterNativeFunction(s, "io.stdin", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
            return inter::NativeToFakeluaNativeObject(state, obj_in);
        });
        RegisterNativeFunction(s, "io.stdout", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
            return inter::NativeToFakeluaNativeObject(state, obj_out);
        });
        RegisterNativeFunction(s, "io.stderr", 0, false, [](State *state, CVar * /*args*/, int /*n*/) -> CVar {
            return inter::NativeToFakeluaNativeObject(state, obj_err);
        });
    }
}

}  // namespace fakelua
