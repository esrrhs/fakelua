-- Native bool AND expression in if: a == 1 and b > 0.
-- Both sub-expressions compare T_INT locals, so TryCompileNativeBoolExpr must
-- emit a direct C &&-expression without IsTrue.
function test(a, b)
    local x = a + 0   -- x is T_INT in int specialization
    local y = b + 0   -- y is T_INT in int specialization
    local r = 0
    if x == 1 and y > 0 then
        r = 1
    end
    return r
end
