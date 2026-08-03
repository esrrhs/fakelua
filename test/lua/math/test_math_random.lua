function test_math_random()
    math.randomseed(12345)
    local r1 = math.random()
    if not (r1 >= 0.0 and r1 < 1.0) then return 0 end

    local r2 = math.random(10)
    if not (r2 >= 1 and r2 <= 10) then return 0 end

    local r3 = math.random(50, 100)
    if not (r3 >= 50 and r3 <= 100) then return 0 end

    -- 极值边界测试：l >= u
    local r4 = math.random(100, 100)
    if r4 ~= 100 then return 0 end

    local r5 = math.random(50, 50)
    if r5 ~= 50 then return 0 end

    return 300
end
