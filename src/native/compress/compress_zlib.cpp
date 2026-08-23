#include "compress_zlib.h"

#include <zlib.h>

#include <cstring>

namespace fakelua::compress {

// ── zlib raw deflate ──

std::vector<uint8_t> zlib_compress(const uint8_t *data, size_t len, int level) {
    if (level < 1) level = 1;
    if (level > 9) level = 9;

    uLongf dst_capacity = compressBound(static_cast<uLong>(len));
    std::vector<uint8_t> out(dst_capacity);

    int ret = compress2(out.data(), &dst_capacity, data, static_cast<uLong>(len), level);
    if (ret != Z_OK) return {};

    out.resize(dst_capacity);
    return out;
}

std::vector<uint8_t> zlib_decompress(const uint8_t *data, size_t len) {
    if (len == 0) return {};

    // Start with an estimate; zlib doesn't embed original size.
    // For highly compressible data, the ratio can be 100:1 or more,
    // so keep growing the buffer until decompression succeeds.
    uLongf dst_capacity = len * 4;
    if (dst_capacity < 4096) dst_capacity = 4096;

    std::vector<uint8_t> out(dst_capacity);

    int ret = uncompress(out.data(), &dst_capacity, data, static_cast<uLong>(len));
    while (ret == Z_BUF_ERROR && dst_capacity < (1ULL << 30)) {
        // Buffer too small, grow by 4x
        dst_capacity *= 4;
        out.resize(dst_capacity);
        ret = uncompress(out.data(), &dst_capacity, data, static_cast<uLong>(len));
    }
    if (ret != Z_OK) return {};

    out.resize(dst_capacity);
    return out;
}

// ── gzip format ──

std::vector<uint8_t> gzip_compress(const uint8_t *data, size_t len, int level) {
    if (level < 1) level = 1;
    if (level > 9) level = 9;

    z_stream zs{};
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;

    // windowBits = 15 + 16 = 31 → gzip format
    int ret = deflateInit2(&zs, level, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) return {};

    uLongf dst_capacity = deflateBound(&zs, static_cast<uLong>(len));
    std::vector<uint8_t> out(dst_capacity);

    zs.next_in = const_cast<Bytef *>(data);
    zs.avail_in = static_cast<uInt>(len);
    zs.next_out = out.data();
    zs.avail_out = static_cast<uInt>(dst_capacity);

    ret = deflate(&zs, Z_FINISH);
    deflateEnd(&zs);

    if (ret != Z_STREAM_END) return {};

    out.resize(zs.total_out);
    return out;
}

std::vector<uint8_t> gzip_decompress(const uint8_t *data, size_t len) {
    if (len == 0) return {};

    z_stream zs{};
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.next_in = const_cast<Bytef *>(data);
    zs.avail_in = static_cast<uInt>(len);

    // windowBits = 15 + 16 = 31 → auto-detect gzip format
    int ret = inflateInit2(&zs, 15 + 16);
    if (ret != Z_OK) return {};

    // Start with an estimate
    uLongf dst_capacity = len * 4;
    if (dst_capacity < 4096) dst_capacity = 4096;
    std::vector<uint8_t> out(dst_capacity);

    zs.next_out = out.data();
    zs.avail_out = static_cast<uInt>(dst_capacity);

    ret = inflate(&zs, Z_NO_FLUSH);
    while (ret == Z_OK) {
        // Need more space
        size_t old_size = out.size();
        out.resize(old_size * 2);
        zs.next_out = out.data() + old_size;
        zs.avail_out = static_cast<uInt>(old_size);
        ret = inflate(&zs, Z_NO_FLUSH);
    }

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) return {};

    out.resize(zs.total_out);
    return out;
}

}  // namespace fakelua::compress
