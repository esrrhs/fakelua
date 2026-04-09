function test(a, b)
    local result = a
    while a < b do
        result = b
        break
    end
    return result
end
