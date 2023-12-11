#include "file_util.h"
#include "util/common.h"

namespace fakelua {

std::string generate_tmp_filename(const std::string &head, const std::string &tail) {
    auto tmpdir = std::filesystem::temp_directory_path();
    tmpdir += std::filesystem::path::preferred_separator;
    tmpdir += head;
    // create tmp file in system temp dir
    std::string fileName;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    do {
        fileName = tmpdir.string() + std::to_string(std::rand()) + tail;
    } while (std::ifstream(fileName));
    return fileName;
}

}// namespace fakelua
