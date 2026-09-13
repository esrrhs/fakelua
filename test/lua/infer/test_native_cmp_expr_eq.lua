-- Native comparison == in expression context: when both operands are
-- numeric, CompileBinop generates SET_BOOL with native C == operator.
-- helper(n) uses n*2 so n is a math param (T_INT specialization).
-- test(5) returns helper(5)==10 -> true -> 1.
-- test(3) returns helper(3)==10 -> false -> 0.
function helper(n)
    return n * 2
end

function test(n)
    local r = helper(n)
    if r == 10 then
        return 1
    end
    return 0
end
