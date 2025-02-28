function test(a, b)
    local result = 0
    local map = { a, a }
    for k, v in pairs(map) do
        return v
    end
    return result
end
