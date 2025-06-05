#include "file_util.h"
#include "util/common.h"

namespace fakelua {

std::string generate_tmp_filename(const std::string &head, const std::string &tail) {
    auto tmpdir = std::filesystem::temp_directory_path();
    tmpdir += std::filesystem::path::preferred_separator;
    tmpdir += "fakelua";
    tmpdir += std::filesystem::path::preferred_separator;
    if (!std::filesystem::exists(tmpdir)) {
        std::filesystem::create_directories(tmpdir);
    }
    tmpdir += head;

    static std::mt19937 rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<int> dist(100000, 999999);

    // create tmp file in system temp dir
    std::string fileName;
    do {
        fileName = tmpdir.string() + std::to_string(dist(rng)) + tail;
    } while (std::ifstream(fileName));
    return fileName;
}

}// namespace fakelua
