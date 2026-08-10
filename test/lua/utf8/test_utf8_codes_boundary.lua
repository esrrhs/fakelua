function test_utf8_codes_boundary()
    -- 1. 正常字符串（fakelua 简化实现返回字符串）
    local s = "AB"
    local r2 = utf8.codes(s)
    if r2 == nil then return 1 end

    -- 2. 单字符
    local r3 = utf8.codes("A")
    if r3 == nil then return 2 end

    return 5000
end
