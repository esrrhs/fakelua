package "CryptoTest"

-- AES-128 CTR test
-- Key: "1234567890123456"
-- IV (nonce, 8 bytes used): first 8 bytes of "abcdefghijklmnop" = "abcdefgh"
-- Plaintext: "Hello, World! This is a test of CTR mode."
-- Expected ciphertext: 1bf963e646909d78365f96849a4aa72b58c0956db3a1f55b34e8c3b7055dfc5eaf3b6e54214583e1bf
function test_ctr()
    local key = "1234567890123456"
    local iv = "abcdefghijklmnop"
    local plaintext = "Hello, World! This is a test of CTR mode."
    local expected = "1bf963e646909d78365f96849a4aa72b58c0956db3a1f55b34e8c3b7055dfc5eaf3b6e54214583e1bf"

    local ciphertext = crypto.aes_encrypt_ctr(plaintext, key, iv)
    if crypto.hex_encode(ciphertext) ~= expected then
        print("CTR encrypt failed:", crypto.hex_encode(ciphertext), "expected:", expected)
        return 0
    end

    local decrypted = crypto.aes_decrypt_ctr(ciphertext, key, iv)
    if decrypted ~= plaintext then
        print("CTR decrypt failed:", decrypted, "expected:", plaintext)
        return 0
    end

    return 1
end
