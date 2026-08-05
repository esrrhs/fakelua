function test_string_sub()
    local s = "hello world"
    if string.sub(s, 1, 5) ~= "hello" then return 0 end
    if string.sub(s, 7) ~= "world" then return 0 end
    if string.sub(s, -5) ~= "world" then return 0 end

    -- 验证数字参数隐式转换 (Lua 标准规范)
    if string.sub(12345, 2, 4) ~= "234" then return 0 end

    return 200
end
