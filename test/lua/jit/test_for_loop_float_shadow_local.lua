-- Shadow the loop variable with a local inside the typed-float for-loop body.
-- i=1.0: inner_i=1.5; i=2.0: inner_i=2.5; i=3.0: inner_i=3.5  => sum=7.5
function test()
    local sum = 0.0
    for i = 1.0, 3.0, 1.0 do
        local i = i + 0.5
        sum = sum + i
    end
    return sum
end
