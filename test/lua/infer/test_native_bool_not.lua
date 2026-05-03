-- Native bool NOT expression in if: not (x > 5).
-- The inner comparison is T_INT, so TryCompileNativeBoolExpr must emit
-- !((x) > (5)) without IsTrue.
function test(n)
    local x = n + 0   -- x is T_INT in int specialization
    local r = 0
    if not (x > 5) then
        r = 1
    end
    return r
end
