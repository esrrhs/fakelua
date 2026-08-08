function test_math_tointeger_bad_tbl()
    -- math.tointeger requires a number argument; Table is invalid per standard Lua
    local t = {1, 2, 3}
    math.tointeger(t)
    return 0
end
