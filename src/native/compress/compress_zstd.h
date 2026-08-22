#pragma once

// compress_zstd.h — Zstandard compression/decompression.
// Uses ZSTD simple API.

#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::compress {

// Zstd compress. level: 1-22, default 3.
std::vector<uint8_t> zstd_compress(const uint8_t *data, size_t len, int level = 3);

// Zstd decompress. Auto-detects original size from frame header.
std::vector<uint8_t> zstd_decompress(const uint8_t *data, size_t len);

}  // namespace fakelua::compress
