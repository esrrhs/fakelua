function test_math_atan2()
    -- atan2(0, 1) = 0
    local a1 = math.atan2(0, 1)
    if math.abs(a1 - 0.0) > 1e-6 then return 0 end

    -- atan2(1, 0) = pi/2
    local a2 = math.atan2(1, 0)
    if math.abs(a2 - math.pi / 2) > 1e-6 then return 0 end

    -- atan2(1, 1) = pi/4
    local a3 = math.atan2(1, 1)
    if math.abs(a3 - math.pi / 4) > 1e-6 then return 0 end

    -- atan2(-1, -1) = -3*pi/4
    local a4 = math.atan2(-1, -1)
    if math.abs(a4 - (-3 * math.pi / 4)) > 1e-6 then return 0 end

    -- 整数参数也应工作
    local a5 = math.atan2(0.0, -1.0)
    if math.abs(a5 - math.pi) > 1e-6 then return 0 end

    return 6000
end
