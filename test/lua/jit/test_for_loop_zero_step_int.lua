-- Typed-int for-loop with step=0 must throw "'for' step is zero"
function test()
    local sum = 0
    for i = 1, 10, 0 do
        sum = sum + i
    end
    return sum
end
