function test_utf8_offset_boundary()
    -- 1. 基本偏移
    local s = "ABC"
    local r1 = utf8.offset(s, 1, 1)
    if r1 ~= 1 then return 1 end

    -- 2. 第二个字符位置
    local r2 = utf8.offset(s, 2, 1)
    if r2 ~= 2 then return 2 end

    -- 3. 最后一个字符
    local r3 = utf8.offset(s, 3, 1)
    if r3 ~= 3 then return 3 end

    return 5000
end
