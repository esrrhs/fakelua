#pragma once

// compress_lz4.h — LZ4 frame format compression/decompression.
// Uses LZ4 frame API (lz4frame.h) which embeds content size for self-describing output.

#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::compress {

// LZ4 frame compress. Returns compressed data with frame header (embeds original size).
std::vector<uint8_t> Lz4Compress(const uint8_t *data, size_t len);

// LZ4 frame decompress. Extracts original size from frame header.
std::vector<uint8_t> Lz4Decompress(const uint8_t *data, size_t len);

}// namespace fakelua::compress
