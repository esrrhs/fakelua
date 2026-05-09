-- Equality comparison should also participate in math-param specialization
-- detection when both operands become numeric under specialization.
function test(n, m)
    if n == m then
        return n + 1
    end
    return n - 1
end
