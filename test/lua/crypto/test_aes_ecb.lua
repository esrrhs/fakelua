package "CryptoTest"

-- AES-128 ECB test
-- Key: "1234567890123456"
-- Plaintext: "Hello, World!!!!" (16 bytes)
-- Expected ciphertext: 61b80625e3f5b36cfd4cea22045061c6
function test_ecb()
    local key = "1234567890123456"
    local plaintext = "Hello, World!!!!"
    local expected = "61b80625e3f5b36cfd4cea22045061c6"

    local ciphertext = crypto.aes_encrypt_ecb(plaintext, key)
    if crypto.hex_encode(ciphertext) ~= expected then
        print("ECB encrypt failed:", crypto.hex_encode(ciphertext), "expected:", expected)
        return 0
    end

    local decrypted = crypto.aes_decrypt_ecb(ciphertext, key)
    if decrypted ~= plaintext then
        print("ECB decrypt failed:", decrypted, "expected:", plaintext)
        return 0
    end

    return 1
end
