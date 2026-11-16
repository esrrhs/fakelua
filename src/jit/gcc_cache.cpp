#include "jit/gcc_cache.h"
#include "jit_build_identity.h"
#include <openssl/sha.h>
#include <array>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <string_view>
#include <mutex>
#include <sstream>
#include <vector>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace fakelua {
namespace {
namespace fs = std::filesystem;
std::string ReadBytes(const fs::path &p) {
    std::ifstream in(p, std::ios::binary | std::ios::ate);
    if (!in) throw std::runtime_error("cache read");
    const auto size = in.tellg();
    if (size < 0) throw std::runtime_error("cache size");
    std::string data(static_cast<size_t>(size), '\0');
    in.seekg(0);
    if (!data.empty() && !in.read(data.data(), static_cast<std::streamsize>(data.size())))
        throw std::runtime_error("cache read");
    return data;
}
std::string Digest(const std::string &data) {
    std::array<unsigned char, SHA256_DIGEST_LENGTH> hash{};
    if (!SHA256(reinterpret_cast<const unsigned char *>(data.data()), data.size(), hash.data()))
        throw std::runtime_error("cache digest failed");
    std::ostringstream out;
    for (auto c : hash) out << std::hex << std::setw(2) << std::setfill('0') << unsigned(c);
    return out.str();
}
std::string FileStamp(const fs::path &path) {
    std::ostringstream stamp;
    stamp << (fs::is_directory(path) ? 0 : fs::file_size(path)) << ':' << fs::last_write_time(path).time_since_epoch().count();
#if !defined(_WIN32)
    struct stat st{};
    if (stat(path.c_str(), &st)) throw std::runtime_error("cache dependency stat");
    stamp << ':' << st.st_dev << ':' << st.st_ino << ':' << st.st_ctime;
#if defined(__APPLE__)
    stamp << ':' << st.st_ctimespec.tv_nsec;
#else
    stamp << ':' << st.st_ctim.tv_nsec;
#endif
#endif
    return stamp.str();
}
std::string FileDigest(const fs::path &path) {
    struct Entry { std::string stamp, digest; };
    static std::mutex mutex;
    static std::map<std::string, Entry> known;
    std::lock_guard lock(mutex);
    const auto stamp = FileStamp(path);
    auto &entry = known[path.string()];
    if (entry.stamp != stamp) {
        auto digest = Digest(ReadBytes(path));
        if (FileStamp(path) != stamp) throw std::runtime_error("cache changing dependency");
        entry = {stamp, std::move(digest)};
    }
    return entry.digest;
}
void WriteBytes(const fs::path &p, const std::string &data) {
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
    out.close();
    if (!out) throw std::runtime_error("cache write");
}
fs::path CompilerPath(const std::string &compiler) {
    if (fs::path(compiler).has_parent_path()) return fs::absolute(compiler);
    const char *env = std::getenv("PATH");
    std::istringstream paths(env ? env : "");
    std::string part;
#if defined(_WIN32)
    constexpr char sep = ';';
#else
    constexpr char sep = ':';
#endif
    while (std::getline(paths, part, sep)) {
        fs::path p = fs::path(part) / compiler;
#if defined(_WIN32)
        if (!p.has_extension()) p += ".exe";
#endif
        if (fs::is_regular_file(p)) return fs::absolute(p);
    }
    throw std::runtime_error("cache compiler not found");
}
// Query only compiler metadata, without a shell or process-global stdio changes.
std::string QueryCompiler(const fs::path &compiler, const char *option) {
    std::string output;
    std::array<char, 4096> buf{};
#if defined(_WIN32)
    SECURITY_ATTRIBUTES sa{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE read_pipe = nullptr, write_pipe = nullptr;
    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) throw std::runtime_error("cache pipe");
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOA startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = write_pipe; startup.hStdError = write_pipe; startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION process{};
    std::string command = "\"" + compiler.string() + "\" " + option;
    BOOL ok = CreateProcessA(compiler.string().c_str(), command.data(), nullptr, nullptr, TRUE,
                            CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
    CloseHandle(write_pipe);
    if (!ok) { CloseHandle(read_pipe); throw std::runtime_error("cache compiler query"); }
    DWORD size = 0;
    while (ReadFile(read_pipe, buf.data(), static_cast<DWORD>(buf.size()), &size, nullptr) && size)
        output.append(buf.data(), size);
    CloseHandle(read_pipe);
    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD status = 1; GetExitCodeProcess(process.hProcess, &status);
    CloseHandle(process.hThread); CloseHandle(process.hProcess);
    if (status) throw std::runtime_error("cache compiler query");
#else
    int pipes[2];
    if (pipe(pipes)) throw std::runtime_error("cache pipe");
    fcntl(pipes[0], F_SETFD, FD_CLOEXEC); fcntl(pipes[1], F_SETFD, FD_CLOEXEC);
    auto exe = compiler.string();
    pid_t child = fork();
    if (child == 0) {
        dup2(pipes[1], STDOUT_FILENO);
        // Metadata switches produce tiny output, never generated code.
        close(pipes[0]); close(pipes[1]);
        execl(exe.c_str(), exe.c_str(), option, static_cast<char *>(nullptr));
        _exit(127);
    }
    close(pipes[1]);
    if (child < 0) { close(pipes[0]); throw std::runtime_error("cache fork"); }
    ssize_t size;
    while ((size = read(pipes[0], buf.data(), buf.size())) != 0) {
        if (size < 0) { if (errno == EINTR) continue; break; }
        output.append(buf.data(), static_cast<size_t>(size));
    }
    close(pipes[0]);
    int status = 0; pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    if (waited < 0 || !WIFEXITED(status) || WEXITSTATUS(status)) throw std::runtime_error("cache compiler query");
#endif
    while (!output.empty() && std::isspace(static_cast<unsigned char>(output.back()))) output.pop_back();
    return output;
}
std::vector<fs::path> ToolchainFiles(const fs::path &compiler) {
    std::vector<fs::path> files{compiler};
    for (const char *option : {"-print-prog-name=cc1", "-print-prog-name=as", "-print-prog-name=ld",
                              "-print-file-name=specs", "-print-libgcc-file-name",
                              "-print-file-name=crtbeginS.o", "-print-file-name=crtendS.o",
                              "-print-file-name=crti.o", "-print-file-name=crtn.o"}) {
        auto result = QueryCompiler(compiler, option);
        if (result.empty()) throw std::runtime_error("cache compiler identity");
        if (fs::is_regular_file(result)) files.push_back(fs::absolute(result));
        else if (std::string_view(option).starts_with("-print-prog-name=") &&
                 !fs::path(result).has_parent_path() && result != "cc1")
            files.push_back(CompilerPath(result));
    }
    return files;
}
std::string HostIdentity() {
    // Avoid hashing a potentially huge debug-enabled host DLL on every process
    // startup. CMake hashes source contents and the ABI/build configuration.
    return FAKELUA_JIT_BUILD_ID ":" FAKELUA_JIT_BUILD_CONFIGURATION;
}
std::shared_ptr<std::mutex> KeyMutex(const std::string &key) {
    static std::mutex guard;
    static std::map<std::string, std::weak_ptr<std::mutex>> locks;
    std::lock_guard lock(guard);
    // Drop unused keys: long-lived services may compile arbitrarily many files.
    for (auto it = locks.begin(); it != locks.end();) {
        if (it->second.expired()) it = locks.erase(it); else ++it;
    }
    auto &weak = locks[key];
    auto ret = weak.lock();
    if (!ret) weak = ret = std::make_shared<std::mutex>();
    return ret;
}
fs::path CacheRoot(const StateGCCConfig &cfg) {
    fs::path root = cfg.cache_directory;
    if (root.empty()) {
#if defined(_WIN32)
        auto home = std::getenv("LOCALAPPDATA");
        if (!home || !*home) throw std::runtime_error("cache home");
        root = fs::path(home) / "fakelua" / "jit";
#else
        if (auto xdg = std::getenv("XDG_CACHE_HOME"); xdg && *xdg)
            root = fs::path(xdg) / "fakelua" / "jit";
        else if (auto home = std::getenv("HOME"); home && *home)
            root = fs::path(home) / ".cache" / "fakelua" / "jit";
        else
            root = fs::temp_directory_path() / ("fakelua-jit-" + std::to_string(getuid()));
#endif
    }
    root = fs::absolute(root);
#if !defined(_WIN32)
    // Never execute artifacts in a shared/writable-by-others cache. mkdir with
    // 0700 from the outset, and reject symlinks instead of chmod'ing user paths.
    fs::create_directories(root.parent_path());
    if (::mkdir(root.c_str(), 0700) != 0 && errno != EEXIST)
        throw std::runtime_error("cache mkdir");
    struct stat st{};
    if (lstat(root.c_str(), &st) || !S_ISDIR(st.st_mode) || st.st_uid != getuid() || (st.st_mode & 0077))
        throw std::runtime_error("cache directory must be private");
#else
    fs::create_directories(root);
#endif
    return root;
}
// GCC makefile dependency syntax: escaped spaces/backslashes and continuations.
std::vector<fs::path> Dependencies(const std::string &text) {
    std::vector<fs::path> result;
    // -MT flua has no drive-letter colon, even on Windows.
    auto colon = text.find(':');
    if (colon == std::string::npos) throw std::runtime_error("cache dependencies");
    std::string token;
    for (size_t i = colon + 1; i <= text.size(); ++i) {
        char c = i == text.size() ? ' ' : text[i];
        if (c == '\\' && i + 1 < text.size()) {
            char next = text[i + 1];
            if (next == '\n') { ++i; continue; }
            if (next == '\r' && i + 2 < text.size() && text[i + 2] == '\n') { i += 2; continue; }
            if (next == ' ' || next == '\t' || next == '\\' || next == '#' || next == ':') { token += next; ++i; continue; }
        }
        if (c == '$' && i + 1 < text.size() && text[i + 1] == '$') ++i;
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!token.empty()) { result.emplace_back(token); token.clear(); }
        } else token += c;
    }
    if (result.empty()) throw std::runtime_error("cache empty dependencies");
    return result;
}
} // namespace
struct GccCache::Impl {
    fs::path entry;
    fs::path compiler;
    std::shared_ptr<std::mutex> mutex;
    std::unique_lock<std::mutex> thread_lock;
#if defined(_WIN32)
    HANDLE lock_handle = INVALID_HANDLE_VALUE;
    ~Impl() { if (lock_handle != INVALID_HANDLE_VALUE) CloseHandle(lock_handle); }
#else
    int lock_fd = -1;
    ~Impl() { if (lock_fd >= 0) close(lock_fd); }
#endif
};
GccCache::~GccCache() = default;
void GccCache::Release() { impl_.reset(); }
GccCache::GccCache(const StateGCCConfig &cfg, const std::string &compiler,
                   const std::string &code, bool debug) {
    // Arbitrary user headers/libraries (and coverage files) cannot be reliably
    // fingerprinted as a closed toolchain. Retain the uncached behavior there.
#if !defined(USE_COV)
    if (!cfg.enable_cache || !cfg.include_paths.empty() || !cfg.library_paths.empty() || !cfg.libraries.empty()) return;
    // Search-path overrides can add a newly shadowing header/library without
    // changing any previously recorded dependency. Such open-ended toolchains
    // deliberately retain the uncached path.
    for (auto name : {"CPATH", "C_INCLUDE_PATH", "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX",
                      "CCC_OVERRIDE_OPTIONS", "CLANG_CONFIG_FILE_SYSTEM_DIR", "CLANG_CONFIG_FILE_USER_DIR"})
        if (auto value = std::getenv(name); value && *value) return;
    try {
        auto root = CacheRoot(cfg);
        auto cc = CompilerPath(compiler);
        const auto driver_name = fs::canonical(cc).filename().string();
        if (driver_name.find("gcc") == std::string::npos && driver_name.find("clang") == std::string::npos) return;
        // Do not persist arbitrary wrapper scripts with hidden inputs.
        { std::ifstream driver(cc, std::ios::binary); char magic[2]{}; driver.read(magic, 2);
          if (!driver || (magic[0] == '#' && magic[1] == '!')) return; }
        std::ostringstream key;
        key << "fakelua-gcc-cache-v1\n" << HostIdentity() << '\n' << debug << '\n'
            << sizeof(void *) << '\n' << compiler << '\n' << cc.string() << '\n' << FileDigest(cc) << '\n'
            // Apple /usr/bin/clang can be a stable shim for a newly selected
            // Xcode toolchain. Version/InstalledDir must be part of the key.
            << QueryCompiler(cc, "--version") << '\n';
        // Environment affecting compiler search, preprocessing, linking, SDK and
        // reproducibility. Each value is length-delimited to avoid ambiguity.
        for (auto name : {"PATH", "CPATH", "C_INCLUDE_PATH", "LIBRARY_PATH", "COMPILER_PATH",
                          "GCC_EXEC_PREFIX", "SDKROOT", "MACOSX_DEPLOYMENT_TARGET", "DEVELOPER_DIR",
                          "SOURCE_DATE_EPOCH", "LANG", "LC_ALL", "LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH"}) {
            const char *value = std::getenv(name);
            key << name << ':' << (value ? std::string(value).size() : 0) << ':' << (value ? value : "") << '\n';
        }
        if (auto sdk = std::getenv("SDKROOT"); sdk && *sdk && fs::exists(sdk))
            key << "SDKROOT-real:" << fs::canonical(sdk).string() << '\n';
        key << fs::current_path().string() << '\n' << code;
        auto hash = Digest(key.str());
        auto p = std::make_unique<Impl>();
        p->entry = root / hash;
        p->compiler = cc;
        p->mutex = KeyMutex(p->entry.string());
        p->thread_lock = std::unique_lock(*p->mutex);
        const auto lock_path = root / (hash + ".lock");
#if defined(_WIN32)
        p->lock_handle = CreateFileA(lock_path.string().c_str(), GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        if (p->lock_handle == INVALID_HANDLE_VALUE) return;
        OVERLAPPED overlap{};
        if (!LockFileEx(p->lock_handle, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &overlap)) return;
#else
        p->lock_fd = open(lock_path.c_str(), O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (p->lock_fd < 0) return;
        int status;
        do { status = flock(p->lock_fd, LOCK_EX); } while (status < 0 && errno == EINTR);
        if (status < 0) return;
#endif
        impl_ = std::move(p);
    } catch (...) { /* Caching is an optimization, never a compile requirement. */ }
#endif
}
bool GccCache::Restore(const std::string &destination) {
    if (!impl_) return false;
    try {
        std::istringstream meta(ReadBytes(impl_->entry / "manifest"));
        std::string binary_hash, path, hash;
        if (!(meta >> binary_hash) || binary_hash.size() != 64) throw std::runtime_error("cache manifest");
        size_t count = 0;
        if (!(meta >> count) || count == 0 || count > 10000) throw std::runtime_error("cache dependency count");
        for (size_t i = 0; i < count; ++i) {
            if (!(meta >> std::quoted(path) >> hash)) throw std::runtime_error("cache dependency");
            const auto actual = hash.starts_with("directory:")
                ? "directory:" + Digest(FileStamp(path)) : FileDigest(path);
            if (actual != hash) throw std::runtime_error("cache stale dependency");
        }
        std::string bytes = ReadBytes(impl_->entry / "image");
        if (Digest(bytes) != binary_hash) throw std::runtime_error("cache corrupt image");
        // A byte copy, not a hardlink: dlopen deduplicates mapped inodes and would
        // otherwise share _S, init flags, Lua locations and module globals.
        WriteBytes(destination, bytes);
        return true;
    } catch (...) { Invalidate(); return false; }
}
void GccCache::Invalidate() {
    if (!impl_) return;
    std::error_code ec;
    fs::remove_all(impl_->entry, ec);
}
void GccCache::Publish(const std::string &binary, const std::string &dependencies, const std::string &source) {
    if (!impl_) return;
    fs::path staging;
    try {
        auto bytes = ReadBytes(binary);
        std::map<std::string, std::string> deps;
        auto src = fs::canonical(source);
        // The source itself is already hashed as part of the cache key.
        for (auto p : Dependencies(ReadBytes(dependencies))) {
            p = fs::absolute(p).lexically_normal();
            if (!fs::equivalent(p, src)) deps.emplace(p.string(), FileDigest(p));
        }
        for (const auto &p : ToolchainFiles(impl_->compiler)) {
            deps[p.string()] = FileDigest(p);
            // Also detect added/removed default specs/config/startup files.
            const auto parent = p.parent_path();
            deps[parent.string()] = "directory:" + Digest(FileStamp(parent));
        }
        if (deps.empty()) return;
        std::ostringstream manifest;
        manifest << Digest(bytes) << '\n' << deps.size() << '\n';
        for (const auto &[p, h] : deps) manifest << std::quoted(p) << ' ' << h << '\n';
        // Same-filesystem rename publishes a complete entry. OS file locks are
        // released on crashes; abandoned staging dirs are harmless cache misses.
        staging = impl_->entry.string() + ".tmp";
        std::error_code stale; fs::remove_all(staging, stale);
        fs::create_directory(staging);
        WriteBytes(staging / "image", bytes);
        WriteBytes(staging / "manifest", manifest.str());
        Invalidate();
        fs::rename(staging, impl_->entry);
    } catch (...) {
        std::error_code ec;
        if (!staging.empty()) fs::remove_all(staging, ec);
    }
}
} // namespace fakelua
