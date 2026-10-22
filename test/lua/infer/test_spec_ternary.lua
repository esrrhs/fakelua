-- Test numeric ternary operator fast path optimization
-- (n > 0) and 1 or 2 should propagate numeric type
-- and generate an if/else so only the selected arm runs
function test(n)
    local x = (n > 0) and 1 or 2
    return x
end
