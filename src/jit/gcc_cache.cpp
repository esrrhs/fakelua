#include "jit/gcc_cache.h"
#include "jit_build_identity.h"
#include "util/file_util.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>
#include <mutex>
#include <openssl/sha.h>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <vector>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fakelua {
namespace {
namespace fs = std::filesystem;

#if defined(_WIN32)
constexpr DWORD kShareAll = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

class WinHandle {
public:
    HANDLE h = INVALID_HANDLE_VALUE;
    WinHandle() = default;
    explicit WinHandle(HANDLE handle) : h(handle) {}
    ~WinHandle() { reset(); }
    WinHandle(const WinHandle &) = delete;
    WinHandle &operator=(const WinHandle &) = delete;
    WinHandle(WinHandle &&other) noexcept : h(other.h) { other.h = INVALID_HANDLE_VALUE; }
    void reset() {
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
        h = INVALID_HANDLE_VALUE;
    }
    explicit operator bool() const { return h != INVALID_HANDLE_VALUE; }
};

// One-pass POSIX unlink: never retries, and does not block on a share-delete open handle.
bool UnlinkPath(const fs::path &p) {
    WinHandle handle(CreateFileW(p.wstring().c_str(), DELETE, kShareAll, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
    if (!handle) {
        const auto err = GetLastError();
        return err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND;
    }
    struct DispositionEx {
        DWORD Flags;
    } posix{0x00000001 | 0x00000002};// FILE_DISPOSITION_FLAG_DELETE | POSIX_SEMANTICS
    if (SetFileInformationByHandle(handle.h, static_cast<FILE_INFO_BY_HANDLE_CLASS>(21), &posix, sizeof(posix))) return true;
    FILE_DISPOSITION_INFO info{};
    info.DeleteFile = TRUE;
    return SetFileInformationByHandle(handle.h, FileDispositionInfo, &info, sizeof(info));
}

void RemoveTree(const fs::path &p, std::error_code &ec) {
    WIN32_FILE_ATTRIBUTE_DATA attrs{};
    if (!GetFileAttributesExW(p.wstring().c_str(), GetFileExInfoStandard, &attrs)) {
        const auto err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) ec.clear();
        else
            ec.assign(err, std::system_category());
        return;
    }
    if (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        WIN32_FIND_DATAW data{};
        HANDLE find = FindFirstFileW((p / "*").wstring().c_str(), &data);
        if (find != INVALID_HANDLE_VALUE) {
            do {
                if (data.cFileName[0] == L'.' && (data.cFileName[1] == L'\0' || (data.cFileName[1] == L'.' && data.cFileName[2] == L'\0'))) continue;
                std::error_code child;
                RemoveTree(p / data.cFileName, child);
            } while (FindNextFileW(find, &data));
            FindClose(find);
        }
    }
    if (UnlinkPath(p)) {
        ec.clear();
        return;
    }
    const auto err = GetLastError();
    if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) ec.clear();
    else
        ec.assign(err, std::system_category());
}

std::string ReadBytes(const fs::path &p) {
    WinHandle handle(CreateFileW(p.wstring().c_str(), GENERIC_READ, kShareAll, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!handle) throw std::runtime_error("cache read");
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(handle.h, &size) || size.QuadPart < 0) throw std::runtime_error("cache size");
    std::string data(static_cast<size_t>(size.QuadPart), '\0');
    size_t offset = 0;
    while (offset < data.size()) {
        DWORD chunk = 0;
        const DWORD want = static_cast<DWORD>(std::min(data.size() - offset, size_t(1 << 20)));
        if (!ReadFile(handle.h, data.data() + offset, want, &chunk, nullptr) || !chunk) throw std::runtime_error("cache read");
        offset += chunk;
    }
    return data;
}

void WriteBytes(const fs::path &p, const std::string &data) {
    WinHandle handle(CreateFileW(p.wstring().c_str(), GENERIC_WRITE, kShareAll, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!handle) throw std::runtime_error("cache write");
    size_t offset = 0;
    while (offset < data.size()) {
        DWORD chunk = 0;
        const DWORD want = static_cast<DWORD>(std::min(data.size() - offset, size_t(1 << 20)));
        if (!WriteFile(handle.h, data.data() + offset, want, &chunk, nullptr) || !chunk) throw std::runtime_error("cache write");
        offset += chunk;
    }
    if (!FlushFileBuffers(handle.h)) throw std::runtime_error("cache write");
}
#else
std::string ReadBytes(const fs::path &p) {
    std::ifstream in(p, std::ios::binary | std::ios::ate);
    if (!in) throw std::runtime_error("cache read");
    const auto size = in.tellg();
    if (size < 0) throw std::runtime_error("cache size");
    std::string data(static_cast<size_t>(size), '\0');
    in.seekg(0);
    if (!data.empty() && !in.read(data.data(), static_cast<std::streamsize>(data.size()))) throw std::runtime_error("cache read");
    return data;
}

void WriteBytes(const fs::path &p, const std::string &data) {
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
    out.close();
    if (!out) throw std::runtime_error("cache write");
}

void RemoveTree(const fs::path &p, std::error_code &ec) { fs::remove_all(p, ec); }
#endif

std::string Digest(const std::string &data) {
    std::array<unsigned char, SHA256_DIGEST_LENGTH> hash{};
    if (!SHA256(reinterpret_cast<const unsigned char *>(data.data()), data.size(), hash.data())) throw std::runtime_error("cache digest failed");
    std::ostringstream out;
    for (auto c: hash) out << std::hex << std::setw(2) << std::setfill('0') << unsigned(c);
    return out.str();
}

// libc++ file_clock ticks may use __int128, which has no ostream operator.
template<typename T>
void AppendInteger(std::ostream &out, T value) {
    char buf[80];
    char *p = buf + sizeof(buf);
    *--p = '\0';
    const bool negative = value < T{0};
    do {
        T digit = value % 10;
        if (digit < T{0}) digit = -digit;
        *--p = static_cast<char>('0' + static_cast<int>(digit));
        value /= 10;
    } while (value != T{0});
    if (negative) *--p = '-';
    out << p;
}

std::string FileStamp(const fs::path &path) {
    std::ostringstream stamp;
#if defined(_WIN32)
    // MinGW last_write_time often comes from stat() and is only 1-second
    // resolution, so a same-size rewrite in the same second would reuse a
    // stale FileDigest. Use the 100ns FILETIME plus volume/file identity.
    WIN32_FILE_ATTRIBUTE_DATA info{};
    if (!GetFileAttributesExW(path.wstring().c_str(), GetFileExInfoStandard, &info)) throw std::runtime_error("cache dependency stat");
    const auto size = (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 0ull : ((static_cast<unsigned long long>(info.nFileSizeHigh) << 32) | info.nFileSizeLow);
    stamp << size << ':';
    const auto filetime = [](const FILETIME &t) { return (static_cast<unsigned long long>(t.dwHighDateTime) << 32) | t.dwLowDateTime; };
    AppendInteger(stamp, filetime(info.ftLastWriteTime));
    stamp << ':';
    AppendInteger(stamp, filetime(info.ftCreationTime));
    BY_HANDLE_FILE_INFORMATION ident{};
    HANDLE handle = CreateFileW(path.wstring().c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (handle != INVALID_HANDLE_VALUE) {
        if (GetFileInformationByHandle(handle, &ident)) {
            stamp << ':' << ident.dwVolumeSerialNumber << ':' << ident.nFileIndexHigh << ':' << ident.nFileIndexLow;
        }
        CloseHandle(handle);
    }
#else
    stamp << (fs::is_directory(path) ? 0 : fs::file_size(path)) << ':';
    AppendInteger(stamp, fs::last_write_time(path).time_since_epoch().count());

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
    struct Entry {
        std::string stamp, digest;
    };

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
    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = write_pipe;
    startup.hStdError = write_pipe;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION process{};
    std::string command = "\"" + compiler.string() + "\" " + option;
    BOOL ok = CreateProcessA(compiler.string().c_str(), command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
    CloseHandle(write_pipe);
    if (!ok) {
        CloseHandle(read_pipe);
        throw std::runtime_error("cache compiler query");
    }
    DWORD size = 0;
    while (ReadFile(read_pipe, buf.data(), static_cast<DWORD>(buf.size()), &size, nullptr) && size) output.append(buf.data(), size);
    CloseHandle(read_pipe);
    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD status = 1;
    GetExitCodeProcess(process.hProcess, &status);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (status) throw std::runtime_error("cache compiler query");
#else
    int pipes[2];
    if (pipe(pipes)) throw std::runtime_error("cache pipe");
    fcntl(pipes[0], F_SETFD, FD_CLOEXEC);
    fcntl(pipes[1], F_SETFD, FD_CLOEXEC);
    auto exe = compiler.string();
    pid_t child = fork();
    if (child == 0) {
        dup2(pipes[1], STDOUT_FILENO);
        // Metadata switches produce tiny output, never generated code.
        close(pipes[0]);
        close(pipes[1]);
        execl(exe.c_str(), exe.c_str(), option, static_cast<char *>(nullptr));
        _exit(127);
    }
    close(pipes[1]);
    if (child < 0) {
        close(pipes[0]);
        throw std::runtime_error("cache fork");
    }
    ssize_t size;
    while ((size = read(pipes[0], buf.data(), buf.size())) != 0) {
        if (size < 0) {
            if (errno == EINTR) continue;
            break;
        }
        output.append(buf.data(), static_cast<size_t>(size));
    }
    close(pipes[0]);
    int status = 0;
    pid_t waited;
    do {
        waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited < 0 || !WIFEXITED(status) || WEXITSTATUS(status)) throw std::runtime_error("cache compiler query");
#endif
    while (!output.empty() && std::isspace(static_cast<unsigned char>(output.back()))) output.pop_back();
    return output;
}

std::vector<fs::path> ToolchainFiles(const fs::path &compiler) {
    std::vector<fs::path> files{compiler};
    for (const char *option: {"-print-prog-name=cc1", "-print-prog-name=as", "-print-prog-name=ld", "-print-file-name=specs", "-print-libgcc-file-name", "-print-file-name=crtbeginS.o",
                              "-print-file-name=crtendS.o", "-print-file-name=crti.o", "-print-file-name=crtn.o"}) {
        auto result = QueryCompiler(compiler, option);
        if (result.empty()) throw std::runtime_error("cache compiler identity");
        if (fs::is_regular_file(result)) files.push_back(fs::absolute(result));
        else if (std::string_view(option).starts_with("-print-prog-name=") && !fs::path(result).has_parent_path() && result != "cc1")
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
        if (it->second.expired()) it = locks.erase(it);
        else
            ++it;
    }
    auto &weak = locks[key];
    auto ret = weak.lock();
    if (!ret) weak = ret = std::make_shared<std::mutex>();
    return ret;
}

// Fixed lock stripes bound synchronization metadata independently of entry count.
// Lock files are never unlinked by eviction; key -> index is the only lock order.
class CacheFileLock {
    fs::path path_;
    std::shared_ptr<std::mutex> mutex_;
    std::unique_lock<std::mutex> thread_lock_;
#if defined(_WIN32)
    HANDLE handle_ = INVALID_HANDLE_VALUE;
#else
    int fd_ = -1;
#endif
public:
    explicit CacheFileLock(const fs::path &path) : path_(path), mutex_(KeyMutex(path.string())), thread_lock_(*mutex_) {
#if defined(_WIN32)
        handle_ = CreateFileW(path.wstring().c_str(), GENERIC_READ | GENERIC_WRITE, kShareAll, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle_ == INVALID_HANDLE_VALUE) throw std::runtime_error("cache lock open");
        OVERLAPPED overlap{};
        if (!LockFileEx(handle_, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &overlap)) {
            CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
            throw std::runtime_error("cache lock");
        }
#else
        fd_ = open(path.c_str(), O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (fd_ < 0) throw std::runtime_error("cache lock open");
        int status;
        do {
            status = flock(fd_, LOCK_EX);
        } while (status < 0 && errno == EINTR);
        if (status < 0) {
            close(fd_);
            fd_ = -1;
            throw std::runtime_error("cache lock");
        }
#endif
    }

    ~CacheFileLock() {
#if defined(_WIN32)
        if (handle_ != INVALID_HANDLE_VALUE) CloseHandle(handle_);
#else
        if (fd_ >= 0) close(fd_);
#endif
    }

    bool StillLinked() const {
#if defined(_WIN32)
        // Share-delete opens allow an external cleaner to unlink the lock name.
        if (handle_ == INVALID_HANDLE_VALUE) return false;
        FILE_STANDARD_INFO standard{};
        if (GetFileInformationByHandleEx(handle_, FileStandardInfo, &standard, sizeof(standard)) && (standard.DeletePending || standard.NumberOfLinks == 0)) return false;
        BY_HANDLE_FILE_INFORMATION held{};
        if (!GetFileInformationByHandle(handle_, &held)) return false;
        WinHandle named(CreateFileW(path_.wstring().c_str(), 0, kShareAll, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
        if (!named) return false;
        BY_HANDLE_FILE_INFORMATION info{};
        return GetFileInformationByHandle(named.h, &info) && held.dwVolumeSerialNumber == info.dwVolumeSerialNumber && held.nFileIndexHigh == info.nFileIndexHigh && held.nFileIndexLow == info.nFileIndexLow;
#else
        struct stat held{}, named{};

        return fstat(fd_, &held) == 0 && lstat(path_.c_str(), &named) == 0 && held.st_dev == named.st_dev && held.st_ino == named.st_ino;
#endif
    }
};

bool IsCacheKey(const std::string &name) {
    return name.size() == 64 && name.find_first_not_of("0123456789abcdef") == std::string::npos;
}

struct CacheEntry {
    fs::path path;
    uint64_t order = 0;
};

std::vector<CacheEntry> CompleteEntries(const fs::path &root) {
    std::vector<CacheEntry> result;
    for (const auto &item: fs::directory_iterator(root)) {
        if (!IsCacheKey(item.path().filename().string()) || !fs::is_directory(item.symlink_status())) continue;
        if (!fs::is_regular_file(fs::symlink_status(item.path() / "image")) || !fs::is_regular_file(fs::symlink_status(item.path() / "manifest"))) continue;
        uint64_t order = 0;
        std::ifstream(item.path() / "published") >> order;// pre-FIFO entries are oldest
        result.push_back({item.path(), order});
    }
    std::sort(result.begin(), result.end(), [](const auto &a, const auto &b) { return a.order != b.order ? a.order < b.order : a.path < b.path; });
    return result;
}

void TrimEntries(const fs::path &root, size_t limit) {
    if (!limit) return;// zero explicitly means unlimited
    auto entries = CompleteEntries(root);
    auto count = entries.size();
    for (const auto &entry: entries) {
        if (count <= limit) break;
        std::error_code ec;
        RemoveTree(entry.path, ec);
        if (!ec) --count;// permission/cleanup races are best effort, never Lua errors
    }
}

fs::path CacheRoot(const StateGCCConfig &cfg) {
    fs::path root = cfg.cache_directory;
    if (root.empty()) {
        root = fs::path(GetTmpDirectory()) / "cache";
#if !defined(_WIN32)
        // System temporary directories may be shared by several accounts.
        root /= "uid-" + std::to_string(getuid());
#endif
    }
    root = fs::absolute(root);
#if !defined(_WIN32)
    // Never execute artifacts in a shared/writable-by-others cache. mkdir with
    // 0700 from the outset, and reject symlinks instead of chmod'ing user paths.
    fs::create_directories(root.parent_path());
    // A private leaf is insufficient if another account can replace it through
    // a writable/foreign-owned ancestor. Resolve platform symlinks (/var on
    // macOS), then trust only our account/root and sticky shared temp parents.
    for (auto parent = fs::canonical(root.parent_path());; parent = parent.parent_path()) {
        struct stat ancestor{};

        if (stat(parent.c_str(), &ancestor) || (ancestor.st_uid != getuid() && ancestor.st_uid != 0) || ((ancestor.st_mode & 0022) && !(ancestor.st_mode & S_ISVTX)))
            throw std::runtime_error("cache parent directory is not trusted");
        if (parent == parent.root_path()) break;
    }
    if (::mkdir(root.c_str(), 0700) != 0 && errno != EEXIST) throw std::runtime_error("cache mkdir");

    struct stat st{};

    if (lstat(root.c_str(), &st) || !S_ISDIR(st.st_mode) || st.st_uid != getuid() || (st.st_mode & 0077)) throw std::runtime_error("cache directory must be private");
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
            if (next == '\n') {
                ++i;
                continue;
            }
            if (next == '\r' && i + 2 < text.size() && text[i + 2] == '\n') {
                i += 2;
                continue;
            }
            if (next == ' ' || next == '\t' || next == '\\' || next == '#' || next == ':') {
                token += next;
                ++i;
                continue;
            }
        }
        if (c == '$' && i + 1 < text.size() && text[i + 1] == '$') ++i;
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!token.empty()) {
                result.emplace_back(token);
                token.clear();
            }
        } else
            token += c;
    }
    if (result.empty()) throw std::runtime_error("cache empty dependencies");
    return result;
}
}// namespace

struct GccCache::Impl {
    fs::path entry;
    fs::path compiler;
    size_t max_entries = 256;
    std::unique_ptr<CacheFileLock> key_lock;

    CacheFileLock IndexLock() const {
        if (!key_lock->StillLinked()) throw std::runtime_error("cache lock removed");
        return CacheFileLock(entry.parent_path() / "index.lock");
    }
};

GccCache::~GccCache() = default;

void GccCache::Release() {
    impl_.reset();
}

GccCache::GccCache(const StateGCCConfig &cfg, const std::string &compiler, const std::string &code, bool debug) {
    // Arbitrary user headers/libraries (and coverage files) cannot be reliably
    // fingerprinted as a closed toolchain. Retain the uncached behavior there.
#if !defined(USE_COV)
    if (!cfg.enable_cache || !cfg.include_paths.empty() || !cfg.library_paths.empty() || !cfg.libraries.empty()) return;
    // Search-path overrides can add a newly shadowing header/library without
    // changing any previously recorded dependency. Such open-ended toolchains
    // deliberately retain the uncached path.
    for (auto name: {"CPATH", "C_INCLUDE_PATH", "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX", "CCC_OVERRIDE_OPTIONS", "CLANG_CONFIG_FILE_SYSTEM_DIR", "CLANG_CONFIG_FILE_USER_DIR"})
        if (auto value = std::getenv(name); value && *value) return;
    try {
        auto root = CacheRoot(cfg);
        auto cc = CompilerPath(compiler);
        const auto driver_name = fs::canonical(cc).filename().string();
        if (driver_name.find("gcc") == std::string::npos && driver_name.find("clang") == std::string::npos) return;
        // Do not persist arbitrary wrapper scripts with hidden inputs.
        {
            std::ifstream driver(cc, std::ios::binary);
            char magic[2]{};
            driver.read(magic, 2);
            if (!driver || (magic[0] == '#' && magic[1] == '!')) return;
        }
        std::ostringstream key;
        key << "fakelua-gcc-cache-v1\n"
            << HostIdentity() << '\n'
            << debug << '\n'
            << sizeof(void *) << '\n'
            << compiler << '\n'
            << cc.string() << '\n'
            << FileDigest(cc)
            << '\n'
            // Apple /usr/bin/clang can be a stable shim for a newly selected
            // Xcode toolchain. Version/InstalledDir must be part of the key.
            << QueryCompiler(cc, "--version") << '\n';
        // Environment affecting compiler search, preprocessing, linking, SDK and
        // reproducibility. Each value is length-delimited to avoid ambiguity.
        for (auto name: {"PATH", "CPATH", "C_INCLUDE_PATH", "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX", "SDKROOT", "MACOSX_DEPLOYMENT_TARGET", "DEVELOPER_DIR", "SOURCE_DATE_EPOCH", "LANG",
                         "LC_ALL", "LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH"}) {
            const char *value = std::getenv(name);
            key << name << ':' << (value ? std::string(value).size() : 0) << ':' << (value ? value : "") << '\n';
        }
        if (auto sdk = std::getenv("SDKROOT"); sdk && *sdk && fs::exists(sdk)) key << "SDKROOT-real:" << fs::canonical(sdk).string() << '\n';
        key << fs::current_path().string() << '\n' << code;
        auto hash = Digest(key.str());
        auto p = std::make_unique<Impl>();
        p->entry = root / hash;
        p->compiler = cc;
        p->max_entries = cfg.cache_max_entries;
        p->key_lock = std::make_unique<CacheFileLock>(root / ("key-" + hash.substr(0, 2) + ".lock"));
        impl_ = std::move(p);
    } catch (...) { /* Caching is an optimization, never a compile requirement. */
    }
#endif
}

bool GccCache::Restore(const std::string &destination) {
    if (!impl_) return false;
    try {
        auto index_lock = impl_->IndexLock();
        if (!index_lock.StillLinked()) return false;
        TrimEntries(impl_->entry.parent_path(), impl_->max_entries);
        std::istringstream meta(ReadBytes(impl_->entry / "manifest"));
        std::string binary_hash, path, hash;
        if (!(meta >> binary_hash) || binary_hash.size() != 64) throw std::runtime_error("cache manifest");
        size_t count = 0;
        if (!(meta >> count) || count == 0 || count > 10000) throw std::runtime_error("cache dependency count");
        for (size_t i = 0; i < count; ++i) {
            if (!(meta >> std::quoted(path) >> hash)) throw std::runtime_error("cache dependency");
            const auto actual = hash.starts_with("directory:") ? "directory:" + Digest(FileStamp(path)) : FileDigest(path);
            if (actual != hash) throw std::runtime_error("cache stale dependency");
        }
        std::string bytes = ReadBytes(impl_->entry / "image");
        if (Digest(bytes) != binary_hash) throw std::runtime_error("cache corrupt image");
        // A byte copy, not a hardlink: dlopen deduplicates mapped inodes and would
        // otherwise share _S, init flags, Lua locations and module globals.
        WriteBytes(destination, bytes);
        return true;
    } catch (...) {
        Invalidate();
        return false;
    }
}

void GccCache::Invalidate() {
    if (!impl_) return;
    try {
        auto index_lock = impl_->IndexLock();
        if (!index_lock.StillLinked()) return;
        std::error_code ec;
        RemoveTree(impl_->entry, ec);
    } catch (...) { /* An externally removed cache is just a miss. */
    }
}

void GccCache::Publish(const std::string &binary, const std::string &dependencies, const std::string &source) {
    if (!impl_) return;
    fs::path staging;
    try {
        auto bytes = ReadBytes(binary);
        std::map<std::string, std::string> deps;
        auto src = fs::canonical(source);
        // The source itself is already hashed as part of the cache key.
        for (auto p: Dependencies(ReadBytes(dependencies))) {
            p = fs::absolute(p).lexically_normal();
            if (!fs::equivalent(p, src)) deps.emplace(p.string(), FileDigest(p));
        }
        for (const auto &p: ToolchainFiles(impl_->compiler)) {
            deps[p.string()] = FileDigest(p);
            // Also detect added/removed default specs/config/startup files.
            const auto parent = p.parent_path();
            deps[parent.string()] = "directory:" + Digest(FileStamp(parent));
        }
        if (deps.empty()) return;
        std::ostringstream manifest;
        manifest << Digest(bytes) << '\n' << deps.size() << '\n';
        for (const auto &[p, h]: deps) manifest << std::quoted(p) << ' ' << h << '\n';
        auto index_lock = impl_->IndexLock();
        if (!index_lock.StillLinked()) return;
        const auto root = impl_->entry.parent_path();
        auto entries = CompleteEntries(root);
        uint64_t order = entries.empty() ? 1 : entries.back().order + 1;
        if (!order) {
            // Recover a corrupt/overflowed age without poisoning future writes.
            order = 1;
            for (const auto &entry: entries) WriteBytes(entry.path / "published", std::to_string(order++));
        }
        // Each writer owns its staging directory, even if an external cleaner
        // unlinks a lock inode and another process acquires a replacement lock.
        staging = impl_->entry.string() + ".tmp." + fs::path(GenerateTmpFilename("", "")).filename().string();
        std::error_code stale;
        RemoveTree(impl_->entry.string() + ".tmp", stale);// old-format abandoned staging only
        if (!fs::create_directory(staging)) return;
        WriteBytes(staging / "image", bytes);
        WriteBytes(staging / "manifest", manifest.str());
        WriteBytes(staging / "published", std::to_string(order));
        if (!index_lock.StillLinked() || !impl_->key_lock->StillLinked()) {
            RemoveTree(staging, stale);
            return;
        }
        RemoveTree(impl_->entry, stale);
        fs::rename(staging, impl_->entry);// only complete, validated images become visible
        TrimEntries(root, impl_->max_entries);
    } catch (...) {
        std::error_code ec;
        if (!staging.empty()) RemoveTree(staging, ec);
    }
}
}// namespace fakelua
