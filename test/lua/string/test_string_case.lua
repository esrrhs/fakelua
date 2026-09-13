function test_string_case()
    local s = "Hello World"
    if string.lower(s) ~= "hello world" then return 0 end
    if string.upper(s) ~= "HELLO WORLD" then return 0 end
    if string.reverse("abc") ~= "cba" then return 0 end

    -- 验证数字参数隐式转换 (Lua 标准规范)
    if string.reverse(12345) ~= "54321" then return 0 end
    if string.lower(12345) ~= "12345" then return 0 end
    if string.upper(12345) ~= "12345" then return 0 end

    return 400
end
