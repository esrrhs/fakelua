function test_string_rep()
    local s = "abc"
    if string.rep(s, 3) ~= "abcabcabc" then return 0 end
    if string.rep(s, 3, "-") ~= "abc-abc-abc" then return 0 end

    -- Float 浮点数值参数与数字分隔符支持测试 (Lua 兼容)
    if string.rep(s, 3.0) ~= "abcabcabc" then return 0 end
    if string.rep("a", 3, 0) ~= "a0a0a" then return 0 end

    -- 首个参数为数字时的隐式字符串转换测试 (Lua 标准规范)
    if string.rep(123, 3) ~= "123123123" then return 0 end

    return 300
end
