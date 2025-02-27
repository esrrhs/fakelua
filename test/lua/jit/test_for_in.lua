function test(a, b)
    local result = 0
    local map = { a, b, a, b, a }
    for k, v in pairs(map) do
        result = result + k + v
    end
    return result
end
