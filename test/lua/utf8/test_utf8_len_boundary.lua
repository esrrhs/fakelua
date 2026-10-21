function test_utf8_len_boundary()
    -- 1. i > j 返回 0
    local r2 = utf8.len("ABC", 3, 1)
    if r2 ~= 0 then return 1 end

    -- 2. 空串上 j=1 越出末尾：Lua 5.4 报 final position out of bounds
    local ok_empty, err_empty = pcall(function() utf8.len("", 1, 1) end)
    if ok_empty or not string.find(tostring(err_empty), "out of bounds", 1, true) then return 2 end

    -- 3. 完整字符串长度
    local r5 = utf8.len("Hello")
    if r5 ~= 5 then return 3 end

    -- 4. 默认参数（完整长度）
    local r6 = utf8.len("ABC")
    if r6 ~= 3 then return 4 end

    return 5000
end
