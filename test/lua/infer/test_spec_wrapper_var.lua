-- A function that wraps a variable and uses it in arithmetic.
-- The wrapper variable should trace back to the math param.
function test(n)
    local x = n
    return x * x
end
