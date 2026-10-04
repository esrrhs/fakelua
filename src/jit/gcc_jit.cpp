#include "jit/gcc_jit.h"
#include "jit/gcc_cache.h"
#include "jit/gcc_handle.h"
#include "state/state.h"
#include "util/file_util.h"
#include "util/logging.h"
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#include <process.h>
#include <windows.h>
#else
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fakelua {

namespace {
constexpr size_t kCExtLen = 2;// 文件扩展名 ".c" 的长度
constexpr int kExecFailedStatus = 127;

// JIT 产物的动态库扩展名。
constexpr const char *kSoSuffix =
#if defined(_WIN32)
        ".dll";
#elif defined(__APPLE__)
        ".dylib";
#else
        ".so";
#endif

std::string JoinCommand(const std::vector<std::string> &args) {
    std::ostringstream oss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) {
            oss << ' ';
        }
        oss << args[i];
    }
    return oss.str();
}

#if defined(_WIN32)
std::string WinErrToString(const DWORD err) {
    if (err == 0) {
        return "unknown error";
    }
    LPSTR msg = nullptr;
    const DWORD len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                     reinterpret_cast<LPSTR>(&msg), 0, nullptr);
    if (len == 0 || !msg) {
        return std::format("error code {}", err);
    }
    std::string ret(msg, len);
    LocalFree(msg);
    return ret;
}

std::vector<std::string> GetWindowsDefaultLibraryPaths() {
    std::vector<std::string> ret;
    HMODULE module = nullptr;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(&GetWindowsDefaultLibraryPaths), &module) || !module) {
        return ret;
    }
    char module_path[MAX_PATH] = {0};
    if (const DWORD len = GetModuleFileNameA(module, module_path, MAX_PATH); len == 0 || len >= MAX_PATH) {
        return ret;
    }
    const std::filesystem::path dll_path(module_path);
    const auto dll_dir = dll_path.parent_path();
    ret.emplace_back(dll_dir.string());
    if (dll_dir.has_parent_path()) {
        ret.emplace_back((dll_dir.parent_path() / "lib").string());
    }
    return ret;
}
#endif
}// namespace

// 产物加载完成后的统一收尾：注入 State → 注册函数 → 调用 init。
//
// 单独抽出是因为缓存命中与正常编译两条路径都要走这一步，
// 放在一处可以避免两条路径的行为漂移。
void GccJitter::RegisterAndInit(const ParseResult &pr, const GenResult &gr,
                                const std::shared_ptr<JITHandle> &handle,
                                const std::function<void *(const std::string &)> &dlsym_lambda,
                                const std::string &artifact_path) {
    // 生成代码里 _S 初值为 0，必须在 init（以及任何函数调用）之前注入。
    void *set_state_ptr = dlsym_lambda(kSetStateFunctionName);
    if (!set_state_ptr) {
        ThrowFakeluaException(std::format("GCC compile failed, symbol resolution failed for {} in {}", kSetStateFunctionName, artifact_path));
    }
    reinterpret_cast<void (*)(void *)>(set_state_ptr)(s_);

    // 注册常量字符串 ID（生成代码把它们存在静态变量里）。
    // 只有用到字符串表键时才会生成该函数，此时必须有。
    if (void *const_init_ptr = dlsym_lambda(kConstInitFunctionName)) {
        reinterpret_cast<void (*)()>(const_init_ptr)();
    }

    LOG_DEBUG(s_, "engine", "GCC: registering {} functions from {}", gr.function_names.size(), artifact_path);
    for (const auto &[name, info]: gr.function_names) {
        const std::string &sym = info.c_symbol_name.empty() ? name : info.c_symbol_name;
        void *func_ptr = dlsym_lambda(sym);
        if (!func_ptr) {
            ThrowFakeluaException(std::format("GCC compile failed, symbol resolution failed for {} in {}", name, artifact_path));
        }
        s_->GetVM().RegisterFunction(VmFunction(name, info.params_count, JIT_GCC, func_ptr, handle, info.is_vararg));
        LOG_DEBUG(s_, "engine", "Registered gcc function {} with {} params (vararg: {}) at address {}", name, info.params_count, info.is_vararg, func_ptr);
    }

    void *init_ptr = dlsym_lambda(kInitFunctionName);
    if (init_ptr) {
        State::ConstAllocScope const_alloc(s_);
        inter::DispatchCall(s_, init_ptr, nullptr, 0, JIT_GCC);
    }
}

