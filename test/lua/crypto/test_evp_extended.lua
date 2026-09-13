package("CryptoTest")

-- ── Extended digest tests — algorithms newly accessible via crypto.digest ────

function test_evp_sha224()
    -- sha224("hello") = ea09ae9cc6768c50fcee903ed054556e5bfc8347907f12598aa24193
    local h = crypto.digest("sha224", "hello")
    if h ~= "ea09ae9cc6768c50fcee903ed054556e5bfc8347907f12598aa24193" then
        print("sha224 mismatch: " .. h)
        return 0
    end
    if #h ~= 56 then
        print("sha224 hex length should be 56, got " .. #h)
        return 0
    end
    return 1
end

function test_evp_sha384()
    -- sha384 produces 384-bit = 96 hex chars
    local h = crypto.digest("sha384", "hello")
    if #h ~= 96 then
        print("sha384 hex length should be 96, got " .. #h)
        return 0
    end
    -- sha384("hello") = 59e1748777448c69de6b800d7a33bbfb9ff1b463e44354c3553bcdb9c666fa9...
    if string.sub(h, 1, 8) ~= "59e17487" then
        print("sha384 prefix mismatch: " .. string.sub(h, 1, 8))
        return 0
    end
    return 1
end

function test_evp_sha3_256()
    -- sha3-256("hello") = 3338be694f50c5f338814986cdf0686453a888b84f424d792af4b9202398f392
    local h = crypto.digest("sha3-256", "hello")
    if h ~= "3338be694f50c5f338814986cdf0686453a888b84f424d792af4b9202398f392" then
        print("sha3-256 mismatch: " .. h)
        return 0
    end
    return 1
end

function test_evp_sha3_512()
    -- sha3-512 produces 512-bit = 128 hex chars
    local h = crypto.digest("sha3-512", "hello")
    if #h ~= 128 then
        print("sha3-512 hex length should be 128, got " .. #h)
        return 0
    end
    return 1
end

function test_evp_blake2b512()
    -- blake2b512("hello") known value (first 16 chars)
    local h = crypto.digest("blake2b512", "hello")
    if #h ~= 128 then
        print("blake2b512 hex length should be 128, got " .. #h)
        return 0
    end
    if string.sub(h, 1, 8) ~= "e4cfa39a" then
        print("blake2b512 prefix mismatch: " .. string.sub(h, 1, 8))
        return 0
    end
    return 1
end

function test_evp_blake2s256()
    -- blake2s256 produces 256-bit = 64 hex chars
    local h = crypto.digest("blake2s256", "hello")
    if #h ~= 64 then
        print("blake2s256 hex length should be 64, got " .. #h)
        return 0
    end
    return 1
end

function test_evp_ripemd160()
    -- ripemd160 produces 160-bit = 40 hex chars
    local h = crypto.digest("ripemd160", "hello")
    if #h ~= 40 then
        print("ripemd160 hex length should be 40, got " .. #h)
        return 0
    end
    -- ripemd160("hello") = 108f07b8382412612c048d07d13f814118445acd
    if h ~= "108f07b8382412612c048d07d13f814118445acd" then
        print("ripemd160 mismatch: " .. h)
        return 0
    end
    return 1
end

-- ── AES-192 — was dead code before, now fully accessible ────────────────────

function test_evp_aes192_cbc()
    local key = "123456789012345678901234" -- 24 bytes = AES-192
    local iv = "abcdefgh12345678"          -- 16 bytes
    local pt = "AES-192 was unreachable before the EVP interface!"
    local ct = crypto.encrypt("aes-192-cbc", key, iv, pt)
    local pt2 = crypto.decrypt("aes-192-cbc", key, iv, ct)
    if pt2 ~= pt then
        print("aes-192-cbc roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_aes192_ecb()
    local key = "123456789012345678901234" -- 24 bytes
    local pt = "AES192ECBblock!!"          -- 16 bytes exactly
    local ct = crypto.encrypt("aes-192-ecb", key, "", pt, true)
    local pt2 = crypto.decrypt("aes-192-ecb", key, "", ct, true)
    if pt2 ~= pt then
        print("aes-192-ecb roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

-- ── AES modes — CFB, OFB, CTR ───────────────────────────────────────────────

function test_evp_aes128_cfb()
    local key = "1234567890abcdef"
    local iv = "abcdefgh12345678"
    local pt = "CFB mode test — stream-like, no padding needed"
    -- CFB is a stream mode: output length = input length, no padding
    local ct = crypto.encrypt("aes-128-cfb", key, iv, pt, true)
    if #ct ~= #pt then
        print("aes-128-cfb ciphertext length mismatch: " .. #ct .. " vs " .. #pt)
        return 0
    end
    local pt2 = crypto.decrypt("aes-128-cfb", key, iv, ct, true)
    if pt2 ~= pt then
        print("aes-128-cfb roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_aes128_ofb()
    local key = "1234567890abcdef"
    local iv = "abcdefgh12345678"
    local pt = "OFB mode test — output feedback, stream-like"
    local ct = crypto.encrypt("aes-128-ofb", key, iv, pt, true)
    if #ct ~= #pt then
        print("aes-128-ofb ciphertext length mismatch: " .. #ct .. " vs " .. #pt)
        return 0
    end
    local pt2 = crypto.decrypt("aes-128-ofb", key, iv, ct, true)
    if pt2 ~= pt then
        print("aes-128-ofb roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_aes128_ctr()
    local key = "1234567890abcdef"
    local iv = "abcdefgh12345678"
    local pt = "CTR mode via native OpenSSL EVP — no padding, any length"
    local ct = crypto.encrypt("aes-128-ctr", key, iv, pt, true)
    if #ct ~= #pt then
        print("aes-128-ctr ciphertext length mismatch: " .. #ct .. " vs " .. #pt)
        return 0
    end
    local pt2 = crypto.decrypt("aes-128-ctr", key, iv, ct, true)
    if pt2 ~= pt then
        print("aes-128-ctr roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_aes256_ctr()
    local key = "12345678901234567890123456789012" -- 32 bytes
    local iv = "abcdefgh12345678"
    local pt = "AES-256-CTR mode"
    local ct = crypto.encrypt("aes-256-ctr", key, iv, pt, true)
    local pt2 = crypto.decrypt("aes-256-ctr", key, iv, ct, true)
    if pt2 ~= pt then
        print("aes-256-ctr roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

-- ── AES-256 ECB ──────────────────────────────────────────────────────────────

function test_evp_aes256_ecb()
    local key = "12345678901234567890123456789012" -- 32 bytes
    local pt = "AES256ECBblock!!"                  -- 16 bytes
    local ct = crypto.encrypt("aes-256-ecb", key, "", pt, true)
    local pt2 = crypto.decrypt("aes-256-ecb", key, "", ct, true)
    if pt2 ~= pt then
        print("aes-256-ecb roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

-- ── ChaCha20 ─────────────────────────────────────────────────────────────────
-- OpenSSL ChaCha20: key=32 bytes, iv=16 bytes (bytes 0-3=counter LE, bytes 4-15=nonce)

function test_evp_chacha20()
    local key = "12345678901234567890123456789012" -- 32 bytes
    local iv = "abcdefgh12345678"                  -- 16 bytes
    local pt = "ChaCha20 stream cipher — modern, fast, no padding"
    local ct = crypto.encrypt("chacha20", key, iv, pt, true)
    if #ct ~= #pt then
        print("chacha20 ciphertext length mismatch: " .. #ct .. " vs " .. #pt)
        return 0
    end
    local pt2 = crypto.decrypt("chacha20", key, iv, ct, true)
    if pt2 ~= pt then
        print("chacha20 roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

-- ── Camellia (alternative block cipher, same security as AES) ───────────────

function test_evp_camellia128_cbc()
    local key = "1234567890abcdef" -- 16 bytes
    local iv = "abcdefgh12345678"  -- 16 bytes
    local pt = "Camellia-128 is a Japanese block cipher with AES-equivalent security"
    local ct = crypto.encrypt("camellia-128-cbc", key, iv, pt)
    local pt2 = crypto.decrypt("camellia-128-cbc", key, iv, ct)
    if pt2 ~= pt then
        print("camellia-128-cbc roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_camellia256_cbc()
    local key = "12345678901234567890123456789012" -- 32 bytes
    local iv = "abcdefgh12345678"
    local pt = "Camellia-256-CBC"
    local ct = crypto.encrypt("camellia-256-cbc", key, iv, pt)
    local pt2 = crypto.decrypt("camellia-256-cbc", key, iv, ct)
    if pt2 ~= pt then
        print("camellia-256-cbc roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

-- ── DES-CBC (new mode, previously only ECB was tested) ──────────────────────

function test_evp_des_cbc()
    local key = "des8key!" -- 8 bytes
    local iv = "iv123456"  -- 8 bytes (DES block size)
    local pt = "DES-CBC mode test with padding"
    local ct = crypto.encrypt("des-cbc", key, iv, pt)
    local pt2 = crypto.decrypt("des-cbc", key, iv, ct)
    if pt2 ~= pt then
        print("des-cbc roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_3des_cbc()
    local key = "123456789012345678901234" -- 24 bytes
    local iv = "iv123456"                  -- 8 bytes (DES block size)
    local pt = "3DES-CBC with PKCS#7 padding"
    local ct = crypto.encrypt("des-ede3-cbc", key, iv, pt)
    local pt2 = crypto.decrypt("des-ede3-cbc", key, iv, ct)
    if pt2 ~= pt then
        print("des-ede3-cbc roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

-- ── ARIA (Korean block cipher standard, ISO/IEC 18033-3) ────────────────────

function test_evp_aria128_cbc()
    local key = "1234567890abcdef"
    local iv = "abcdefgh12345678"
    local pt = "ARIA-128-CBC — Korean national cipher standard"
    local ct = crypto.encrypt("aria-128-cbc", key, iv, pt)
    local pt2 = crypto.decrypt("aria-128-cbc", key, iv, ct)
    if pt2 ~= pt then
        print("aria-128-cbc roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

-- ── digest_raw cross-checks ──────────────────────────────────────────────────

function test_evp_digest_raw_sha256()
    local raw = crypto.digest_raw("sha256", "hello")
    if #raw ~= 32 then
        print("sha256 raw should be 32 bytes, got " .. #raw)
        return 0
    end
    local hex = crypto.hex_encode(raw)
    local expected = crypto.digest("sha256", "hello")
    if hex ~= expected then
        print("sha256 digest_raw vs digest mismatch")
        return 0
    end
    return 1
end

function test_evp_digest_raw_sha3_256()
    local raw = crypto.digest_raw("sha3-256", "")
    if #raw ~= 32 then
        print("sha3-256 raw of empty should be 32 bytes, got " .. #raw)
        return 0
    end
    return 1
end
