#include "common.h"

#include <atomic>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <fstream>
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fakelua {

std::string GetTmpDirectory() {
    boost::system::error_code ec;
    auto base = boost::filesystem::temp_directory_path(ec);
    if (ec) {
        base = boost::filesystem::path(".");
    }
    auto tmpdir = base / "fakelua";
    boost::filesystem::create_directories(tmpdir, ec);
#if !defined(_WIN32)
    struct stat st{};
    if (ec || ::lstat(tmpdir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode) || st.st_uid != ::getuid()) {
        tmpdir = base / ("fakelua-" + std::to_string(static_cast<unsigned long>(::getuid())));
        ec.clear();
        (void) ::mkdir(tmpdir.c_str(), 0700);
    }
#endif
    return tmpdir.string();
}

std::string GenerateTmpFilename(const std::string &head, const std::string &tail) {
    const boost::filesystem::path tmpdir(GetTmpDirectory());
    static std::atomic<uint64_t> seq{0};
#if defined(_WIN32)
    const int pid = _getpid();
#else
    const int pid = getpid();
#endif
    const auto count = seq.fetch_add(1, std::memory_order_relaxed);
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();

    std::string fileName;
    uint64_t sub_seq = 0;
    do {
        std::string suffix = std::to_string(pid) + "_" + std::to_string(count);
        if (sub_seq > 0) {
            suffix += "_" + std::to_string(sub_seq);
        } else {
            suffix += "_" + std::to_string(now % 1000000);
        }
        fileName = (tmpdir / (head + suffix + tail)).string();
        ++sub_seq;
    } while (std::ifstream(fileName));
    return fileName;
}

}// namespace fakelua
