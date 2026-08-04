function test_utf8_offset()
    -- ASCII: offset of 1st char is 1
    local o1 = utf8.offset("abc", 1)
    if o1 ~= 1 then return 0 end

    -- ASCII: offset of 2nd char is 2
    local o2 = utf8.offset("abc", 2)
    if o2 ~= 2 then return 0 end

    -- Mixed: "aé" = [a(1 byte)][é(2 bytes)]
    -- offset of 1st char = 1, offset of 2nd char (é) = 2
    local o3 = utf8.offset("aé", 1)
    if o3 ~= 1 then return 0 end

    local o4 = utf8.offset("aé", 2)
    if o4 ~= 2 then return 0 end

    -- 3-byte char: "€b" = [€(3 bytes)][b(1 byte)]
    -- offset of 1st char = 1, offset of 2nd char (b) = 4
    local o5 = utf8.offset("€b", 2)
    if o5 ~= 4 then return 0 end

    -- Negative n: -1 means last char
    local o6 = utf8.offset("abc", -1)
    if o6 ~= 3 then return 0 end

    -- n = 0: 寻址包含字节 i 的字符首字节位置 ("中文" = 6 字节，第 2 字节包含在 "中")
    local o7 = utf8.offset("中文", 0, 2)
    if o7 ~= 1 then return 0 end

    return 6000
end
