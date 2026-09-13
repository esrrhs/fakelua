function test_math_tointeger_bad_arg()
    -- math.tointeger requires a number argument; Bool is invalid per standard Lua
    math.tointeger(true)
    return 0
end
