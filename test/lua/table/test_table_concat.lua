function test_table_concat()
    local t = {"hello", "world", "fakelua"}
    local s1 = table.concat(t, ", ")
    local s2 = table.concat(t, "-", 2, 3)

    -- Float 与数字字符串转换测试 (CVarToInteger 机制)
    local s3 = table.concat(t, "-", 2.0, 3.0)
    local s4 = table.concat(t, "-", "2", "3")

    -- 数字分隔符测试
    local s5 = table.concat({"a", "b"}, 123)
    if s5 ~= "a123b" then return 0.0 end

    -- Float 浮点数元素无尾随 0 格式化测试
    local s6 = table.concat({3.14})
    if s6 ~= "3.14" then return 0.0 end

    return (s1 == "hello, world, fakelua" and s2 == "world-fakelua" and s3 == "world-fakelua" and s4 == "world-fakelua") and 100.0 or 0.0
end
