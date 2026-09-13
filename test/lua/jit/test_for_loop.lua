function test(a, b)
    local result = 0
    for i = a, b, 2 do
        result = result + i
    end
    return result
end
