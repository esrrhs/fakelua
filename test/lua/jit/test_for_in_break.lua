function test(a, b)
    local result = 0
    local map = { a, a, a, a, a }
    for k, v in pairs(map) do
        for kk, vv in ipairs(map) do
            result = result + v + vv
            if result > b then
                break
            end
        end
    end
    return result
end
