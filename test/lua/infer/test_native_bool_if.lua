-- Native bool expression in if: both operands are T_INT (local variables)
-- so TryCompileNativeBoolExpr must emit a direct C comparison
-- instead of IsTrue + OpXxx.
function test(n)
    local x = n + 0   -- x is T_INT in int specialization
    local r = 0
    if x > 5 then
        r = 1
    end
    return r
end
