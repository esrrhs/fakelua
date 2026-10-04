#include "jit/gcc_cache.h"

#include <boost/filesystem.hpp>
#include <boost/hash2/xxhash.hpp>
#include <boost/system/error_code.hpp>
#include <array>
#include <cstdio>
#include <fstream>
#include <string>

namespace fakelua {

namespace {

// 缓存格式版本。参与摘要计算，便于将来改变布局时让旧缓存自然失效。
constexpr int kCacheFormatVersion = 1;

std::string ToHex(std::uint64_t v) {
    static const char* kDigits = "0123456789abcdef";
    std::string out(16, '0');
    for (int i = 15; i >= 0; i--) {
        out[static_cast<size_t>(i)] = kDigits[v & 0xF];
        v >>= 4;
    }
    return out;
}

}// namespace

std::string ComputeGccCacheKey(std::string_view c_code, std::string_view compile_flags) {
    boost::hash2::xxhash_64 hasher;

    // 长度前缀，避免 "ab"+"c" 与 "a"+"bc" 撞成同一个键。
    const auto mix = [&hasher](std::string_view s) {
        const auto len = static_cast<std::uint64_t>(s.size());
        hasher.update(reinterpret_cast<const unsigned char *>(&len), sizeof(len));
        hasher.update(reinterpret_cast<const unsigned char *>(s.data()), s.size());
    };

    mix(std::to_string(kCacheFormatVersion));
    mix(c_code);
    // 编译参数（-O0/-O3、include/library 路径等）不同则产物不同，必须进键。
    mix(compile_flags);

    return ToHex(hasher.result());
}

std::string GccCacheDir() {
    boost::system::error_code ec;
    auto dir = boost::filesystem::temp_directory_path(ec);
    if (ec) {
        dir = boost::filesystem::path(".");
    }
    // 刻意不放在 <tmp>/fakelua/ 下：那个目录属于 GenerateTmpFilename 的临时产物，
    // 测试（如 runtime.generate_tmp_filename_creates_dir）会 remove_all 整个目录，
    // 连带把编译缓存一起清掉。独立命名可避免这种互相干扰。
    dir /= "fakelua-jit-cache";
    boost::filesystem::create_directories(dir, ec);
    if (ec && !boost::filesystem::is_directory(dir)) {
        return {};
    }
    return dir.string();
}

std::string GccCacheArtifactPath(std::string_view key, std::string_view suffix) {
    const std::string dir = GccCacheDir();
    if (dir.empty()) {
        return {};
    }
    return (boost::filesystem::path(dir) / (std::string(key) + std::string(suffix))).string();
}

}// namespace fakelua
