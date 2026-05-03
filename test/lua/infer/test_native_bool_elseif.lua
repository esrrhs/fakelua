-- Native bool expression in elseif: sign function using local typed variable.
-- Both the if and elseif conditions compare a T_INT local variable against T_INT
-- literals, so TryCompileNativeBoolExpr must emit direct C comparisons for both.
function test(n)
    local x = n + 0   -- x is T_INT in int specialization
    local r = 0
    if x < 0 then
        r = -1
    elseif x == 0 then
        r = 0
    else
        r = 1
    end
    return r
end
