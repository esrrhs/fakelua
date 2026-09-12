package "CryptoTest"

-- RFC 4122 v4: 8-4-4-4-12 hex, version nibble 4, variant 8/9/a/b
function test_uuid()
    local a = crypto.uuid()
    local b = crypto.uuid()
    if type(a) ~= "string" or #a ~= 36 then
        print("uuid length:", type(a), tostring(a))
        return 0
    end
    if a == b then
        print("uuid not unique:", a)
        return 0
    end
    if not string.find(a, "^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$") then
        print("uuid format:", a)
        return 0
    end
    if not string.find(b, "^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$") then
        print("uuid format:", b)
        return 0
    end
    return 1
end
