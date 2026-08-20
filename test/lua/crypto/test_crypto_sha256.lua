package "CryptoTest"

-- SHA256("") = e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
function test_sha256()
    local h = crypto.sha256("")
    if h ~= "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" then
        print("sha256 empty failed:", h)
        return 0
    end
    return 1
end
