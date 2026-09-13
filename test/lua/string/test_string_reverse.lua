function test_string_reverse()
    -- 1. 普通字符串
    if string.reverse("hello") ~= "olleh" then return 0 end

    -- 2. 空字符串
    if string.reverse("") ~= "" then return 0 end

    -- 3. 单字符
    if string.reverse("x") ~= "x" then return 0 end

    -- 4. 回文字符串
    if string.reverse("madam") ~= "madam" then return 0 end

    -- 5. 数字字符串
    if string.reverse("12345") ~= "54321" then return 0 end

    -- 6. 含空格
    if string.reverse("a b c") ~= "c b a" then return 0 end

    return 5000
end
