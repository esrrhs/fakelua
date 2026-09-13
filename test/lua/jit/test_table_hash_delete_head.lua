function test()
    local t = {
        [1] = 10,
        [17] = 170,
        [2] = 20,
        [3] = 30,
        [4] = 40,
        [5] = 50,
        [6] = 60,
        [7] = 70,
        [8] = 80,
        [9] = 90
    }
    t[1] = nil
    if t[1] == nil then
        return t[17]
    end
    return -1
end
