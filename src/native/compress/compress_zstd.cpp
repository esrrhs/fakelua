#include "compress_zstd.h"

#include <zstd.h>

namespace fakelua::compress {

std::vector<uint8_t> zstd_compress(const uint8_t *data, size_t len, int level) {
    if (level < 1) level = 1;
    if (level > 22) level = 22;

    size_t dst_capacity = ZSTD_compressBound(len);
    std::vector<uint8_t> out(dst_capacity);

    size_t written = ZSTD_compress(out.data(), out.size(), data, len, level);
    if (ZSTD_isError(written)) return {};

    out.resize(written);
    return out;
}

std::vector<uint8_t> zstd_decompress(const uint8_t *data, size_t len) {
    if (len == 0) return {};

    unsigned long long content_size = ZSTD_getFrameContentSize(data, len);
    if (content_size == ZSTD_CONTENTSIZE_ERROR) return {};
    if (content_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        // Fallback: use streaming decompression with a growing buffer
        content_size = len * 10;
        if (content_size < 4096) content_size = 4096;
    }

    std::vector<uint8_t> out(content_size);

    size_t ret = ZSTD_decompress(out.data(), out.size(), data, len);
    if (ZSTD_isError(ret)) {
        // Try with larger buffer (content_size might have been wrong)
        out.resize(content_size * 4);
        ret = ZSTD_decompress(out.data(), out.size(), data, len);
        if (ZSTD_isError(ret)) return {};
    }

    out.resize(ret);
    return out;
}

}  // namespace fakelua::compress
