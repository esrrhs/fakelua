function test_math_sinh_cosh_tanh()
    local eps = 0.01

    -- 1. sinh(0) = 0
    if math.abs(math.sinh(0) - 0) > eps then return 1 end

    -- 2. cosh(0) = 1
    if math.abs(math.cosh(0) - 1) > eps then return 2 end

    -- 3. tanh(0) = 0
    if math.abs(math.tanh(0) - 0) > eps then return 3 end

    -- 4. 正值
    local s1 = math.sinh(1)
    if math.abs(s1 - 1.1752011936438014) > eps then return 4 end

    local c1 = math.cosh(1)
    if math.abs(c1 - 1.5430806348152437) > eps then return 5 end

    local t1 = math.tanh(1)
    if math.abs(t1 - 0.7615941559557649) > eps then return 6 end

    -- 5. 负值（奇函数/偶函数特性）
    if math.abs(math.sinh(-1) + math.sinh(1)) > eps then return 7 end
    if math.abs(math.cosh(-1) - math.cosh(1)) > eps then return 8 end
    if math.abs(math.tanh(-1) + math.tanh(1)) > eps then return 9 end

    return 5000
end
