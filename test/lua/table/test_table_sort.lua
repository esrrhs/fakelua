function test_table_sort()
    -- 1. 整数默认升序排序
    local t1 = {5, 2, 8, 1, 9}
    table.sort(t1)
    if t1[1] ~= 1 or t1[2] ~= 2 or t1[3] ~= 5 or t1[4] ~= 8 or t1[5] ~= 9 then
        return 1.0
    end

    -- 2. 浮点数默认升序排序
    local t2 = {5.5, 2.1, 8.9, 1.0, 3.14}
    table.sort(t2)
    if t2[1] ~= 1.0 or t2[2] ~= 2.1 or t2[3] ~= 3.14 or t2[4] ~= 5.5 or t2[5] ~= 8.9 then
        return 2.0
    end

    -- 3. 混合数自定义闭包降序排序
    local t3 = {10, 50.5, 20, 40.2, 30}
    table.sort(t3, function(a, b) return a > b end)
    if t3[1] ~= 50.5 or t3[2] ~= 40.2 or t3[3] ~= 30 or t3[4] ~= 20 or t3[5] ~= 10 then
        return 3.0
    end

    -- 4. Table 嵌套对象数组按内部属性字段排序
    local o1 = { id = 3, name = "C" }
    local o2 = { id = 1, name = "A" }
    local o3 = { id = 4, name = "D" }
    local o4 = { id = 2, name = "B" }
    local t4 = { o1, o2, o3, o4 }
    table.sort(t4, function(a, b) return a.id < b.id end)
    if t4[1].name ~= "A" or t4[2].name ~= "B" or t4[3].name ~= "C" or t4[4].name ~= "D" then
        return 4.0
    end

    return 100.0
end
