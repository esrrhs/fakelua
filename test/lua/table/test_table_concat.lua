function test_table_concat()
    local t = { "hello", "world", "fakelua" }
    local s1 = table.concat(t, ", ")
    local s2 = table.concat(t, "-", 2, 3)

    -- Float 与数字字符串转换测试 (CVarToInteger 机制)
    local s3 = table.concat(t, "-", 2.0, 3.0)
    local s4 = table.concat(t, "-", "2", "3")

    -- table.insert 越界 pos：Lua 5.4 报 position out of bounds，表不变
    local ok_ins = pcall(function() table.insert(t, 100, "invalid") end)

    return (not ok_ins) and s1 == "hello, world, fakelua" and s2 == "world-fakelua" and s3 == "world-fakelua" and s4 == "world-fakelua" and
        100.0 or 0.0
end
