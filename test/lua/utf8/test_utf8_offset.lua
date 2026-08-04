function test_utf8_offset()
    -- ASCII: offset of 1st char is 1
    local o1 = utf8.offset("abc", 1)
    if o1 ~= 1 then return 0 end

    -- ASCII: offset of 2nd char is 2
    local o2 = utf8.offset("abc", 2)
    if o2 ~= 2 then return 0 end

    -- Mixed: "aé" = [a(1 byte)][é(2 bytes)]
    local o3 = utf8.offset("aé", 1)
    if o3 ~= 1 then return 0 end

    local o4 = utf8.offset("aé", 2)
    if o4 ~= 2 then return 0 end

    -- 3-byte char: "€b" = [€(3 bytes)][b(1 byte)]
    local o5 = utf8.offset("€b", 2)
    if o5 ~= 4 then return 0 end

    -- n = 0 且 i 指向续字节(byte 2, byte 3)时，应定位回该字符起始 byte 1
    local o0_seq = utf8.offset("€b", 0, 3)
    if o0_seq ~= 1 then return 0 end

    -- Negative n: -1 means last char
    local o6 = utf8.offset("abc", -1)
    if o6 ~= 3 then return 0 end

    return 6000
end
