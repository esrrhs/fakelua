#include "common.h"

namespace fakelua {

std::string GenerateTmpFilename(const std::string &head, const std::string &tail) {
    auto tmpdir = std::filesystem::temp_directory_path();
    tmpdir += std::filesystem::path::preferred_separator;
    tmpdir += "fakelua";
    tmpdir += std::filesystem::path::preferred_separator;
    if (!std::filesystem::exists(tmpdir)) {
        std::filesystem::create_directories(tmpdir);
    }
    tmpdir += head;

    // 局部的：编译一个脚本只走几次，没必要为它留一份长期状态。种子用 random_device 而不是
    // 时钟，免得同时启动的两个线程拿到同一个序列、生成同名文件。
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(100000, 999999);

    // 在系统临时目录中创建临时文件
    std::string fileName;
    do {
        fileName = tmpdir.string() + std::to_string(dist(rng)) + tail;
    } while (std::ifstream(fileName));
    return fileName;
}

}// namespace fakelua
