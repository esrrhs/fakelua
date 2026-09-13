function test_math_sqrt()
    -- 1. 完全平方数
    if math.sqrt(16) ~= 4 then return 0 end
    if math.sqrt(100) ~= 10 then return 0 end

    -- 2. 非完全平方数
    local r = math.sqrt(2)
    if math.abs(r - 1.4142135623730951) > 1e-10 then return 0 end

    -- 3. 0
    if math.sqrt(0) ~= 0 then return 0 end

    -- 4. 1
    if math.sqrt(1) ~= 1 then return 0 end

    -- 5. 大数
    local big = math.sqrt(1000000)
    if big ~= 1000 then return 0 end

    -- 6. 浮点数
    if math.sqrt(0.25) ~= 0.5 then return 0 end

    return 5000
end
