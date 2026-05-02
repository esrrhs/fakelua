-- Native bool expression with float operand: x is T_FLOAT (after x + 0.0)
-- and the comparison x >= 0.0 must produce a direct C comparison (not IsTrue).
function test(n)
    local x = n + 0.0   -- x is T_FLOAT
    local r = 0
    if x >= 0.0 then
        r = 1
    end
    return r
end
