-- Typed-float for-loop with step=0.0 must throw "'for' step is zero"
function test()
    local sum = 0.0
    for i = 1.0, 10.0, 0.0 do
        sum = sum + i
    end
    return sum
end
