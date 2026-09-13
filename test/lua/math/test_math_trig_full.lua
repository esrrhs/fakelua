function test_math_trig_full()
    local eps = 1e-10

    -- 1. sin 特殊角度
    if math.abs(math.sin(0) - 0) > eps then return 1 end
    if math.abs(math.sin(math.pi / 6) - 0.5) > eps then return 2 end
    if math.abs(math.sin(math.pi / 2) - 1) > eps then return 3 end

    -- 2. cos 特殊角度
    if math.abs(math.cos(0) - 1) > eps then return 4 end
    if math.abs(math.cos(math.pi / 3) - 0.5) > eps then return 5 end
    if math.abs(math.cos(math.pi) - (-1)) > eps then return 6 end

    -- 3. tan 特殊角度
    if math.abs(math.tan(0) - 0) > eps then return 7 end
    if math.abs(math.tan(math.pi / 4) - 1) > eps then return 8 end

    -- 4. asin 反函数
    if math.abs(math.asin(0) - 0) > eps then return 9 end
    -- asin(1) should be pi/2 (approximately 1.5708)
    if math.abs(math.asin(1) - math.pi / 2) > 0.01 then return 10 end
    if math.abs(math.asin(-1) - (-math.pi / 2)) > 0.01 then return 11 end

    -- 5. acos 反函数
    if math.abs(math.acos(1) - 0) > 0.01 then return 12 end
    if math.abs(math.acos(-1) - math.pi) > 0.01 then return 13 end
    if math.abs(math.acos(0) - math.pi / 2) > 0.01 then return 14 end

    -- 6. atan 反函数
    if math.abs(math.atan(0) - 0) > 0.01 then return 15 end
    if math.abs(math.atan(1) - math.pi / 4) > 0.01 then return 16 end

    -- 7. 字符串参数隐式转换
    if math.abs(math.sin("0") - 0) > 0.01 then return 17 end
    if math.abs(math.cos("0") - 1) > 0.01 then return 18 end

    return 5000
end
