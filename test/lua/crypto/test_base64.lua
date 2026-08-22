package "CryptoTest"

function test_base64()
    -- Test vectors from RFC 4648
    local inputs = {"", "f", "fo", "foo", "foob", "fooba", "foobar", "Hello, World!"}
    local expecteds = {"", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==", "Zm9vYmE=", "Zm9vYmFy", "SGVsbG8sIFdvcmxkIQ=="}

    for i = 1, #inputs do
        local encoded = crypto.base64_encode(inputs[i])
        if encoded ~= expecteds[i] then
            print("base64_encode failed for '" .. inputs[i] .. "': got '" .. encoded .. "' expected '" .. expecteds[i] .. "'")
            return 0
        end

        local decoded = crypto.base64_decode(encoded)
        if decoded ~= inputs[i] then
            print("base64_decode failed for '" .. encoded .. "': got '" .. decoded .. "' expected '" .. inputs[i] .. "'")
            return 0
        end
    end

    return 1
end
