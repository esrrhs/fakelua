function test_table_concat_boundary()
    -- 1. 空表
    local t1 = {}
    local s1 = table.concat(t1)
    if s1 ~= "" then return 1 end

    -- 2. start > end
    local t2 = {"a", "b", "c"}
    local s2 = table.concat(t2, ",", 3, 1)
    if s2 ~= "" then return 2 end

    -- 3. 单元素表
    local t5 = {"only"}
    local s5 = table.concat(t5, "-")
    if s5 ~= "only" then return 3 end

    -- 4. 多元素拼接
    local t6 = {"x", "y", "z"}
    local s6 = table.concat(t6, "-")
    if s6 ~= "x-y-z" then return 4 end

    return 5000
end
