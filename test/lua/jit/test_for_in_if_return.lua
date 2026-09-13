function test(a, b)
    local result = 0
    local map = { a, a, a, a, a }
    for k, v in pairs(map) do
        if v > b then
            result = b
        else
            return a
        end
    end
    return result
end
