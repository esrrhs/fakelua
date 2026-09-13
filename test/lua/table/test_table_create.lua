function test_table_create()
    -- 1. 创建指定长度但不赋初值
    local t1 = table.create(5)
    if #t1 ~= 0 then return 1.0 end

    -- 2. 创建指定长度且用默认初值填充
    local t2 = table.create(4, "hello")
    if #t2 ~= 4 then return 2.0 end
    if t2[1] ~= "hello" or t2[2] ~= "hello" or t2[3] ~= "hello" or t2[4] ~= "hello" then
        return 3.0
    end

    -- 3. 创建数字充填
    local t3 = table.create(3, 99)
    if t3[1] ~= 99 or t3[2] ~= 99 or t3[3] ~= 99 then
        return 4.0
    end

    return 100.0
end
