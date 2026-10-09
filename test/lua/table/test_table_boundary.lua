function test_table_boundary()
    -- 1. table.remove: 空表返回 nil
    if table.remove({}) ~= nil then return 1 end

    -- 2. table.remove: pos == #t+1 合法，返回 nil，表不变
    local t = { 10, 20 }
    if table.remove(t, 3) ~= nil then return 2 end
    if #t ~= 2 then return 3 end

    -- 2b. 更远的越界位置：Lua 5.4 报 position out of bounds
    local ok_rm = pcall(function() table.remove(t, 5) end)
    if ok_rm then return 8 end

    -- 3. table.insert: 越界位置 Lua 5.4 报错，不插入也不挖洞
    local t2 = { 1, 2, 3 }
    local ok_ins = pcall(function() table.insert(t2, 10, 99) end)
    if ok_ins then return 4 end
    if t2[4] ~= nil then return 7 end
    if t2[10] ~= nil then return 9 end

    -- 4. table.insert: 不指定位置默认追加到末尾
    local t3 = { 1, 2 }
    table.insert(t3, 3)
    if t3[3] ~= 3 then return 5 end

    -- 5. table.create: count 为 0 返回空表
    local t4 = table.create(0)
    if #t4 ~= 0 then return 6 end

    return 5000
end
