-- Type inference: negative modulo with Lua semantics.
-- -7 % 2 = 1 (Lua: a - b*floor(a/b) = -7 - 2*(-4) = -7+8 = 1).
function test()
    local x = -7 % 2
    return x
end
