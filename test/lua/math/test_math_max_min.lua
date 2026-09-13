function test_math_max_min()
    -- 1. 多个参数（变长）
    if math.max(1, 5, 3, 9, 2) ~= 9 then return 0 end
    if math.min(1, 5, 3, 9, 2) ~= 1 then return 0 end

    -- 2. 整数和浮点混合
    if math.max(1, 2.5, 3) ~= 3 then return 0 end
    if math.min(1, 0.5, 3) ~= 0.5 then return 0 end

    -- 3. 字符串数字隐式转换
    if math.max("10", 25, 5, 100, 30) ~= 100 then return 0 end
    if math.min("10", 25, 5, 100, 30) ~= 5 then return 0 end

    -- 4. 单参数
    if math.max(42) ~= 42 then return 0 end
    if math.min(42) ~= 42 then return 0 end

    -- 5. 两个参数
    if math.max(10, 20) ~= 20 then return 0 end
    if math.min(10, 20) ~= 10 then return 0 end

    -- 6. 负数
    if math.max(-5, -1, -10) ~= -1 then return 0 end
    if math.min(-5, -1, -10) ~= -10 then return 0 end

    return 5000
end
