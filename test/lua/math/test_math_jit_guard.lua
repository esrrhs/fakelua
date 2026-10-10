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

function test_math_sqrt_bad_string()
    math.sqrt("not a number")
end

function test_math_randomseed_out_of_range()
    math.randomseed(1e308)
end

function test_math_randomseed_fractional()
    math.randomseed(1.5)
end

-- Invalid floating-point seeds must be rejected without an invalid conversion.
function test_math_randomseed_nan()
    math.randomseed(0 / 0)
end

function test_math_randomseed_2pow63()
    math.randomseed(2 ^ 63)
end
