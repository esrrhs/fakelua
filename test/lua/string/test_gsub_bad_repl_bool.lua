function test_gsub_bad_repl_bool()
    -- string.gsub 的替换参数不能是 Bool，标准 Lua 会报错
    string.gsub("hello", "l", true)
    return 0
end
