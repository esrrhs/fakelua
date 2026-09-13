-- T_FLOAT for-loop with mixed int/float bounds (begin=int, end=float).
-- for i = 1, 5.0: sum = 1.0+2.0+3.0+4.0+5.0 = 15.0
function test()
    local sum = 0.0
    for i = 1, 5.0 do
        sum = sum + i
    end
    return sum
end
