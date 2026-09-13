function test_math_abs_bad_arg()
    -- math.abs arg #1 must be a number; Bool is invalid per standard Lua
    math.abs(true)
    return 0
end
