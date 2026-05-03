-- Native bool expression in while: the loop condition x > 0 has T_INT operands
-- so TryCompileNativeBoolExpr must emit while ((x) > (0)) directly.
function test(n)
    local x = n
    local s = 0
    while x > 0 do
        s = s + x
        x = x - 1
    end
    return s
end
