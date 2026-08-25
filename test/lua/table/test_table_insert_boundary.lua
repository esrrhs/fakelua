function test_table_insert_boundary()
    -- 1. 插入到位置 1（头部）
    local t1 = {2, 3, 4}
    table.insert(t1, 1, 1)
    if t1[1] ~= 1 or t1[2] ~= 2 or t1[3] ~= 3 or t1[4] ~= 4 then return 1 end

    -- 2. 插入到 len+1（尾部，默认行为）
    local t2 = {1, 2, 3}
    table.insert(t2, 4)
    if t2[4] ~= 4 then return 2 end

    -- 3. 插入到越界位置（pos > len+1，应不插入）
    local t3 = {1, 2, 3}
    table.insert(t3, 100, 99)
    if #t3 ~= 3 then return 3 end
    -- JIT 以前仍会 FlSetTableInt(t, 100, 99) 挖洞
    if t3[100] ~= nil then return 6 end

    -- 4. 连续插入触发 bucket 路径（> 8 个元素）
    local t4 = {}
    for i = 1, 15 do
        table.insert(t4, i * 10)
    end
    if #t4 ~= 15 then return 4 end
    if t4[9] ~= 90 or t4[15] ~= 150 then return 5 end

    -- pos=0：不得插入、不得写下标 0
    local t0 = {1, 2, 3}
    table.insert(t0, 0, 99)
    if #t0 ~= 3 or t0[0] ~= nil or t0[1] ~= 1 then return 7 end

    -- mininteger：JIT 以前 for (idx >= pos; idx--) 下溢死循环
    local tmin = {1, 2, 3}
    table.insert(tmin, math.mininteger, 99)
    if #tmin ~= 3 or tmin[1] ~= 1 then return 8 end

    return 5000
end
