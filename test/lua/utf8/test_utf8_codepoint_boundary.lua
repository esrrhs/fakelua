function test_utf8_codepoint_boundary()
    -- 1. i > j（应返回 nil）
    local r2 = utf8.codepoint("ABC", 3, 1)
    if r2 ~= nil then return 1 end

    -- 2. 空字符串
    local r5 = utf8.codepoint("", 1, 1)
    if r5 ~= nil then return 2 end

    -- 3. 正常范围
    local a = utf8.codepoint("ABC", 1, 3)
    if a ~= 65 then return 3 end

    return 5000
end
