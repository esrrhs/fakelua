package "CryptoTest"

-- CRC-32/ISO-HDLC (PKZIP): empty → 0, "123456789" → 0xCBF43926
function test_crc32()
    local empty = crypto.crc32("")
    if empty ~= 0 then
        print("crc32 empty failed:", empty)
        return 0
    end
    local check = crypto.crc32("123456789")
    if check ~= 3421780262 then
        print("crc32 check failed:", check)
        return 0
    end
    local hello = crypto.crc32("hello")
    if hello ~= 907060870 then
        print("crc32 hello failed:", hello)
        return 0
    end
    return 1
end
