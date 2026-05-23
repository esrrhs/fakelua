-- ForIn with only 1 loop variable (key only, no value variable).
-- Exercises c_gen.cpp CompileStmtForIn lines 2138-2142 (dummy val path).
-- Keys are 1, 2, 3 => result = 1+2+3 = 6.
function test(a, b)
    local result = 0
    local map = {a, b, a}
    for k in pairs(map) do
        result = result + k
    end
    return result
end
