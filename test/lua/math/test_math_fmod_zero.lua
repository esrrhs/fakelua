-- Test math.fmod with divisor zero (returns NaN)

function test_math_fmod_zero()
    -- fmod(x, 0) should return NaN (represented as a number)
    local r = math.fmod(5, 0)
    -- NaN is not equal to itself
    if r == r then return 1 end

    -- Verify normal fmod works
    local r2 = math.fmod(10, 3)
    if math.abs(r2 - 1.0) > 0.0001 then return 2 end

    -- fmod with float
    local r3 = math.fmod(10.5, 3)
    if math.abs(r3 - 1.5) > 0.0001 then return 3 end

    return 5000
end
