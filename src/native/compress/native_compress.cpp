#include "native/compress/native_compress.h"
#include "native/compress/compress_lz4.h"
#include "native/compress/compress_zlib.h"
#include "native/compress/compress_zstd.h"
#include "native/native_common.h"

#include <string>

namespace fakelua::compress {

// ── helper: read binary data from Lua string ──
static std::string read_data_arg(State *s, CVar arg) {
    return inter::FakeluaToNativeString(s, arg);
}

// ── helper: read optional compression level ──
static int read_level_arg(State *s, CVar *args, int n, int argno, int default_val) {
    if (n <= argno) return default_val;
    CVar arg = inter::GetNativeArg(s, args, n, argno);
    if (arg.type_ == static_cast<int>(VarType::Int)) {
        return static_cast<int>(arg.data_.i);
    }
    if (arg.type_ == static_cast<int>(VarType::Float)) {
        return static_cast<int>(arg.data_.f);
    }
    return default_val;
}

// ── LZ4 ──

// compress.lz4_compress(data) → compressed data
static CVar compress_lz4_compress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.lz4_compress", "data expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = lz4_compress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// compress.lz4_decompress(data) → original data
static CVar compress_lz4_decompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.lz4_decompress", "data expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = lz4_decompress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// ── zlib ──

// compress.zlib_compress(data, level?) → compressed data (level 1-9, default 6)
static CVar compress_zlib_compress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.zlib_compress", "data expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    int level = read_level_arg(s, args, n, 1, 6);
    auto out = zlib_compress(reinterpret_cast<const uint8_t *>(data.data()), data.size(), level);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// compress.zlib_decompress(data) → original data
static CVar compress_zlib_decompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.zlib_decompress", "data expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = zlib_decompress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// ── gzip ──

// compress.gzip_compress(data, level?) → gzip data (level 1-9, default 6)
static CVar compress_gzip_compress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.gzip_compress", "data expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    int level = read_level_arg(s, args, n, 1, 6);
    auto out = gzip_compress(reinterpret_cast<const uint8_t *>(data.data()), data.size(), level);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// compress.gzip_decompress(data) → original data
static CVar compress_gzip_decompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.gzip_decompress", "data expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = gzip_decompress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// ── Zstd ──

// compress.zstd_compress(data, level?) → compressed data (level 1-22, default 3)
static CVar compress_zstd_compress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.zstd_compress", "data expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    int level = read_level_arg(s, args, n, 1, 3);
    auto out = zstd_compress(reinterpret_cast<const uint8_t *>(data.data()), data.size(), level);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// compress.zstd_decompress(data) → original data
static CVar compress_zstd_decompress(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "compress.zstd_decompress", "data expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    auto out = zstd_decompress(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

void RegisterCompressLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "compress.lz4_compress", 1, false, compress_lz4_compress);
    RegisterNativeFunction(s, "compress.lz4_decompress", 1, false, compress_lz4_decompress);
    RegisterNativeFunction(s, "compress.zlib_compress", 1, true, compress_zlib_compress);
    RegisterNativeFunction(s, "compress.zlib_decompress", 1, false, compress_zlib_decompress);
    RegisterNativeFunction(s, "compress.gzip_compress", 1, true, compress_gzip_compress);
    RegisterNativeFunction(s, "compress.gzip_decompress", 1, false, compress_gzip_decompress);
    RegisterNativeFunction(s, "compress.zstd_compress", 1, true, compress_zstd_compress);
    RegisterNativeFunction(s, "compress.zstd_decompress", 1, false, compress_zstd_decompress);
}

}  // namespace fakelua::compress
