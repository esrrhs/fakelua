function test_gsub_bad_func_return_bool()
    -- string.gsub 函数替换时，函数返回值不能是 Bool，标准 Lua 会报错
    string.gsub("hello", "l", function()
        return true
    end)
    return 0
end
