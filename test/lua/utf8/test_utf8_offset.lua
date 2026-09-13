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

    -- n = 0: 寻址包含字节 i 的字符首字节位置 ("中文" = 6 字节，第 2 字节包含在 "中")
    local o7 = utf8.offset("中文", 0, 2)
    if o7 ~= 1 then return 0 end

    -- 负 n 从位置 i 往回数，不是从字符串末尾
    local o8 = utf8.offset("abc", -1, 2)
    if o8 ~= 1 then return 0 end
    local o9 = utf8.offset("abc", -1, 3)
    if o9 ~= 2 then return 0 end

    -- 空串：offset("", 1) 为 1（与 Lua 5.4 一致，n 在 [1, len+1] 范围内）
    local o10 = utf8.offset("", 1)
    if o10 ~= 1 then return 0 end

    -- 空串：offset("", 0) 为 1（Lua 5.4：n=0 从当前位置找字符起始，默认 i=1，返回 1）
    local o11 = utf8.offset("", 0, 1)
    if o11 ~= 1 then return 0 end

    -- 空串：offset("", 2) 为 nil
    local o12 = utf8.offset("", 2)
    if o12 ~= nil then return 0 end

    -- 单字符：offset("a", 2) 为 2（指向字符串末尾之后）
    local o13 = utf8.offset("a", 2)
    if o13 ~= 2 then return 0 end

    -- 单字符：offset("a", 3) 为 nil
    local o14 = utf8.offset("a", 3)
    if o14 ~= nil then return 0 end

    return 6000
end
