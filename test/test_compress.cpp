#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// ─────────────────────────────────────────────────────────────────────────────
// compress 模块测试 — LZ4
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_compress, lz4_compress_decompress) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_lz4_compress_decompress", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_lz4_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_binary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_lz4_binary", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_large_data) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_lz4_large_data", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// compress 模块测试 — zlib
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_compress, zlib_compress_decompress) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_zlib_compress_decompress", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zlib_level) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_zlib_level", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zlib_binary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_zlib_binary", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// compress 模块测试 — gzip
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_compress, gzip_compress_decompress) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_gzip_compress_decompress", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, gzip_level) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_gzip_level", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, gzip_concat_members) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_gzip_concat_members", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, gzip_trailing_garbage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zlib.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "CompressTest.test_gzip_trailing_garbage", ret), std::exception);
    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// compress 模块测试 — Zstd
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_compress, zstd_compress_decompress) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_zstd_compress_decompress", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_level) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_zstd_level", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_binary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_zstd_binary", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_large_data) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressTest.test_zstd_large_data", ret);
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
    Call(s, JIT_TCC, "CompressZstdCases.test_zstd_decompress_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_decompress_invalid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressZstdCases.test_zstd_decompress_invalid", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_compress_level_too_low) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressZstdCases.test_zstd_compress_level_too_low", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_compress_level_too_high) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressZstdCases.test_zstd_compress_level_too_high", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_compress_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressZstdCases.test_zstd_compress_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_extra_large_data) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressZstdCases.test_zstd_large_data", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_various_levels) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressZstdCases.test_zstd_various_levels", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_binary_with_nulls) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_zstd_cases.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CompressZstdCases.test_zstd_binary_with_nulls", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_garbage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "CompressTest.test_lz4_garbage_throw", ret), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_truncated) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "CompressTest.test_lz4_truncated_throw", ret), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_trailing_garbage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_lz4.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "CompressTest.test_lz4_trailing_garbage", ret), std::exception);
    FakeluaDeleteState(s);
}

// Compress error path tests (GCC backend for coverage)
TEST(test_compress, zstd_decompress_empty_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_zstd_decompress_empty", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_decompress_invalid_frame_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_zstd_decompress_invalid_frame", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_compress_level_boundary_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_zstd_compress_level_boundary", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_large_data_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_zstd_large_data", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, zstd_binary_with_nulls_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_zstd_binary_with_nulls", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_decompress_invalid_frame_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_lz4_decompress_invalid_frame", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_decompress_truncated_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_lz4_decompress_truncated", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_decompress_trailing_garbage_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_lz4_decompress_trailing_garbage", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_large_data_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_lz4_large_data", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_compress, lz4_binary_with_nulls_gcc) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./compress/test_compress_error_paths.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "CompressErrorPaths.test_lz4_binary_with_nulls", ret);
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
    Call(s, JIT_TCC, "CompressZstdEdge.test_zstd_decompress_empty_input", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    // Exception tests use GCC
    EXPECT_THROW(Call(s, JIT_GCC, "CompressZstdEdge.test_zstd_decompress_truncated", ret), std::exception);
    ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "CompressZstdEdge.test_zstd_decompress_random_data", ret), std::exception);
    ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "CompressZstdEdge.test_zstd_decompress_trailing_garbage", ret), std::exception);
    ret = 0;
    // Normal tests use TCC
    Call(s, JIT_TCC, "CompressZstdEdge.test_zstd_compress_level_min", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    Call(s, JIT_TCC, "CompressZstdEdge.test_zstd_compress_level_max", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    Call(s, JIT_TCC, "CompressZstdEdge.test_zstd_compress_level_out_of_range", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    Call(s, JIT_TCC, "CompressZstdEdge.test_zstd_large_file", ret);
    EXPECT_EQ(ret, 1);
    ret = 0;
    Call(s, JIT_TCC, "CompressZstdEdge.test_zstd_binary_with_nulls", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
