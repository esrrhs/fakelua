#include "compress_lz4.h"

#include <lz4frame.h>

namespace fakelua::compress {

std::vector<uint8_t> lz4_compress(const uint8_t *data, size_t len) {
    LZ4F_cctx *cctx = nullptr;
    LZ4F_errorCode_t err = LZ4F_createCompressionContext(&cctx, LZ4F_VERSION);
    if (LZ4F_isError(err) || !cctx) return {};

    LZ4F_preferences_t prefs{};
    prefs.frameInfo.contentSize = len;
    prefs.compressionLevel = 0;  // default level

    size_t dst_capacity = LZ4F_compressBound(len, &prefs);
    std::vector<uint8_t> out(dst_capacity);

    size_t written = LZ4F_compressBegin(cctx, out.data(), out.size(), &prefs);
    if (LZ4F_isError(written)) {
        LZ4F_freeCompressionContext(cctx);
        return {};
    }

    size_t compressed = LZ4F_compressUpdate(cctx, out.data() + written, out.size() - written,
                                           data, len, nullptr);
    if (LZ4F_isError(compressed)) {
        LZ4F_freeCompressionContext(cctx);
        return {};
    }
    written += compressed;

    size_t end_written = LZ4F_compressEnd(cctx, out.data() + written, out.size() - written, nullptr);
    if (LZ4F_isError(end_written)) {
        LZ4F_freeCompressionContext(cctx);
        return {};
    }

    LZ4F_freeCompressionContext(cctx);
    out.resize(written + end_written);
    return out;
}

std::vector<uint8_t> lz4_decompress(const uint8_t *data, size_t len) {
    if (len == 0) return {};

    LZ4F_dctx *dctx = nullptr;
    LZ4F_errorCode_t err = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
    if (LZ4F_isError(err) || !dctx) return {};

    // First pass: get frame info to determine content size
    LZ4F_frameInfo_t frameInfo;
    size_t src_consumed = len;  // INPUT: size of buffer; OUTPUT: bytes consumed
    size_t fi_ret = LZ4F_getFrameInfo(dctx, &frameInfo, data, &src_consumed);
    if (LZ4F_isError(fi_ret)) {
        LZ4F_freeDecompressionContext(dctx);
        return {};
    }

    uint64_t content_size = frameInfo.contentSize;
    if (content_size == 0) {
        content_size = len * 10;
        if (content_size < 4096) content_size = 4096;
    }

    std::vector<uint8_t> out(content_size);
    const uint8_t *src = data + src_consumed;
    size_t src_remaining = len - src_consumed;
    size_t dst_pos = 0;

    while (src_remaining > 0) {
        size_t src_size = src_remaining;
        size_t dst_capacity = out.size() - dst_pos;
        if (dst_capacity == 0) {
            out.resize(out.size() * 2);
            dst_capacity = out.size() - dst_pos;
        }

        size_t ret = LZ4F_decompress(dctx, out.data() + dst_pos, &dst_capacity,
                                     src, &src_size, nullptr);
        if (LZ4F_isError(ret)) {
            LZ4F_freeDecompressionContext(dctx);
            return {};
        }

        dst_pos += dst_capacity;
        src += src_size;
        src_remaining -= src_size;

        if (ret == 0) break;  // frame fully decoded
    }

    LZ4F_freeDecompressionContext(dctx);
    out.resize(dst_pos);
    return out;
}

}  // namespace fakelua::compress
