#include "common.h"

#include <atomic>
#include <chrono>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <fstream>
#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

namespace fakelua {

std::string GenerateTmpFilename(const std::string &head, const std::string &tail) {
    boost::system::error_code ec;
    auto tmpdir = boost::filesystem::temp_directory_path(ec);
    if (ec) {
        tmpdir = boost::filesystem::path(".");
    }
    tmpdir /= "fakelua";
    boost::filesystem::create_directories(tmpdir, ec);

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
