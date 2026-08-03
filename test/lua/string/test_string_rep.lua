function test_string_rep()
    local s = "abc"
    if string.rep(s, 3) ~= "abcabcabc" then return 0 end
    if string.rep(s, 3, "-") ~= "abc-abc-abc" then return 0 end

    -- Float 浮点数值参数支持测试 (Lua 兼容)
    if string.rep(s, 3.0) ~= "abcabcabc" then return 0 end

    return 300
end
