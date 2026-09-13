-- Bitwise OR: both operands T_INT → result T_INT; declared as int64_t.
-- 12 | 3 = 15
function test()
    local x = 12
    local y = x | 3
    return y
end
