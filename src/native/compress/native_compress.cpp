#include "native/compress/native_compress.h"
#include "native/compress/compress_lz4.h"
#include "native/compress/compress_zlib.h"
#include "native/compress/compress_zstd.h"
#include "native/native_common.h"
#include "util/logging.h"

#include <climits>
#include <string>
#include <vector>

namespace fakelua::compress {

static void RequireCompressOk(State *s, const std::string &data, const std::vector<uint8_t> &out, const char *api) {
    if (out.empty() && !data.empty()) {
        LOG_ERROR(s, "compress", "{}: compress failed (input_len={})", api, data.size());
        ThrowFakeluaException(std::string(api) + ": compress failed");
    }
}

// helper: read binary data from Lua string
static std::string ReadDataArg(State *s, CVar arg) {
    return inter::FakeluaToNativeString(s, arg);
}

// helper: read optional compression level
static int ReadLevelArg(State *s, CVar *args, int n, int argno, int default_val) {
    if (n <= argno) return default_val;
    CVar arg = inter::GetNativeArg(s, args, n, argno);
    int64_t lv = 0;
    if (arg.type_ == static_cast<int>(VarType::Int)) {
        lv = arg.data_.i;
    } else if (arg.type_ == static_cast<int>(VarType::Float)) {
        if (!DoubleFitsInt64(arg.data_.f, &lv)) {
            return default_val;
        }
    } else {
        return default_val;
    }
    if (lv < INT_MIN || lv > INT_MAX) {
        return default_val;
    }
    return static_cast<int>(lv);
}

// LZ4
// compress.lz4_compress(data) → compressed data
static CVar CompressLz4Compress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.lz4_compress", "data expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = Lz4Compress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    RequireCompressOk(s, data, out, "compress.lz4_compress");
    LOG_DEBUG(s, "compress", "lz4_compress: in={} out={}", data.size(), out.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// compress.lz4_decompress(data) → original data
static CVar CompressLz4Decompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.lz4_decompress", "data expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = Lz4Decompress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    LOG_DEBUG(s, "compress", "lz4_decompress: in={} out={}", data.size(), out.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// zlib
// compress.zlib_compress(data, level?) → compressed data (level 1-9, default 6)
static CVar CompressZlibCompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.zlib_compress", "data expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    int level = ReadLevelArg(s, args, n, 1, 6);
    auto out = ZlibCompress(reinterpret_cast<const uint8_t *>(data.data()), data.size(), level);
    RequireCompressOk(s, data, out, "compress.zlib_compress");
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// compress.zlib_decompress(data) → original data
static CVar CompressZlibDecompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.zlib_decompress", "data expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = ZlibDecompress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// gzip
// compress.gzip_compress(data, level?) → gzip data (level 1-9, default 6)
static CVar CompressGzipCompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.gzip_compress", "data expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    int level = ReadLevelArg(s, args, n, 1, 6);
    auto out = GzipCompress(reinterpret_cast<const uint8_t *>(data.data()), data.size(), level);
    RequireCompressOk(s, data, out, "compress.gzip_compress");
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// compress.gzip_decompress(data) → original data
static CVar CompressGzipDecompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.gzip_decompress", "data expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = GzipDecompress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// Zstd
// compress.zstd_compress(data, level?) → compressed data (level 1-22, default 3)
static CVar CompressZstdCompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.zstd_compress", "data expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    int level = ReadLevelArg(s, args, n, 1, 3);
    auto out = ZstdCompress(reinterpret_cast<const uint8_t *>(data.data()), data.size(), level);
    RequireCompressOk(s, data, out, "compress.zstd_compress");
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// compress.zstd_decompress(data) → original data
static CVar CompressZstdDecompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.zstd_decompress", "data expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = ZstdDecompress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

void RegisterCompressLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "compress.lz4_compress", 1, false, CompressLz4Compress);
    RegisterNativeFunction(s, "compress.lz4_decompress", 1, false, CompressLz4Decompress);
    RegisterNativeFunction(s, "compress.zlib_compress", 1, true, CompressZlibCompress);
    RegisterNativeFunction(s, "compress.zlib_decompress", 1, false, CompressZlibDecompress);
    RegisterNativeFunction(s, "compress.gzip_compress", 1, true, CompressGzipCompress);
    RegisterNativeFunction(s, "compress.gzip_decompress", 1, false, CompressGzipDecompress);
    RegisterNativeFunction(s, "compress.zstd_compress", 1, true, CompressZstdCompress);
    RegisterNativeFunction(s, "compress.zstd_decompress", 1, false, CompressZstdDecompress);
}

}// namespace fakelua::compress
