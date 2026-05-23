-- Native bool OR expression in if: x < 0 or x > 10.
-- Both sub-expressions compare T_INT locals, so TryCompileNativeBoolExpr must
-- emit a direct C ||-expression without IsTrue.
function test(n)
    local x = n + 0   -- x is T_INT in int specialization
    local r = 0
    if x < 0 or x > 10 then
        r = 1
    end
    return r
end
