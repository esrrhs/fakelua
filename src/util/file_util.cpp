#include "common.h"

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <fstream>

namespace fakelua {

std::string GenerateTmpFilename(const std::string &head, const std::string &tail) {
    boost::system::error_code ec;
    auto tmpdir = boost::filesystem::temp_directory_path(ec);
    if (ec) {
        tmpdir = boost::filesystem::path(".");
    }
    tmpdir /= "fakelua";
    boost::filesystem::create_directories(tmpdir, ec);

    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(100000, 999999);

    std::string fileName;
    do {
        fileName = (tmpdir / (head + std::to_string(dist(rng)) + tail)).string();
    } while (std::ifstream(fileName));
    return fileName;
}

}// namespace fakelua
