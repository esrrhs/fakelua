function test(x)
    -- No step specified; exercises the no-step branch in for-loop compilation
    local sum = 0
    for i = 1, x do
        sum = sum + i
    end
    return sum
end
