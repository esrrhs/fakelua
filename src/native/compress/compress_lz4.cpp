#include "compress_lz4.h"
#include "util/exception.h"

#include <lz4frame.h>
#include <memory>

namespace fakelua::compress {

namespace {

struct Lz4CCtxDeleter {
    void operator()(LZ4F_compressionContext_t p) const noexcept {
        LZ4F_freeCompressionContext(p);
    }
};

struct Lz4DCtxDeleter {
    void operator()(LZ4F_decompressionContext_t p) const noexcept {
        LZ4F_freeDecompressionContext(p);
    }
};

}// namespace

static constexpr uint64_t kMaxDecompressBytes = 64ull * 1024 * 1024;

std::vector<uint8_t> Lz4Compress(const uint8_t *data, size_t len) {
    LZ4F_cctx *raw_cctx = nullptr;
    LZ4F_errorCode_t err = LZ4F_createCompressionContext(&raw_cctx, LZ4F_VERSION);
    if (LZ4F_isError(err) || !raw_cctx) return {};
    std::unique_ptr<LZ4F_cctx, Lz4CCtxDeleter> cctx(raw_cctx);

    LZ4F_preferences_t prefs{};
    prefs.frameInfo.contentSize = len;
    prefs.compressionLevel = 0;// default level

    size_t dst_capacity = LZ4F_compressBound(len, &prefs);
    std::vector<uint8_t> out(dst_capacity);

    size_t written = LZ4F_compressBegin(cctx.get(), out.data(), out.size(), &prefs);
    if (LZ4F_isError(written)) {
        return {};
    }

    size_t compressed = LZ4F_compressUpdate(cctx.get(), out.data() + written, out.size() - written, data, len, nullptr);
    if (LZ4F_isError(compressed)) {
        return {};
    }
    written += compressed;

    size_t end_written = LZ4F_compressEnd(cctx.get(), out.data() + written, out.size() - written, nullptr);
    if (LZ4F_isError(end_written)) {
        return {};
    }

    out.resize(written + end_written);
    return out;
}

std::vector<uint8_t> Lz4Decompress(const uint8_t *data, size_t len) {
    if (len == 0) return {};

    LZ4F_dctx *raw_dctx = nullptr;
    LZ4F_errorCode_t err = LZ4F_createDecompressionContext(&raw_dctx, LZ4F_VERSION);
    if (LZ4F_isError(err) || !raw_dctx) {
        ThrowFakeluaException("lz4_decompress: failed to create context");
    }
    std::unique_ptr<LZ4F_dctx, Lz4DCtxDeleter> dctx(raw_dctx);

    // First pass: get frame info to determine content size
    LZ4F_frameInfo_t frameInfo{};
    size_t src_consumed = len;// INPUT: size of buffer; OUTPUT: bytes consumed
    size_t fi_ret = LZ4F_getFrameInfo(dctx.get(), &frameInfo, data, &src_consumed);
    if (LZ4F_isError(fi_ret)) {
        ThrowFakeluaException("lz4_decompress: invalid frame");
    }

    uint64_t content_size = frameInfo.contentSize;
    if (content_size > kMaxDecompressBytes) {
        ThrowFakeluaException("lz4_decompress: payload too large");
    }
    if (content_size == 0) {
        content_size = len * 10;
        if (content_size < 4096) content_size = 4096;
        if (content_size > kMaxDecompressBytes) content_size = kMaxDecompressBytes;
    }

    std::vector<uint8_t> out(content_size);
    const uint8_t *src = data + src_consumed;
    size_t src_remaining = len - src_consumed;
    size_t dst_pos = 0;
    size_t last_ret = fi_ret;

    while (src_remaining > 0) {
        size_t src_size = src_remaining;
        size_t dst_capacity = out.size() - dst_pos;
        if (dst_capacity == 0) {
            if (out.size() >= kMaxDecompressBytes) {
                ThrowFakeluaException("lz4_decompress: payload too large");
            }
            uint64_t next = out.size() * 2;
            if (next > kMaxDecompressBytes) next = kMaxDecompressBytes;
            out.resize(next);
            dst_capacity = out.size() - dst_pos;
        }

        last_ret = LZ4F_decompress(dctx.get(), out.data() + dst_pos, &dst_capacity, src, &src_size, nullptr);
        if (LZ4F_isError(last_ret)) {
            ThrowFakeluaException("lz4_decompress: failed");
        }

        dst_pos += dst_capacity;
        src += src_size;
        src_remaining -= src_size;

        if (last_ret == 0) {
            if (src_remaining > 0) {
                ThrowFakeluaException("lz4_decompress: trailing garbage");
            }
            break;// frame fully decoded
        }
    }

    if (last_ret != 0) {
        ThrowFakeluaException("lz4_decompress: truncated frame");
    }

    out.resize(dst_pos);
    return out;
}

}// namespace fakelua::compress
