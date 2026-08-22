package "CryptoTest"

-- SHA1("") = da39a3ee5e6b4b0d3255bfef95601890afd80709
function test_sha1()
    local h = crypto.sha1("")
    if h ~= "da39a3ee5e6b4b0d3255bfef95601890afd80709" then
        print("sha1 empty failed:", h)
        return 0
    end
    return 1
end
