-- Shadow the loop variable with a local inside the typed-int for-loop body.
-- Lua semantics: local i creates a new variable in the inner scope;
-- the outer i (loop counter) is unchanged.
-- i=1: inner_i=2; i=2: inner_i=4; i=3: inner_i=6  => sum=12
function test(n)
    local sum = 0
    for i = 1, n do
        local i = i * 2
        sum = sum + i
    end
    return sum
end
