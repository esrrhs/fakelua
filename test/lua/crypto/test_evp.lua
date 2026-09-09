package "CryptoTest"

-- ── crypto.digest tests ──────────────────────────────────────────────────────

function test_evp_digest_md5()
    local h = crypto.digest("md5", "hello")
    if h ~= "5d41402abc4b2a76b9719d911017c592" then
        print("md5 mismatch: " .. h)
        return 0
    end
    -- Empty string
    local h2 = crypto.digest("md5", "")
    if h2 ~= "d41d8cd98f00b204e9800998ecf8427e" then
        print("md5 empty mismatch: " .. h2)
        return 0
    end
    return 1
end

function test_evp_digest_sha1()
    local h = crypto.digest("sha1", "hello")
    if h ~= "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d" then
        print("sha1 mismatch: " .. h)
        return 0
    end
    return 1
end

function test_evp_digest_sha256()
    local h = crypto.digest("sha256", "hello")
    if h ~= "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824" then
        print("sha256 mismatch: " .. h)
        return 0
    end
    return 1
end

function test_evp_digest_sha512()
    local h = crypto.digest("sha512", "hello")
    if #h ~= 128 then
        print("sha512 hex length should be 128, got " .. #h)
        return 0
    end
    return 1
end

function test_evp_digest_raw()
    local raw = crypto.digest_raw("md5", "hello")
    if #raw ~= 16 then
        print("md5 raw should be 16 bytes, got " .. #raw)
        return 0
    end
    -- Cross-check with hex digest
    local hex = crypto.hex_encode(raw)
    local expected = crypto.digest("md5", "hello")
    if hex ~= expected then
        print("digest_raw vs digest mismatch")
        return 0
    end
    return 1
end

function test_evp_digest_unknown()
    local ok, err = pcall(function()
        crypto.digest("notanalgo_xyz", "data")
    end)
    if ok then
        print("expected error for unknown algo")
        return 0
    end
    return 1
end

-- ── crypto.encrypt / crypto.decrypt tests ───────────────────────────────────

function test_evp_aes128_cbc()
    local key = "1234567890abcdef"
    local iv  = "abcdefgh12345678"
    local pt  = "Hello, World! EVP interface test."
    local ct  = crypto.encrypt("aes-128-cbc", key, iv, pt)
    local pt2 = crypto.decrypt("aes-128-cbc", key, iv, ct)
    if pt2 ~= pt then
        print("aes-128-cbc roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_aes256_cbc()
    local key = "12345678901234567890123456789012"
    local iv  = "abcdefgh12345678"
    local pt  = "AES-256-CBC via generic EVP interface."
    local ct  = crypto.encrypt("aes-256-cbc", key, iv, pt)
    local pt2 = crypto.decrypt("aes-256-cbc", key, iv, ct)
    if pt2 ~= pt then
        print("aes-256-cbc roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_aes128_ecb()
    local key = "1234567890abcdef"
    local pt  = "ECBBlock1234567"   -- 15 bytes, padded to 16 by PKCS#7
    local ct  = crypto.encrypt("aes-128-ecb", key, "", pt)
    local pt2 = crypto.decrypt("aes-128-ecb", key, "", ct)
    if pt2 ~= pt then
        print("aes-128-ecb roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_rc4()
    local key = "secretkey"
    local pt  = "RC4 stream cipher via EVP!"
    local ct  = crypto.encrypt("rc4", key, "", pt)
    local pt2 = crypto.decrypt("rc4", key, "", ct)
    if pt2 ~= pt then
        print("rc4 roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_blowfish()
    local key = "blowkey"
    local pt  = "BFBLOCK!"   -- 8 bytes
    local ct  = crypto.encrypt("bf-ecb", key, "", pt, true)
    local pt2 = crypto.decrypt("bf-ecb", key, "", ct, true)
    if pt2 ~= pt then
        print("bf-ecb roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_des()
    local key = "des8key!"
    local pt  = "DES_ECB!"   -- 8 bytes
    local ct  = crypto.encrypt("des-ecb", key, "", pt, true)
    local pt2 = crypto.decrypt("des-ecb", key, "", ct, true)
    if pt2 ~= pt then
        print("des-ecb roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_3des()
    local key = "123456789012345678901234"
    local pt  = "3DEStest"   -- 8 bytes
    local ct  = crypto.encrypt("des-ede3-ecb", key, "", pt, true)
    local pt2 = crypto.decrypt("des-ede3-ecb", key, "", ct, true)
    if pt2 ~= pt then
        print("des-ede3-ecb roundtrip failed: " .. pt2)
        return 0
    end
    return 1
end

function test_evp_unknown_cipher()
    local ok, err = pcall(function()
        crypto.encrypt("notacipher_xyz", "key1234567890abc", "", "data")
    end)
    if ok then
        print("expected error for unknown cipher")
        return 0
    end
    return 1
end

function test_evp_wrong_key_length()
    local ok, err = pcall(function()
        -- AES-128-CBC requires exactly 16-byte key; pass 8 bytes
        crypto.encrypt("aes-128-cbc", "shortkey", "abcdefgh12345678", "data")
    end)
    if ok then
        print("expected error for wrong key length")
        return 0
    end
    return 1
end

function test_evp_wrong_iv_length()
    local ok, err = pcall(function()
        -- AES-128-CBC requires 16-byte IV; pass 8 bytes
        crypto.encrypt("aes-128-cbc", "1234567890abcdef", "shortiv", "data")
    end)
    if ok then
        print("expected error for wrong IV length")
        return 0
    end
    return 1
end
