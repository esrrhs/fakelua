function test_math_constants_full()
    -- 1. pi 精度验证
    if math.abs(math.pi - 3.141592653589793) > 0.01 then return 1 end

    -- 2. huge 是无穷大
    if not (math.huge > 1e308) then return 2 end
    if not (-math.huge < -1e308) then return 3 end

    -- 3. maxinteger 边界
    if math.maxinteger <= 0 then return 4 end
    if math.type(math.maxinteger) ~= "integer" then return 5 end

    -- 4. mininteger 边界
    if math.mininteger >= 0 then return 6 end
    if math.type(math.mininteger) ~= "integer" then return 7 end

    -- 5. pi 与 huge 关系
    if not (math.pi < math.huge) then return 8 end

    return 5000
end
