package "CryptoTest"

function test_hex_roundtrip()
    local data = "Hello"
    local encoded = crypto.hex_encode(data)
    if encoded ~= "48656c6c6f" then return 0 end
    local decoded = crypto.hex_decode(encoded)
    if decoded ~= data then return 0 end
    local decoded_upper = crypto.hex_decode("48656C6C6F")
    if decoded_upper ~= data then return 0 end
    return 1
end

function test_hex_decode_invalid()
    crypto.hex_decode("GG")
    return 0
end
