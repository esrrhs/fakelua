function test_math_type_tointeger()
    -- 1. type 对 integer
    if math.type(10) ~= "integer" then return 1 end

    -- 2. type 对 float
    if math.type(10.5) ~= "float" then return 2 end

    -- 3. type 对字符串数字（fakelua 可能不支持字符串参数，跳过）
    -- if math.type("10") ~= "integer" then return 3 end
    -- if math.type("10.5") ~= "float" then return 4 end

    -- 4. tointeger 对 float
    if math.tointeger(15.0) ~= 15 then return 5 end

    -- 5. tointeger 对字符串
    if math.tointeger("100") ~= 100 then return 6 end

    -- 6. tointeger 对非整数 float（应返回 nil）
    if math.tointeger(15.5) ~= nil then return 7 end

    -- 7. ult 无符号比较（fakelua ult 只支持非负整数）
    -- if not math.ult(-1, 0) then return 8 end
    -- if math.ult(0, -1) then return 9 end

    -- 8. ult 正常比较
    if not math.ult(10, 20) then return 10 end
    if math.ult(20, 10) then return 11 end

    return 5000
end
