function test(a, b)
    local result = 0
    for i = a, b, 2 do
        for j = 0, i do
            result = result + j
        end
    end
    return result
end
