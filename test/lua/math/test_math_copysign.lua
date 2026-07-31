function test_math_copysign()
    -- copysign(1.0, -2.0) = -1.0
    local c1 = math.copysign(1.0, -2.0)
    if c1 >= 0 then return 0 end

    -- copysign(-1.0, 2.0) = 1.0
    local c2 = math.copysign(-1.0, 2.0)
    if c2 < 0 then return 0 end

    -- copysign(3.14, -1) = -3.14
    local c3 = math.copysign(3.14, -1)
    if math.abs(c3 - (-3.14)) > 1e-6 then return 0 end

    -- copysign(-5.5, 0.0) = 5.5 (zero is positive)
    local c4 = math.copysign(-5.5, 0.0)
    if c4 < 0 then return 0 end

    -- 整数参数也应工作
    local c5 = math.copysign(10, -20)
    if c5 >= 0 then return 0 end

    return 7000
end
