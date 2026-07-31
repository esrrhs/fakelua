function test_string_charpattern()
    -- string.charpattern 应该是匹配任意单个字符的模式
    local pat = string.charpattern
    if type(pat) ~= "string" then return 0 end

    -- 用 string.match + charpattern 匹配单个字符
    local matched = string.match("hello", pat)
    if matched ~= "h" then return 0 end

    -- charpattern 应该匹配任意非空字符
    local digits = "12345"
    local count = 0
    for c in string.gmatch(digits, pat) do
        count = count + 1
    end
    if count ~= 5 then return 0 end

    -- 空字符串不匹配
    local m2 = string.match("", pat)
    if m2 ~= nil then return 0 end

    -- charpattern 应该能匹配特殊字符
    local m3 = string.match("!@#", pat)
    if m3 ~= "!" then return 0 end

    return 6000
end
