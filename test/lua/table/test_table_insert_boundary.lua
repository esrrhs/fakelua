function test_table_insert_boundary()
    -- 1. 插入到位置 1（头部）
    local t1 = { 2, 3, 4 }
    table.insert(t1, 1, 1)
    if t1[1] ~= 1 or t1[2] ~= 2 or t1[3] ~= 3 or t1[4] ~= 4 then return 1 end

    -- 2. 插入到 len+1（尾部，默认行为）
    local t2 = { 1, 2, 3 }
    table.insert(t2, 4)
    if t2[4] ~= 4 then return 2 end

    -- 3. pos > len+1：Lua 报 position out of bounds，表保持不变
    local t3 = { 1, 2, 3 }
    local ok3 = pcall(function() table.insert(t3, 100, 99) end)
    if ok3 then return 3 end
    if #t3 ~= 3 or t3[100] ~= nil then return 4 end

    -- 4. 连续插入触发 bucket 路径（> 8 个元素）
    local t4 = {}
    for i = 1, 15 do
        table.insert(t4, i * 10)
    end
    if #t4 ~= 15 then return 5 end
    if t4[9] ~= 90 or t4[15] ~= 150 then return 6 end

    -- 5. pos=0：报错，不得插入、不得写下标 0
    local t0 = { 1, 2, 3 }
    local ok0 = pcall(function() table.insert(t0, 0, 99) end)
    if ok0 then return 7 end
    if #t0 ~= 3 or t0[0] ~= nil or t0[1] ~= 1 then return 8 end

    -- 6. 负数位置：报错
    local tn = { 1, 2, 3 }
    local okn = pcall(function() table.insert(tn, -1, 99) end)
    if okn then return 9 end
    if #tn ~= 3 then return 10 end

    -- 7. mininteger：报错（Lua 无符号比较兜住，fakelua 以前会下溢死循环）
    local tmin = { 1, 2, 3 }
    local okm = pcall(function() table.insert(tmin, math.mininteger, 99) end)
    if okm then return 11 end
    if #tmin ~= 3 or tmin[1] ~= 1 then return 12 end

    -- 8. 非整数位置：报 number has no integer representation
    local tf = { 1, 2, 3 }
    local okf = pcall(function() table.insert(tf, 1.5, 99) end)
    if okf then return 13 end

    return 5000
end
