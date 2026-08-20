package "CryptoTest"

-- Blowfish test: encrypt then decrypt should roundtrip
function test_blowfish()
    local key = "MyBlowfishKey"
    local plaintext = "HelloWorld"  -- 10 bytes, will be zero-padded to 16

    local encrypted = crypto.blowfish_encrypt(key, plaintext)
    local decrypted = crypto.blowfish_decrypt(key, encrypted)

    -- decrypted is zero-padded to multiple of 8, compare first #plaintext bytes
    if string.sub(decrypted, 1, #plaintext) ~= plaintext then
        print("blowfish roundtrip failed")
        return 0
    end

    -- Verify encrypted differs from plaintext
    if string.sub(encrypted, 1, #plaintext) == plaintext then
        print("blowfish output same as input")
        return 0
    end

    -- Test with exact 8-byte block
    local block8 = "12345678"
    local enc8 = crypto.blowfish_encrypt(key, block8)
    local dec8 = crypto.blowfish_decrypt(key, enc8)
    if dec8 ~= block8 then
        print("blowfish 8-byte block roundtrip failed")
        return 0
    end

    return 1
end
