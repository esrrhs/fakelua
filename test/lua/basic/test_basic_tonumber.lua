function test_basic_tonumber()
    -- 基本转换
    if tonumber("42") ~= 42 then return 1 end
    if math.abs(tonumber("3.14") - 3.14) > 0.001 then return 2 end
    if tonumber("-100") ~= -100 then return 3 end

    -- 已经是数字
    if tonumber(42) ~= 42 then return 4 end
    if math.abs(tonumber(3.14) - 3.14) > 0.001 then return 5 end

    -- 无效输入
    if tonumber("abc") ~= nil then return 6 end
    if tonumber("") ~= nil then return 7 end
    if tonumber("abc123") ~= nil then return 8 end

    -- 带 base 参数
    if tonumber("ff", 16) ~= 255 then return 9 end
    if tonumber("1010", 2) ~= 10 then return 10 end
    if tonumber("z", 36) ~= 35 then return 11 end

    -- 非法 base
    if tonumber("123", 1) ~= nil then return 12 end
    if tonumber("123", 37) ~= nil then return 13 end

    return 5000
end
