function test_math_exp_log()
    local eps = 1e-10

    -- 1. exp(0) = 1
    if math.abs(math.exp(0) - 1) > eps then return 1 end

    -- 2. exp(1) = e (approximately 2.71828)
    local e = math.exp(1)
    if e < 2.7 or e > 2.8 then return 2 end

    -- 3. log(e) = 1
    if math.abs(math.log(e) - 1) > 0.01 then return 3 end

    -- 4. log10(10) = 1
    if math.abs(math.log10(10) - 1) > eps then return 4 end

    -- 5. log10(100) = 2
    if math.abs(math.log10(100) - 2) > eps then return 5 end

    -- 6. log10(1) = 0
    if math.abs(math.log10(1) - 0) > eps then return 6 end

    return 5000
end
