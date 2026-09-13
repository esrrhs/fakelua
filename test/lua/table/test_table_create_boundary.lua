function test_table_create_boundary()
    -- 1. count = 0
    local t1 = table.create(0)
    if #t1 ~= 0 then return 1 end

    -- 2. 无第二个参数
    local t2 = table.create(3)
    if #t2 ~= 0 then return 2 end
    if t2[1] ~= nil then return 3 end

    -- 3. 负 count
    local t3 = table.create(-5)
    if #t3 ~= 0 then return 4 end

    -- 4. 大 count（触发 bucket 路径，> 8 个 quick_data 槽）
    local t4 = table.create(20, 42)
    if #t4 ~= 20 then return 5 end
    if t4[1] ~= 42 or t4[10] ~= 42 or t4[20] ~= 42 then return 6 end

    -- 5. 字符串填充值
    local t5 = table.create(5, "x")
    if #t5 ~= 5 then return 7 end
    if t5[3] ~= "x" then return 8 end

    -- 6. 整数 count 边界（1 个元素）
    local t6 = table.create(1, "only")
    if #t6 ~= 1 then return 9 end
    if t6[1] ~= "only" then return 10 end

    return 5000
end
