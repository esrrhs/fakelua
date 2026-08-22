package "CryptoTest"

-- DES test: encrypt then decrypt should roundtrip
function test_des()
    local key = "DESkey12"  -- 8 bytes
    local plaintext = "HelloWorld"  -- 10 bytes, zero-padded to 16

    local encrypted = crypto.des_encrypt(key, plaintext)
    local decrypted = crypto.des_decrypt(key, encrypted)

    if string.sub(decrypted, 1, #plaintext) ~= plaintext then
        print("des roundtrip failed")
        return 0
    end

    -- Verify encrypted differs from plaintext
    if string.sub(encrypted, 1, #plaintext) == plaintext then
        print("des output same as input")
        return 0
    end

    -- Test with exact 8-byte block
    local block8 = "12345678"
    local enc8 = crypto.des_encrypt(key, block8)
    local dec8 = crypto.des_decrypt(key, enc8)
    if dec8 ~= block8 then
        print("des 8-byte block roundtrip failed")
        return 0
    end

    return 1
end

-- 3DES test: encrypt then decrypt should roundtrip
function test_triple_des()
    local key = "123456789012345678901234"  -- 24 bytes
    local plaintext = "HelloWorld"  -- 10 bytes, zero-padded to 16

    local encrypted = crypto.triple_des_encrypt(key, plaintext)
    local decrypted = crypto.triple_des_decrypt(key, encrypted)

    if string.sub(decrypted, 1, #plaintext) ~= plaintext then
        print("triple des roundtrip failed")
        return 0
    end

    -- Verify encrypted differs from plaintext
    if string.sub(encrypted, 1, #plaintext) == plaintext then
        print("triple des output same as input")
        return 0
    end

    return 1
end
