package "CryptoTest"

-- RC4 test vectors (keystream only, from B-Con)
-- Hex values converted to decimal for fakelua compatibility
function test_rc4_keystream()
    -- Test 1: key="Key", 10 bytes
    local key = "Key"
    local zeros = string.rep("\0", 10)
    local ks = crypto.rc4(key, zeros)
    local expected1 = {235,159,119,129,183,52,202,114,167,25}
    if #ks ~= 10 then
        print("rc4 keystream test 1: wrong length " .. #ks)
        return 0
    end
    for i = 1, 10 do
        if string.byte(ks, i) ~= expected1[i] then
            print("rc4 keystream test 1: mismatch at byte " .. i)
            return 0
        end
    end

    -- Test 2: key="Wiki", 6 bytes
    key = "Wiki"
    zeros = string.rep("\0", 6)
    ks = crypto.rc4(key, zeros)
    local expected2 = {96,68,219,109,65,183}
    if #ks ~= 6 then
        print("rc4 keystream test 2: wrong length " .. #ks)
        return 0
    end
    for i = 1, 6 do
        if string.byte(ks, i) ~= expected2[i] then
            print("rc4 keystream test 2: mismatch at byte " .. i)
            return 0
        end
    end

    -- Test 3: key="Secret", 8 bytes
    key = "Secret"
    zeros = string.rep("\0", 8)
    ks = crypto.rc4(key, zeros)
    local expected3 = {4,212,107,5,60,168,123,89}
    if #ks ~= 8 then
        print("rc4 keystream test 3: wrong length " .. #ks)
        return 0
    end
    for i = 1, 8 do
        if string.byte(ks, i) ~= expected3[i] then
            print("rc4 keystream test 3: mismatch at byte " .. i)
            return 0
        end
    end

    return 1
end

-- RC4 is symmetric: encrypt twice with same key = decrypt
function test_rc4_encrypt_decrypt()
    local key = "MySecretKey123"
    local plaintext = "Hello, World! This is a test of RC4 encryption."

    local encrypted = crypto.rc4(key, plaintext)
    local decrypted = crypto.rc4(key, encrypted)

    if decrypted ~= plaintext then
        print("rc4 encrypt/decrypt roundtrip failed")
        return 0
    end

    -- Verify encrypted differs from plaintext
    if encrypted == plaintext then
        print("rc4 output same as input")
        return 0
    end

    return 1
end
