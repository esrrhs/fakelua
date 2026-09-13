function test_math_sqrt_bad_arg()
    math.sqrt({})
end

function test_math_sin_bad_arg()
    math.sin(true)
end

function test_math_fmod_bad_arg()
    math.fmod(1, {})
end

function test_math_randomseed_bad_table()
    math.randomseed({})
end

function test_math_modf_bad_arg()
    math.modf({})
end

-- NaN / 2^63 以前 JIT 会 (unsigned)float，属 UB。现在走 native，不得崩。
function test_math_randomseed_nan()
    math.randomseed(0 / 0)
    math.randomseed(2 ^ 63)
    local _ = math.random()
    return 5000
end
