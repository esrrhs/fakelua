-- Left shift: both operands T_INT → result T_INT; declared as int64_t.
-- 1 << 4 = 16
function test()
    local x = 1
    local y = x << 4
    return y
end
