#include "compress_zlib.h"
#include "util/exception.h"

#include <zlib.h>

#include <cstring>
#include <limits>

namespace fakelua::compress {

static constexpr uLongf kMaxDecompressBytes = 64ul * 1024 * 1024;

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
    uLongf dst_capacity = static_cast<uLongf>(len * 4);
    if (dst_capacity < 4096) dst_capacity = 4096;
    if (dst_capacity > kMaxDecompressBytes) dst_capacity = kMaxDecompressBytes;

    std::vector<uint8_t> out(dst_capacity);

    int ret = uncompress(out.data(), &dst_capacity, data, static_cast<uLong>(len));
    while (ret == Z_BUF_ERROR && dst_capacity < kMaxDecompressBytes) {
        uLongf next = dst_capacity * 4;
        if (next > kMaxDecompressBytes || next < dst_capacity) next = kMaxDecompressBytes;
        if (next == dst_capacity) break;
        dst_capacity = next;
        out.resize(dst_capacity);
        ret = uncompress(out.data(), &dst_capacity, data, static_cast<uLong>(len));
    }
    if (ret == Z_BUF_ERROR) {
        ThrowFakeluaException("zlib_decompress: payload too large");
    }
    if (ret != Z_OK) {
        ThrowFakeluaException("zlib_decompress: failed");
    }

    out.resize(dst_capacity);
    return out;
}

// ── gzip format ──

std::vector<uint8_t> gzip_compress(const uint8_t *data, size_t len, int level) {
    if (level < 1) level = 1;
    if (level > 9) level = 9;
    if (len > std::numeric_limits<uInt>::max()) {
        ThrowFakeluaException("gzip_compress: payload too large");
    }

    z_stream zs{};
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;

    // windowBits = 15 + 16 = 31 → gzip format
    int ret = deflateInit2(&zs, level, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) return {};

    uLongf dst_capacity = deflateBound(&zs, static_cast<uLong>(len));
    if (dst_capacity > std::numeric_limits<uInt>::max()) {
        deflateEnd(&zs);
        ThrowFakeluaException("gzip_compress: payload too large");
    }
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
    if (len > std::numeric_limits<uInt>::max()) {
        ThrowFakeluaException("gzip_decompress: payload too large");
    }

    z_stream zs{};
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.next_in = const_cast<Bytef *>(data);
    zs.avail_in = static_cast<uInt>(len);

    // windowBits = 15 + 16 = 31 → gzip format
    int ret = inflateInit2(&zs, 15 + 16);
    if (ret != Z_OK) {
        ThrowFakeluaException("gzip_decompress: inflateInit2 failed");
    }

    auto fail = [&](const char *msg) {
        inflateEnd(&zs);
        ThrowFakeluaException(msg);
    };

    std::vector<uint8_t> out;

    auto inflate_one_member = [&]() {
        for (;;) {
            if (out.size() >= kMaxDecompressBytes) {
                fail("gzip_decompress: payload too large");
            }
            size_t old_size = out.size();
            size_t next = old_size < 4096 ? 4096 : old_size * 2;
            if (next > kMaxDecompressBytes || next < old_size) next = kMaxDecompressBytes;
            if (next <= old_size) {
                fail("gzip_decompress: payload too large");
            }
            out.resize(next);
            zs.next_out = out.data() + old_size;
            zs.avail_out = static_cast<uInt>(out.size() - old_size);
            ret = inflate(&zs, Z_NO_FLUSH);
            out.resize(out.size() - zs.avail_out);
            if (ret == Z_STREAM_END) return;
            if (ret != Z_OK) {
                fail("gzip_decompress: failed");
            }
        }
    };

    inflate_one_member();
    while (zs.avail_in > 0) {
        ret = inflateReset2(&zs, 15 + 16);
        if (ret != Z_OK) {
            fail("gzip_decompress: failed");
        }
        inflate_one_member();
    }

    inflateEnd(&zs);
    return out;
}

}  // namespace fakelua::compress
