package "CryptoTest"

-- MD5("hello") = 5d41402abc4b2a76b9719d911017c592
function test_md5_hello()
    local h = crypto.md5("hello")
    if h ~= "5d41402abc4b2a76b9719d911017c592" then
        print("md5 hello failed:", h)
        return 0
    end
    return 1
end