GccJitter::GccJitter(State *s) : s_(s) {
}

void GccJitter::Compile(const ParseResult &pr, const GenResult &gr, const CompileConfig &cfg) {
    LOG_INFO(s_, "engine", "GCC JIT compile start: {}, {} functions", pr.file_name, gr.function_names.size());

    const auto &gcc_cfg = s_->GetStateConfig().gcc_config;
    const bool use_cache = gcc_cfg.enable_compile_cache;

    std::string c_file;
    std::string so_file;
    std::string log_file;
    // 命中缓存时为 true：此时产物归缓存目录所有，析构时不能删。
    bool artifact_owned_by_cache = false;
    // 复用已有产物，跳过写 .c 与 gcc 调用。
    bool reuse_cached_artifact = false;

    std::string cache_key;
    std::string cache_so_file;

    std::vector<std::string> args;
    args.emplace_back("gcc");
    args.emplace_back("-x");
    args.emplace_back("c");
    args.emplace_back("-shared");
#if !defined(_WIN32)
    args.emplace_back("-fPIC");
#endif
#if defined(__APPLE__)
    // macOS 的 .dylib 需要 -undefined dynamic_lookup 选项，
    // 允许运行时由宿主进程提供未解析的符号。
    args.emplace_back("-undefined");
    args.emplace_back("dynamic_lookup");
#endif
    args.emplace_back(cfg.debug_mode ? "-O0" : "-O3");
    args.emplace_back("-DFAKELUA_JIT_TYPE=" + std::to_string(static_cast<int>(JIT_GCC)));
    for (const auto &path: s_->GetStateConfig().gcc_config.include_paths) {
        args.emplace_back("-I" + path);
    }
#if defined(_WIN32)
    for (const auto &path: GetWindowsDefaultLibraryPaths()) {
        args.emplace_back("-L" + path);
    }
    args.emplace_back("-lfakelua");
#endif
    for (const auto &path: s_->GetStateConfig().gcc_config.library_paths) {
        args.emplace_back("-L" + path);
    }
    for (const auto &lib: s_->GetStateConfig().gcc_config.libraries) {
        args.emplace_back("-l" + lib);
    }

    // ---- 编译产物缓存 ----
    //
    // 缓存键包含生成的 C 代码与全部影响产物的编译参数，因此不同 debug_mode /
    // 不同 include / library 配置会各自落到不同的缓存条目，不会互相污染。
    //
    // 命中时复用同一路径的 .so，从而命中 dyld 的路径缓存（macOS 上首次加载一个新
    // 路径需要约 4.5~5.4 秒走代码签名验证，同路径后续约 0.2 毫秒）。
    if (use_cache) {
        cache_key = ComputeGccCacheKey(gr.c_code, JoinCommand(args));
        cache_so_file = GccCacheArtifactPath(cache_key, kSoSuffix);
        if (!cache_so_file.empty() && std::filesystem::exists(cache_so_file)) {
            reuse_cached_artifact = true;
        }
    }

    if (reuse_cached_artifact) {
        so_file = cache_so_file;
        c_file.clear();
        log_file.clear();
        // 缓存文件归缓存目录所有，GCCHandle 析构时不得删除，否则缓存永不生效。
        artifact_owned_by_cache = true;
        LOG_DEBUG(s_, "engine", "GCC: cache hit for {} -> {}", pr.file_name, cache_key);
    } else {
        c_file = GenerateTmpFilename("fakelua_jit_", ".c");
        log_file = c_file.substr(0, c_file.size() - kCExtLen) + ".gcc.log";
        so_file = c_file.substr(0, c_file.size() - kCExtLen) + kSoSuffix;

        if (std::ofstream ofs(c_file); !ofs.is_open()) {
            ThrowFakeluaException(std::format("GCC compile failed, cannot open c file {}", c_file));
        } else {
            ofs << gr.c_code;
            ofs.close();
        }

        if (use_cache) {
            // 编译成功后把产物放进缓存，下次同样输入即可命中。
            // 写入采用「先临时文件再 rename」的原子替换，避免并发进程读到半截文件。
            artifact_owned_by_cache = true;
        }
    }

    // RAII cleanup for temp files on early error paths (before GCCHandle takes ownership).
    // 缓存命中时不删任何东西：产物归缓存目录所有，临时文件本来也没创建。
    const auto CleanupOnError = [&]() {
        std::error_code ec;
        if (!c_file.empty()) {
            std::filesystem::remove(c_file, ec);
        }
        if (!so_file.empty() && !reuse_cached_artifact) {
            std::filesystem::remove(so_file, ec);
        }
        if (!log_file.empty()) {
            std::filesystem::remove(log_file, ec);
        }
    };

    args.emplace_back("-o");
    args.emplace_back(so_file);
    args.emplace_back(c_file);

    // Helper: build null-terminated argv from std::vector<std::string>.
    auto build_argv = [&]() {
        std::vector<char *> argv;
        argv.reserve(args.size() + 1);
        for (auto &arg: args) {
            argv.emplace_back(arg.data());
        }
        argv.emplace_back(nullptr);
        return argv;
    };

    // 编译成功后把产物原子地放进缓存目录。失败时静默忽略 —— 缓存只是优化，
    // 不该让编译本身失败。
    const auto PublishToCache = [&]() {
        if (!artifact_owned_by_cache || cache_so_file.empty()) {
            return;
        }
        std::error_code ec;
        std::filesystem::rename(so_file, cache_so_file, ec);
        if (ec) {
            // 跨设备等场景下 rename 可能失败，退化为拷贝后删除。
            ec.clear();
            std::filesystem::copy_file(so_file, cache_so_file, std::filesystem::copy_options::overwrite_existing, ec);
            if (!ec) {
                std::filesystem::remove(so_file, ec);
                ec.clear();
            }
        }
        if (ec) {
            // 缓存写入失败则本次仍按普通临时文件处理，避免产物被误删。
            artifact_owned_by_cache = false;
            LOG_DEBUG(s_, "engine", "GCC: cache write failed for {}: {}", pr.file_name, ec.message());
        }
    };

#if defined(_WIN32)
    auto argv = build_argv();
    if (!reuse_cached_artifact) {
        const int compile_status = _spawnvp(_P_WAIT, "gcc", argv.data());
        if (compile_status == -1) {
            CleanupOnError();
            ThrowFakeluaException(std::format("GCC compile failed for {}: cannot execute gcc (errno {}: {}). cmd: {}", pr.file_name, errno, std::strerror(errno), JoinCommand(args)));
        }
        if (compile_status != 0) {
            CleanupOnError();
            ThrowFakeluaException(std::format("GCC compile failed for {} with exit code {}. cmd: {}", pr.file_name, compile_status, JoinCommand(args)));
        }
        PublishToCache();
    }

    // 产物若已进缓存，实际路径变成了 cache_so_file。
    const std::string load_path = artifact_owned_by_cache ? cache_so_file : so_file;

    HMODULE module_handle = LoadLibraryA(load_path.c_str());
    if (!module_handle) {
        CleanupOnError();
        ThrowFakeluaException(std::format("GCC compile failed, LoadLibrary failed for {}: {}", load_path, WinErrToString(GetLastError())));
    }
    // 产物归缓存所有时传空路径，GCCHandle 析构便不会删除缓存文件。
    const auto handle = std::make_shared<GCCHandle>(
            artifact_owned_by_cache ? std::string{} : c_file,
            artifact_owned_by_cache ? std::string{} : so_file,
            artifact_owned_by_cache ? std::string{} : log_file,
            module_handle);

    // Common post-load: register compiled functions + call init.
    auto dlsym_lambda = [&](const std::string &sym) -> void * {
        FARPROC p = GetProcAddress(module_handle, sym.c_str());
        return p ? reinterpret_cast<void *>(p) : nullptr;
    };
#else
    auto argv = build_argv();

    // 缓存命中：直接加载已缓存的产物，跳过 gcc 调用。
    // 若加载失败（文件被清理或损坏），删除该条目、退回未命中路径重新编译，
    // 而不是直接失败——缓存只是优化，不该成为故障源。
    if (reuse_cached_artifact) {
        void *cached = dlopen(so_file.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (cached) {
            const auto handle = std::make_shared<GCCHandle>(std::string{}, std::string{}, std::string{}, cached);
            auto dlsym_cached = [&](const std::string &sym) -> void * { return dlsym(cached, sym.c_str()); };
            RegisterAndInit(pr, gr, handle, dlsym_cached, so_file);
            return;
        }
        LOG_DEBUG(s_, "engine", "GCC: stale cache entry {}, recompiling", cache_key);
        std::error_code rm_ec;
        std::filesystem::remove(so_file, rm_ec);
        reuse_cached_artifact = false;
        artifact_owned_by_cache = true;
        c_file = GenerateTmpFilename("fakelua_jit_", ".c");
        log_file = c_file.substr(0, c_file.size() - kCExtLen) + ".gcc.log";
        so_file = c_file.substr(0, c_file.size() - kCExtLen) + kSoSuffix;
        if (std::ofstream ofs(c_file); ofs.is_open()) {
            ofs << gr.c_code;
        }
        args[args.size() - 2] = so_file;
        args[args.size() - 1] = c_file;
        argv = build_argv();
    }

    // 未命中（或缓存失效回退）：走完整编译流程。
    {
        const int log_fd = open(log_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (log_fd < 0) {
            CleanupOnError();
            ThrowFakeluaException(std::format("GCC compile failed, cannot open log file {}", log_file));
        }
    // Block SIGCHLD before fork to prevent the default handler from turning
    // the child into a zombie before we get to waitpid.
    sigset_t old_mask, block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &block_mask, &old_mask);

    const pid_t pid = fork();
    if (pid < 0) {
        pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
        close(log_fd);
        CleanupOnError();
        ThrowFakeluaException(std::format("GCC compile failed, fork failed for {}", pr.file_name));
    }

    if (pid == 0) {
        // Restore default signal mask in child before exec.
        pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
        dup2(log_fd, STDOUT_FILENO);
        dup2(log_fd, STDERR_FILENO);
        close(log_fd);
        execvp("gcc", argv.data());
        _exit(kExecFailedStatus);
    }
    close(log_fd);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
        CleanupOnError();
        ThrowFakeluaException(std::format("GCC compile failed, waitpid failed for {}", pr.file_name));
    }
    pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        std::string gcc_log;
        if (std::ifstream ifs(log_file); ifs.is_open()) {
            gcc_log.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        }
        CleanupOnError();
        ThrowFakeluaException(std::format("GCC compile failed for {} (log: {})\n{}", pr.file_name, log_file, gcc_log));
    }

    PublishToCache();

    // 产物若已进缓存，实际路径变成了 cache_so_file。
    const std::string load_path = artifact_owned_by_cache ? cache_so_file : so_file;

    void *dl_handle = dlopen(load_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!dl_handle) {
        CleanupOnError();
        ThrowFakeluaException(std::format("GCC compile failed, dlopen failed for {}: {}", load_path, dlerror()));
    }

    // 产物归缓存所有时传空路径，GCCHandle 析构便不会删除缓存文件。
    const auto handle = std::make_shared<GCCHandle>(
            artifact_owned_by_cache ? std::string{} : c_file,
            artifact_owned_by_cache ? std::string{} : so_file,
            artifact_owned_by_cache ? std::string{} : log_file,
            dl_handle);

    auto dlsym_lambda = [&](const std::string &sym) -> void * { return dlsym(dl_handle, sym.c_str()); };

    RegisterAndInit(pr, gr, handle, dlsym_lambda, artifact_owned_by_cache ? cache_so_file : so_file);
    }// 编译 + 加载
#endif

#if !defined(_WIN32)
    LOG_INFO(s_, "engine", "GCC JIT compilation finished for {}", pr.file_name);
#endif
}

}// namespace fakelua
