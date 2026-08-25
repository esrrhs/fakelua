function test_table_boundary()
    -- 1. table.remove: 空表返回 nil
    if table.remove({}) ~= nil then return 1 end

    -- 2. table.remove: 越界位置返回 nil，表不变
    local t = {10, 20}
    if table.remove(t, 5) ~= nil then return 2 end
    if #t ~= 2 then return 3 end

    -- 3. table.insert: 越界位置不插入（标准 Lua 行为）
    local t2 = {1, 2, 3}
    table.insert(t2, 10, 99)
    if t2[4] ~= nil then return 4 end
    if t2[10] ~= nil then return 7 end

    -- 4. table.insert: 不指定位置默认追加到末尾
    local t3 = {1, 2}
    table.insert(t3, 3)
    if t3[3] ~= 3 then return 5 end

    -- 5. table.create: count 为 0 返回空表
    local t4 = table.create(0)
    if #t4 ~= 0 then return 6 end

    return 5000
end
