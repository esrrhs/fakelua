package "CryptoTest"

-- AES-128 CBC test
-- Key: "1234567890123456"
-- IV:  "abcdefghijklmnop"
-- Plaintext: "Hello, World! This is a longer message for CBC mode testing."
-- Expected ciphertext: f2c200d426828e2b51f4a2cf6d62c4e4d1b86a6150e04d9cfa6794d69e54abba701d1d8b56a6774436be39165dd2461a9bc12fd0467c6c7f4d3a95e47c200afc
function test_cbc()
    local key = "1234567890123456"
    local iv = "abcdefghijklmnop"
    local plaintext = "Hello, World! This is a longer message for CBC mode testing."
    local expected = "f2c200d426828e2b51f4a2cf6d62c4e4d1b86a6150e04d9cfa6794d69e54abba701d1d8b56a6774436be39165dd2461a9bc12fd0467c6c7f4d3a95e47c200afc"

    local ciphertext = crypto.aes_encrypt_cbc(plaintext, key, iv)
    if crypto.hex_encode(ciphertext) ~= expected then
        print("CBC encrypt failed:", crypto.hex_encode(ciphertext), "expected:", expected)
        return 0
    end

    local decrypted = crypto.aes_decrypt_cbc(ciphertext, key, iv)
    if decrypted ~= plaintext then
        print("CBC decrypt failed:", decrypted, "expected:", plaintext)
        return 0
    end

    return 1
end
