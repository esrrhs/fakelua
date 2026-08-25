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
