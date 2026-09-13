-- T_FLOAT for-loop with explicit float step.
-- for i = 0.0, 1.0, 0.5: sum = 0.0+0.5+1.0 = 1.5
function test()
    local sum = 0.0
    for i = 0.0, 1.0, 0.5 do
        sum = sum + i
    end
    return sum
end
