-- Test math error paths: non-number arguments must throw
-- Covers: math.random / math.randomseed type validation

function test_math_random_bad_arg()
    math.random("bad")
end

function test_math_randomseed_bad_arg()
    math.randomseed("bad")
end
