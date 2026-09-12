#include "fakelua.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

// crypto 模块测试 — 哈希

TEST(test_crypto, md5_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_md5.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_md5", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, sha1_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_sha1.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_sha1", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, sha256_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_sha256.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_sha256", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, md5_hello) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_md5_hello.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_md5_hello", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, sha1_hello) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_sha1_hello.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_sha1_hello", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — Base64

TEST(test_crypto, base64_encode_decode) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_base64", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, base64_whitespace) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_base64_whitespace", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, base64_invalid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    CallThrow(s, "CryptoTest.test_base64_invalid", ret);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — AES-128

TEST(test_crypto, aes_ecb_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_ecb.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_ecb", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, aes_cbc_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_cbc.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_cbc", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, aes_ctr_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_ctr.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_ctr", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — RC4

TEST(test_crypto, rc4_keystream) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_rc4.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_rc4_keystream", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, rc4_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_rc4.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_rc4_encrypt_decrypt", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — Blowfish

TEST(test_crypto, blowfish_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_blowfish.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_blowfish", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — DES / 3DES

TEST(test_crypto, des_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_des.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_des", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, triple_des_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_des.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_triple_des", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, hex_roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_hex.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_hex_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, hex_decode_invalid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_hex.lua", config);
    int64_t ret = 0;
    CallThrow(s, "CryptoTest.test_hex_decode_invalid", ret);
    FakeluaDeleteState(s);
}

// Generic EVP interface tests
static void RunEvpTest(const char *func_name) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_evp.lua", config);
    int64_t ret = 0;
    CallAll(s, func_name, ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, evp_digest_md5) {
    RunEvpTest("CryptoTest.test_evp_digest_md5");
}

TEST(test_crypto, evp_digest_sha1) {
    RunEvpTest("CryptoTest.test_evp_digest_sha1");
}

TEST(test_crypto, evp_digest_sha256) {
    RunEvpTest("CryptoTest.test_evp_digest_sha256");
}

TEST(test_crypto, evp_digest_sha512) {
    RunEvpTest("CryptoTest.test_evp_digest_sha512");
}

TEST(test_crypto, evp_digest_raw) {
    RunEvpTest("CryptoTest.test_evp_digest_raw");
}

TEST(test_crypto, evp_digest_unknown) {
    RunEvpTest("CryptoTest.test_evp_digest_unknown");
}

TEST(test_crypto, evp_aes128_cbc) {
    RunEvpTest("CryptoTest.test_evp_aes128_cbc");
}

TEST(test_crypto, evp_aes256_cbc) {
    RunEvpTest("CryptoTest.test_evp_aes256_cbc");
}

TEST(test_crypto, evp_aes128_ecb) {
    RunEvpTest("CryptoTest.test_evp_aes128_ecb");
}

TEST(test_crypto, evp_rc4) {
    RunEvpTest("CryptoTest.test_evp_rc4");
}

TEST(test_crypto, evp_blowfish) {
    RunEvpTest("CryptoTest.test_evp_blowfish");
}

TEST(test_crypto, evp_des) {
    RunEvpTest("CryptoTest.test_evp_des");
}

TEST(test_crypto, evp_3des) {
    RunEvpTest("CryptoTest.test_evp_3des");
}

TEST(test_crypto, evp_unknown_cipher) {
    RunEvpTest("CryptoTest.test_evp_unknown_cipher");
}

TEST(test_crypto, evp_wrong_key) {
    RunEvpTest("CryptoTest.test_evp_wrong_key_length");
}

TEST(test_crypto, evp_wrong_iv) {
    RunEvpTest("CryptoTest.test_evp_wrong_iv_length");
}

// Extended EVP tests — algorithms newly accessible via generic interface
static void RunEvpExtTest(const char *func_name) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_evp_extended.lua", config);
    int64_t ret = 0;
    CallAll(s, func_name, ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// Digests
TEST(test_crypto, evp_sha224) {
    RunEvpExtTest("CryptoTest.test_evp_sha224");
}

TEST(test_crypto, evp_sha384) {
    RunEvpExtTest("CryptoTest.test_evp_sha384");
}

TEST(test_crypto, evp_sha3_256) {
    RunEvpExtTest("CryptoTest.test_evp_sha3_256");
}

TEST(test_crypto, evp_sha3_512) {
    RunEvpExtTest("CryptoTest.test_evp_sha3_512");
}

TEST(test_crypto, evp_blake2b512) {
    RunEvpExtTest("CryptoTest.test_evp_blake2b512");
}

TEST(test_crypto, evp_blake2s256) {
    RunEvpExtTest("CryptoTest.test_evp_blake2s256");
}

TEST(test_crypto, evp_ripemd160) {
    RunEvpExtTest("CryptoTest.test_evp_ripemd160");
}

// AES-192 (was dead code before)
TEST(test_crypto, evp_aes192_cbc) {
    RunEvpExtTest("CryptoTest.test_evp_aes192_cbc");
}

TEST(test_crypto, evp_aes192_ecb) {
    RunEvpExtTest("CryptoTest.test_evp_aes192_ecb");
}

// AES modes
TEST(test_crypto, evp_aes128_cfb) {
    RunEvpExtTest("CryptoTest.test_evp_aes128_cfb");
}

TEST(test_crypto, evp_aes128_ofb) {
    RunEvpExtTest("CryptoTest.test_evp_aes128_ofb");
}

TEST(test_crypto, evp_aes128_ctr) {
    RunEvpExtTest("CryptoTest.test_evp_aes128_ctr");
}

TEST(test_crypto, evp_aes256_ctr) {
    RunEvpExtTest("CryptoTest.test_evp_aes256_ctr");
}

TEST(test_crypto, evp_aes256_ecb) {
    RunEvpExtTest("CryptoTest.test_evp_aes256_ecb");
}

// ChaCha20
TEST(test_crypto, evp_chacha20) {
    RunEvpExtTest("CryptoTest.test_evp_chacha20");
}

// Camellia
TEST(test_crypto, evp_camellia128) {
    RunEvpExtTest("CryptoTest.test_evp_camellia128_cbc");
}

TEST(test_crypto, evp_camellia256) {
    RunEvpExtTest("CryptoTest.test_evp_camellia256_cbc");
}

// DES-CBC / 3DES-CBC
TEST(test_crypto, evp_des_cbc) {
    RunEvpExtTest("CryptoTest.test_evp_des_cbc");
}

TEST(test_crypto, evp_3des_cbc) {
    RunEvpExtTest("CryptoTest.test_evp_3des_cbc");
}

// ARIA
TEST(test_crypto, evp_aria128_cbc) {
    RunEvpExtTest("CryptoTest.test_evp_aria128_cbc");
}

// digest_raw
TEST(test_crypto, evp_digest_raw_sha256) {
    RunEvpExtTest("CryptoTest.test_evp_digest_raw_sha256");
}

TEST(test_crypto, evp_digest_raw_sha3_256) {
    RunEvpExtTest("CryptoTest.test_evp_digest_raw_sha3_256");
}

TEST(test_crypto, uuid_v4) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_uuid.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_uuid", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, crc32) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_crc32.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_crc32", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, xxhash) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_xxhash.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_xxhash", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
