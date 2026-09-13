#pragma once

// compress_zlib.h — zlib deflate/inflate and gzip compress/decompress.
// Wraps zlib (deflate with zlib header, and gzip format).

#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::compress {

// zlib raw deflate (with zlib header)
// level: 1-9 (default 6). Returns zlib-wrapped deflate data.
std::vector<uint8_t> ZlibCompress(const uint8_t *data, size_t len, int level = 6);

// Inflate zlib-wrapped data.
std::vector<uint8_t> ZlibDecompress(const uint8_t *data, size_t len);

// gzip format
// level: 1-9 (default 6). Returns gzip-formatted data.
std::vector<uint8_t> GzipCompress(const uint8_t *data, size_t len, int level = 6);

// Decompress gzip-formatted data.
std::vector<uint8_t> GzipDecompress(const uint8_t *data, size_t len);

}// namespace fakelua::compress
