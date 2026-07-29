function test_table_concat()
    local t = {"hello", "world", "fakelua"}
    local s1 = table.concat(t, ", ")
    local s2 = table.concat(t, "-", 2, 3)
    return (s1 == "hello, world, fakelua" and s2 == "world-fakelua") and 100.0 or 0.0
end
