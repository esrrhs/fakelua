function test_crypto_input_contracts()
    local key = string.rep(string.char(0), 16)
    local function rejects(fn, text)
        local ok, message = pcall(fn)
        assert(not ok and string.find(message, text, 1, true))
    end
    rejects(function() crypto.aes_encrypt_ecb("x", key) end, "multiple of 16")
    rejects(function() crypto.aes_decrypt_ecb("x", key) end, "multiple of 16")
    rejects(function() crypto.aes_encrypt_ecb(key, "short") end, "16 bytes")
    rejects(function() crypto.aes_encrypt_cbc(key, key, "short") end, "16 bytes")
    rejects(function() crypto.hex_decode("abc") end, "even length")
    rejects(function() crypto.digest_raw("fakelua-not-a-digest", "data") end, "unknown digest")
    rejects(function() crypto.encrypt("aes-128-cbc", key, key, "x", true) end, "EVP_Final")
    local encrypted = crypto.encrypt("aes-128-cbc", key, key, key)
    assert(crypto.decrypt("aes-128-cbc", key, key, encrypted) == key)
    local corrupted = string.sub(encrypted, 1, 15) .. string.char(string.byte(encrypted, 16) ~ 1) .. string.sub(encrypted, 17)
    rejects(function() crypto.decrypt("aes-128-cbc", key, key, corrupted) end, "EVP_Final")
    return 1
end
