function test_gsub_bad_table_value_bool()
    -- Lua 5.4 语义：表值为 nil 或 false 时保留原匹配（不报错、不删除）。
    local keep = { l = false }
    local r1, c1 = string.gsub("hello", "l", keep)
    if r1 ~= "hello" or c1 ~= 2 then return 0 end

    local missing = {}
    local r2, c2 = string.gsub("abc", "b", missing)
    if r2 ~= "abc" or c2 ~= 1 then return 0 end

    -- 表值为 true（非 nil/false 的布尔）仍然是非法替换值
    local bad = { a = true }
    local ok = pcall(function()
        string.gsub("a", "a", bad)
    end)
    if ok then return 0 end

    return 1
end
