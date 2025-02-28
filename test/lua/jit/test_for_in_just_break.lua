function test(a, b)
    local result = 0
    local map = { a, a, a, a, a }
    for k, v in pairs(map) do
        result = v
        break
    end
    return result
end
