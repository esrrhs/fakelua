function test_math_fmod_ldexp()
    local eps = 1e-10

    -- 1. fmod 正数
    if math.abs(math.fmod(5.5, 2.0) - 1.5) > eps then return 0 end

    -- 2. fmod 负数被除数
    if math.abs(math.fmod(-5.5, 2.0) - (-1.5)) > eps then return 0 end

    -- 3. fmod 负数除数
    if math.abs(math.fmod(5.5, -2.0) - 1.5) > eps then return 0 end

    -- 4. fmod 整除
    if math.abs(math.fmod(6.0, 2.0) - 0.0) > eps then return 0 end

    -- 5. ldexp 基本
    if math.abs(math.ldexp(1.5, 3) - 12.0) > eps then return 0 end

    -- 6. ldexp 负指数
    if math.abs(math.ldexp(1.5, -1) - 0.75) > eps then return 0 end

    -- 7. ldexp 零指数
    if math.abs(math.ldexp(1.5, 0) - 1.5) > eps then return 0 end

    -- 8. ldexp 零尾数
    if math.abs(math.ldexp(0.0, 10) - 0.0) > eps then return 0 end

    return 5000
end
