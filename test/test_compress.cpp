#include "fakelua.h"
#include "native/compress/compress_lz4.h"
#include "test_jit.h"
#include "gtest/gtest.h"
#include <lz4frame.h>

using namespace fakelua;

// compress 模块测试 — LZ4

TEST(test_compress, lz4_compress_decompress) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_lz4_compress_decompress", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_lz4_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_binary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_lz4_binary", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_large_data) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_lz4_large_data", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// compress 模块测试 — zlib

TEST(test_compress, zlib_compress_decompress) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_zlib_compress_decompress", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zlib_level) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_zlib_level", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zlib_binary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_zlib_binary", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// compress 模块测试 — gzip

TEST(test_compress, gzip_compress_decompress) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_gzip_compress_decompress", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, gzip_level) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_gzip_level", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, gzip_concat_members) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_gzip_concat_members", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, gzip_trailing_garbage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    CallThrow(s, "CompressTest.test_gzip_trailing_garbage", ret);
    FakeluaDeleteState(s);
}

// compress 模块测试 — Zstd

TEST(test_compress, zstd_compress_decompress) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_zstd_compress_decompress", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_level) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_zstd_level", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_binary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_zstd_binary", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_large_data) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressTest.test_zstd_large_data", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// Zstd extra edge case tests
TEST(test_compress, zstd_decompress_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressZstdCases.test_zstd_decompress_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_decompress_invalid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressZstdCases.test_zstd_decompress_invalid", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_compress_level_too_low) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressZstdCases.test_zstd_compress_level_too_low", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_compress_level_too_high) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressZstdCases.test_zstd_compress_level_too_high", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_compress_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressZstdCases.test_zstd_compress_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_extra_large_data) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressZstdCases.test_zstd_large_data", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_various_levels) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressZstdCases.test_zstd_various_levels", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_binary_with_nulls) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressZstdCases.test_zstd_binary_with_nulls", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_garbage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    CallThrow(s, "CompressTest.test_lz4_garbage_throw", ret);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_truncated) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    CallThrow(s, "CompressTest.test_lz4_truncated_throw", ret);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_trailing_garbage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    CallThrow(s, "CompressTest.test_lz4_trailing_garbage", ret);
    FakeluaDeleteState(s);
}

// Compress error path tests (GCC backend for coverage)
TEST(test_compress, zstd_decompress_empty_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_zstd_decompress_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_decompress_invalid_frame_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_zstd_decompress_invalid_frame", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_compress_level_boundary_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_zstd_compress_level_boundary", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_large_data_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_zstd_large_data", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_binary_with_nulls_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_zstd_binary_with_nulls", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_decompress_invalid_frame_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_lz4_decompress_invalid_frame", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_decompress_truncated_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_lz4_decompress_truncated", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_decompress_trailing_garbage_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_lz4_decompress_trailing_garbage", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_large_data_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_lz4_large_data", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_binary_with_nulls_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    CallAll(s, "CompressErrorPaths.test_lz4_binary_with_nulls", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// 测试 zstd 边界情况
// Normal tests use TCC, exception tests use GCC (TCC doesn't support C++ exception propagation)
TEST(test_compress, zstd_edge_cases) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_edge.lua", config);
    int64_t ret = 0;
    // Empty input returns empty result (no exception)
    CallAll(s, "CompressZstdEdge.test_zstd_decompress_empty_input", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    // Exception tests use GCC
    CallThrow(s, "CompressZstdEdge.test_zstd_decompress_truncated", ret);
    ret = 0;
    CallThrow(s, "CompressZstdEdge.test_zstd_decompress_random_data", ret);
    ret = 0;
    CallThrow(s, "CompressZstdEdge.test_zstd_decompress_trailing_garbage", ret);
    ret = 0;
    // Normal tests use TCC
    CallAll(s, "CompressZstdEdge.test_zstd_compress_level_min", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "CompressZstdEdge.test_zstd_compress_level_max", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "CompressZstdEdge.test_zstd_compress_level_out_of_range", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "CompressZstdEdge.test_zstd_large_file", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    CallAll(s, "CompressZstdEdge.test_zstd_binary_with_nulls", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_unknown_size_growth_checksum_failure_and_declared_limit) {
    const std::vector<uint8_t> plain(100000, 'x');
    LZ4F_preferences_t prefs{};
    prefs.frameInfo.contentChecksumFlag = LZ4F_contentChecksumEnabled;
    std::vector<uint8_t> frame(LZ4F_compressFrameBound(plain.size(), &prefs));
    const size_t size = LZ4F_compressFrame(frame.data(), frame.size(), plain.data(), plain.size(), &prefs);
    ASSERT_FALSE(LZ4F_isError(size));
    frame.resize(size);
    EXPECT_EQ(compress::Lz4Decompress(frame.data(), frame.size()), plain);
    frame.back() ^= 1;
    EXPECT_THROW(compress::Lz4Decompress(frame.data(), frame.size()), std::exception);
    LZ4F_cctx *raw = nullptr;
    ASSERT_FALSE(LZ4F_isError(LZ4F_createCompressionContext(&raw, LZ4F_VERSION)));
    std::unique_ptr<LZ4F_cctx, decltype(&LZ4F_freeCompressionContext)> context(raw, LZ4F_freeCompressionContext);
    prefs.frameInfo.contentSize = 64ull * 1024 * 1024 + 1;
    std::vector<uint8_t> header(LZ4F_HEADER_SIZE_MAX);
    const size_t header_size = LZ4F_compressBegin(context.get(), header.data(), header.size(), &prefs);
    ASSERT_FALSE(LZ4F_isError(header_size));
    EXPECT_THROW(compress::Lz4Decompress(header.data(), header_size), std::exception);
}
