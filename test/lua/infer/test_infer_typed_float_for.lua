-- T_FLOAT for-loop fast path: all bounds are float literals.
-- for i = 1.0, 3.0: sum = 1.0+2.0+3.0 = 6.0
function test()
    local sum = 0.0
    for i = 1.0, 3.0 do
        sum = sum + i
    end
    return sum
end
