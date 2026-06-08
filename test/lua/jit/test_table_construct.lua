function test_table_construct()
    local t = { ["a"] = 1, [2] = 2, [true] = 3 }
    return t["a"] + t[2] + t[true]
end
