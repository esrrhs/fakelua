function test_string_boundary()
    -- 1. string.sub: 负索引
    if string.sub("hello", -2) ~= "lo" then return 1 end

    -- 2. string.sub: 结束位置越界，截断到串尾
    if string.sub("hello", 1, 100) ~= "hello" then return 2 end

    -- 3. string.sub: 起始 > 结束，返回空串
    if string.sub("hello", 3, 2) ~= "" then return 3 end

    -- 4. string.sub: 空串
    if string.sub("", 1, 1) ~= "" then return 4 end

    -- 5. string.sub: 双负索引
    if string.sub("hello", -3, -1) ~= "llo" then return 5 end

    -- 6. string.byte: 空串返回 nil
    if string.byte("") ~= nil then return 6 end

    -- 7. string.char: 无参数返回空串
    if string.char() ~= "" then return 7 end

    -- 8. string.find: 空模式匹配到位置 1, 0
    local a, b = string.find("hello", "", 1)
    if a ~= 1 or b ~= 0 then return 8 end

    -- 9. string.gsub: 替换次数限制 n=2
    local s, n = string.gsub("aaa", "a", "b", 2)
    if s ~= "bba" or n ~= 2 then return 9 end

    -- 10. string.gsub: n=0 不做替换，仅返回原串与计数 0
    local s2, n2 = string.gsub("aaa", "a", "b", 0)
    if s2 ~= "aaa" or n2 ~= 0 then return 10 end

    return 5000
end
