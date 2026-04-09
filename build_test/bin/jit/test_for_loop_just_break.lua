function test(a, b)
    local result = a
    for i = a, b do
        result = b
        break
    end
    return result
end
