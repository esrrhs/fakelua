-- Float for-loop: begin=1.0, end=2.0, step=0.5 => sum = 1.0+1.5+2.0 = 4.5
function test()
    local sum = 0.0
    for i = 1.0, 2.0, 0.5 do
        sum = sum + i
    end
    return sum
end
