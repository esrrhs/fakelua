-- CollectReassignedVars must traverse elseif blocks.
-- n is a math param (used in n + 1); the function body contains an elseif.
function test(n)
    local r = n + 1
    if n > 10 then
        r = r + 1
    elseif n > 5 then
        r = r + 2
    end
    return r
end
