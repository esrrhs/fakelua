package "CryptoTest"

-- AES-128 CTR: full 16-byte IV, must match OpenSSL EVP AES-CTR / crypto.encrypt.
function test_ctr()
    local key = "1234567890123456"
    local iv = "abcdefghijklmnop"
    local plaintext = "Hello, World! This is a test of CTR mode."

    local ciphertext = crypto.aes_encrypt_ctr(plaintext, key, iv)
    if #ciphertext ~= #plaintext then
        print("CTR ciphertext length mismatch")
        return 0
    end

    local via_evp = crypto.encrypt("aes-128-ctr", key, iv, plaintext, true)
    if ciphertext ~= via_evp then
        print("CTR mismatch with crypto.encrypt:", crypto.hex_encode(ciphertext), crypto.hex_encode(via_evp))
        return 0
    end

    local decrypted = crypto.aes_decrypt_ctr(ciphertext, key, iv)
    if decrypted ~= plaintext then
        print("CTR decrypt failed:", decrypted, "expected:", plaintext)
        return 0
    end

    return 1
end
