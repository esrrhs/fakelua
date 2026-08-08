function test_gsub_bad_table_value_bool()
    -- string.gsub 表替换时，表值不能是 Bool，标准 Lua 会报错
    local t = {l = true}
    string.gsub("hello", "l", t)
    return 0
end
