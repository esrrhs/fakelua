function test(a, b)
    local result = 0
    local map = { a, b, a, b, a }
    for k, v in pairs(map) do
        for kk, vv in ipairs(map) do
            result = result + k + v + kk + vv
        end
    end
    return result
end
