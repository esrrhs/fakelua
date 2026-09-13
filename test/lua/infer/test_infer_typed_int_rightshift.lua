-- Right shift: both operands T_INT → result T_INT; declared as int64_t.
-- 256 >> 3 = 32
function test()
    local x = 256
    local y = x >> 3
    return y
end
