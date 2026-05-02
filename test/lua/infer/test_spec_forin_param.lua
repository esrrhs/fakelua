-- CollectReassignedVars must traverse ForIn blocks.
-- n is a math param (used in n + 0); the function body contains a for-in loop.
function test(n)
    local sum = n + 0
    for k, v in pairs({1, 2, 3}) do
        sum = sum + v
    end
    return sum
end
