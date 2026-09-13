function test_math_floor_ceil()
    -- 1. floor 正数
    if math.floor(3.9) ~= 3 then return 0 end

    -- 2. floor 负数
    if math.floor(-3.1) ~= -4 then return 0 end

    -- 3. floor 整数
    if math.floor(5) ~= 5 then return 0 end

    -- 4. floor 零
    if math.floor(0.0) ~= 0 then return 0 end

    -- 5. ceil 正数
    if math.ceil(3.1) ~= 4 then return 0 end

    -- 6. ceil 负数
    if math.ceil(-3.9) ~= -3 then return 0 end

    -- 7. ceil 整数
    if math.ceil(5) ~= 5 then return 0 end

    -- 8. 字符串隐式转换
    if math.floor("3.9") ~= 3 then return 0 end
    if math.ceil("3.1") ~= 4 then return 0 end

    return 5000
end
