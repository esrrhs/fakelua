-- Bitwise NOT on integer literal → T_INT, declared as int64_t.
-- ~7 = -8 (two's complement bitwise NOT)
function test()
    local x = ~7
    return x
end
