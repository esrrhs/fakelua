package "CryptoTest"

-- SHA1("hello") = aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d
function test_sha1_hello()
    local h = crypto.sha1("hello")
    if h ~= "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d" then
        print("sha1 hello failed:", h)
        return 0
    end
    return 1
end
