local function helper(x)
    local dummy = x + 1
    local res = 0
    -- Double-parentheses around a comparison
    if ((x > 0)) then
        res = res + 1
    end
    -- Logical NOT of a parenthesized comparison
    if not (x > 0) then
        res = res + 10
    end
    return res
end

function test(x)
    local temp = x + 0
    return helper(x)
end
