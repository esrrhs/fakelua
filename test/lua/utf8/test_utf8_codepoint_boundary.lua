function test_utf8_codepoint_boundary()
    -- 1. i > j：0 个值，赋给局部变量后是 nil
    local r2 = utf8.codepoint("ABC", 3, 1)
    if r2 ~= nil then return 1 end

    -- 2. 空串上 j=1 越出末尾：Lua 5.4 报 out of bounds
    local ok_empty, err_empty = pcall(function() utf8.codepoint("", 1, 1) end)
    if ok_empty or not string.find(tostring(err_empty), "out of bounds", 1, true) then return 2 end

    -- 3. 正常范围
    local a = utf8.codepoint("ABC", 1, 3)
    if a ~= 65 then return 3 end

    return 5000
end
