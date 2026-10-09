function test_table_remove_boundary()
    -- 1. 从空表移除（应返回 nil）
    local t1 = {}
    local r1 = table.remove(t1)
    if r1 ~= nil then return 1 end
    if #t1 ~= 0 then return 2 end

    -- 2. 移除位置 0：Lua 报 position out of bounds
    local t2 = { 1, 2, 3 }
    local ok2 = pcall(function() table.remove(t2, 0) end)
    if ok2 then return 3 end
    if #t2 ~= 3 then return 4 end

    -- 3. 移除位置 len+1（Lua 合法，取不到元素，返回 nil，表不变）
    local t3 = { 1, 2, 3 }
    local r3 = table.remove(t3, 4)
    if r3 ~= nil then return 5 end
    if #t3 ~= 3 then return 6 end

    -- 3b. 位置 > len+1：报错
    local t3b = { 1, 2, 3 }
    local ok3b = pcall(function() table.remove(t3b, 5) end)
    if ok3b then return 7 end

    -- 3c. 负数位置：报错
    local t3c = { 1, 2, 3 }
    local ok3c = pcall(function() table.remove(t3c, -1) end)
    if ok3c then return 8 end

    -- 4. 移除最后一个元素
    local t4 = { 42 }
    local r4 = table.remove(t4)
    if r4 ~= 42 then return 9 end

    -- 5. 移除中间元素，后续元素前移
    local t5 = { 10, 20, 30, 40 }
    local r5 = table.remove(t5, 2)
    if r5 ~= 20 then return 10 end
    if t5[1] ~= 10 or t5[2] ~= 30 or t5[3] ~= 40 then return 11 end

    return 5000
end
