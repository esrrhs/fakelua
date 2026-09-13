-- Type inference: negative floor division with Lua semantics.
-- -7 // 2 = -4 (floors toward -inf, NOT truncates toward 0).
function test()
    local x = -7 // 2
    return x
end
