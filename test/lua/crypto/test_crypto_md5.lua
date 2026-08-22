package "CryptoTest"

-- MD5("") = d41d8cd98f00b204e9800998ecf8427e
function test_md5()
    local h = crypto.md5("")
    if h ~= "d41d8cd98f00b204e9800998ecf8427e" then
        print("md5 empty failed:", h)
        return 0
    end
    return 1
end
