function test()
    local tbl = {a = 10, b = 20, c = 30}
    local getters = {}
    local idx = 1
    for k, v in pairs(tbl) do
        getters[idx] = function()
            return v
        end
        idx = idx + 1
    end
    local sum = 0
    for i = 1, #getters do
        sum = sum + getters[i]()
    end
    return sum
end
