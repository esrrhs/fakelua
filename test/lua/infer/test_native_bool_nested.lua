-- Deeply nested native bool expression: a > 0 and b > 0 and c > 0.
-- All comparisons involve T_INT locals; TryCompileNativeBoolExpr must handle
-- the left-associative tree and emit a direct C &&-chain.
function test(a, b, c)
    local x = a + 0   -- x is T_INT in int specialization
    local y = b + 0   -- y is T_INT in int specialization
    local z = c + 0   -- z is T_INT in int specialization
    local r = 0
    if x > 0 and y > 0 and z > 0 then
        r = 1
    end
    return r
end
