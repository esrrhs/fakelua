function test_math_abs_bad_tbl()
    -- math.abs arg #1 must be a number; Table is invalid per standard Lua
    local t = {1, 2, 3}
    math.abs(t)
    return 0
end
