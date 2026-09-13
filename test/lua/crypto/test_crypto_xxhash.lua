package "CryptoTest"

-- xxHash64 seed 0: empty → ef46db3751d8e999
function test_xxhash()
    local empty = crypto.xxhash("")
    if empty ~= "ef46db3751d8e999" then
        print("xxhash empty failed:", empty)
        return 0
    end
    local hello = crypto.xxhash("hello")
    if hello ~= crypto.xxhash("hello") then
        print("xxhash hello not stable")
        return 0
    end
    if hello == empty then
        print("xxhash hello collided with empty")
        return 0
    end
    if #hello ~= 16 then
        print("xxhash hex length", #hello)
        return 0
    end
    return 1
end
