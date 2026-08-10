function test_table_sort_boundary()
    -- 1. 空表
    local t1 = {}
    table.sort(t1)
    if #t1 ~= 0 then return 0 end

    -- 2. 单元素表
    local t2 = {42}
    table.sort(t2)
    if t2[1] ~= 42 then return 0 end

    -- 3. 已排序表
    local t3 = {1, 2, 3, 4, 5}
    table.sort(t3)
    if t3[1] ~= 1 or t3[5] ~= 5 then return 0 end

    -- 4. 逆序表
    local t4 = {5, 4, 3, 2, 1}
    table.sort(t4)
    if t4[1] ~= 1 or t4[5] ~= 5 then return 0 end

    -- 5. 所有元素相等
    local t5 = {7, 7, 7, 7}
    table.sort(t5)
    if t5[1] ~= 7 or t5[4] ~= 7 then return 0 end

    -- 6. 包含 nil 的表（排序应处理有效部分）
    local t6 = {3, 1, 2}
    table.sort(t6)
    if t6[1] ~= 1 or t6[2] ~= 2 or t6[3] ~= 3 then return 0 end

    return 5000
end
